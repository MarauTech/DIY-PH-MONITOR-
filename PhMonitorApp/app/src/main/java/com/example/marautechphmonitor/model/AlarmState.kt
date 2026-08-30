package com.example.marautechphmonitor.model

import androidx.compose.ui.graphics.Color

enum class AlarmState(
    val code: Int,
    val title: String,
    val description: String,
    val color: Color,
    val backgroundColor: Color,
    val borderColor: Color
) {
    NORMAL(
        code = 0,
        title = "NORMAL",
        description = "Parametry w normie",
        color = Color(0xFF10B981), // Emerald 500
        backgroundColor = Color(0x2610B981),
        borderColor = Color(0xFF34D399)
    ),
    LOW_PH(
        code = 1,
        title = "LOW pH",
        description = "Zbyt niskie pH (kwaśno)",
        color = Color(0xFFF59E0B), // Amber 500
        backgroundColor = Color(0x33F59E0B),
        borderColor = Color(0xFFFBBF24)
    ),
    HIGH_PH(
        code = 2,
        title = "HIGH pH",
        description = "Zbyt wysokie pH (zasadowo)",
        color = Color(0xFFEC4899), // Pink 500 / Magenta
        backgroundColor = Color(0x33EC4899),
        borderColor = Color(0xFFF472B6)
    ),
    OFFLINE(
        code = -1,
        title = "OFFLINE",
        description = "Brak połączenia z ESP32",
        color = Color(0xFF94A3B8), // Slate 400
        backgroundColor = Color(0x2664748B),
        borderColor = Color(0xFF64748B)
    ),
    UNKNOWN(
        code = -2,
        title = "UNKNOWN",
        description = "Status nieznany",
        color = Color(0xFFCBD5E1),
        backgroundColor = Color(0x1ACBD5E1),
        borderColor = Color(0xFF94A3B8)
    );

    companion object {
        fun fromCode(code: Int, isConnected: Boolean = true): AlarmState {
            if (!isConnected) return OFFLINE
            return when (code) {
                0 -> NORMAL
                1 -> LOW_PH
                2 -> HIGH_PH
                else -> UNKNOWN
            }
        }
    }
}
