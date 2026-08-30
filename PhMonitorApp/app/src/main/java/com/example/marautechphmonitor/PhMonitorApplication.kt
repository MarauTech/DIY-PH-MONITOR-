package com.example.marautechphmonitor

import android.app.Application
import com.example.marautechphmonitor.data.local.DataStoreManager
import com.example.marautechphmonitor.notification.NotificationHelper
import com.example.marautechphmonitor.worker.WorkManagerScheduler
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

class PhMonitorApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        NotificationHelper.createNotificationChannels(this)

        // Ensure background monitoring is scheduled if enabled
        val dataStoreManager = DataStoreManager(this)
        CoroutineScope(Dispatchers.IO).launch {
            val enabled = dataStoreManager.notificationsEnabledFlow.first()
            val interval = dataStoreManager.checkIntervalFlow.first()
            if (enabled) {
                WorkManagerScheduler.scheduleMonitoring(this@PhMonitorApplication, interval)
            }
        }
    }
}
