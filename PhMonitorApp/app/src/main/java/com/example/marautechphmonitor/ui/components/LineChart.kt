package com.example.marautechphmonitor.ui.components

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import java.util.Locale
import kotlin.math.max
import kotlin.math.min

@Composable
fun LineChart(
    points: List<Float>,
    lineColor: Color = MaterialTheme.colorScheme.primary,
    fillColor: Color = lineColor.copy(alpha = 0.25f),
    unit: String = "",
    minLimit: Float? = null,
    maxLimit: Float? = null,
    modifier: Modifier = Modifier
) {
    var selectedIndex by remember { mutableStateOf<Int?>(null) }

    Box(
        modifier = modifier
            .fillMaxWidth()
            .height(220.dp)
            .clip(RoundedCornerShape(16.dp))
            .background(MaterialTheme.colorScheme.surface)
            .border(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = 0.4f), RoundedCornerShape(16.dp))
            .padding(12.dp)
    ) {
        if (points.isEmpty()) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = "Brak danych pomiarowych",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            return@Box
        }

        val rawMin = points.minOrNull() ?: 0f
        val rawMax = points.maxOrNull() ?: 14f

        val effectiveMin = if (minLimit != null) min(rawMin, minLimit) else rawMin
        val effectiveMax = if (maxLimit != null) max(rawMax, maxLimit) else rawMax

        val padding = if (effectiveMax - effectiveMin < 0.1f) 0.5f else (effectiveMax - effectiveMin) * 0.15f
        val minY = effectiveMin - padding
        val maxY = effectiveMax + padding
        val rangeY = if (maxY - minY == 0f) 1f else maxY - minY

        val outlineColor = MaterialTheme.colorScheme.outline.copy(alpha = 0.25f)
        val surfaceColor = MaterialTheme.colorScheme.surface

        Canvas(
            modifier = Modifier
                .fillMaxSize()
                .pointerInput(points) {
                    detectTapGestures { offset ->
                        if (points.size > 1) {
                            val stepX = size.width / (points.size - 1)
                            val idx = (offset.x / stepX).toInt().coerceIn(0, points.size - 1)
                            selectedIndex = idx
                        }
                    }
                }
        ) {
            val width = size.width
            val height = size.height

            // 1. Draw Grid Lines (3 horizontal lines)
            val gridLines = 4
            for (i in 0..gridLines) {
                val y = height * i / gridLines
                drawLine(
                    color = outlineColor,
                    start = Offset(0f, y),
                    end = Offset(width, y),
                    strokeWidth = 1.dp.toPx()
                )
            }

            if (points.size == 1) {
                val y = height - ((points[0] - minY) / rangeY * height)
                drawCircle(
                    color = lineColor,
                    radius = 6.dp.toPx(),
                    center = Offset(width / 2, y)
                )
                return@Canvas
            }

            // 2. Build Line Path & Gradient Fill Path
            val stepX = width / (points.size - 1)
            val strokePath = Path()
            val fillPath = Path()

            val firstY = height - ((points[0] - minY) / rangeY * height)
            strokePath.moveTo(0f, firstY)
            fillPath.moveTo(0f, height)
            fillPath.lineTo(0f, firstY)

            for (i in 1 until points.size) {
                val x = i * stepX
                val y = height - ((points[i] - minY) / rangeY * height)
                strokePath.lineTo(x, y)
                fillPath.lineTo(x, y)
            }

            fillPath.lineTo(width, height)
            fillPath.close()

            // Draw Fill Gradient
            drawPath(
                path = fillPath,
                brush = Brush.verticalGradient(
                    colors = listOf(fillColor, Color.Transparent),
                    startY = 0f,
                    endY = height
                )
            )

            // Draw Line Stroke
            drawPath(
                path = strokePath,
                color = lineColor,
                style = Stroke(
                    width = 3.dp.toPx(),
                    cap = StrokeCap.Round
                )
            )

            // Draw Selected Point Highlight
            selectedIndex?.let { idx ->
                if (idx in points.indices) {
                    val px = idx * stepX
                    val py = height - ((points[idx] - minY) / rangeY * height)

                    drawLine(
                        color = lineColor.copy(alpha = 0.5f),
                        start = Offset(px, 0f),
                        end = Offset(px, height),
                        strokeWidth = 1.5.dp.toPx()
                    )

                    drawCircle(
                        color = surfaceColor,
                        radius = 7.dp.toPx(),
                        center = Offset(px, py)
                    )
                    drawCircle(
                        color = lineColor,
                        radius = 5.dp.toPx(),
                        center = Offset(px, py)
                    )
                }
            }
        }

        // Top labels: Min & Max
        Box(modifier = Modifier.fillMaxSize()) {
            Text(
                text = String.format(Locale.US, "Max: %.2f %s", effectiveMax, unit),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.8f),
                fontSize = 10.sp,
                modifier = Modifier.align(Alignment.TopStart)
            )

            Text(
                text = String.format(Locale.US, "Min: %.2f %s", effectiveMin, unit),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.8f),
                fontSize = 10.sp,
                modifier = Modifier.align(Alignment.BottomStart)
            )

            selectedIndex?.let { idx ->
                if (idx in points.indices) {
                    Text(
                        text = "Punkt $idx: ${String.format(Locale.US, "%.2f %s", points[idx], unit)}",
                        style = MaterialTheme.typography.labelSmall,
                        color = MaterialTheme.colorScheme.primary,
                        fontSize = 11.sp,
                        modifier = Modifier.align(Alignment.TopEnd)
                    )
                }
            }
        }
    }
}
