package com.example.marautechphmonitor.worker

import android.content.Context
import androidx.work.Constraints
import androidx.work.ExistingPeriodicWorkPolicy
import androidx.work.NetworkType
import androidx.work.PeriodicWorkRequestBuilder
import androidx.work.WorkManager
import java.util.concurrent.TimeUnit

object WorkManagerScheduler {

    fun scheduleMonitoring(context: Context, intervalMinutes: Int = 15) {
        val safeInterval = if (intervalMinutes < 15) 15L else intervalMinutes.toLong()

        val constraints = Constraints.Builder()
            .setRequiredNetworkType(NetworkType.CONNECTED)
            .build()

        val workRequest = PeriodicWorkRequestBuilder<DeviceMonitorWorker>(
            safeInterval, TimeUnit.MINUTES,
            5, TimeUnit.MINUTES // flex interval
        )
            .setConstraints(constraints)
            .build()

        WorkManager.getInstance(context).enqueueUniquePeriodicWork(
            DeviceMonitorWorker.WORK_NAME,
            ExistingPeriodicWorkPolicy.UPDATE,
            workRequest
        )
    }

    fun cancelMonitoring(context: Context) {
        WorkManager.getInstance(context).cancelUniqueWork(DeviceMonitorWorker.WORK_NAME)
    }
}
