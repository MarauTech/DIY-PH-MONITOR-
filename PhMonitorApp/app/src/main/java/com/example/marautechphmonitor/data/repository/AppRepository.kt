package com.example.marautechphmonitor.data.repository

import com.example.marautechphmonitor.data.local.DataStoreManager
import com.example.marautechphmonitor.data.remote.CalibrationRequest
import com.example.marautechphmonitor.data.remote.ConfigRequest
import com.example.marautechphmonitor.data.remote.HistoryPoint
import com.example.marautechphmonitor.data.remote.PhMonitorApi
import com.example.marautechphmonitor.data.remote.RetrofitHelper
import com.example.marautechphmonitor.data.remote.StatusResponse
import kotlinx.coroutines.flow.first

class AppRepository(val dataStoreManager: DataStoreManager) {

    suspend fun getBaseTarget(): String {
        val ip = dataStoreManager.deviceIpFlow.first() ?: "192.168.1.1"
        val port = dataStoreManager.devicePortFlow.first()
        return if (port == 80 || port <= 0) ip else "$ip:$port"
    }

    suspend fun getApi(): PhMonitorApi {
        val target = getBaseTarget()
        return RetrofitHelper.getApi(
            ip = target,
            user = dataStoreManager.getAdminUser(),
            pass = dataStoreManager.getAdminPass()
        )
    }

    suspend fun testConnection(ip: String, port: Int = 80, user: String? = null, pass: String? = null): StatusResponse {
        val target = if (port == 80 || port <= 0) ip.trim() else "${ip.trim()}:$port"
        val api = RetrofitHelper.getApi(target, user, pass)
        return api.getStatus()
    }
    
    suspend fun getStatus(): StatusResponse {
        val status = getApi().getStatus()
        dataStoreManager.cacheStatus(
            ph = status.ph,
            temp = if (status.tempConnected) status.temperature else null,
            voltage = status.voltage,
            alarmState = status.alarmState,
            deviceName = status.deviceName
        )
        return status
    }

    suspend fun getConfig() = getApi().getConfig()
    suspend fun saveConfig(req: ConfigRequest) = getApi().saveConfig(req)
    suspend fun calibrate(req: CalibrationRequest) = getApi().calibrate(req)
    suspend fun getCalibrationStatus() = getApi().getCalibrationStatus()
    suspend fun getHistory(limit: Int = 1440): List<HistoryPoint> = getApi().getHistory(limit)
    suspend fun testPushover() = getApi().testPushover()
    suspend fun resetStats() = getApi().resetStats()
    
    suspend fun saveDeviceConnection(ip: String, port: Int = 80, name: String = "MarauTech pH") {
        dataStoreManager.saveDeviceConnection(ip, port, name)
    }

    fun saveCredentials(user: String, pass: String) = dataStoreManager.saveCredentials(user, pass)
    fun clearCredentials() = dataStoreManager.clearCredentials()
    
    suspend fun getDeviceIp() = dataStoreManager.deviceIpFlow.first()
    suspend fun getDevicePort() = dataStoreManager.devicePortFlow.first()
    suspend fun getDeviceName() = dataStoreManager.deviceNameFlow.first()
    fun getAdminUser() = dataStoreManager.getAdminUser()
    fun getAdminPass() = dataStoreManager.getAdminPass()
}
