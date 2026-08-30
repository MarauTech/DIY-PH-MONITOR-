package com.example.marautechphmonitor.widget

import android.app.PendingIntent
import android.appwidget.AppWidgetManager
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.view.View
import android.widget.RemoteViews
import com.example.marautechphmonitor.MainActivity
import com.example.marautechphmonitor.R
import com.example.marautechphmonitor.model.AlarmState
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

object WidgetUpdateHelper {

    fun updateAllWidgets(
        context: Context,
        ph: Float,
        temp: Float?,
        voltage: Float?,
        alarmState: AlarmState,
        deviceName: String,
        ipAddress: String? = null,
        uptimeSeconds: Long = 0L,
        lastUpdatedTime: Long = System.currentTimeMillis()
    ) {
        val appWidgetManager = AppWidgetManager.getInstance(context)
        val timeFormat = SimpleDateFormat("HH:mm:ss", Locale.getDefault())
        val timeStr = timeFormat.format(Date(lastUpdatedTime))
        val phStr = String.format(Locale.US, "%.2f", ph)
        val tempStr = if (temp != null) String.format(Locale.US, "%.1f °C", temp) else "--.- °C"
        val voltStr = if (voltage != null) String.format(Locale.US, "%.2f V", voltage) else "--.-- V"
        val uptimeStr = formatUptime(uptimeSeconds)

        // PendingIntent to launch MainActivity on click
        val clickIntent = Intent(context, MainActivity::class.java).apply {
            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
        }
        val pendingIntent = PendingIntent.getActivity(
            context,
            0,
            clickIntent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        // 1. Update Small Widgets
        val smallWidgetComponent = ComponentName(context, SmallPhWidgetProvider::class.java)
        val smallIds = appWidgetManager.getAppWidgetIds(smallWidgetComponent)
        for (id in smallIds) {
            val views = RemoteViews(context.packageName, R.layout.widget_small)
            views.setTextViewText(R.id.widget_ph_value, phStr)
            views.setTextViewText(R.id.widget_status_badge, alarmState.title)
            views.setTextViewText(R.id.widget_last_update, timeStr)
            applyBadgeStyle(views, R.id.widget_status_badge, alarmState)
            views.setOnClickPendingIntent(R.id.widget_root, pendingIntent)
            appWidgetManager.updateAppWidget(id, views)
        }

        // 2. Update Medium Widgets
        val mediumWidgetComponent = ComponentName(context, MediumPhWidgetProvider::class.java)
        val mediumIds = appWidgetManager.getAppWidgetIds(mediumWidgetComponent)
        for (id in mediumIds) {
            val views = RemoteViews(context.packageName, R.layout.widget_medium)
            views.setTextViewText(R.id.widget_device_name, deviceName)
            views.setTextViewText(R.id.widget_ph_value, phStr)
            views.setTextViewText(R.id.widget_temp_value, tempStr)
            views.setTextViewText(R.id.widget_status_badge, alarmState.title)
            views.setTextViewText(R.id.widget_last_update, "Aktualizacja: $timeStr")
            applyBadgeStyle(views, R.id.widget_status_badge, alarmState)
            views.setOnClickPendingIntent(R.id.widget_root, pendingIntent)
            appWidgetManager.updateAppWidget(id, views)
        }

        // 3. Update Large Widgets
        val largeWidgetComponent = ComponentName(context, LargePhWidgetProvider::class.java)
        val largeIds = appWidgetManager.getAppWidgetIds(largeWidgetComponent)
        for (id in largeIds) {
            val views = RemoteViews(context.packageName, R.layout.widget_large)
            views.setTextViewText(R.id.widget_device_name, deviceName)
            views.setTextViewText(R.id.widget_ip_address, ipAddress ?: "")
            views.setTextViewText(R.id.widget_ph_value, phStr)
            views.setTextViewText(R.id.widget_temp_value, tempStr)
            views.setTextViewText(R.id.widget_voltage_value, voltStr)
            views.setTextViewText(R.id.widget_uptime_value, uptimeStr)
            views.setTextViewText(R.id.widget_status_badge, alarmState.title)
            views.setTextViewText(R.id.widget_last_update, "Ostatni odczyt: $timeStr")
            applyBadgeStyle(views, R.id.widget_status_badge, alarmState)
            views.setOnClickPendingIntent(R.id.widget_root, pendingIntent)
            appWidgetManager.updateAppWidget(id, views)
        }
    }

    private fun applyBadgeStyle(views: RemoteViews, viewId: Int, state: AlarmState) {
        when (state) {
            AlarmState.NORMAL -> {
                views.setInt(viewId, "setBackgroundResource", R.drawable.widget_badge_normal)
                views.setTextColor(viewId, android.graphics.Color.parseColor("#34D399"))
            }
            AlarmState.LOW_PH, AlarmState.HIGH_PH -> {
                views.setInt(viewId, "setBackgroundResource", R.drawable.widget_badge_alarm)
                views.setTextColor(viewId, android.graphics.Color.parseColor("#F87171"))
            }
            AlarmState.OFFLINE, AlarmState.UNKNOWN -> {
                views.setInt(viewId, "setBackgroundResource", R.drawable.widget_badge_offline)
                views.setTextColor(viewId, android.graphics.Color.parseColor("#94A3B8"))
            }
        }
    }

    private fun formatUptime(seconds: Long): String {
        if (seconds <= 0) return "--"
        val d = seconds / 86400
        val h = (seconds % 86400) / 3600
        val m = (seconds % 3600) / 60
        return when {
            d > 0 -> "${d}d ${h}h"
            h > 0 -> "${h}h ${m}m"
            else -> "${m}m"
        }
    }
}
