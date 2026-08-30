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
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.unit.dp
import com.example.marautechphmonitor.data.remote.ConfigRequest
import com.example.marautechphmonitor.ui.components.SectionHeader
import com.example.marautechphmonitor.ui.viewmodel.MainViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MqttScreen(
    viewModel: MainViewModel,
    onNavigateBack: () -> Unit,
    modifier: Modifier = Modifier
) {
    val config by viewModel.config.collectAsState()
    val isConfigLoading by viewModel.isConfigLoading.collectAsState()

    var enabled by remember { mutableStateOf(false) }
    var broker by remember { mutableStateOf("") }
    var port by remember { mutableStateOf("1883") }
    var user by remember { mutableStateOf("") }
    var pass by remember { mutableStateOf("") }

    LaunchedEffect(Unit) {
        viewModel.loadConfig()
    }

    LaunchedEffect(config) {
        config?.let {
            enabled = it.mqttEnabled
            broker = it.mqttBroker
            port = it.mqttPort.toString()
            user = it.mqttUser
        }
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "Integracja MQTT",
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
            SectionHeader(title = "Broker MQTT i Home Assistant")

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(16.dp))
                    .background(MaterialTheme.colorScheme.surface)
                    .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.3f), RoundedCornerShape(16.dp))
                    .padding(16.dp)
            ) {
                Column(verticalArrangement = Arrangement.spacedBy(14.dp)) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = "Włącz klienta MQTT",
                            style = MaterialTheme.typography.bodyMedium,
                            fontWeight = FontWeight.SemiBold
                        )
                        Switch(
                            checked = enabled,
                            onCheckedChange = { enabled = it }
                        )
                    }

                    OutlinedTextField(
                        value = broker,
                        onValueChange = { broker = it },
                        label = { Text("Adres brokera (IP lub domena)") },
                        placeholder = { Text("np. 192.168.1.100") },
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    OutlinedTextField(
                        value = port,
                        onValueChange = { port = it },
                        label = { Text("Port brokera") },
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    OutlinedTextField(
                        value = user,
                        onValueChange = { user = it },
                        label = { Text("Użytkownik (opcjonalnie)") },
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    OutlinedTextField(
                        value = pass,
                        onValueChange = { pass = it },
                        label = { Text("Hasło (opcjonalnie)") },
                        placeholder = { Text(if (config?.mqttConfigured == true) "•••••••• (zapisane)" else "Wpisz hasło") },
                        visualTransformation = PasswordVisualTransformation(),
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Password),
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    Text(
                        text = "Urządzenie automatycznie publikuje konfigurację Home Assistant MQTT Discovery oraz regularne stany encji.",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            Button(
                onClick = {
                    val p = port.toIntOrNull() ?: 1883
                    val req = ConfigRequest(
                        mqttEnabled = enabled,
                        mqttBroker = broker.trim(),
                        mqttPort = p,
                        mqttUser = user.trim(),
                        mqttPass = if (pass.isNotBlank()) pass else null
                    )
                    viewModel.saveConfig(req)
                    pass = ""
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(52.dp),
                shape = RoundedCornerShape(12.dp),
                enabled = !isConfigLoading
            ) {
                Icon(imageVector = Icons.Default.Save, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text("Zapisz MQTT", fontWeight = FontWeight.Bold)
            }
        }
    }
}
