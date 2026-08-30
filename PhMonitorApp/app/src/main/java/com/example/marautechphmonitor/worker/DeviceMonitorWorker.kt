package com.example.marautechphmonitor.worker

import android.content.Context
import android.util.Log
import androidx.work.CoroutineWorker
import androidx.work.WorkerParameters
import com.example.marautechphmonitor.data.local.DataStoreManager
import com.example.marautechphmonitor.data.remote.RetrofitHelper
import com.example.marautechphmonitor.model.AlarmState
import com.example.marautechphmonitor.notification.NotificationHelper
import com.example.marautechphmonitor.widget.WidgetUpdateHelper
import kotlinx.coroutines.flow.first
import java.util.Locale

class DeviceMonitorWorker(
    private val context: Context,
    workerParams: WorkerParameters
) : CoroutineWorker(context, workerParams) {

    companion object {
        private const val TAG = "DeviceMonitorWorker"
        const val WORK_NAME = "ph_device_monitor_work"
    }

    override suspend fun doWork(): Result {
        val dataStoreManager = DataStoreManager(context)

        val notificationsEnabled = dataStoreManager.notificationsEnabledFlow.first()
        val notifyLow = dataStoreManager.notifyLowFlow.first()
        val notifyHigh = dataStoreManager.notifyHighFlow.first()
        val notifyRecovery = dataStoreManager.notifyRecoveryFlow.first()
        val notifyOffline = dataStoreManager.notifyOfflineFlow.first()
        val lastStateCode = dataStoreManager.lastAlarmStateFlow.first()

        val ip = dataStoreManager.deviceIpFlow.first()
        val port = dataStoreManager.devicePortFlow.first()
        val adminUser = dataStoreManager.getAdminUser()
        val adminPass = dataStoreManager.getAdminPass()

        if (ip.isNullOrBlank()) {
            Log.d(TAG, "No device IP configured, skipping work.")
            return Result.success()
        }

        val target = if (port == 80 || port <= 0) ip else "$ip:$port"

        return try {
            val api = RetrofitHelper.getApi(target, adminUser, adminPass)
            val status = api.getStatus()

            val currentStateCode = status.alarmState
            val currentAlarmState = AlarmState.fromCode(currentStateCode)

            // Cache latest status
            dataStoreManager.cacheStatus(
                ph = status.ph,
                temp = if (status.tempConnected) status.temperature else null,
                voltage = status.voltage,
                alarmState = status.alarmState,
                deviceName = status.deviceName
            )

            // Update all widgets
            WidgetUpdateHelper.updateAllWidgets(
                context = context,
                ph = status.ph,
                temp = if (status.tempConnected) status.temperature else null,
                voltage = status.voltage,
                alarmState = currentAlarmState,
                deviceName = status.deviceName,
                ipAddress = ip,
                uptimeSeconds = status.uptime
            )

            // Check if state changed for notifications
            if (notificationsEnabled) {
                if (lastStateCode == -1 && notifyRecovery) {
                    NotificationHelper.showRecoveryNotification(
                        context = context,
                        title = "Połączenie przywrócone",
                        message = "Monitor pH (${status.deviceName}) jest ponownie online!"
                    )
                } else if (currentStateCode != lastStateCode) {
                    when (currentStateCode) {
                        1 -> { // LOW PH
                            if (notifyLow) {
                                val msg = String.format(Locale.US, "Wykryto zbyt niskie pH: %.2f (Norma przekroczona)", status.ph)
                                NotificationHelper.showAlarmNotification(
                                    context = context,
                                    title = "ALARM: Zbyt niskie pH!",
                                    message = msg,
                                    alarmState = currentAlarmState
                                )
                            }
                        }
                        2 -> { // HIGH PH
                            if (notifyHigh) {
                                val msg = String.format(Locale.US, "Wykryto zbyt wysokie pH: %.2f (Norma przekroczona)", status.ph)
                                NotificationHelper.showAlarmNotification(
                                    context = context,
                                    title = "ALARM: Zbyt wysokie pH!",
                                    message = msg,
                                    alarmState = currentAlarmState
                                )
                            }
                        }
                        0 -> { // NORMAL (Recovery)
                            if (notifyRecovery && (lastStateCode == 1 || lastStateCode == 2)) {
                                val msg = String.format(Locale.US, "Wartość pH powróciła do normy: %.2f", status.ph)
                                NotificationHelper.showRecoveryNotification(
                                    context = context,
                                    title = "pH powróciło do normy",
                                    message = msg
                                )
                            }
                        }
                    }
                }
                dataStoreManager.saveLastAlarmState(currentStateCode)
            }

            Result.success()
        } catch (e: Exception) {
            Log.w(TAG, "Failed to monitor device in background: ${e.message}")
            if (notificationsEnabled && notifyOffline && lastStateCode != -1) {
                NotificationHelper.showOfflineNotification(
                    context = context,
                    title = "Brak połączenia z pH Monitorem!",
                    message = "Urządzenie nie odpowiada pod adresem $target. Sprawdź zasilanie i sieć Wi-Fi."
                )
                dataStoreManager.saveLastAlarmState(-1)
            }
            Result.retry()
        }
    }
}
