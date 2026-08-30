package com.example.marautechphmonitor.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
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
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.marautechphmonitor.ui.components.SectionHeader
import com.example.marautechphmonitor.ui.viewmodel.MainViewModel
import java.util.Locale

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CalibrationScreen(
    viewModel: MainViewModel,
    modifier: Modifier = Modifier
) {
    val status by viewModel.status.collectAsState()
    val isCalibrating by viewModel.isCalibrating.collectAsState()
    val calibMessage by viewModel.calibrationMessage.collectAsState()
    val calibError by viewModel.calibrationError.collectAsState()
    val calibStatus by viewModel.calibrationStatus.collectAsState()

    var showResetDialog by remember { mutableStateOf(false) }
    var customBufferInput by remember { mutableStateOf("6.86") }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "Kalibracja Sondy",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold
                    )
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
            // Live Probe Readout Card
            val curVolt = (status?.voltage ?: 0f) / 1000f
            val curPh = status?.ph ?: 7.0f

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(MaterialTheme.colorScheme.surface)
                    .border(1.dp, MaterialTheme.colorScheme.primary.copy(alpha = 0.5f), RoundedCornerShape(16.dp))
                    .padding(16.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column {
                        Text(
                            text = "ODCZYT SONDY NA ŻYWO",
                            style = MaterialTheme.typography.labelSmall,
                            color = MaterialTheme.colorScheme.primary,
                            fontWeight = FontWeight.Bold,
                            letterSpacing = 0.5.sp
                        )
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = String.format(Locale.US, "%.3f V  /  %.2f pH", curVolt, curPh),
                            style = MaterialTheme.typography.titleLarge,
                            fontWeight = FontWeight.Bold,
                            color = MaterialTheme.colorScheme.onSurface
                        )
                    }

                    Icon(
                        imageVector = Icons.Default.Sensors,
                        contentDescription = null,
                        tint = MaterialTheme.colorScheme.primary,
                        modifier = Modifier.size(28.dp)
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Calibration Progress Box
            if (isCalibrating || calibMessage != null || calibError != null) {
                val isSuccess = calibMessage != null && !isCalibrating && calibError == null
                val isFailed = calibError != null

                val boxBg = when {
                    isFailed -> Color(0x33EF4444)
                    isSuccess -> Color(0x2610B981)
                    else -> Color(0x2638BDF8)
                }
                val boxBorder = when {
                    isFailed -> Color(0xFFEF4444)
                    isSuccess -> Color(0xFF10B981)
                    else -> Color(0xFF38BDF8)
                }
                val textColor = when {
                    isFailed -> Color(0xFFF87171)
                    isSuccess -> Color(0xFF34D399)
                    else -> Color(0xFF38BDF8)
                }

                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(14.dp))
                        .background(boxBg)
                        .border(1.dp, boxBorder, RoundedCornerShape(14.dp))
                        .padding(16.dp)
                ) {
                    Column {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            if (isCalibrating) {
                                CircularProgressIndicator(
                                    modifier = Modifier.size(18.dp),
                                    strokeWidth = 2.dp,
                                    color = MaterialTheme.colorScheme.primary
                                )
                                Spacer(modifier = Modifier.width(10.dp))
                            }
                            Text(
                                text = calibError ?: calibMessage ?: "",
                                style = MaterialTheme.typography.bodyMedium,
                                fontWeight = FontWeight.SemiBold,
                                color = textColor
                            )
                        }

                        if (isCalibrating && calibStatus?.status == "collecting") {
                            Spacer(modifier = Modifier.height(10.dp))
                            LinearProgressIndicator(
                                progress = { (calibStatus?.progress ?: 0) / 100f },
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .height(6.dp)
                                    .clip(RoundedCornerShape(3.dp)),
                                color = MaterialTheme.colorScheme.primary,
                            )
                        }
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))
            }

            // Standard Buffers Section
            SectionHeader(title = "Standardowe punkty buforowe")

            // Buffer 7.00
            val cal7 = status?.calibration?.ph7 == true
            val v7 = (status?.voltagePH7 ?: 2500f) / 1000f
            BufferCard(
                title = "Bufor neutralny (pH 7.00)",
                phValue = "7.00",
                voltage = v7,
                isCalibrated = cal7,
                isBusy = isCalibrating,
                onCalibrate = { viewModel.startCalibration("7.00") }
            )

            Spacer(modifier = Modifier.height(10.dp))

            // Buffer 4.01
            val cal4 = status?.calibration?.ph4 == true
            val v4 = (status?.voltagePH4 ?: 3038f) / 1000f
            BufferCard(
                title = "Bufor kwaśny (pH 4.01)",
                phValue = "4.01",
                voltage = v4,
                isCalibrated = cal4,
                isBusy = isCalibrating,
                onCalibrate = { viewModel.startCalibration("4.01") }
            )

            Spacer(modifier = Modifier.height(10.dp))

            // Buffer 9.18
            val cal9 = status?.calibration?.ph9 == true
            val v9 = (status?.voltagePH9 ?: 2108f) / 1000f
            BufferCard(
                title = "Bufor zasadowy (pH 9.18)",
                phValue = "9.18",
                voltage = v9,
                isCalibrated = cal9,
                isBusy = isCalibrating,
                onCalibrate = { viewModel.startCalibration("9.18") }
            )

            Spacer(modifier = Modifier.height(20.dp))

            // Custom Buffer Section
            SectionHeader(title = "Własny bufor kalibracyjny")

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(MaterialTheme.colorScheme.surface)
                    .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.3f), RoundedCornerShape(16.dp))
                    .padding(16.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    OutlinedTextField(
                        value = customBufferInput,
                        onValueChange = { customBufferInput = it },
                        label = { Text("Wartość pH") },
                        placeholder = { Text("np. 6.86") },
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Decimal),
                        modifier = Modifier.weight(1f),
                        singleLine = true,
                        shape = RoundedCornerShape(12.dp)
                    )

                    Button(
                        onClick = {
                            val parsed = customBufferInput.toFloatOrNull()
                            if (parsed != null && parsed in 0f..14f) {
                                viewModel.startCalibration("custom", parsed)
                            }
                        },
                        enabled = !isCalibrating && customBufferInput.toFloatOrNull()?.let { it in 0f..14f } == true,
                        shape = RoundedCornerShape(12.dp),
                        modifier = Modifier.height(56.dp)
                    ) {
                        Text("Kalibruj")
                    }
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            // Factory Reset Button
            OutlinedButton(
                onClick = { showResetDialog = true },
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.outlinedButtonColors(contentColor = Color(0xFFEF4444)),
                border = ButtonDefaults.outlinedButtonBorder.copy(brush = androidx.compose.ui.graphics.SolidColor(Color(0xFFEF4444).copy(alpha = 0.6f))),
                shape = RoundedCornerShape(12.dp),
                enabled = !isCalibrating
            ) {
                Icon(imageVector = Icons.Default.RestartAlt, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text("Resetuj do ustawień fabrycznych", fontWeight = FontWeight.SemiBold)
            }

            Spacer(modifier = Modifier.height(20.dp))
        }
    }

    if (showResetDialog) {
        AlertDialog(
            onDismissRequest = { showResetDialog = false },
            title = { Text("Potwierdzenie resetu") },
            text = { Text("Czy na pewno chcesz przywrócić fabryczne wartości napięć kalibracyjnych sondy?") },
            confirmButton = {
                Button(
                    onClick = {
                        showResetDialog = false
                        viewModel.startCalibration("reset")
                    },
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFFEF4444))
                ) {
                    Text("Resetuj")
                }
            },
            dismissButton = {
                TextButton(onClick = { showResetDialog = false }) {
                    Text("Anuluj")
                }
            }
        )
    }
}

@Composable
private fun BufferCard(
    title: String,
    phValue: String,
    voltage: Float,
    isCalibrated: Boolean,
    isBusy: Boolean,
    onCalibrate: () -> Unit
) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(16.dp))
            .background(MaterialTheme.colorScheme.surface)
            .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.3f), RoundedCornerShape(16.dp))
            .padding(14.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        text = title,
                        style = MaterialTheme.typography.titleSmall,
                        fontWeight = FontWeight.Bold,
                        color = MaterialTheme.colorScheme.onSurface
                    )
                }

                Spacer(modifier = Modifier.height(4.dp))

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        text = String.format(Locale.US, "Zapisane: %.3f V", voltage),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = if (isCalibrated) "• (skalibrowany)" else "• (domyślny)",
                        style = MaterialTheme.typography.bodySmall,
                        color = if (isCalibrated) Color(0xFF10B981) else Color(0xFF94A3B8),
                        fontWeight = FontWeight.Medium
                    )
                }
            }

            Button(
                onClick = onCalibrate,
                enabled = !isBusy,
                shape = RoundedCornerShape(10.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.primaryContainer,
                    contentColor = MaterialTheme.colorScheme.onPrimaryContainer
                )
            ) {
                Text("Kalibruj", fontWeight = FontWeight.Bold)
            }
        }
    }
}
