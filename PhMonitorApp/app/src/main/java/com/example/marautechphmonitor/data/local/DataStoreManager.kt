package com.example.marautechphmonitor.data.local

import android.content.Context
import android.content.SharedPreferences
import android.util.Log
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.floatPreferencesKey
import androidx.datastore.preferences.core.intPreferencesKey
import androidx.datastore.preferences.core.longPreferencesKey
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

val Context.dataStore by preferencesDataStore(name = "ph_monitor_settings")

class DataStoreManager(private val context: Context) {
    companion object {
        private const val TAG = "DataStoreManager"
        val DEVICE_IP_KEY = stringPreferencesKey("device_ip")
        val DEVICE_PORT_KEY = intPreferencesKey("device_port")
        val DEVICE_NAME_KEY = stringPreferencesKey("device_name")
        
        // Notifications
        val NOTIFICATIONS_ENABLED_KEY = booleanPreferencesKey("notifications_enabled")
        val NOTIFY_LOW_KEY = booleanPreferencesKey("notify_low")
        val NOTIFY_HIGH_KEY = booleanPreferencesKey("notify_high")
        val NOTIFY_RECOVERY_KEY = booleanPreferencesKey("notify_recovery")
        val CHECK_INTERVAL_MINUTES_KEY = intPreferencesKey("check_interval_minutes")
        val LAST_ALARM_STATE_KEY = intPreferencesKey("last_alarm_state")

        // Cached status for offline & widget
        val CACHED_PH_KEY = floatPreferencesKey("cached_ph")
        val CACHED_TEMP_KEY = floatPreferencesKey("cached_temp")
        val CACHED_VOLTAGE_KEY = floatPreferencesKey("cached_voltage")
        val CACHED_ALARM_STATE_KEY = intPreferencesKey("cached_alarm_state")
        val CACHED_DEVICE_NAME_KEY = stringPreferencesKey("cached_device_name")
        val CACHED_UPDATE_TIMESTAMP_KEY = longPreferencesKey("cached_update_timestamp")

        private const val SECURE_PREFS_NAME = "secure_ph_prefs"
        private const val ADMIN_USER_KEY = "admin_user"
        private const val ADMIN_PASS_KEY = "admin_pass"
    }

    val deviceIpFlow: Flow<String?> = context.dataStore.data.map { it[DEVICE_IP_KEY] }
    val devicePortFlow: Flow<Int> = context.dataStore.data.map { it[DEVICE_PORT_KEY] ?: 80 }
    val deviceNameFlow: Flow<String> = context.dataStore.data.map { it[DEVICE_NAME_KEY] ?: "MarauTech pH" }

    val notificationsEnabledFlow: Flow<Boolean> = context.dataStore.data.map { it[NOTIFICATIONS_ENABLED_KEY] ?: true }
    val notifyLowFlow: Flow<Boolean> = context.dataStore.data.map { it[NOTIFY_LOW_KEY] ?: true }
    val notifyHighFlow: Flow<Boolean> = context.dataStore.data.map { it[NOTIFY_HIGH_KEY] ?: true }
    val notifyRecoveryFlow: Flow<Boolean> = context.dataStore.data.map { it[NOTIFY_RECOVERY_KEY] ?: true }
    val checkIntervalFlow: Flow<Int> = context.dataStore.data.map { it[CHECK_INTERVAL_MINUTES_KEY] ?: 15 }
    val lastAlarmStateFlow: Flow<Int> = context.dataStore.data.map { it[LAST_ALARM_STATE_KEY] ?: 0 }

    suspend fun saveDeviceConnection(ip: String, port: Int = 80, name: String = "MarauTech pH") {
        context.dataStore.edit { prefs ->
            prefs[DEVICE_IP_KEY] = ip.trim()
            prefs[DEVICE_PORT_KEY] = port
            prefs[DEVICE_NAME_KEY] = name.trim()
        }
    }

    suspend fun saveNotificationSettings(
        enabled: Boolean,
        notifyLow: Boolean,
        notifyHigh: Boolean,
        notifyRecovery: Boolean,
        intervalMinutes: Int
    ) {
        context.dataStore.edit { prefs ->
            prefs[NOTIFICATIONS_ENABLED_KEY] = enabled
            prefs[NOTIFY_LOW_KEY] = notifyLow
            prefs[NOTIFY_HIGH_KEY] = notifyHigh
            prefs[NOTIFY_RECOVERY_KEY] = notifyRecovery
            prefs[CHECK_INTERVAL_MINUTES_KEY] = intervalMinutes
        }
    }

    suspend fun saveLastAlarmState(state: Int) {
        context.dataStore.edit { prefs ->
            prefs[LAST_ALARM_STATE_KEY] = state
        }
    }

    suspend fun cacheStatus(
        ph: Float,
        temp: Float?,
        voltage: Float,
        alarmState: Int,
        deviceName: String,
        timestamp: Long = System.currentTimeMillis()
    ) {
        context.dataStore.edit { prefs ->
            prefs[CACHED_PH_KEY] = ph
            if (temp != null) prefs[CACHED_TEMP_KEY] = temp
            prefs[CACHED_VOLTAGE_KEY] = voltage
            prefs[CACHED_ALARM_STATE_KEY] = alarmState
            prefs[CACHED_DEVICE_NAME_KEY] = deviceName
            prefs[CACHED_UPDATE_TIMESTAMP_KEY] = timestamp
        }
    }

    // Secure credentials using EncryptedSharedPreferences with fallback
    private val securePrefs: SharedPreferences by lazy {
        try {
            val masterKey = MasterKey.Builder(context)
                .setKeyScheme(MasterKey.KeyScheme.AES256_GCM)
                .build()
            EncryptedSharedPreferences.create(
                context,
                SECURE_PREFS_NAME,
                masterKey,
                EncryptedSharedPreferences.PrefKeyEncryptionScheme.AES256_SIV,
                EncryptedSharedPreferences.PrefValueEncryptionScheme.AES256_GCM
            )
        } catch (e: Exception) {
            Log.e(TAG, "Failed to initialize EncryptedSharedPreferences, falling back to private prefs", e)
            context.getSharedPreferences("fallback_secure_prefs", Context.MODE_PRIVATE)
        }
    }

    fun saveCredentials(user: String, pass: String) {
        securePrefs.edit().apply {
            putString(ADMIN_USER_KEY, user)
            putString(ADMIN_PASS_KEY, pass)
            apply()
        }
    }

    fun getAdminUser(): String? = securePrefs.getString(ADMIN_USER_KEY, null)
    fun getAdminPass(): String? = securePrefs.getString(ADMIN_PASS_KEY, null)

    fun clearCredentials() {
        securePrefs.edit().remove(ADMIN_USER_KEY).remove(ADMIN_PASS_KEY).apply()
    }
}
