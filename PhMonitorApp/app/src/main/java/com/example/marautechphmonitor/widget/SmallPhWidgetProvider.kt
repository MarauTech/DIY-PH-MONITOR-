package com.example.marautechphmonitor.widget

import android.appwidget.AppWidgetManager
import android.appwidget.AppWidgetProvider
import android.content.Context
import android.content.Intent
import com.example.marautechphmonitor.data.local.DataStoreManager
import com.example.marautechphmonitor.data.local.dataStore
import com.example.marautechphmonitor.model.AlarmState
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

class SmallPhWidgetProvider : AppWidgetProvider() {

    override fun onUpdate(context: Context, appWidgetManager: AppWidgetManager, appWidgetIds: IntArray) {
        super.onUpdate(context, appWidgetManager, appWidgetIds)
        refreshWidget(context)
    }

    override fun onReceive(context: Context, intent: Intent) {
        super.onReceive(context, intent)
        if (intent.action == "com.example.marautechphmonitor.ACTION_REFRESH_WIDGET" ||
            intent.action == AppWidgetManager.ACTION_APPWIDGET_UPDATE) {
            refreshWidget(context)
        }
    }

    private fun refreshWidget(context: Context) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                val prefs = context.dataStore.data.first()
                val ph = prefs[DataStoreManager.CACHED_PH_KEY] ?: 7.0f
                val temp = prefs[DataStoreManager.CACHED_TEMP_KEY]
                val voltage = prefs[DataStoreManager.CACHED_VOLTAGE_KEY]
                val alarmCode = prefs[DataStoreManager.CACHED_ALARM_STATE_KEY] ?: 0
                val deviceName = prefs[DataStoreManager.CACHED_DEVICE_NAME_KEY] ?: "MarauTech pH"
                val timestamp = prefs[DataStoreManager.CACHED_UPDATE_TIMESTAMP_KEY] ?: System.currentTimeMillis()

                WidgetUpdateHelper.updateAllWidgets(
                    context = context,
                    ph = ph,
                    temp = temp,
                    voltage = voltage,
                    alarmState = AlarmState.fromCode(alarmCode),
                    deviceName = deviceName,
                    lastUpdatedTime = timestamp
                )
            } catch (e: Exception) {
                // Ignore widget background errors
            }
        }
    }
}
