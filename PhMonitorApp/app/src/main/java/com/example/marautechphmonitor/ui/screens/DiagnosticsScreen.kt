package com.example.marautechphmonitor.ui.screens

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.widget.Toast
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.ContentCopy
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.RestartAlt
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.marautechphmonitor.ui.components.SectionHeader
import com.example.marautechphmonitor.ui.viewmodel.MainViewModel
import java.util.Locale

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DiagnosticsScreen(
    viewModel: MainViewModel,
    onNavigateBack: () -> Unit,
    modifier: Modifier = Modifier
) {
    val status by viewModel.status.collectAsState()
    val isConnected by viewModel.isConnected.collectAsState()
    val context = LocalContext.current

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "Diagnostyka Urządzenia",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold
                    )
                },
                navigationIcon = {
                    IconButton(onClick = onNavigateBack) {
                        Icon(imageVector = Icons.Default.ArrowBack, contentDescription = "Wróć")
                    }
                },
                actions = {
                    IconButton(onClick = { viewModel.refreshStatus() }) {
                        Icon(imageVector = Icons.Default.Refresh, contentDescription = "Odśwież")
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.background
                )
            )
        }
    ) { innerPadding ->
        Column(
            modifier = modifier
                .fillMaxSize()
                .padding(innerPadding)
                .verticalScroll(rememberScrollState())
                .padding(horizontal = 16.dp, vertical = 8.dp)
        ) {
            SectionHeader(title = "Parametry Techniczne")

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(MaterialTheme.colorScheme.surface)
                    .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.3f), RoundedCornerShape(16.dp))
                    .padding(16.dp)
            ) {
                Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
                    DiagRow("Adres IP urządzenia", status?.ip ?: "--")
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.15f))
                    DiagRow("Sygnał WiFi (RSSI)", "${status?.wifiRSSI ?: 0} dBm")
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.15f))
                    DiagRow("Wolna pamięć RAM (Free Heap)", "${(status?.freeHeap ?: 0) / 1024} KB")
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.15f))
                    DiagRow("Czas pracy (Uptime)", formatUptime(status?.uptime ?: 0))
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.15f))
                    DiagRow("Wersja oprogramowania", "v${status?.firmwareVersion ?: "2.0.0"}")
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.15f))
                    DiagRow("Czujnik temperatury DS18B20", if (status?.tempConnected == true) "Połączony (OK)" else "Odłączony / Błąd")
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.15f))
                    DiagRow("Czas NTP", if (status?.ntpSynced == true) "Zsynchronizowany (${status?.ntpTime ?: ""})" else "Niezsynchronizowany")
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.15f))
                    DiagRow("Napięcie przetwornika pH", String.format(Locale.US, "%.3f V", (status?.voltage ?: 0f) / 1000f))
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.15f))
                    DiagRow("Status Pushover", status?.pushoverStatusText ?: if (status?.pushoverStatus == 1) "OK" else "Brak")
                }
            }

            Spacer(modifier = Modifier.height(20.dp))

            // Action Buttons
            Button(
                onClick = {
                    val diagText = buildString {
                        appendLine("=== MarauTech pH Monitor - Raport Diagnostyczny ===")
                        appendLine("Device Name: ${status?.deviceName ?: "pH Monitor"}")
                        appendLine("IP: ${status?.ip ?: "--"}")
                        appendLine("WiFi RSSI: ${status?.wifiRSSI ?: 0} dBm")
                        appendLine("Free Heap: ${(status?.freeHeap ?: 0) / 1024} KB")
                        appendLine("Uptime: ${status?.uptime ?: 0}s")
                        appendLine("Firmware: v${status?.firmwareVersion ?: "2.0.0"}")
                        appendLine("Temp Sensor Connected: ${status?.tempConnected ?: false}")
                        appendLine("Current pH: ${status?.ph ?: 7.0f}")
                        appendLine("Current Voltage: ${(status?.voltage ?: 0f) / 1000f} V")
                        appendLine("Calibration Complete: ${status?.calibration?.complete ?: false}")
                        appendLine("=================================================")
                    }

                    val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                    val clip = ClipData.newPlainText("pH Monitor Diagnostyka", diagText)
                    clipboard.setPrimaryClip(clip)
                    Toast.makeText(context, "Skopiowano diagnostykę do schowka!", Toast.LENGTH_SHORT).show()
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(50.dp),
                shape = RoundedCornerShape(12.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.primaryContainer,
                    contentColor = MaterialTheme.colorScheme.onPrimaryContainer
                )
            ) {
                Icon(imageVector = Icons.Default.ContentCopy, contentDescription = null, modifier = Modifier.size(18.dp))
                Spacer(modifier = Modifier.width(8.dp))
                Text("Kopiuj Diagnostykę", fontWeight = FontWeight.Bold)
            }

            Spacer(modifier = Modifier.height(12.dp))

            OutlinedButton(
                onClick = {
                    viewModel.resetStats { success ->
                        if (success) {
                            Toast.makeText(context, "Zresetowano statystyki Min/Max!", Toast.LENGTH_SHORT).show()
                        } else {
                            Toast.makeText(context, "Błąd resetowania statystyk", Toast.LENGTH_SHORT).show()
                        }
                    }
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(50.dp),
                shape = RoundedCornerShape(12.dp)
            ) {
                Icon(imageVector = Icons.Default.RestartAlt, contentDescription = null, modifier = Modifier.size(18.dp))
                Spacer(modifier = Modifier.width(8.dp))
                Text("Resetuj Statystyki Min/Max")
            }

            Spacer(modifier = Modifier.height(20.dp))
        }
    }
}

@Composable
private fun DiagRow(label: String, value: String) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = label,
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Text(
            text = value,
            style = MaterialTheme.typography.bodySmall,
            fontWeight = FontWeight.Bold,
            color = MaterialTheme.colorScheme.onSurface
        )
    }
}

private fun formatUptime(seconds: Long): String {
    if (seconds <= 0) return "--"
    val d = seconds / 86400
    val h = (seconds % 86400) / 3600
    val m = (seconds % 3600) / 60
    return when {
        d > 0 -> "${d}d ${h}h ${m}m"
        h > 0 -> "${h}h ${m}m"
        else -> "${m}m"
    }
}
