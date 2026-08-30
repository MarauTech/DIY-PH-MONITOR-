package com.example.marautechphmonitor.data.remote

import com.google.gson.annotations.SerializedName
import okhttp3.ResponseBody
import retrofit2.Response
import retrofit2.http.Body
import retrofit2.http.GET
import retrofit2.http.POST
import retrofit2.http.Query

data class StatusResponse(
    @SerializedName("ph") val ph: Float = 7.0f,
    @SerializedName("voltage") val voltage: Float = 0.0f,
    @SerializedName("temperature") val temperature: Float? = null,
    @SerializedName("tempConnected") val tempConnected: Boolean = false,
    @SerializedName("alarmState") val alarmState: Int = 0,
    @SerializedName("wifiRSSI") val wifiRSSI: Int = 0,
    @SerializedName("ip") val ip: String = "",
    @SerializedName("uptime") val uptime: Long = 0L,
    @SerializedName("freeHeap") val freeHeap: Long = 0L,
    @SerializedName("firmwareVersion") val firmwareVersion: String = "2.0.0",
    @SerializedName("ntpSynced") val ntpSynced: Boolean = false,
    @SerializedName("ntpTime") val ntpTime: String? = null,
    @SerializedName("calibrated") val calibrated: Boolean = false,
    @SerializedName("pushoverStatus") val pushoverStatus: Int = 0,
    @SerializedName("pushoverStatusText") val pushoverStatusText: String? = null,
    @SerializedName("buzzerMuted") val buzzerMuted: Boolean = false,
    @SerializedName("deviceName") val deviceName: String = "pH Monitor",
    @SerializedName("minPH") val minPH: Float? = null,
    @SerializedName("maxPH") val maxPH: Float? = null,
    @SerializedName("minTemp") val minTemp: Float? = null,
    @SerializedName("maxTemp") val maxTemp: Float? = null,
    @SerializedName("voltagePH4") val voltagePH4: Float = 3038f,
    @SerializedName("voltagePH7") val voltagePH7: Float = 2500f,
    @SerializedName("voltagePH9") val voltagePH9: Float = 2108f,
    @SerializedName("ph4Value") val ph4Value: Float = 4.01f,
    @SerializedName("ph7Value") val ph7Value: Float = 7.00f,
    @SerializedName("ph9Value") val ph9Value: Float = 9.18f,
    @SerializedName("calibration") val calibration: CalibrationStatus? = null
)

data class CalibrationStatus(
    @SerializedName("ph4") val ph4: Boolean = false,
    @SerializedName("ph7") val ph7: Boolean = false,
    @SerializedName("ph9") val ph9: Boolean = false,
    @SerializedName("complete") val complete: Boolean = false
)

data class ConfigResponse(
    @SerializedName("alarmLow") val alarmLow: Float = 6.5f,
    @SerializedName("alarmHigh") val alarmHigh: Float = 7.5f,
    @SerializedName("hysteresis") val hysteresis: Float = 0.1f,
    @SerializedName("alarmHoldSec") val alarmHoldSec: Int = 30,
    @SerializedName("pushoverConfigured") val pushoverConfigured: Boolean = false,
    @SerializedName("pushoverEnabled") val pushoverEnabled: Boolean = false,
    @SerializedName("mqttConfigured") val mqttConfigured: Boolean = false,
    @SerializedName("mqttEnabled") val mqttEnabled: Boolean = false,
    @SerializedName("mqttBroker") val mqttBroker: String = "",
    @SerializedName("mqttPort") val mqttPort: Int = 1883,
    @SerializedName("mqttUser") val mqttUser: String = "",
    @SerializedName("deviceName") val deviceName: String = "pH Monitor",
    @SerializedName("buzzerMuted") val buzzerMuted: Boolean = false,
    @SerializedName("adminUser") val adminUser: String = "admin",
    @SerializedName("wifiSSID") val wifiSSID: String = "",
    @SerializedName("calibration") val calibration: CalibrationStatus? = null
)

data class ConfigRequest(
    @SerializedName("alarmLow") val alarmLow: Float? = null,
    @SerializedName("alarmHigh") val alarmHigh: Float? = null,
    @SerializedName("hysteresis") val hysteresis: Float? = null,
    @SerializedName("alarmHoldSec") val alarmHoldSec: Int? = null,
    @SerializedName("deviceName") val deviceName: String? = null,
    @SerializedName("pushoverEnabled") val pushoverEnabled: Boolean? = null,
    @SerializedName("pushoverUser") val pushoverUser: String? = null,
    @SerializedName("pushoverToken") val pushoverToken: String? = null,
    @SerializedName("mqttEnabled") val mqttEnabled: Boolean? = null,
    @SerializedName("mqttBroker") val mqttBroker: String? = null,
    @SerializedName("mqttPort") val mqttPort: Int? = null,
    @SerializedName("mqttUser") val mqttUser: String? = null,
    @SerializedName("mqttPass") val mqttPass: String? = null,
    @SerializedName("buzzerMuted") val buzzerMuted: Boolean? = null,
    @SerializedName("adminUser") val adminUser: String? = null,
    @SerializedName("adminPass") val adminPass: String? = null,
    @SerializedName("wifiSSID") val wifiSSID: String? = null,
    @SerializedName("wifiPass") val wifiPass: String? = null
)

data class CalibrationRequest(
    @SerializedName("type") val type: String,
    @SerializedName("customPH") val customPH: Float? = null
)

data class CalibrationStatusResponse(
    @SerializedName("status") val status: String = "idle",
    @SerializedName("progress") val progress: Int = 0,
    @SerializedName("message") val message: String? = null,
    @SerializedName("voltage") val voltage: Float? = null,
    @SerializedName("stdDev") val stdDev: Float? = null,
    @SerializedName("error") val error: String? = null
)

data class HistoryPoint(
    @SerializedName("t") val timestamp: Long = 0L,
    @SerializedName("p") val ph: Float = 7.0f,
    @SerializedName("te") val temperature: Float? = null,
    @SerializedName("v") val voltage: Float? = null,
    @SerializedName("a") val alarmState: Int? = 0
)

interface PhMonitorApi {
    @GET("api/status")
    suspend fun getStatus(): StatusResponse

    @GET("api/config")
    suspend fun getConfig(): ConfigResponse

    @POST("api/config")
    suspend fun saveConfig(@Body req: ConfigRequest): Response<ResponseBody>

    @POST("api/calibrate")
    suspend fun calibrate(@Body req: CalibrationRequest): Response<ResponseBody>

    @GET("api/calibrate/status")
    suspend fun getCalibrationStatus(): CalibrationStatusResponse

    @GET("api/history")
    suspend fun getHistory(@Query("limit") limit: Int = 1440): List<HistoryPoint>

    @POST("api/pushover/test")
    suspend fun testPushover(): Response<ResponseBody>

    @POST("api/reset-stats")
    suspend fun resetStats(): Response<ResponseBody>
}
