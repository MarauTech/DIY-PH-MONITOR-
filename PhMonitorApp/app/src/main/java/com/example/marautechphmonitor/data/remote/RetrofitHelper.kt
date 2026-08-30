package com.example.marautechphmonitor.data.remote

import okhttp3.Credentials
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.util.concurrent.TimeUnit

object RetrofitHelper {
    private var retrofit: Retrofit? = null
    private var currentIp: String? = null
    private var currentUser: String? = null
    private var currentPass: String? = null

    fun getApi(ip: String, user: String?, pass: String?): PhMonitorApi {
        if (retrofit != null && ip == currentIp && user == currentUser && pass == currentPass) {
            return retrofit!!.create(PhMonitorApi::class.java)
        }

        val logger = HttpLoggingInterceptor().apply {
            level = HttpLoggingInterceptor.Level.BASIC
        }

        val clientBuilder = OkHttpClient.Builder()
            .addInterceptor(logger)
            .connectTimeout(5, TimeUnit.SECONDS)
            .readTimeout(5, TimeUnit.SECONDS)

        if (!user.isNullOrEmpty() && !pass.isNullOrEmpty()) {
            clientBuilder.addInterceptor { chain ->
                val request = chain.request().newBuilder()
                    .header("Authorization", Credentials.basic(user, pass))
                    .build()
                chain.proceed(request)
            }
        }

        val baseUrl = if (ip.startsWith("http")) {
            if (ip.endsWith("/")) ip else "$ip/"
        } else {
            "http://$ip/"
        }

        retrofit = Retrofit.Builder()
            .baseUrl(baseUrl)
            .client(clientBuilder.build())
            .addConverterFactory(GsonConverterFactory.create())
            .build()

        currentIp = ip
        currentUser = user
        currentPass = pass

        return retrofit!!.create(PhMonitorApi::class.java)
    }
}
