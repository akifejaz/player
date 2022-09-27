package com.example.player

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.SystemClock
import android.util.Log
import android.widget.Button
import android.widget.Chronometer
import android.widget.Chronometer.OnChronometerTickListener
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.player.databinding.ActivityMainBinding
import java.io.File
import java.text.SimpleDateFormat
import java.util.*

var timeCount : String = ""
val TAG = MainActivity::class.java.simpleName

class MainActivity : AppCompatActivity() {

    private val AUDIO_RECORD_REQUEST = 12446
    private val PERMISSIONS = arrayOf(

        Manifest.permission.RECORD_AUDIO,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    )

    private val TAG = MainActivity::class.java.name
    private var isPlaying = false
    private lateinit var binding: ActivityMainBinding
    var fullPathToFile = ""

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
//        initUI()
        initUI_revertBack()
//        init_RecAndsaveInFile()

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
    external fun revertBackPlay(): Boolean
    external fun stopRevertBack():Boolean
    external fun startPlayingRecordedStream()

    external fun recAndSaveInFile(path : String): Boolean
    external fun stopRec():Boolean

    companion object {

        init {
            System.loadLibrary("player")
        }
    }

    /**
     * Used for creating audio and then playing that
     *
     */
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

    /**
     * Used for revertBack audio
     *
     */
    private fun initUI_revertBack(){
        getSupportActionBar()?.setTitle("Revert Back Player")

        val btn_startRecording = findViewById<Button>(R.id.btn_startRec)
        btn_startRecording.setOnClickListener(){
            Toast.makeText(this, "Starting : Revert Back ", Toast.LENGTH_SHORT).show()
            timeCount = startTimer()
            startRevertBack()

           // Toast.makeText(this, "Recording & Player Started", Toast.LENGTH_SHORT).show()
        }

        val btn_stopRecording = findViewById<Button>(R.id.btn_stopRec)
        btn_stopRecording.setOnClickListener(){

            stopTimer()
            Thread(Runnable { stopRevertBack() }).start()
            Toast.makeText(this, "Recording Stopped", Toast.LENGTH_SHORT).show()
        }

    }

    private fun startRevertBack() {
        Log.d(TAG, "Attempting to start")

        var success: Boolean = false
        //start revert back and get return value using thread
        Thread(Runnable { success = revertBackPlay() }).start()
        if (success) {
            Toast.makeText(this, "Recording Started", Toast.LENGTH_SHORT).show()
            isPlaying = true
        } else {
            Toast.makeText(this, "Recording Start Failed ", Toast.LENGTH_SHORT).show()
            isPlaying = false
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

    /**
     * Used for savingToFile Input Data audio
     *
     */
    private fun init_RecAndsaveInFile() {
        Log.d(TAG," init_RecAndsaveInFile()")

        val dirPath : String = createDir()
        val timeStamp: String = SimpleDateFormat("yyyyMMdd_HHmmss", Locale("fr")).format(Date())
        // Full path
        fullPathToFile = Environment.getExternalStorageDirectory().path.toString() + "/Recordings/${timeStamp}_record.wav"
        Log.d(TAG,fullPathToFile)

        val btn_startRecording = findViewById<Button>(R.id.btn_startRec)
        btn_startRecording.setOnClickListener() {

            startRec(fullPathToFile)
        }

        val btn_stopRecording = findViewById<Button>(R.id.btn_stopRec)
        btn_stopRecording.setOnClickListener() {

            Thread(Runnable { stopRec() }).start()
        }

    }

    private fun startRec(pathToFile: String) {

        if(!isRecordPermissionGranted()){
            requestPermissions()
        }
        timeCount = startTimer()
        Toast.makeText(this, "Recording Started", Toast.LENGTH_SHORT).show()

        Thread(Runnable { recAndSaveInFile(pathToFile) }).start()

    }

    private fun createDir() : String {
        // Check if the Recorders ("/storage/emulated/0/Recorders/") directory exists, and if not then create it
        val folder = File(Environment.getExternalStorageDirectory().path.toString() + "/Recordings")
        if (folder.exists()) {
            if (folder.isDirectory) {
                // print out the absolute path to folder
                Log.d(TAG, "init_RecAndsaveInFile: " + folder.absolutePath)

            } else {
                // Create the Recorders directory
                folder.mkdir()
                Log.d(TAG, "init_RecAndsaveInFile: " + folder.absolutePath)
            }
        } else {
            // Create the Recorders directory
            folder.mkdir()
            Log.d(TAG, "init_RecAndsaveInFile: " + folder.absolutePath)
        }

        return folder.absolutePath
    }


}