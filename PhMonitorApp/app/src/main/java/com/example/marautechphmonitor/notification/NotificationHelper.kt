package com.example.marautechphmonitor.notification

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import androidx.core.app.NotificationCompat
import com.example.marautechphmonitor.MainActivity
import com.example.marautechphmonitor.R
import com.example.marautechphmonitor.model.AlarmState

object NotificationHelper {
    const val CHANNEL_ALARMS_ID = "ph_alarms_channel"
    const val CHANNEL_STATUS_ID = "ph_status_channel"
    private const val ALARM_NOTIFICATION_ID = 1001
    private const val RECOVERY_NOTIFICATION_ID = 1002

    fun createNotificationChannels(context: Context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val notificationManager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

            // High priority channel for pH Alarms
            val alarmsChannel = NotificationChannel(
                CHANNEL_ALARMS_ID,
                context.getString(R.string.notification_channel_alarms_name),
                NotificationManager.IMPORTANCE_HIGH
            ).apply {
                description = context.getString(R.string.notification_channel_alarms_desc)
                enableVibration(true)
                enableLights(true)
            }

            // Normal priority channel for Status & Recovery
            val statusChannel = NotificationChannel(
                CHANNEL_STATUS_ID,
                context.getString(R.string.notification_channel_status_name),
                NotificationManager.IMPORTANCE_DEFAULT
            ).apply {
                description = context.getString(R.string.notification_channel_status_desc)
            }

            notificationManager.createNotificationChannel(alarmsChannel)
            notificationManager.createNotificationChannel(statusChannel)
        }
    }

    fun showAlarmNotification(
        context: Context,
        title: String,
        message: String,
        alarmState: AlarmState
    ) {
        val notificationManager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        createNotificationChannels(context)

        val intent = Intent(context, MainActivity::class.java).apply {
            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
        }
        val pendingIntent = PendingIntent.getActivity(
            context,
            0,
            intent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val notification = NotificationCompat.Builder(context, CHANNEL_ALARMS_ID)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentTitle(title)
            .setContentText(message)
            .setStyle(NotificationCompat.BigTextStyle().bigText(message))
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setCategory(NotificationCompat.CATEGORY_ALARM)
            .setAutoCancel(true)
            .setContentIntent(pendingIntent)
            .build()

        notificationManager.notify(ALARM_NOTIFICATION_ID, notification)
    }

    fun showRecoveryNotification(
        context: Context,
        title: String,
        message: String
    ) {
        val notificationManager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        createNotificationChannels(context)

        val intent = Intent(context, MainActivity::class.java).apply {
            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
        }
        val pendingIntent = PendingIntent.getActivity(
            context,
            0,
            intent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val notification = NotificationCompat.Builder(context, CHANNEL_STATUS_ID)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentTitle(title)
            .setContentText(message)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setAutoCancel(true)
            .setContentIntent(pendingIntent)
            .build()

        notificationManager.notify(RECOVERY_NOTIFICATION_ID, notification)
    }
}
