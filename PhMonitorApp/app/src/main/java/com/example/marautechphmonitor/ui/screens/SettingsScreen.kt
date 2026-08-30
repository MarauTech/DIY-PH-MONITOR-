package com.example.marautechphmonitor.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.Save
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.marautechphmonitor.data.remote.ConfigRequest
import com.example.marautechphmonitor.ui.components.SectionHeader
import com.example.marautechphmonitor.ui.viewmodel.MainViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
    viewModel: MainViewModel,
    onNavigateBack: () -> Unit,
    modifier: Modifier = Modifier
) {
    val config by viewModel.config.collectAsState()
    val isConfigLoading by viewModel.isConfigLoading.collectAsState()
    val configMessage by viewModel.configMessage.collectAsState()

    val notificationsEnabled by viewModel.notificationsEnabled.collectAsState(initial = true)
    val notifyLow by viewModel.notifyLow.collectAsState(initial = true)
    val notifyHigh by viewModel.notifyHigh.collectAsState(initial = true)
    val notifyRecovery by viewModel.notifyRecovery.collectAsState(initial = true)
    val checkInterval by viewModel.checkInterval.collectAsState(initial = 15)

    var alarmLow by remember { mutableStateOf("6.50") }
    var alarmHigh by remember { mutableStateOf("7.50") }
    var hysteresis by remember { mutableStateOf("0.10") }
    var alarmHoldSec by remember { mutableStateOf("30") }
    var deviceName by remember { mutableStateOf("MarauTech pH") }
    var buzzerMuted by remember { mutableStateOf(false) }

    var localNotifEn by remember { mutableStateOf(true) }
    var localNotifLow by remember { mutableStateOf(true) }
    var localNotifHigh by remember { mutableStateOf(true) }
    var localNotifRec by remember { mutableStateOf(true) }

    LaunchedEffect(Unit) {
        viewModel.loadConfig()
    }

    LaunchedEffect(config) {
        config?.let {
            alarmLow = it.alarmLow.toString()
            alarmHigh = it.alarmHigh.toString()
            hysteresis = it.hysteresis.toString()
            alarmHoldSec = it.alarmHoldSec.toString()
            deviceName = it.deviceName
            buzzerMuted = it.buzzerMuted
        }
    }

    LaunchedEffect(notificationsEnabled, notifyLow, notifyHigh, notifyRecovery) {
        localNotifEn = notificationsEnabled
        localNotifLow = notifyLow
        localNotifHigh = notifyHigh
        localNotifRec = notifyRecovery
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "Ustawienia Alarmów",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold
                    )
                },
                navigationIcon = {
                    IconButton(onClick = onNavigateBack) {
                        Icon(imageVector = Icons.Default.ArrowBack, contentDescription = "Wróć")
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
            // Status feedback message
            configMessage?.let { (isSuccess, msg) ->
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp))
                        .background(if (isSuccess) Color(0x2610B981) else Color(0x33EF4444))
                        .border(1.dp, if (isSuccess) Color(0xFF10B981) else Color(0xFFEF4444), RoundedCornerShape(12.dp))
                        .padding(14.dp)
                ) {
                    Text(
                        text = msg,
                        style = MaterialTheme.typography.bodyMedium,
                        fontWeight = FontWeight.SemiBold,
                        color = if (isSuccess) Color(0xFF34D399) else Color(0xFFF87171)
                    )
                }
                Spacer(modifier = Modifier.height(14.dp))
            }

            // Progi pH Section
            SectionHeader(title = "Progi Alarmowe pH")

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(MaterialTheme.colorScheme.surface)
                    .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.3f), RoundedCornerShape(16.dp))
                    .padding(16.dp)
            ) {
                Column(verticalArrangement = Arrangement.spacedBy(14.dp)) {
                    OutlinedTextField(
                        value = alarmLow,
                        onValueChange = { alarmLow = it },
                        label = { Text("Dolny próg pH (LOW)") },
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Decimal),
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    OutlinedTextField(
                        value = alarmHigh,
                        onValueChange = { alarmHigh = it },
                        label = { Text("Górny próg pH (HIGH)") },
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Decimal),
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    OutlinedTextField(
                        value = hysteresis,
                        onValueChange = { hysteresis = it },
                        label = { Text("Histereza (pH)") },
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Decimal),
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    OutlinedTextField(
                        value = alarmHoldSec,
                        onValueChange = { alarmHoldSec = it },
                        label = { Text("Czas potwierdzenia alarmu (sekundy)") },
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = "Wycisz fizyczny buzer",
                            style = MaterialTheme.typography.bodyMedium,
                            fontWeight = FontWeight.Medium
                        )
                        Switch(
                            checked = buzzerMuted,
                            onCheckedChange = { buzzerMuted = it }
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(20.dp))

            // Background Notifications Section
            SectionHeader(title = "Powiadomienia Android w tle (WorkManager)")

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(MaterialTheme.colorScheme.surface)
                    .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.3f), RoundedCornerShape(16.dp))
                    .padding(16.dp)
            ) {
                Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = "Włącz monitoring w tle",
                            style = MaterialTheme.typography.bodyMedium,
                            fontWeight = FontWeight.SemiBold
                        )
                        Switch(
                            checked = localNotifEn,
                            onCheckedChange = { localNotifEn = it }
                        )
                    }

                    if (localNotifEn) {
                        Divider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.2f))

                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text("Alert przy zbyt niskim pH (LOW)", style = MaterialTheme.typography.bodySmall)
                            Checkbox(checked = localNotifLow, onCheckedChange = { localNotifLow = it })
                        }

                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text("Alert przy zbyt wysokim pH (HIGH)", style = MaterialTheme.typography.bodySmall)
                            Checkbox(checked = localNotifHigh, onCheckedChange = { localNotifHigh = it })
                        }

                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text("Powiadomienie o powrocie do normy", style = MaterialTheme.typography.bodySmall)
                            Checkbox(checked = localNotifRec, onCheckedChange = { localNotifRec = it })
                        }

                        Text(
                            text = "Interwał odpytywania w tle: 15 minut (standardowe ograniczenie systemu Android dla oszczędzania baterii).",
                            style = MaterialTheme.typography.labelSmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            Button(
                onClick = {
                    val low = alarmLow.toFloatOrNull()
                    val high = alarmHigh.toFloatOrNull()
                    val hyst = hysteresis.toFloatOrNull()
                    val hold = alarmHoldSec.toIntOrNull()

                    if (low != null && high != null && hyst != null && hold != null) {
                        viewModel.saveConfig(
                            ConfigRequest(
                                alarmLow = low,
                                alarmHigh = high,
                                hysteresis = hyst,
                                alarmHoldSec = hold,
                                deviceName = deviceName,
                                buzzerMuted = buzzerMuted
                            )
                        )
                    }

                    viewModel.saveNotificationPreferences(
                        enabled = localNotifEn,
                        notifyLow = localNotifLow,
                        notifyHigh = localNotifHigh,
                        notifyRecovery = localNotifRec,
                        intervalMinutes = checkInterval
                    )
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(52.dp),
                shape = RoundedCornerShape(12.dp),
                enabled = !isConfigLoading
            ) {
                if (isConfigLoading) {
                    CircularProgressIndicator(modifier = Modifier.size(20.dp), color = MaterialTheme.colorScheme.onPrimary)
                } else {
                    Icon(imageVector = Icons.Default.Save, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Zapisz Ustawienia", fontWeight = FontWeight.Bold)
                }
            }

            Spacer(modifier = Modifier.height(20.dp))
        }
    }
}
