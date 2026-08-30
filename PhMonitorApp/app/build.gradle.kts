plugins {
  alias(libs.plugins.android.application)
  alias(libs.plugins.compose.compiler)
  alias(libs.plugins.kotlin.serialization)
}

android {
    namespace = "com.example.marautechphmonitor"
    compileSdk = 36
    defaultConfig {
        applicationId = "com.example.marautechphmonitor"
        minSdk = 26
        targetSdk = 36
        versionCode = 2
        versionName = "2.0.0"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    signingConfigs {
        create("release") {
            // For development/demonstration, sign release build with debug keys if no custom keystore is supplied
            val debugKeystore = signingConfigs.getByName("debug")
            storeFile = debugKeystore.storeFile
            storePassword = debugKeystore.storePassword
            keyAlias = debugKeystore.keyAlias
            keyPassword = debugKeystore.keyPassword
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            signingConfig = signingConfigs.getByName("release")
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
        debug {
            applicationIdSuffix = ".debug"
            isDebuggable = true
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    buildFeatures {
      compose = true
      buildConfig = true
    }
}

kotlin {
    jvmToolchain(17)
}

dependencies {
  val composeBom = platform(libs.androidx.compose.bom)
  implementation(composeBom)
  androidTestImplementation(composeBom)

  implementation(libs.androidx.core.ktx)
  implementation(libs.androidx.lifecycle.runtime.ktx)
  implementation(libs.androidx.activity.compose)
  implementation(libs.androidx.lifecycle.runtime.compose)
  implementation(libs.androidx.lifecycle.viewmodel.compose)
  implementation(libs.androidx.compose.ui)
  implementation(libs.androidx.compose.ui.tooling.preview)
  implementation(libs.androidx.compose.material3)
  implementation("androidx.compose.material:material-icons-extended")
  debugImplementation(libs.androidx.compose.ui.tooling)
  
  implementation("androidx.navigation:navigation-compose:2.7.7")

  implementation("com.squareup.retrofit2:retrofit:2.11.0")
  implementation("com.squareup.retrofit2:converter-gson:2.11.0")
  implementation("com.squareup.okhttp3:okhttp:4.12.0")
  implementation("com.squareup.okhttp3:logging-interceptor:4.12.0")

  implementation("androidx.datastore:datastore-preferences:1.1.1")
  implementation("androidx.security:security-crypto-ktx:1.1.0-alpha06")

  implementation("androidx.work:work-runtime-ktx:2.9.0")

  testImplementation("junit:junit:4.13.2")
  testImplementation("com.squareup.okhttp3:mockwebserver:4.12.0")
  testImplementation("org.jetbrains.kotlinx:kotlinx-coroutines-test:1.8.0")
}
