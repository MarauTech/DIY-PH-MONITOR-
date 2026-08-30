package com.example.marautechphmonitor.widget

import android.app.Activity
import android.appwidget.AppWidgetManager
import android.content.Intent
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.marautechphmonitor.theme.MarauTechPHMonitorTheme

class WidgetConfigActivity : ComponentActivity() {

    private var appWidgetId = AppWidgetManager.INVALID_APPWIDGET_ID

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Set the result to CANCELED. This will cause the widget host to cancel
        // out of the widget placement if the user presses the back button.
        setResult(Activity.RESULT_CANCELED)

        // Find the widget id from the intent.
        intent?.extras?.let {
            appWidgetId = it.getInt(AppWidgetManager.EXTRA_APPWIDGET_ID, AppWidgetManager.INVALID_APPWIDGET_ID)
        }

        if (appWidgetId == AppWidgetManager.INVALID_APPWIDGET_ID) {
            finish()
            return
        }

        setContent {
            MarauTechPHMonitorTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    Column(
                        modifier = Modifier
                            .fillMaxSize()
                            .padding(24.dp),
                        verticalArrangement = Arrangement.Center,
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        Text(
                            text = "Dodaj widżet pH Monitor",
                            style = MaterialTheme.typography.headlineSmall,
                            color = MaterialTheme.colorScheme.onBackground
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = "Widżet będzie automatycznie aktualizowany po odświeżeniu danych w aplikacji oraz przez monitoring w tle.",
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        Spacer(modifier = Modifier.height(32.dp))
                        Button(
                            onClick = {
                                // Trigger immediate widget update
                                sendBroadcast(Intent("com.example.marautechphmonitor.ACTION_REFRESH_WIDGET"))

                                val resultValue = Intent().apply {
                                    putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, appWidgetId)
                                }
                                setResult(Activity.RESULT_OK, resultValue)
                                finish()
                            },
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Text("Zatwierdź i dodaj")
                        }
                    }
                }
            }
        }
    }
}
