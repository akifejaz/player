#include <jni.h>
#include <string>
#include "logging_macros.h"
#include "AudioEngine.h"
#include <oboe/Oboe.h>


const char *TAG = "native-lib.cpp:: %s";
static AudioEngine *audioEngine = nullptr;
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