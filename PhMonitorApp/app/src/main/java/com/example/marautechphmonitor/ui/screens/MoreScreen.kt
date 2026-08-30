package com.example.marautechphmonitor.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.marautechphmonitor.ui.components.SectionHeader

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MoreScreen(
    onNavigateToSettings: () -> Unit,
    onNavigateToPushover: () -> Unit,
    onNavigateToMqtt: () -> Unit,
    onNavigateToDiagnostics: () -> Unit,
    onNavigateToConnect: () -> Unit,
    modifier: Modifier = Modifier
) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "Więcej Opcji",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold
                    )
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
            SectionHeader(title = "Konfiguracja Urządzenia")

            MoreItemCard(
                title = "Ustawienia Alarmów",
                subtitle = "Progi dolne, górne, histereza i powiadomienia",
                icon = Icons.Default.NotificationsActive,
                iconTint = Color(0xFFF59E0B),
                onClick = onNavigateToSettings
            )

            Spacer(modifier = Modifier.height(10.dp))

            MoreItemCard(
                title = "Powiadomienia Pushover",
                subtitle = "Konfiguracja tokenów i test wysyłki",
                icon = Icons.Default.Send,
                iconTint = Color(0xFF38BDF8),
                onClick = onNavigateToPushover
            )

            Spacer(modifier = Modifier.height(10.dp))

            MoreItemCard(
                title = "Integracja MQTT",
                subtitle = "Broker, Home Assistant Discovery i autoryzacja",
                icon = Icons.Default.CloudQueue,
                iconTint = Color(0xFF818CF8),
                onClick = onNavigateToMqtt
            )

            Spacer(modifier = Modifier.height(20.dp))

            SectionHeader(title = "Narzędzia i Połączenie")

            MoreItemCard(
                title = "Diagnostyka Systemu",
                subtitle = "Pamięć, WiFi RSSI, Uptime i kopiowanie logów",
                icon = Icons.Default.Assessment,
                iconTint = Color(0xFF10B981),
                onClick = onNavigateToDiagnostics
            )

            Spacer(modifier = Modifier.height(10.dp))

            MoreItemCard(
                title = "Połączenie z ESP32",
                subtitle = "Zmiana adresu IP, portu i logowanie administratora",
                icon = Icons.Default.Link,
                iconTint = Color(0xFFEC4899),
                onClick = onNavigateToConnect
            )

            Spacer(modifier = Modifier.height(24.dp))
        }
    }
}

@Composable
private fun MoreItemCard(
    title: String,
    subtitle: String,
    icon: ImageVector,
    iconTint: Color,
    onClick: () -> Unit
) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(16.dp))
            .background(MaterialTheme.colorScheme.surface)
            .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.3f), RoundedCornerShape(16.dp))
            .clickable(onClick = onClick)
            .padding(16.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .size(42.dp)
                    .clip(RoundedCornerShape(12.dp))
                    .background(iconTint.copy(alpha = 0.15f)),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = icon,
                    contentDescription = null,
                    tint = iconTint,
                    modifier = Modifier.size(22.dp)
                )
            }

            Spacer(modifier = Modifier.width(14.dp))

            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = title,
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.Bold,
                    color = MaterialTheme.colorScheme.onSurface
                )
                Spacer(modifier = Modifier.height(2.dp))
                Text(
                    text = subtitle,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }

            Icon(
                imageVector = Icons.Default.ChevronRight,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.5f)
            )
        }
    }
}
