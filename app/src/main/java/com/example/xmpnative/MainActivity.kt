package com.example.xmpnative

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    companion object {
        const val STORAGE_PERMISSION_REQUEST_CODE = 101
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        requestPermissionOf(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
    }

    private fun requestPermissionOf(permission: String) {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            if (this.checkSelfPermission(permission) == PackageManager.PERMISSION_GRANTED && this.checkSelfPermission(
                    Manifest.permission.READ_EXTERNAL_STORAGE
                ) == PackageManager.PERMISSION_GRANTED
            ) {
                startLoad()
            } else this.requestPermissions(
                arrayOf(permission, Manifest.permission.READ_EXTERNAL_STORAGE),
                STORAGE_PERMISSION_REQUEST_CODE,
            )
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == STORAGE_PERMISSION_REQUEST_CODE) {
            if (grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
                startLoad()
            }
        }
    }

    private fun startLoad() {
        val path = "/sdcard/cloud/ytk200607100/3f/2a/3f2a632ab8951814120c51c746dc93f5.pdf"
        findViewById<TextView>(R.id.sample_text).text = "start loading"
        XmpNative.getMetaData(path)
    }

/*
    */
    /**
     * A native method that is implemented by the 'xmpmark' native library,
     * which is packaged with this application.
     *//*
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'xmpmark' library on application startup.
        init {
            System.loadLibrary("xmpmark")
        }
    }
    */
}