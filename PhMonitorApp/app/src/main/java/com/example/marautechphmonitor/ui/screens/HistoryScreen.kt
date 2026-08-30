package com.example.marautechphmonitor.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.marautechphmonitor.ui.components.LineChart
import com.example.marautechphmonitor.ui.components.SectionHeader
import com.example.marautechphmonitor.ui.viewmodel.MainViewModel
import java.util.Locale

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HistoryScreen(
    viewModel: MainViewModel,
    modifier: Modifier = Modifier
) {
    val history by viewModel.history.collectAsState()
    val isLoading by viewModel.isHistoryLoading.collectAsState()
    val selectedRange by viewModel.selectedHistoryRange.collectAsState()
    val selectedMetric by viewModel.selectedHistoryMetric.collectAsState()

    LaunchedEffect(Unit) {
        viewModel.loadHistory()
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "Historia Pomiarów",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold
                    )
                },
                actions = {
                    IconButton(onClick = { viewModel.loadHistory() }) {
                        Icon(
                            imageVector = Icons.Default.Refresh,
                            contentDescription = "Odśwież historię",
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
            // Metric selector chips (pH, Temp, Voltage)
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                listOf("pH", "Temp", "Volt").forEach { metric ->
                    val isSelected = selectedMetric == metric
                    FilterChip(
                        selected = isSelected,
                        onClick = { viewModel.setHistoryMetric(metric) },
                        label = {
                            Text(
                                text = when (metric) {
                                    "pH" -> "Poziom pH"
                                    "Temp" -> "Temperatura"
                                    else -> "Napięcie"
                                },
                                fontWeight = if (isSelected) FontWeight.Bold else FontWeight.Normal
                            )
                        },
                        colors = FilterChipDefaults.filterChipColors(
                            selectedContainerColor = MaterialTheme.colorScheme.primaryContainer,
                            selectedLabelColor = MaterialTheme.colorScheme.onPrimaryContainer
                        )
                    )
                }
            }

            Spacer(modifier = Modifier.height(12.dp))

            // Time range selector chips
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                listOf("1h", "6h", "12h", "24h", "all").forEach { range ->
                    val isSelected = selectedRange == range
                    FilterChip(
                        selected = isSelected,
                        onClick = { viewModel.setHistoryRange(range) },
                        label = { Text(range.uppercase()) },
                        shape = RoundedCornerShape(20.dp)
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Filter points based on selected range
            val filteredPoints = remember(history, selectedRange) {
                if (history.isEmpty()) emptyList()
                else {
                    val count = when (selectedRange) {
                        "1h" -> 60
                        "6h" -> 360
                        "12h" -> 720
                        "24h" -> 1440
                        else -> history.size
                    }
                    history.takeLast(count)
                }
            }

            val values = remember(filteredPoints, selectedMetric) {
                filteredPoints.map {
                    when (selectedMetric) {
                        "pH" -> it.ph
                        "Temp" -> it.temperature ?: 0f
                        else -> (it.voltage ?: 0f) / 1000f
                    }
                }
            }

            val metricColor = when (selectedMetric) {
                "pH" -> Color(0xFF38BDF8)
                "Temp" -> Color(0xFFF97316)
                else -> Color(0xFF818CF8)
            }

            val metricUnit = when (selectedMetric) {
                "pH" -> "pH"
                "Temp" -> "°C"
                else -> "V"
            }

            // Interactive Chart
            LineChart(
                points = values,
                lineColor = metricColor,
                unit = metricUnit,
                modifier = Modifier.fillMaxWidth()
            )

            Spacer(modifier = Modifier.height(20.dp))

            // Summary Statistics Grid
            SectionHeader(title = "Podsumowanie okresu")

            if (values.isNotEmpty()) {
                val minVal = values.minOrNull() ?: 0f
                val maxVal = values.maxOrNull() ?: 0f
                val avgVal = values.average().toFloat()
                val lastVal = values.lastOrNull() ?: 0f

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    StatCard(
                        title = "MINIMUM",
                        value = String.format(Locale.US, "%.2f %s", minVal, metricUnit),
                        color = Color(0xFF38BDF8),
                        modifier = Modifier.weight(1f)
                    )
                    StatCard(
                        title = "MAKSIMUM",
                        value = String.format(Locale.US, "%.2f %s", maxVal, metricUnit),
                        color = Color(0xFFEC4899),
                        modifier = Modifier.weight(1f)
                    )
                }

                Spacer(modifier = Modifier.height(12.dp))

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    StatCard(
                        title = "ŚREDNIA",
                        value = String.format(Locale.US, "%.2f %s", avgVal, metricUnit),
                        color = Color(0xFF10B981),
                        modifier = Modifier.weight(1f)
                    )
                    StatCard(
                        title = "OSTATNI",
                        value = String.format(Locale.US, "%.2f %s", lastVal, metricUnit),
                        color = MaterialTheme.colorScheme.onSurface,
                        modifier = Modifier.weight(1f)
                    )
                }
            } else {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clip(RoundedCornerShape(12.dp))
                        .background(MaterialTheme.colorScheme.surface)
                        .padding(20.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = if (isLoading) "Pobieranie historii..." else "Brak zarejestrowanych punktów historii w pamięci urządzenia.",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            Spacer(modifier = Modifier.height(20.dp))
        }
    }
}

@Composable
private fun StatCard(
    title: String,
    value: String,
    color: Color,
    modifier: Modifier = Modifier
) {
    Box(
        modifier = modifier
            .clip(RoundedCornerShape(14.dp))
            .background(MaterialTheme.colorScheme.surface)
            .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.3f), RoundedCornerShape(14.dp))
            .padding(14.dp)
    ) {
        Column {
            Text(
                text = title,
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                fontWeight = FontWeight.SemiBold,
                letterSpacing = 0.5.sp
            )
            Spacer(modifier = Modifier.height(4.dp))
            Text(
                text = value,
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold,
                color = color
            )
        }
    }
}
