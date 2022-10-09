package com.example.player

import android.Manifest
import android.content.pm.PackageManager
import android.os.*
import android.os.StrictMode.ThreadPolicy
import android.util.Log
import android.widget.*
import android.widget.Chronometer.OnChronometerTickListener
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import java.io.File
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.net.SocketTimeoutException
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.text.SimpleDateFormat
import java.util.*
import java.util.regex.Pattern


var timeCount : String = ""
var shortDataFromJNI : ShortArray = ShortArray(3072)
var ByteDataFromJNI : ByteArray = ByteArray(6144)
var udpSend : Boolean = true
var ip: InetAddress? = null
var datagramSock : DatagramSocket? = null
var ReadingDataIndex : Int = 0
var PlayingDataIndex : Int = 0

class MainActivity : AppCompatActivity() {

    private val AUDIO_RECORD_REQUEST = 12446
    private val PERMISSIONS = arrayOf(

        Manifest.permission.RECORD_AUDIO,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    )

    private val TAG = MainActivity::class.java.name
    private var isPlaying = false
    var fullPathToFile = ""


    override fun onCreate(savedInstanceState: Bundle?) {

        Log.d(TAG, "onCreate: ")

        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        getSupportActionBar()?.setTitle("Recorder and Player")
        //create object of class RingBuffer


        if(!isRecordPermissionGranted()){
            requestPermissions()
        }
        create()

        /*
        * This is simple recorder : Records First and then Plays that
        */
//        initUI()
        /*
        * This Records and Plays at the same time
        */
//        initUI_revertBack()

        /*
        * Records and saves in file .wav
        */
//        init_RecAndsaveInFile()

        /*
        * Revert Back app but sends data to other using udp protocol
        */

        initUi_PlayerOverUdpProtocol()

    }

    //This is the function called from JNI Side : gets the audio data
    fun getData(shortArr : ShortArray) {   // <=== We will call this
        Log.d(TAG,"getData()")

        //read buffer
        val s = shortArr.size
        Log.d(TAG, "[ $ReadingDataIndex ] Data Size : $s")
        ReadingDataIndex += 1

        //Warn: Uncoment below Line Logcat crash due to size fill
        //Log.d(TAG, "shortArr.indices : " + Arrays.toString(shortArr) )

        sendBroadcast(shortArr)
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

    //  by refrence
    external fun udpStartRec(): Boolean
    external fun udpStartPlaying(shortData : ShortArray): Boolean


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

            stoprecording()
        }

    }

    private fun stoprecording(){

        Thread(Runnable { stopRec()  }).start()
    }

    private fun startRec(pathToFile: String) {

        if(!isRecordPermissionGranted()){
            requestPermissions()
        }
        timeCount = startTimer()
        Toast.makeText(this, "Recording Started", Toast.LENGTH_SHORT).show()

        Thread(Runnable { recAndSaveInFile(pathToFile) }).start()

    }

    //returns the path to file ..
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


    /**
     * Used for sending data over the network
     *
     */
    private fun initUi_PlayerOverUdpProtocol(){
        Log.d(TAG,"initUi_PlayerOverUdpProtocol()")

        val udptransfer = findViewById<CheckBox>(R.id.udpOnOff)
        val btn_startRecording = findViewById<Button>(R.id.btn_startRec)

        btn_startRecording.setOnClickListener() {

            if(udptransfer.isChecked){
                Log.d(TAG,"Attempting to send Data On UDP")
                Toast.makeText(this, "Recording Started", Toast.LENGTH_SHORT).show()

//                Thread(Runnable { receiveBroadcast() }).start()
                udpStartRecording()

            }
            else{
                Log.d(TAG,"Attempting to receive Data On UDP")
//                receiveBroadcast()

                Thread(Runnable { receiveBroadcast() }).start()
            }

        }


        val btn_stopRecording = findViewById<Button>(R.id.btn_stopRec)
        btn_stopRecording.setOnClickListener(){
            udpSend = false
            stopTimer()
            Thread(Runnable { stopRec() }).start()
            Toast.makeText(this, "Recording Stopped", Toast.LENGTH_SHORT).show()
        }

    }

    private fun udpStartRecording(){
        Log.d(TAG,"udpStartRecording")

        startTimer()
        Thread(Runnable { udpStartRec() }).start()

    }

    private fun sendBroadcast(messageStr: ShortArray) {
        Log.d(TAG,"sendBroadcast()")

        val ipp = findViewById<EditText>(R.id.ipAdress).text.toString()
        val ip = InetAddress.getByName(ipp)
        val port = findViewById<EditText>(R.id.ipPort).text.toString().toInt()

        val policy = ThreadPolicy.Builder().permitAll().build()
        StrictMode.setThreadPolicy(policy)
        try {
            Log.d(TAG,"Attempting to create UDP Packet")
            val socket = DatagramSocket()
            val sendData = messageStr.toByteArray()

            val sendPacket =
                DatagramPacket(sendData, sendData.size, ip, port)

            val s = sendData.size
            Log.d(TAG, "[ $ReadingDataIndex ] sendData Size : $s")

            socket.send(sendPacket)
            Log.d(TAG,"Sent UDP Packet @ $ipp")

        } catch (e: Exception) {
            Log.e(TAG, "IOException: ")
            e.printStackTrace()
        }
    }

    private fun receiveBroadcast() {

        Log.d(TAG,"receiveBroadcast()")
        val ipp = findViewById<EditText>(R.id.ipAdress).text.toString()
        val ip = InetAddress.getByName(ipp)
        val port = findViewById<EditText>(R.id.ipPort).text.toString().toInt()

        val policy = ThreadPolicy.Builder().permitAll().build()
        StrictMode.setThreadPolicy(policy)

        try {
            Log.d(TAG,"Try : receiveBroadcast() ")
            
            socket.setSoTimeout(1000); //10 sec
            //create socket with ip and port
            while (udpSend) {

                Log.i(TAG, "Ready to receive broadcast packets!")
                val packet = DatagramPacket(ByteDataFromJNI, ByteDataFromJNI.size)
                
                socket.receive(packet)
                Log.i(TAG, "Packet received from : " + packet.address.hostAddress)

                val t = ByteDataFromJNI.size
                Log.d(TAG, "[ $PlayingDataIndex ] ByteDataFromJNI Size : $t")

                ByteBuffer.wrap(ByteDataFromJNI).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer()
                    .get(shortDataFromJNI)

                val s = shortDataFromJNI.size
                Log.d(TAG, "[ $PlayingDataIndex ] shortDataFromJNI Size : $s")
                PlayingDataIndex+=1

                Thread(Runnable { udpStartPlaying(shortDataFromJNI) }).start()
//                udpStartPlaying(shortDataFromJNI)

            }
        } catch (e: Exception) {
            Log.e(TAG, "IOException: ")
            e.printStackTrace()

        } catch (e: SocketTimeoutException) {
            // timeout exception.
            Log.d(TAG,"Timeout reached!!! $e")
        }
        Log.d(TAG,"Stopped : receiveBroadcast() ")
    }

}

private fun ShortArray.toByteArray(): ByteArray {
    //convert short array to byte array
    val bytes = ByteArray(size * 2)
    for (i in indices) {
        bytes[i * 2] = (this[i].toInt() and 0xFF).toByte()
        bytes[i * 2 + 1] = (this[i].toInt() shr 8 and 0xFF).toByte()
    }
    return bytes
}

