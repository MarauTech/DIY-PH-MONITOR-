package com.example.marautechphmonitor.ui.viewmodel

import android.app.Application
import android.util.Log
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.example.marautechphmonitor.data.local.DataStoreManager
import com.example.marautechphmonitor.data.remote.*
import com.example.marautechphmonitor.data.repository.AppRepository
import com.example.marautechphmonitor.model.AlarmState
import com.example.marautechphmonitor.notification.NotificationHelper
import com.example.marautechphmonitor.widget.WidgetUpdateHelper
import com.example.marautechphmonitor.worker.WorkManagerScheduler
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class MainViewModel(application: Application) : AndroidViewModel(application) {

    private val dataStoreManager = DataStoreManager(application)
    val repository = AppRepository(dataStoreManager)

    // Connection & Device State
    private val _isConnected = MutableStateFlow(false)
    val isConnected: StateFlow<Boolean> = _isConnected.asStateFlow()

    private val _isConnecting = MutableStateFlow(false)
    val isConnecting: StateFlow<Boolean> = _isConnecting.asStateFlow()

    private val _connectionError = MutableStateFlow<String?>(null)
    val connectionError: StateFlow<String?> = _connectionError.asStateFlow()

    private val _status = MutableStateFlow<StatusResponse?>(null)
    val status: StateFlow<StatusResponse?> = _status.asStateFlow()

    private val _alarmState = MutableStateFlow(AlarmState.UNKNOWN)
    val alarmState: StateFlow<AlarmState> = _alarmState.asStateFlow()

    private val _lastUpdatedTime = MutableStateFlow<String>("--:--:--")
    val lastUpdatedTime: StateFlow<String> = _lastUpdatedTime.asStateFlow()

    // Config State
    private val _config = MutableStateFlow<ConfigResponse?>(null)
    val config: StateFlow<ConfigResponse?> = _config.asStateFlow()

    private val _isConfigLoading = MutableStateFlow(false)
    val isConfigLoading: StateFlow<Boolean> = _isConfigLoading.asStateFlow()

    private val _configMessage = MutableStateFlow<Pair<Boolean, String>?>(null) // isSuccess to Message
    val configMessage: StateFlow<Pair<Boolean, String>?> = _configMessage.asStateFlow()

    // History State
    private val _history = MutableStateFlow<List<HistoryPoint>>(emptyList())
    val history: StateFlow<List<HistoryPoint>> = _history.asStateFlow()

    private val _isHistoryLoading = MutableStateFlow(false)
    val isHistoryLoading: StateFlow<Boolean> = _isHistoryLoading.asStateFlow()

    private val _selectedHistoryRange = MutableStateFlow("24h") // "1h", "6h", "12h", "24h", "all"
    val selectedHistoryRange: StateFlow<String> = _selectedHistoryRange.asStateFlow()

    private val _selectedHistoryMetric = MutableStateFlow("pH") // "pH", "Temp", "Volt"
    val selectedHistoryMetric: StateFlow<String> = _selectedHistoryMetric.asStateFlow()

    // Calibration State
    private val _calibrationStatus = MutableStateFlow<CalibrationStatusResponse?>(null)
    val calibrationStatus: StateFlow<CalibrationStatusResponse?> = _calibrationStatus.asStateFlow()

    private val _isCalibrating = MutableStateFlow(false)
    val isCalibrating: StateFlow<Boolean> = _isCalibrating.asStateFlow()

    private val _calibrationMessage = MutableStateFlow<String?>(null)
    val calibrationMessage: StateFlow<String?> = _calibrationMessage.asStateFlow()

    private val _calibrationError = MutableStateFlow<String?>(null)
    val calibrationError: StateFlow<String?> = _calibrationError.asStateFlow()

    // Settings (Notification)
    val notificationsEnabled = dataStoreManager.notificationsEnabledFlow
    val notifyLow = dataStoreManager.notifyLowFlow
    val notifyHigh = dataStoreManager.notifyHighFlow
    val notifyRecovery = dataStoreManager.notifyRecoveryFlow
    val checkInterval = dataStoreManager.checkIntervalFlow

    private var pollingJob: Job? = null
    private var calibPollJob: Job? = null

    init {
        viewModelScope.launch {
            val savedIp = dataStoreManager.deviceIpFlow.first()
            if (!savedIp.isNullOrBlank()) {
                connect(savedIp, dataStoreManager.devicePortFlow.first())
            }
        }
    }

    fun connect(ip: String, port: Int = 80, user: String? = null, pass: String? = null) {
        viewModelScope.launch {
            _isConnecting.value = true
            _connectionError.value = null
            try {
                // Save connection
                dataStoreManager.saveDeviceConnection(ip, port)
                if (user != null && pass != null) {
                    dataStoreManager.saveCredentials(user, pass)
                }

                val res = repository.testConnection(ip, port, user, pass)
                _status.value = res
                _alarmState.value = AlarmState.fromCode(res.alarmState, true)
                _isConnected.value = true
                _lastUpdatedTime.value = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())

                // Update widgets immediately
                WidgetUpdateHelper.updateAllWidgets(
                    context = getApplication(),
                    ph = res.ph,
                    temp = if (res.tempConnected) res.temperature else null,
                    voltage = res.voltage,
                    alarmState = _alarmState.value,
                    deviceName = res.deviceName,
                    ipAddress = ip,
                    uptimeSeconds = res.uptime
                )

                startPolling()
            } catch (e: Exception) {
                _isConnected.value = false
                _alarmState.value = AlarmState.OFFLINE
                _connectionError.value = when {
                    e.message?.contains("401") == true -> "Błąd autoryzacji: Nieprawidłowy login lub hasło administratora"
                    e.message?.contains("Failed to connect") == true || e.message?.contains("timeout") == true -> "Nie można połączyć się z monitorem pH pod $ip"
                    else -> "Błąd połączenia: ${e.localizedMessage ?: "Nieznany błąd"}"
                }
            } finally {
                _isConnecting.value = false
            }
        }
    }

    fun disconnect() {
        stopPolling()
        _isConnected.value = false
        _status.value = null
        _alarmState.value = AlarmState.UNKNOWN
    }

    fun startPolling() {
        if (pollingJob?.isActive == true) return
        pollingJob = viewModelScope.launch {
            while (isActive) {
                try {
                    val res = repository.getStatus()
                    _status.value = res
                    _alarmState.value = AlarmState.fromCode(res.alarmState, true)
                    _isConnected.value = true
                    _connectionError.value = null
                    _lastUpdatedTime.value = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())

                    val previousStateCode = dataStoreManager.lastAlarmStateFlow.first()
                    if (res.alarmState != previousStateCode) {
                        val notifEnabled = dataStoreManager.notificationsEnabledFlow.first()
                        val notifyLow = dataStoreManager.notifyLowFlow.first()
                        val notifyHigh = dataStoreManager.notifyHighFlow.first()
                        val notifyRecovery = dataStoreManager.notifyRecoveryFlow.first()

                        if (notifEnabled) {
                            when (res.alarmState) {
                                1 -> if (notifyLow) {
                                    NotificationHelper.showAlarmNotification(
                                        context = getApplication(),
                                        title = "ALARM: Zbyt niskie pH!",
                                        message = String.format(Locale.US, "Wykryto zbyt niskie pH: %.2f (Norma przekroczona)", res.ph),
                                        alarmState = _alarmState.value
                                    )
                                }
                                2 -> if (notifyHigh) {
                                    NotificationHelper.showAlarmNotification(
                                        context = getApplication(),
                                        title = "ALARM: Zbyt wysokie pH!",
                                        message = String.format(Locale.US, "Wykryto zbyt wysokie pH: %.2f (Norma przekroczona)", res.ph),
                                        alarmState = _alarmState.value
                                    )
                                }
                                0 -> if (notifyRecovery && (previousStateCode == 1 || previousStateCode == 2)) {
                                    NotificationHelper.showRecoveryNotification(
                                        context = getApplication(),
                                        title = "pH powróciło do normy",
                                        message = String.format(Locale.US, "Wartość pH powróciła do normy: %.2f", res.ph)
                                    )
                                }
                            }
                        }
                        dataStoreManager.saveLastAlarmState(res.alarmState)
                    }

                    WidgetUpdateHelper.updateAllWidgets(
                        context = getApplication(),
                        ph = res.ph,
                        temp = if (res.tempConnected) res.temperature else null,
                        voltage = res.voltage,
                        alarmState = _alarmState.value,
                        deviceName = res.deviceName,
                        ipAddress = dataStoreManager.deviceIpFlow.first(),
                        uptimeSeconds = res.uptime
                    )
                } catch (e: Exception) {
                    _isConnected.value = false
                    _alarmState.value = AlarmState.OFFLINE
                }
                delay(2000)
            }
        }
    }

    fun testLocalNotification() {
        NotificationHelper.showAlarmNotification(
            context = getApplication(),
            title = "TEST: Alarm pH Monitor",
            message = "To jest testowe powiadomienie z aplikacji MarauTech pH Monitor. System powiadomień działa poprawnie!",
            alarmState = AlarmState.NORMAL
        )
    }

    fun stopPolling() {
        pollingJob?.cancel()
        pollingJob = null
    }

    fun refreshStatus() {
        viewModelScope.launch {
            try {
                val res = repository.getStatus()
                _status.value = res
                _alarmState.value = AlarmState.fromCode(res.alarmState, true)
                _isConnected.value = true
                _lastUpdatedTime.value = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())
            } catch (e: Exception) {
                _isConnected.value = false
                _alarmState.value = AlarmState.OFFLINE
            }
        }
    }

    // Config Methods
    fun loadConfig() {
        viewModelScope.launch {
            _isConfigLoading.value = true
            _configMessage.value = null
            try {
                _config.value = repository.getConfig()
            } catch (e: Exception) {
                _configMessage.value = Pair(false, "Błąd pobierania konfiguracji: ${e.message}")
            } finally {
                _isConfigLoading.value = false
            }
        }
    }

    fun saveConfig(req: ConfigRequest) {
        viewModelScope.launch {
            _isConfigLoading.value = true
            _configMessage.value = null
            try {
                val res = repository.saveConfig(req)
                if (res.isSuccessful) {
                    _configMessage.value = Pair(true, "Konfiguracja zapisana pomyślnie!")
                    loadConfig()
                } else {
                    _configMessage.value = Pair(false, "Błąd zapisu (HTTP ${res.code()})")
                }
            } catch (e: Exception) {
                _configMessage.value = Pair(false, "Błąd połączenia: ${e.message}")
            } finally {
                _isConfigLoading.value = false
            }
        }
    }

    // History Methods
    fun loadHistory() {
        viewModelScope.launch {
            _isHistoryLoading.value = true
            try {
                _history.value = repository.getHistory()
            } catch (e: Exception) {
                Log.e("MainViewModel", "Failed to load history", e)
            } finally {
                _isHistoryLoading.value = false
            }
        }
    }

    fun setHistoryRange(range: String) {
        _selectedHistoryRange.value = range
    }

    fun setHistoryMetric(metric: String) {
        _selectedHistoryMetric.value = metric
    }

    // Calibration Methods
    fun startCalibration(type: String, customPH: Float? = null) {
        viewModelScope.launch {
            _isCalibrating.value = true
            _calibrationError.value = null
            _calibrationMessage.value = "Inicjalizacja kalibracji..."
            try {
                val res = repository.calibrate(CalibrationRequest(type = type, customPH = customPH))
                if (res.isSuccessful) {
                    if (type == "reset") {
                        _calibrationMessage.value = "Kalibracja zresetowana do wartości fabrycznych!"
                        _isCalibrating.value = false
                        refreshStatus()
                    } else {
                        _calibrationMessage.value = "Trwa badanie stabilności bufora..."
                        startCalibrationPolling()
                    }
                } else {
                    _calibrationError.value = "Błąd rozpoczęcia kalibracji (HTTP ${res.code()})"
                    _isCalibrating.value = false
                }
            } catch (e: Exception) {
                _calibrationError.value = "Błąd: ${e.message}"
                _isCalibrating.value = false
            }
        }
    }

    private fun startCalibrationPolling() {
        calibPollJob?.cancel()
        calibPollJob = viewModelScope.launch {
            while (isActive) {
                try {
                    val status = repository.getCalibrationStatus()
                    _calibrationStatus.value = status
                    when (status.status) {
                        "collecting" -> {
                            _calibrationMessage.value = "Trwa zbieranie i badanie stabilności próbek (${status.progress}%)"
                        }
                        "done" -> {
                            val volt = status.voltage?.let { String.format(Locale.US, " (%.3f V)", it / 1000f) } ?: ""
                            _calibrationMessage.value = "Kalibracja zakończona pomyślnie!$volt"
                            _isCalibrating.value = false
                            refreshStatus()
                            break
                        }
                        "failed" -> {
                            val msg = when (status.error) {
                                "invalid_voltage" -> "Napięcie poza zakresem fizycznym (0.5V - 4.5V)"
                                "points_too_close" -> "Zbyt mała różnica napięć między punktami (<100mV)"
                                "invalid_slope" -> "Nieprawidłowy kierunek nachylenia (wymagane vPH4 > vPH7 > vPH9)"
                                else -> status.message ?: "Pomiar niestabilny. Sonda musi ustabilizować się w buforze."
                            }
                            _calibrationError.value = "Błąd kalibracji: $msg"
                            _isCalibrating.value = false
                            break
                        }
                    }
                } catch (e: Exception) {
                    Log.e("MainViewModel", "Calib polling error", e)
                }
                delay(500)
            }
        }
    }

    // Settings
    fun saveNotificationPreferences(
        enabled: Boolean,
        notifyLow: Boolean,
        notifyHigh: Boolean,
        notifyRecovery: Boolean,
        intervalMinutes: Int
    ) {
        viewModelScope.launch {
            dataStoreManager.saveNotificationSettings(enabled, notifyLow, notifyHigh, notifyRecovery, intervalMinutes)
            if (enabled) {
                WorkManagerScheduler.scheduleMonitoring(getApplication(), intervalMinutes)
            } else {
                WorkManagerScheduler.cancelMonitoring(getApplication())
            }
        }
    }

    fun testPushover(onResult: (Boolean, String) -> Unit) {
        viewModelScope.launch {
            try {
                val res = repository.testPushover()
                if (res.isSuccessful) {
                    onResult(true, "Wysłano testowe powiadomienie!")
                } else {
                    onResult(false, "Błąd wysyłania (HTTP ${res.code()})")
                }
            } catch (e: Exception) {
                onResult(false, "Błąd: ${e.message}")
            }
        }
    }

    fun resetStats(onResult: (Boolean) -> Unit) {
        viewModelScope.launch {
            try {
                val res = repository.resetStats()
                if (res.isSuccessful) {
                    refreshStatus()
                    onResult(true)
                } else {
                    onResult(false)
                }
            } catch (e: Exception) {
                onResult(false)
            }
        }
    }
}
