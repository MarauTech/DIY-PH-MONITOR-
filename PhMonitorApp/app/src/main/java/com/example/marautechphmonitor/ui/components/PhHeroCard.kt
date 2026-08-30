package com.example.marautechphmonitor.ui.components

import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.WaterDrop
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.marautechphmonitor.model.AlarmState
import java.util.Locale

@Composable
fun PhHeroCard(
    ph: Float,
    voltage: Float,
    alarmState: AlarmState,
    minPh: Float? = null,
    maxPh: Float? = null,
    modifier: Modifier = Modifier
) {
    val animatedBorderColor by animateColorAsState(
        targetValue = alarmState.borderColor,
        animationSpec = tween(durationMillis = 600),
        label = "borderColor"
    )

    val animatedBgColor by animateColorAsState(
        targetValue = alarmState.backgroundColor,
        animationSpec = tween(durationMillis = 600),
        label = "bgColor"
    )

    Box(
        modifier = modifier
            .clip(RoundedCornerShape(24.dp))
            .background(
                brush = Brush.verticalGradient(
                    colors = listOf(
                        MaterialTheme.colorScheme.surface,
                        animatedBgColor.copy(alpha = 0.4f)
                    )
                )
            )
            .border(1.5.dp, animatedBorderColor.copy(alpha = 0.8f), RoundedCornerShape(24.dp))
            .padding(20.dp)
    ) {
        Column {
            // Header Row
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        imageVector = Icons.Default.WaterDrop,
                        contentDescription = null,
                        tint = MaterialTheme.colorScheme.primary,
                        modifier = Modifier.padding(end = 6.dp)
                    )
                    Text(
                        text = "AKTUALNY POZIOM pH",
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        fontWeight = FontWeight.Bold,
                        letterSpacing = 1.sp
                    )
                }

                StatusBadge(state = alarmState)
            }

            Spacer(modifier = Modifier.height(14.dp))

            // Hero Value & Unit
            Row(
                verticalAlignment = Alignment.Bottom
            ) {
                Text(
                    text = String.format(Locale.US, "%.2f", ph),
                    fontSize = 52.sp,
                    fontWeight = FontWeight.Black,
                    color = MaterialTheme.colorScheme.onSurface,
                    letterSpacing = (-1).sp
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = "pH",
                    fontSize = 22.sp,
                    fontWeight = FontWeight.Bold,
                    color = MaterialTheme.colorScheme.primary,
                    modifier = Modifier.padding(bottom = 8.dp)
                )
            }

            Text(
                text = alarmState.description,
                style = MaterialTheme.typography.bodyMedium,
                color = alarmState.color,
                fontWeight = FontWeight.SemiBold
            )

            Spacer(modifier = Modifier.height(14.dp))

            // Footer info: Voltage & Min/Max
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(12.dp))
                    .background(MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.6f))
                    .padding(horizontal = 12.dp, vertical = 8.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Sonda: ${String.format(Locale.US, "%.3f V", voltage)}",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    fontWeight = FontWeight.Medium
                )

                if (minPh != null && maxPh != null) {
                    Text(
                        text = "Min: ${String.format(Locale.US, "%.2f", minPh)} | Max: ${String.format(Locale.US, "%.2f", maxPh)}",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        fontWeight = FontWeight.Medium
                    )
                }
            }
        }
    }
}
