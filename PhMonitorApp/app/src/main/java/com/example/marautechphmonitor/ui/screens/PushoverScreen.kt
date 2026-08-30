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
import androidx.compose.material.icons.filled.Send
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.unit.dp
import com.example.marautechphmonitor.data.remote.ConfigRequest
import com.example.marautechphmonitor.ui.components.SectionHeader
import com.example.marautechphmonitor.ui.viewmodel.MainViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun PushoverScreen(
    viewModel: MainViewModel,
    onNavigateBack: () -> Unit,
    modifier: Modifier = Modifier
) {
    val config by viewModel.config.collectAsState()
    val isConfigLoading by viewModel.isConfigLoading.collectAsState()

    var enabled by remember { mutableStateOf(false) }
    var userKey by remember { mutableStateOf("") }
    var apiToken by remember { mutableStateOf("") }

    var testStatus by remember { mutableStateOf<Pair<Boolean, String>?>(null) }
    var isTesting by remember { mutableStateOf(false) }

    LaunchedEffect(Unit) {
        viewModel.loadConfig()
    }

    LaunchedEffect(config) {
        config?.let {
            enabled = it.pushoverEnabled
        }
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = "Powiadomienia Pushover",
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
            SectionHeader(title = "Konfiguracja Pushover API")

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
                            text = "Włącz powiadomienia Pushover",
                            style = MaterialTheme.typography.bodyMedium,
                            fontWeight = FontWeight.SemiBold
                        )
                        Switch(
                            checked = enabled,
                            onCheckedChange = { enabled = it }
                        )
                    }

                    OutlinedTextField(
                        value = userKey,
                        onValueChange = { userKey = it },
                        label = { Text("Klucz Użytkownika (User Key)") },
                        placeholder = { Text(if (config?.pushoverConfigured == true) "•••••••• (skonfigurowany)" else "Wpisz klucz") },
                        visualTransformation = PasswordVisualTransformation(),
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Password),
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    OutlinedTextField(
                        value = apiToken,
                        onValueChange = { apiToken = it },
                        label = { Text("Token Aplikacji (API Token)") },
                        placeholder = { Text(if (config?.pushoverConfigured == true) "•••••••• (skonfigurowany)" else "Wpisz token") },
                        visualTransformation = PasswordVisualTransformation(),
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Password),
                        modifier = Modifier.fillMaxWidth(),
                        shape = RoundedCornerShape(12.dp),
                        singleLine = true
                    )

                    Text(
                        text = "Ze względów bezpieczeństwa urządzenie nie zwraca zapisanych kluczy. Pozostaw pola puste, aby zachować obecne tokeny.",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Test Button
            OutlinedButton(
                onClick = {
                    isTesting = true
                    testStatus = null
                    viewModel.testPushover { success, msg ->
                        isTesting = false
                        testStatus = Pair(success, msg)
                    }
                },
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(12.dp),
                enabled = !isTesting
            ) {
                if (isTesting) {
                    CircularProgressIndicator(modifier = Modifier.size(18.dp), strokeWidth = 2.dp)
                } else {
                    Icon(imageVector = Icons.Default.Send, contentDescription = null, modifier = Modifier.size(18.dp))
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Wyślij testowe powiadomienie")
                }
            }

            testStatus?.let { (success, msg) ->
                Spacer(modifier = Modifier.height(10.dp))
                Text(
                    text = msg,
                    style = MaterialTheme.typography.bodySmall,
                    color = if (success) Color(0xFF10B981) else Color(0xFFEF4444),
                    fontWeight = FontWeight.SemiBold
                )
            }

            Spacer(modifier = Modifier.height(24.dp))

            Button(
                onClick = {
                    val req = ConfigRequest(
                        pushoverEnabled = enabled,
                        pushoverUser = if (userKey.isNotBlank()) userKey else null,
                        pushoverToken = if (apiToken.isNotBlank()) apiToken else null
                    )
                    viewModel.saveConfig(req)
                    userKey = ""
                    apiToken = ""
                },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(52.dp),
                shape = RoundedCornerShape(12.dp),
                enabled = !isConfigLoading
            ) {
                Icon(imageVector = Icons.Default.Save, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text("Zapisz Pushover", fontWeight = FontWeight.Bold)
            }
        }
    }
}
