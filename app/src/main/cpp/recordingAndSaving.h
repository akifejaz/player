//
// Created by Moulvii on 26/09/2022.
//

#ifndef PLAYER_RECORDINGANDSAVING_H
#define PLAYER_RECORDINGANDSAVING_H

#include <jni.h>
#include <string>
#include <oboe/Oboe.h>
#include "logging_macros.h"
#include <fstream>
//int32_t requestedFrames ;

namespace little_endian_io
{
    template <typename Word>
    std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
    {
        for (; size; --size, value >>= 8)
            outs.put( static_cast <char> (value & 0xFF) );
        return outs;
    }
}
using namespace little_endian_io;

class OboeAudioRecorder: public oboe::AudioStreamCallback {

private:

//    oboe::ManagedStream outStream;
    oboe::AudioStream *stream{};

    oboe::DataCallbackResult
    onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override {
        LOGE("onAudioReady");
    }

    static OboeAudioRecorder *singleton;
    explicit OboeAudioRecorder() = default;

public:
    static OboeAudioRecorder *get() {

        if (singleton == nullptr)
            singleton = new OboeAudioRecorder();
        return singleton;
    }

    bool isRecording = true;

    void StopAudioRecorder() {

        this->isRecording = false;
    }

    void StartAudioRecorder(const char *fullPathToFile, const int = 48000) ;

    size_t WriteHeader(const char *filePath, const int recordingFreq, std::ofstream &f);

    void UdpStartAudioRecorder(const int recordingFreq,int16_t *mybuffer,JNIEnv *env,jobject thiz,jmethodID methodID);

    void
    StartAudioRecorder(const char *fullPathToFile, const int recordingFreq, JNIEnv *env,
                       jobject obj,
                       jmethodID methodID);

    void
    SaveToFile(const char *fullPathToFile);
};

typedef struct WAV_HEADER
{
    char chunk_id[4];
    int chunk_size;
    char format[4];
    char subchunk1_id[4];
    int subchunk1_size;
    short int audio_format;
    short int num_channels;
    int sample_rate;			// sample_rate denotes the sampling rate.
    int byte_rate;
    short int block_align;
    short int bits_per_sample;
    char subchunk2_id[4];
    int subchunk2_size;			// subchunk2_size denotes the number of samples.

} header;


#endif //PLAYER_RECORDINGANDSAVING_H
