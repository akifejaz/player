#include <jni.h>
#include <string>
#include "logging_macros.h"
#include "vector"

#include "AudioEngine.h"
#include "recordingAndSaving.h"
#include "ringBuffer.h"
#include <oboe/Oboe.h>


const char *TAG = "native-lib.cpp:: %s";
static AudioEngine *audioEngine = nullptr;
static bool stopStatus = false;
//oboe::AudioStreamBuilder builder;
oboe::AudioStream *stream = nullptr;
oboe::AudioStream *stream2 = nullptr;
static int frameReadSeq = 0;
static int frameWriteSeq = 0;
bool startRecordingStream(oboe::AudioStreamBuilder &builder);

bool startPlayingStream(oboe::AudioStreamBuilder &builder);

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_create(JNIEnv *env, jobject thiz) {

    LOGD(TAG, "create(): ");

    if (audioEngine == nullptr) {
        audioEngine = new AudioEngine();
    }

    return (audioEngine != nullptr);

}

bool startRecordingStream(oboe::AudioStreamBuilder &builder) {

    //set stream parameters for recording
    builder.setDirection(oboe::Direction::Input);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

    //create stream for recording
    oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) {
        LOGE("Error opening stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_player_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {

    std::string hello = "Simple Recorder App";
    return env->NewStringUTF(hello.c_str());
}

bool startPlayingStream(oboe::AudioStreamBuilder &builder) {

    //set stream parameters for playing
    builder.setDirection(oboe::Direction::Output);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

    //create stream for playing
    oboe::Result result = builder.openStream(&stream2);
    if (result != oboe::Result::OK) {
        LOGE("Error opening stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_startRecording(JNIEnv *env, jobject thiz) {

    if (audioEngine == nullptr) {
        LOGE(TAG, "audioEngine is null, you must call create() method before calling this method");
        return;
    }
    audioEngine->startRecording();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_stopRecording(JNIEnv *env, jobject thiz) {

    LOGD(TAG, "stopRecording(): ");

    if (audioEngine == nullptr) {
        LOGE(TAG, "audioEngine is null, you must call create() method before calling this method");
        return;
    }

    audioEngine->stopRecording();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_startPlayingRecordedStream(
        JNIEnv *env, jobject thiz) {

    LOGD(TAG, "startPlayingRecordedStream(): ");

    if (audioEngine == nullptr) {
        LOGE(TAG, "audioEngine is null, you must call create() method before calling this method");
        return;
    }

    audioEngine->startPlayingRecordedStream();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_revertBackPlay(JNIEnv *env, jobject thiz) {

    //use oboe to record audio and save in buffer and then play at same time without callback
    oboe::AudioStreamBuilder builder;
    oboe::AudioStream *stream = nullptr;
    oboe::AudioStream *stream2 = nullptr;

//    startRecordingStream(builder);
//    startPlayingStream(builder);

    //set stream parameters for recording
    builder.setDirection(oboe::Direction::Input);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

    //create stream for recording
    oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) {
        LOGE("Error opening stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //set stream parameters for playing
    builder.setDirection(oboe::Direction::Output);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

    //create stream for playing
    result = builder.openStream(&stream2);
    if (result != oboe::Result::OK) {
        LOGE("Error opening stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //create buffer to hold
    int32_t bufferSize = stream->getBufferSizeInFrames();
    int32_t bufferCapacity = bufferSize * stream->getChannelCount();

    auto *buffer = new float[bufferCapacity];

    //start recording  stream
    result = stream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Error starting stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //start playing stream
    result = stream2->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Error starting stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }
    
    //loop to read and write audio
    oboe::Result r,w;
    do {
        //read audio from recording stream
        auto r = stream->read(buffer, bufferSize, 0);
        if (r!= oboe::Result::OK) {
            LOGE("Error reading stream: %s", "oboe::convertToText(r)");
            return JNI_FALSE;
        }
        int framesRead = r.value();
//        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "framesRead = %d seqanceNo = %d", framesRead);

//        for (int i = 0; i < framesRead; i++){
//            __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder","nbFramesRead[%d] = %f", i, buffer[i]);
//        }

        //convert rbuffer to string and print
        std::string str = "\nSeqNo = [" + std::to_string(frameReadSeq) + + "] "; frameReadSeq++;

        for(int i=0; i<framesRead; i++){
            str += ", " + std::to_string(buffer[i]);
        }
        LOGD("rbuffer = %s", str.c_str());

        //write audio to playing stream
        w = stream2->write(buffer, bufferSize, 0);
        if (w!= oboe::Result::OK) {
            LOGE("Error writing stream: %s", oboe::convertToText(w));
            return JNI_FALSE;
        }

    } while (!stopStatus);

    LOGD(TAG,"Attempting to stop");
    stream->requestStop();
    //stream->close();

    LOGD(TAG,"Attempting to stop 1");
    stream2->requestStop();
    //stream2->close();
    delete[] buffer;
    frameReadSeq = 0;

    return JNI_TRUE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_stopRevertBack(JNIEnv *env, jobject thiz) {

    stopStatus = true;
    LOGD(TAG,"Attempting to stop");
    return true;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_recAndSaveInFile(JNIEnv *env, jobject thiz, jstring path) {

    //find class of MainActivity
    jclass cls = env->GetObjectClass(thiz);
    jmethodID mid = env->GetMethodID(cls, "getData", "([S)V");

    const char *pathToFile = (*env).GetStringUTFChars(path, 0);
    static auto a = OboeAudioRecorder::get();
    a->StartAudioRecorder(pathToFile, 48000, env, thiz, mid);

}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_stopRec(JNIEnv *env, jobject thiz) {

    static auto a = OboeAudioRecorder::get();
    a->StopAudioRecorder();
    return true;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_udpStartRec(JNIEnv *env,
        jobject thiz) {

    LOGD(TAG,"MainActivity_udpStartRec()");

    //find class of MainActivity
    jclass cls = env->GetObjectClass(thiz);
    jmethodID mid = env->GetMethodID(cls, "getData", "([S)V");

    /* Working But Other Method
    static auto a = OboeAudioRecorder::get();
    int16_t *mybuffer = 0;
    a->UdpStartAudioRecorder(48000, mybuffer, env, thiz, mid);
    */


    //use oboe to record audio and save in buffer and then play at same time without callback
    oboe::AudioStreamBuilder builder;
    oboe::AudioStream *stream = nullptr;
    oboe::AudioStream *stream2 = nullptr;

    //set stream parameters for recording
    builder.setDirection(oboe::Direction::Input);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

    //create stream for recording
    oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) {
        LOGE("Error opening stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //set stream parameters for playing
    builder.setDirection(oboe::Direction::Output);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

    //create stream for playing
    result = builder.openStream(&stream2);
    if (result != oboe::Result::OK) {
        LOGE("Error opening stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //create buffer to hold
    int32_t bufferSize = stream->getBufferSizeInFrames();
    int32_t bufferCapacity = bufferSize * stream->getChannelCount();

    auto *buffer = new float[bufferCapacity];
    jshortArray jShortArray = env->NewShortArray(bufferCapacity);

    //start recording  stream
    result = stream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Error starting stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //start playing stream
//    result = stream2->requestStart();
//    if (result != oboe::Result::OK) {
//        LOGE("Error starting stream: %s", oboe::convertToText(result));
//        return JNI_FALSE;
//    }

    //loop to read and write audio
    oboe::Result r,w;
    do {
        //read audio from recording stream
        auto r = stream->read(buffer, bufferSize, 0);
        if (r!= oboe::Result::OK) {
            LOGE("Error reading stream: %s", "oboe::convertToText(r)");
            return JNI_FALSE;
        }
        int framesRead = r.value();
//        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "framesRead = %d seqanceNo = %d", framesRead);

//        for (int i = 0; i < framesRead; i++){
//            __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder","nbFramesRead[%d] = %f", i, buffer[i]);
//        }

        //convert rbuffer to string and print
//        std::string str = "\nSeqNo = [" + std::to_string(frameReadSeq) + + "] "; frameReadSeq++;
//
//        for(int i=0; i<framesRead; i++){
//            str += ", " + std::to_string(buffer[i]);
//        }
//        LOGD("rbuffer = %s", str.c_str());

        // ----------
        env->SetShortArrayRegion(jShortArray, 0, framesRead,
                                 reinterpret_cast<const jshort *>(buffer));
        env->CallVoidMethod(thiz, mid, jShortArray);
        // ----------

        //write audio to playing stream
//        w = stream2->write(buffer, bufferSize, 0);
//        if (w!= oboe::Result::OK) {
//            LOGE("Error writing stream: %s", oboe::convertToText(w));
//            return JNI_FALSE;
//        }

    } while (!stopStatus);

    LOGD(TAG,"Attempting to stop");
    stream->requestStop();
    //stream->close();

    LOGD(TAG,"Attempting to stop 1");
//    stream2->requestStop();
    //stream2->close();
    delete[] buffer;
    frameReadSeq = 0;

    return JNI_TRUE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_udpStartPlaying(JNIEnv *env, jobject thiz,
        jshortArray short_data) {

    LOGD(TAG,"JNI udpStartPlaying()"); //192.168.0.110

    //access the short array
    jshort *data = env->GetShortArrayElements(short_data, 0);
    jsize len = env->GetArrayLength(short_data);

    auto *rbuffer = new int16_t[len];
    //copy data of short array to rbuffer
    for(int i=0; i<len; i++){
        rbuffer[i] = data[i];
    }
//
    //convert rbuffer to string and print
    std::string str = std::to_string(rbuffer[0]);
    for(int i=1; i<len; i++){
        str += ", " + std::to_string(rbuffer[i]);
    }
    LOGD("JNI udpStartPlaying() rbuffer = %s", str.c_str());

    env->ReleaseShortArrayElements(short_data, data, 0);

/*
    //trying to play the audio data from udp
    //use oboe to record audio and save in buffer and then play at same time without callback
    oboe::AudioStreamBuilder builder;
    oboe::AudioStream *stream = nullptr;
    oboe::AudioStream *stream2 = nullptr;

    //set stream parameters for recording
    builder.setDirection(oboe::Direction::Input);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

    //create stream for recording
    oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) {
        LOGE("Error opening stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //set stream parameters for playing
    builder.setDirection(oboe::Direction::Output);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

    //create stream for playing
    result = builder.openStream(&stream2);
    if (result != oboe::Result::OK) {
        LOGE("Error opening stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //create buffer to hold
    int32_t bufferSize = stream->getBufferSizeInFrames();
    int32_t bufferCapacity = bufferSize * stream->getChannelCount();

    auto *rbuffer = new float[bufferCapacity];

    //copydata
    //copy data of short array to rbuffer
    for(int i=0; i<len; i++){
        rbuffer[i] = data[i];
    }

    //convert rbuffer to string and print
    std::string str = std::to_string(rbuffer[0]);
    for(int i=1; i<len; i++){
        str += ", " + std::to_string(rbuffer[i]);
    }
    LOGD("JNI udpStartPlaying() rbuffer = %s", str.c_str());

    env->ReleaseShortArrayElements(short_data, data, 0);
    //end

    //start recording  stream
//    result = stream->requestStart();
//    if (result != oboe::Result::OK) {
//        LOGE("Error starting stream: %s", oboe::convertToText(result));
//        return JNI_FALSE;
//    }

    //start playing stream
    result = stream2->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Error starting stream: %s", oboe::convertToText(result));
        return JNI_FALSE;
    }

    //loop to read and write audio
    oboe::Result r,w;
    do {
        //read audio from recording stream
//        auto r = stream->read(buffer, bufferSize, 0);
//        if (r!= oboe::Result::OK) {
//            LOGE("Error reading stream: %s", "oboe::convertToText(r)");
//            return JNI_FALSE;
//        }
//        int framesRead = r.value();
//        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "framesRead = %d seqanceNo = %d", framesRead);

//        for (int i = 0; i < framesRead; i++){
//            __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder","nbFramesRead[%d] = %f", i, buffer[i]);
//        }

        //convert rbuffer to string and print
//        std::string str = "\nSeqNo = [" + std::to_string(frameReadSeq) + + "] "; frameReadSeq++;
//
//        for(int i=0; i<framesRead; i++){
//            str += ", " + std::to_string(buffer[i]);
//        }
//        LOGD("rbuffer = %s", str.c_str());

        // ----------
//        env->SetShortArrayRegion(jShortArray, 0, framesRead,
//                                 reinterpret_cast<const jshort *>(buffer));
//        env->CallVoidMethod(thiz, mid, jShortArray);
        // ----------

        //write audio to playing stream
        w = stream2->write(rbuffer, bufferSize, 0);
        if (w!= oboe::Result::OK) {
            LOGE("Error writing stream: %s", oboe::convertToText(w));
            return JNI_FALSE;
        }

    } while (!stopStatus);

    LOGD(TAG,"Attempting to stop");
    stream->requestStop();
    //stream->close();

    LOGD(TAG,"Attempting to stop 1");
//    stream2->requestStop();
    //stream2->close();
    delete[] rbuffer;
    frameReadSeq = 0;

    return JNI_TRUE;
*/

}