package com.example.marautechphmonitor.ui.navigation

import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.navigation.NavGraph.Companion.findStartDestination
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.example.marautechphmonitor.ui.screens.*
import com.example.marautechphmonitor.ui.viewmodel.MainViewModel

sealed class Screen(val route: String, val title: String, val icon: ImageVector) {
    object Dashboard : Screen("dashboard", "Dashboard", Icons.Default.Dashboard)
    object History : Screen("history", "Historia", Icons.Default.ShowChart)
    object Calibration : Screen("calibration", "Kalibracja", Icons.Default.Tune)
    object More : Screen("more", "Więcej", Icons.Default.MoreHoriz)
    
    // Sub-screens
    object Settings : Screen("settings", "Ustawienia", Icons.Default.Settings)
    object Pushover : Screen("pushover", "Pushover", Icons.Default.Send)
    object Mqtt : Screen("mqtt", "MQTT", Icons.Default.CloudQueue)
    object Diagnostics : Screen("diagnostics", "Diagnostyka", Icons.Default.Assessment)
    object Connect : Screen("connect", "Połączenie", Icons.Default.Link)
}

@Composable
fun AppNavigation(
    viewModel: MainViewModel,
    modifier: Modifier = Modifier
) {
    val navController = rememberNavController()
    val navBackStackEntry by navController.currentBackStackEntryAsState()
    val currentRoute = navBackStackEntry?.destination?.route

    val bottomNavItems = listOf(
        Screen.Dashboard,
        Screen.History,
        Screen.Calibration,
        Screen.More
    )

    val showBottomBar = currentRoute in bottomNavItems.map { it.route }

    Scaffold(
        bottomBar = {
            if (showBottomBar) {
                NavigationBar(
                    containerColor = MaterialTheme.colorScheme.surface,
                    contentColor = MaterialTheme.colorScheme.onSurface
                ) {
                    bottomNavItems.forEach { screen ->
                        val selected = currentRoute == screen.route
                        NavigationBarItem(
                            selected = selected,
                            onClick = {
                                if (currentRoute != screen.route) {
                                    navController.navigate(screen.route) {
                                        popUpTo(navController.graph.findStartDestination().id) {
                                            saveState = true
                                        }
                                        launchSingleTop = true
                                        restoreState = true
                                    }
                                }
                            },
                            icon = { Icon(imageVector = screen.icon, contentDescription = screen.title) },
                            label = { Text(screen.title) },
                            colors = NavigationBarItemDefaults.colors(
                                selectedIconColor = MaterialTheme.colorScheme.primary,
                                selectedTextColor = MaterialTheme.colorScheme.primary,
                                indicatorColor = MaterialTheme.colorScheme.primaryContainer
                            )
                        )
                    }
                }
            }
        }
    ) { innerPadding ->
        NavHost(
            navController = navController,
            startDestination = Screen.Dashboard.route,
            modifier = modifier.padding(innerPadding)
        ) {
            composable(Screen.Dashboard.route) {
                DashboardScreen(
                    viewModel = viewModel,
                    onNavigateToCalibration = { navController.navigate(Screen.Calibration.route) },
                    onNavigateToHistory = { navController.navigate(Screen.History.route) },
                    onNavigateToConnect = { navController.navigate(Screen.Connect.route) }
                )
            }

            composable(Screen.History.route) {
                HistoryScreen(viewModel = viewModel)
            }

            composable(Screen.Calibration.route) {
                CalibrationScreen(viewModel = viewModel)
            }

            composable(Screen.More.route) {
                MoreScreen(
                    onNavigateToSettings = { navController.navigate(Screen.Settings.route) },
                    onNavigateToPushover = { navController.navigate(Screen.Pushover.route) },
                    onNavigateToMqtt = { navController.navigate(Screen.Mqtt.route) },
                    onNavigateToDiagnostics = { navController.navigate(Screen.Diagnostics.route) },
                    onNavigateToConnect = { navController.navigate(Screen.Connect.route) }
                )
            }

            // Sub-routes
            composable(Screen.Settings.route) {
                SettingsScreen(
                    viewModel = viewModel,
                    onNavigateBack = { navController.popBackStack() }
                )
            }

            composable(Screen.Pushover.route) {
                PushoverScreen(
                    viewModel = viewModel,
                    onNavigateBack = { navController.popBackStack() }
                )
            }

            composable(Screen.Mqtt.route) {
                MqttScreen(
                    viewModel = viewModel,
                    onNavigateBack = { navController.popBackStack() }
                )
            }

            composable(Screen.Diagnostics.route) {
                DiagnosticsScreen(
                    viewModel = viewModel,
                    onNavigateBack = { navController.popBackStack() }
                )
            }

            composable(Screen.Connect.route) {
                ConnectScreen(
                    viewModel = viewModel,
                    onNavigateToDashboard = {
                        navController.navigate(Screen.Dashboard.route) {
                            popUpTo(Screen.Dashboard.route) { inclusive = true }
                        }
                    },
                    onNavigateBack = if (navController.previousBackStackEntry != null) {
                        { navController.popBackStack() }
                    } else null
                )
            }
        }
    }
}
