package com.example.player

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.SystemClock
import android.util.Log
import android.widget.Button
import android.widget.Chronometer
import android.widget.Chronometer.OnChronometerTickListener
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.player.databinding.ActivityMainBinding

var timeCount : String = ""
val TAG = MainActivity::class.java.simpleName

class MainActivity : AppCompatActivity() {

    private val AUDIO_RECORD_REQUEST = 12446
    private val PERMISSIONS = arrayOf(
        Manifest.permission.RECORD_AUDIO,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    )

    private lateinit var binding: ActivityMainBinding
    override fun onCreate(savedInstanceState: Bundle?) {

        Log.d("Currunt: ", "onCreate: ")

        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        var req : Boolean = false
        //-----
        getSupportActionBar()?.setTitle("Recorder and Player")

        if(!isRecordPermissionGranted()){
            requestPermissions()
        }

        create()
        initUI()
        // Example of a call to a native method
//        val str = stringFromJNI()
//        Log.i("Return String JNI",str)
    }
    /**
     * A native method that is implemented by the 'player' native library,
     * which is packaged with this application.
     */

    external fun stringFromJNI(): String
    // Native methods
    external fun create(): Boolean
    external fun startRecording()
    external fun stopRecording()
    external fun startPlayingRecordedStream()

    companion object {

        init {
            System.loadLibrary("player")
        }
    }

    private fun initUI() {

        Log.d("Kotlin : MainActivity:", "initUI: ")

        val btn_startRecording = findViewById<Button>(R.id.btn_startRec)
        btn_startRecording.setOnClickListener(){
            //Log.i("RecStarted :", "True")

            timeCount = startTimer()
            startRecording()
            Toast.makeText(this, "Recording Started", Toast.LENGTH_SHORT).show()
        }

        val btn_stopRecording = findViewById<Button>(R.id.btn_stopRec)
        btn_stopRecording.setOnClickListener() {

            stopTimer()
            stopRecording()

            //Toast.makeText(this, "Recording Sttoped", Toast.LENGTH_SHORT).show()

            //playing at same time
            Toast.makeText(this, "Playing Recording: Record Stream ", Toast.LENGTH_SHORT).show()
            startBackTimer()
            startPlayingRecordedStream()
        }
    }

    private fun stopTimer() : String {

        val chronometer = findViewById<Chronometer>(R.id.simpleTimer)

        //return back time in string
        timeCount = chronometer.text.toString()
        chronometer.stop()

        return timeCount
    }

    private fun startTimer() : String {

        val timer = findViewById<Chronometer>(R.id.simpleTimer)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            timer.isCountDown = false
        }
        timer.base = SystemClock.elapsedRealtime()
        timer.start()

       return timer.text.toString()
    }

    private fun startBackTimer() {

        val timer = findViewById<Chronometer>(R.id.simpleTimer)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            timer.isCountDown = true
        }
        
        //previous time in miliseconds
        val time = timeCount.split(":")
        val min = time[0].toLong()
        val sec = time[1].toLong()

        val timeInMilisec : Long = (min * 60 * 1000) + (sec * 1000)
        timer.base = SystemClock.elapsedRealtime() + timeInMilisec
        timer.start()

        timer
            .setOnChronometerTickListener(OnChronometerTickListener { chronometer ->
                if (timer.text.toString().equals("00:00", ignoreCase = true))
                    timer.stop()
            }
        )
    }

    private fun isRecordPermissionGranted(): Boolean {

        val permissionStatus = (ActivityCompat
            .checkSelfPermission(
                this,
                Manifest.permission.RECORD_AUDIO
            ) == PackageManager.PERMISSION_GRANTED) &&
                (ActivityCompat
                    .checkSelfPermission(
                        this,
                        Manifest.permission.READ_EXTERNAL_STORAGE
                    ) == PackageManager.PERMISSION_GRANTED) &&
                (ActivityCompat
                    .checkSelfPermission(
                        this,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE
                    ) == PackageManager.PERMISSION_GRANTED)

        Log.d(TAG, "isRecordPermissionGranted: $permissionStatus")

        return permissionStatus
    }

    private fun requestPermissions(): Boolean {

        Log.d(TAG, "requestRecordPermission: ")
        ActivityCompat.requestPermissions(this, PERMISSIONS, AUDIO_RECORD_REQUEST)
        //request for external write permission

        if (isRecordPermissionGranted()){
            return true
        }

        return false
    }

}