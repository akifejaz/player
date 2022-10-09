//
// Created by Moulvii on 26/09/2022.
//

#include "recordingAndSaving.h"
#include "ringBuffer.h"
OboeAudioRecorder *OboeAudioRecorder::singleton = nullptr;
void SetBuilder(oboe::AudioStreamBuilder &builder, const int = 48000);
typedef struct WAV_HEADER* header_p;
header_p headerData = (header_p)malloc(sizeof(header));


void OboeAudioRecorder::StartAudioRecorder(const char *fullPathToFile, const int recordingFreq,JNIEnv *env, jobject obj, jmethodID methodID ) {
    this->isRecording = true;
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input);
    //builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    builder.setPerformanceMode(oboe::PerformanceMode::None);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setChannelCount(oboe::ChannelCount::Mono);
    builder.setInputPreset(oboe::InputPreset::Unprocessed);
    builder.setSharingMode(oboe::SharingMode::Shared);
    builder.setSampleRate(recordingFreq);
    builder.setAudioApi(oboe::AudioApi::OpenSLES);
    //builder.setCallback(this);

    // Wave file generating stuff (from https://www.cplusplus.com/forum/beginner/166954/)
    int sampleRate = recordingFreq;
    int bitsPerSample = 16; // multiple of 8
    int numChannels = 1; // 2 for stereo, 1 for mono

    std::ofstream f;
    //const char *path = "/storage/emulated/0/Music/record.wav";
    const char *path = fullPathToFile;
    f.open(path, std::ios::binary);
    // Write the file headers
    f << "RIFF----WAVEfmt ";     // (chunk size to be filled in later)
    write_word( f,     16, 4 );  // no extension data
    write_word( f,      1, 2 );  // PCM - integer samples
    write_word( f,      numChannels, 2 );  // one channel (mono) or two channels (stereo file)
    write_word( f,  recordingFreq, 4 );  // samples per second (Hz)
    //write_word( f, 176400, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,(recordingFreq * bitsPerSample * numChannels) / 8, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,      4, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word( f,     bitsPerSample, 2 );  // number of bits per sample (use a multiple of 8)

    // Write the data chunk header
    size_t data_chunk_pos = f.tellp();
    f << "data----";  // (chunk size to be filled in later)
    // f.flush();

    // Write the audio samples
    constexpr double two_pi = 6.283185307179586476925286766559;
    constexpr double max_amplitude = 32760;  // "volume"

    oboe::Result r = builder.openStream(&stream);
    if (r != oboe::Result::OK) {
        return;
    }

    r = stream->requestStart();
    if (r != oboe::Result::OK) {
        return;
    }

    auto a = stream->getState();
    if (a == oboe::StreamState::Started) {

        constexpr int kMillisecondsToRecord = 2;
        auto requestedFrames = (int32_t) (kMillisecondsToRecord * (stream->getSampleRate() / oboe::kMillisPerSecond));
        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "requestedFrames = %d", requestedFrames);

        int16_t mybuffer[requestedFrames];
        jshortArray jShortArray = env->NewShortArray(requestedFrames);
        constexpr int64_t kTimeoutValue = 3 * oboe::kNanosPerMillisecond;

        int framesRead = 0;
        do {
            auto result = stream->read(mybuffer, requestedFrames, 0);
            if (result != oboe::Result::OK) {
                break;
            }
            framesRead = result.value();
            __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "framesRead = %d", framesRead);
            if (framesRead > 0) {
                break;
            }
        } while (framesRead != 0);

        while (isRecording) {
            auto result = stream->read(mybuffer, requestedFrames, kTimeoutValue * 1000);
            if (result == oboe::Result::OK) {
                auto nbFramesRead = result.value();
                __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "nbFramesRead = %d", nbFramesRead);
                for (int i = 0; i < nbFramesRead; i++) { //__android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder","nbFramesRead[%d] = %d", i, mybuffer[i]);
                    write_word( f, (int)(mybuffer[i]), 2 );
                    //write_word( f, (int)(mybuffer[i]), 2 ); // If stereo recording, add this line and write mybuffer[i+1] ?
                }
                //copy data from mybuffer to jShortArray
                env->SetShortArrayRegion(jShortArray, 0, nbFramesRead, mybuffer);
                env->CallVoidMethod(obj, methodID, jShortArray);

            } else {
                auto error = convertToText(result.error());
                __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "error = %s", error);
            }
        }

        stream->requestStop();
        stream->close();

        // (We'll need the final file size to fix the chunk sizes above)
        size_t file_length = f.tellp();

        // Fix the data chunk header to contain the data size
        f.seekp( data_chunk_pos + 4 );
        write_word( f, file_length - data_chunk_pos + 8 );

        // Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
        f.seekp( 0 + 4 );
        write_word( f, file_length - 8, 4 );
        f.close();
    }
}


void OboeAudioRecorder::UdpStartAudioRecorder(const int recordingFreq , int16_t *mybuffer, JNIEnv *env, jobject obj, jmethodID methodID) {
    this->isRecording = true;

    oboe::AudioStreamBuilder builder;
    SetBuilder(builder, recordingFreq);

    // Write the audio samples
    constexpr double two_pi = 6.283185307179586476925286766559;
    constexpr double max_amplitude = 32760;  // "volume"
    //create intance of RingBuffer using its malloc


    oboe::Result r = builder.openStream(&stream);
    if (r != oboe::Result::OK) {
        return;
    }

    r = stream->requestStart();
    if (r != oboe::Result::OK) {
        return;
    }

    auto a = stream->getState();
    if (a == oboe::StreamState::Started) {

        constexpr int kMillisecondsToRecord = 2;
        auto requestedFrames = (int32_t) (kMillisecondsToRecord * (stream->getSampleRate() / oboe::kMillisPerSecond));
        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "requestedFrames = %d", requestedFrames);

//        int16_t mybuffer[requestedFrames];
        mybuffer = new int16_t [requestedFrames];
        constexpr int64_t kTimeoutValue = 3 * oboe::kNanosPerMillisecond;

        //create JNI Short Array
        jshortArray jShortArray = env->NewShortArray(requestedFrames);
        //create JNI Byte Array
        jbyteArray jByteArray = env->NewByteArray(requestedFrames * 2);

        int framesRead = 0;
        do {
            auto result = stream->read(mybuffer, requestedFrames, 0);
            if (result != oboe::Result::OK) {
                break;
            }
            framesRead = result.value();
            __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "framesRead = %d", framesRead);
            if (framesRead > 0) {
                break;
            }
        } while (framesRead != 0);

        while (isRecording) {

            auto result = stream->read(mybuffer, requestedFrames, kTimeoutValue * 1000);
            if (result == oboe::Result::OK) {
                    auto nbFramesRead = result.value();
                    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "nbFramesRead = %d", nbFramesRead);


                //copy data from mybuffer to jShortArray
                 env->SetShortArrayRegion(jShortArray, 0, nbFramesRead, mybuffer);
                 env->CallVoidMethod(obj, methodID, jShortArray);

                //convert data from mybuffer to jByteArray
//                env->SetByteArrayRegion(jByteArray, 0, nbFramesRead * 2,
//                                        reinterpret_cast<const jbyte *>(mybuffer));
//                env->CallVoidMethod(obj, methodID, jByteArray);

//                for (int i = 0; i < nbFramesRead; i++) {
//                   __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder","nbFramesRead[%d] = %d", i, mybuffer[i]);
//                   //set the JNI Short Array with the data from mybuffer
//                }

            } else if(!isRecording){
                break;
            }
            else {
                    auto error = convertToText(result.error());
                    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "error = %s", error);
            }
        }

        LOGD("Trying close", "Attempting");
        this->stream->requestStop();
        this->stream->close();

    }

}

size_t OboeAudioRecorder::WriteHeader(const char *filePath, const int recordingFreq, std::ofstream &f) {
    // Wave file generating stuff (from https://www.cplusplus.com/forum/beginner/166954/)
    int sampleRate = recordingFreq;
    int bitsPerSample = 16; // multiple of 8
    int numChannels = 1; // 2 for stereo, 1 for mono

//    std::ofstream f;
    //const char *path = "/storage/emulated/0/Music/record.wav";
    const char *path = filePath;
    f.open(path, std::ios::binary);
    // Write the file headers
    f << "RIFF----WAVEfmt ";     // (chunk size to be filled in later)
    write_word( f,     16, 4 );  // no extension data
    write_word( f,      1, 2 );  // PCM - integer samples
    write_word( f,      numChannels, 2 );  // one channel (mono) or two channels (stereo file)
    write_word( f,  recordingFreq, 4 );  // samples per second (Hz)
    //write_word( f, 176400, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,(recordingFreq * bitsPerSample * numChannels) / 8, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,      4, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word( f,     bitsPerSample, 2 );  // number of bits per sample (use a multiple of 8)

    // Write the data chunk header
    size_t data_chunk_pos = f.tellp();
    f << "data----";  // (chunk size to be filled in later)
    // f.flush();

    return data_chunk_pos;
}

void OboeAudioRecorder::SaveToFile(const char *fullPathToFile) {

    // Wave file generating stuff (from https://www.cplusplus.com/forum/beginner/166954/)
    int recordingFreq = 48000;
    int sampleRate = recordingFreq;
    int bitsPerSample = 16; // multiple of 8
    int numChannels = 1; // 2 for stereo, 1 for mono

    std::ofstream f;
    //const char *path = "/storage/emulated/0/Music/record.wav";
    const char *path = fullPathToFile;
    f.open(path, std::ios::binary);
    // Write the file headers
    f << "RIFF----WAVEfmt ";     // (chunk size to be filled in later)
    write_word( f,     16, 4 );  // no extension data
    write_word( f,      1, 2 );  // PCM - integer samples
    write_word( f,      numChannels, 2 );  // one channel (mono) or two channels (stereo file)
    write_word( f,  recordingFreq, 4 );  // samples per second (Hz)
    //write_word( f, 176400, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,(recordingFreq * bitsPerSample * numChannels) / 8, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,      4, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word( f,     bitsPerSample, 2 );  // number of bits per sample (use a multiple of 8)

    // Write the data chunk header
    size_t data_chunk_pos = f.tellp();
    f << "data----";  // (chunk size to be filled in later)
    // f.flush();

    // (We'll need the final file size to fix the chunk sizes above)
    size_t file_length = f.tellp();


}

//OboeAudioRecorder::closeFile(std::ofstream &f){
//    // Fix the data chunk header to contain the data size
//    f.seekp( data_chunk_pos + 4 );
//    write_word( f, file_length - data_chunk_pos + 8 );
//
//    // Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
//    f.seekp( 0 + 4 );
//    write_word( f, file_length - 8, 4 );
//    f.close();
//}

void SetBuilder(oboe::AudioStreamBuilder &builder, const int recordingFreq) {
    builder.setDirection(oboe::Direction::Input);
    //builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    builder.setPerformanceMode(oboe::PerformanceMode::None);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setChannelCount(oboe::ChannelCount::Mono);
    builder.setInputPreset(oboe::InputPreset::Unprocessed);
    builder.setSharingMode(oboe::SharingMode::Shared);
    builder.setSampleRate(recordingFreq);
    builder.setAudioApi(oboe::AudioApi::OpenSLES);
}

