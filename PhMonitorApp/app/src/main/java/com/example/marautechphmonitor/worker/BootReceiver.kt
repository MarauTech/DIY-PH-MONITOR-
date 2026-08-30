package com.example.marautechphmonitor.worker

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import com.example.marautechphmonitor.data.local.DataStoreManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

class BootReceiver : BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action == Intent.ACTION_BOOT_COMPLETED ||
            intent.action == Intent.ACTION_MY_PACKAGE_REPLACED) {
            val dataStoreManager = DataStoreManager(context)
            CoroutineScope(Dispatchers.IO).launch {
                val enabled = dataStoreManager.notificationsEnabledFlow.first()
                val interval = dataStoreManager.checkIntervalFlow.first()
                if (enabled) {
                    WorkManagerScheduler.scheduleMonitoring(context, interval)
                }
            }
        }
    }
}
