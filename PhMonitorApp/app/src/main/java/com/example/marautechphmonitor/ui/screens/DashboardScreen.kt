package com.example.marautechphmonitor.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.marautechphmonitor.model.AlarmState
import com.example.marautechphmonitor.ui.components.MetricCard
import com.example.marautechphmonitor.ui.components.PhHeroCard
import com.example.marautechphmonitor.ui.components.SectionHeader
import com.example.marautechphmonitor.ui.viewmodel.MainViewModel
import java.util.Locale

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DashboardScreen(
    viewModel: MainViewModel,
    onNavigateToCalibration: () -> Unit,
    onNavigateToHistory: () -> Unit,
    onNavigateToConnect: () -> Unit,
    modifier: Modifier = Modifier
) {
    val status by viewModel.status.collectAsState()
    val alarmState by viewModel.alarmState.collectAsState()
    val isConnected by viewModel.isConnected.collectAsState()
    val lastUpdated by viewModel.lastUpdatedTime.collectAsState()

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Column {
                        Text(
                            text = status?.deviceName ?: "MarauTech pH Monitor",
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Bold,
                            color = MaterialTheme.colorScheme.onBackground
                        )
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Box(
                                modifier = Modifier
                                    .size(8.dp)
                                    .clip(RoundedCornerShape(4.dp))
                                    .background(if (isConnected) Color(0xFF10B981) else Color(0xFFEF4444))
                            )
                            Spacer(modifier = Modifier.width(6.dp))
                            Text(
                                text = if (isConnected) "Online ($lastUpdated)" else "Offline ($lastUpdated)",
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }
                },
                actions = {
                    IconButton(onClick = { viewModel.refreshStatus() }) {
                        Icon(
                            imageVector = Icons.Default.Refresh,
                            contentDescription = "Odśwież",
                            tint = MaterialTheme.colorScheme.primary
                        )
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
            // Hero Card
            val ph = status?.ph ?: 7.0f
            val volt = (status?.voltage ?: 0f) / 1000f
            PhHeroCard(
                ph = ph,
                voltage = volt,
                alarmState = alarmState,
                minPh = status?.minPH,
                maxPh = status?.maxPH,
                modifier = Modifier.fillMaxWidth()
            )

            Spacer(modifier = Modifier.height(16.dp))

            // Secondary Metrics Grid
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                val tempConnected = status?.tempConnected == true
                val tempVal = if (tempConnected && status?.temperature != null) {
                    String.format(Locale.US, "%.1f", status?.temperature)
                } else "--.-"

                MetricCard(
                    title = "Temperatura",
                    value = tempVal,
                    unit = "°C",
                    icon = Icons.Default.Thermostat,
                    iconColor = Color(0xFFF97316),
                    subValue = if (tempConnected) "Czujnik DS18B20" else "Brak czujnika",
                    modifier = Modifier.weight(1f)
                )

                val rssiVal = status?.wifiRSSI ?: 0
                val rssiQuality = when {
                    rssiVal >= -60 -> "Doskonały"
                    rssiVal >= -75 -> "Dobry"
                    rssiVal >= -85 -> "Słaby"
                    else -> "Brak sygnału"
                }

                MetricCard(
                    title = "Sygnał WiFi",
                    value = "$rssiVal",
                    unit = "dBm",
                    icon = Icons.Default.Wifi,
                    iconColor = Color(0xFF38BDF8),
                    subValue = rssiQuality,
                    modifier = Modifier.weight(1f)
                )
            }

            Spacer(modifier = Modifier.height(20.dp))

            // Device Info Section
            SectionHeader(title = "Stan Urządzenia")

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(MaterialTheme.colorScheme.surface)
                    .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.4f), RoundedCornerShape(16.dp))
                    .padding(16.dp)
            ) {
                Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
                    InfoRow(
                        label = "Czas pracy (Uptime)",
                        value = formatUptime(status?.uptime ?: 0)
                    )
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.2f))
                    InfoRow(
                        label = "Wersja oprogramowania",
                        value = "v${status?.firmwareVersion ?: "2.0.0"}"
                    )
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.2f))
                    InfoRow(
                        label = "Synchronizacja czasu (NTP)",
                        value = if (status?.ntpSynced == true) "Zsynchronizowany" else "Oczekiwanie"
                    )
                    Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.2f))
                    InfoRow(
                        label = "Status kalibracji",
                        value = when {
                            status?.calibration?.complete == true -> "Pełna (3-punktowa)"
                            status?.calibrated == true -> "Częściowa"
                            else -> "Domyślna (fabryczna)"
                        },
                        valueColor = if (status?.calibration?.complete == true) Color(0xFF10B981) else Color(0xFFF59E0B)
                    )
                }
            }

            Spacer(modifier = Modifier.height(20.dp))

            // Quick Actions
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Button(
                    onClick = onNavigateToCalibration,
                    modifier = Modifier.weight(1f),
                    shape = RoundedCornerShape(12.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.surfaceVariant,
                        contentColor = MaterialTheme.colorScheme.onSurface
                    )
                ) {
                    Icon(imageVector = Icons.Default.Tune, contentDescription = null, modifier = Modifier.size(18.dp))
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Kalibracja", fontWeight = FontWeight.SemiBold)
                }

                Button(
                    onClick = onNavigateToHistory,
                    modifier = Modifier.weight(1f),
                    shape = RoundedCornerShape(12.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.surfaceVariant,
                        contentColor = MaterialTheme.colorScheme.onSurface
                    )
                ) {
                    Icon(imageVector = Icons.Default.ShowChart, contentDescription = null, modifier = Modifier.size(18.dp))
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Historia", fontWeight = FontWeight.SemiBold)
                }
            }

            Spacer(modifier = Modifier.height(16.dp))
        }
    }
}

@Composable
private fun InfoRow(
    label: String,
    value: String,
    valueColor: Color = MaterialTheme.colorScheme.onSurface
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = label,
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Text(
            text = value,
            style = MaterialTheme.typography.bodyMedium,
            fontWeight = FontWeight.Bold,
            color = valueColor
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
