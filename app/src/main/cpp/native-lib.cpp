#include <jni.h>
#include <string>
#include "logging_macros.h"
#include "vector"

#include "AudioEngine.h"
#include "recordingAndSaving.h"
#include <oboe/Oboe.h>


const char *TAG = "native-lib.cpp:: %s";
static AudioEngine *audioEngine = nullptr;
static bool stopStatus = false;

//New approach
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_create(JNIEnv *env, jobject thiz) {

    LOGD(TAG, "create(): ");

    if (audioEngine == nullptr) {
        audioEngine = new AudioEngine();
    }

    return (audioEngine != nullptr);

}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_player_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {

    std::string hello = "Simple Recorder App";
    return env->NewStringUTF(hello.c_str());
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
        r = stream->read(buffer, bufferSize, 0);
        if (r!= oboe::Result::OK) {
            LOGE("Error reading stream: %s", oboe::convertToText(r));
            return JNI_FALSE;
        }

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

    return JNI_TRUE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_stopRevertBack(JNIEnv *env, jobject thiz) {

    stopStatus = true;

    return true;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_recAndSaveInFile(JNIEnv *env, jobject thiz, jstring path) {

    const char *pathToFile = (*env).GetStringUTFChars(path, 0);
    static auto a = OboeAudioRecorder::get();
    a->StartAudioRecorder(pathToFile, 48000);


}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_player_MainActivity_stopRec(JNIEnv *env, jobject thiz) {

    static auto a = OboeAudioRecorder::get();
    a->StopAudioRecorder();

    return true;
}