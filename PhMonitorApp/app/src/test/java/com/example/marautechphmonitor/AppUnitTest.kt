package com.example.marautechphmonitor

import com.example.marautechphmonitor.model.AlarmState
import com.google.gson.Gson
import com.example.marautechphmonitor.data.remote.HistoryPoint
import com.example.marautechphmonitor.data.remote.StatusResponse
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test

class AppUnitTest {

    @Test
    fun testAlarmStateMapping() {
        assertEquals(AlarmState.NORMAL, AlarmState.fromCode(0, true))
        assertEquals(AlarmState.LOW_PH, AlarmState.fromCode(1, true))
        assertEquals(AlarmState.HIGH_PH, AlarmState.fromCode(2, true))
        assertEquals(AlarmState.OFFLINE, AlarmState.fromCode(0, false))
        assertEquals(AlarmState.UNKNOWN, AlarmState.fromCode(99, true))
    }

    @Test
    fun testStatusResponseJsonParsing() {
        val json = """
            {
                "ph": 7.21,
                "voltage": 2642.0,
                "temperature": 24.5,
                "tempConnected": true,
                "alarmState": 0,
                "wifiRSSI": -65,
                "ip": "192.168.1.120",
                "uptime": 3600,
                "freeHeap": 180000,
                "firmwareVersion": "2.0.0",
                "ntpSynced": true,
                "ntpTime": "12:00:00",
                "calibrated": true,
                "pushoverStatus": 1,
                "deviceName": "Akwarium Główne",
                "minPH": 7.10,
                "maxPH": 7.35,
                "calibration": {
                    "ph4": true,
                    "ph7": true,
                    "ph9": false,
                    "complete": false
                }
            }
        """.trimIndent()

        val gson = Gson()
        val status = gson.fromJson(json, StatusResponse::class.java)

        assertNotNull(status)
        assertEquals(7.21f, status.ph, 0.001f)
        assertEquals(2642.0f, status.voltage, 0.001f)
        assertEquals(24.5f, status.temperature!!, 0.001f)
        assertEquals(true, status.tempConnected)
        assertEquals(0, status.alarmState)
        assertEquals("192.168.1.120", status.ip)
        assertEquals("Akwarium Główne", status.deviceName)
        assertEquals(true, status.calibration?.ph7)
        assertEquals(false, status.calibration?.complete)
    }

    @Test
    fun testHistoryPointJsonParsing() {
        val json = """
            [
                {"t": 1725000000, "p": 7.20, "te": 24.5, "v": 2.64, "a": 0},
                {"t": 1725000060, "p": 7.22, "te": 24.6, "v": 2.63, "a": 0},
                {"t": 1725000120, "p": 6.45, "te": 24.5, "v": 2.75, "a": 1}
            ]
        """.trimIndent()

        val gson = Gson()
        val points = gson.fromJson(json, Array<HistoryPoint>::class.java).toList()

        assertEquals(3, points.size)
        assertEquals(7.20f, points[0].ph, 0.01f)
        assertEquals(24.5f, points[0].temperature!!, 0.01f)
        assertEquals(1725000000L, points[0].timestamp)
        assertEquals(6.45f, points[2].ph, 0.01f)
        assertEquals(1, points[2].alarmState)
    }

    @Test
    fun testHistoryStatisticsCalculation() {
        val samples = listOf(7.10f, 7.25f, 7.30f, 7.15f, 7.20f)
        val min = samples.minOrNull() ?: 0f
        val max = samples.maxOrNull() ?: 0f
        val avg = samples.average().toFloat()

        assertEquals(7.10f, min, 0.001f)
        assertEquals(7.30f, max, 0.001f)
        assertEquals(7.20f, avg, 0.001f)
    }

    @Test
    fun testIpAddressValidation() {
        val validIp = "192.168.1.120"
        val validIpWithPort = "192.168.1.120:8080"
        val invalidIp = "999.999.999.999"

        val ipRegex = Regex("^((25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)\\.?\\b){4}(:\\d+)?$")
        assertTrue(validIp.matches(ipRegex))
        assertTrue(validIpWithPort.matches(ipRegex))
    }
}
