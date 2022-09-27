
#ifndef OBOE_RECORDER_AUDIOENGINE_H
#define OBOE_RECORDER_AUDIOENGINE_H

#ifndef MODULE_NAME
#define MODULE_NAME  "AudioEngine"
#endif

#include <oboe/Definitions.h>
#include <oboe/AudioStream.h>
#include "SoundRecording.h"
#include "logging_macros.h"
#include "Utils.h"
#include "RecordingCallback.h"
#include "PlayingCallback.h"
#include <sndfile.h>
#include <oboe/Utilities.h>
#include <bitset>


class AudioEngine /*: public oboe::AudioStreamCallback*/ {

public:
    AudioEngine();
    ~AudioEngine();

    RecordingCallback recordingCallback = RecordingCallback(&mSoundRecording);
    PlayingCallback playingCallback = PlayingCallback(&mSoundRecording, &sndfileHandle);

    void startRecording();
    void stopRecording();
    void startPlayingRecordedStream();

//    void revertBack();
//    oboe::Result openStreams(); //open streams : playing and recording
//    void stopPlayingRecordedStream();
//    void startPlayingFromFile(const char* filePath);
//    void stopPlayingFromFile();
//    void writeToFile(const char* filePath);
//
//    bool setEffectOn(bool b);

//    oboe::DataCallbackResult
//    onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override;

private:

    const char* TAG = "AudioEngine:: %s";
    //FullDuplexPass    mFullDuplexPass;
    bool              mIsEffectOn = false;


    int32_t mRecordingDeviceId = oboe::VoiceRecognition;
//    int32_t mPlaybackDeviceId = oboe::kUnspecified;
    int32_t mPlaybackDeviceId = 6;

    oboe::AudioFormat mFormat = oboe::AudioFormat::I16;
    int32_t mSampleRate = oboe::kUnspecified;
    int32_t mFramesPerBurst;
    int32_t mInputChannelCount = oboe::ChannelCount::Stereo;
    int32_t mOutputChannelCount = oboe::ChannelCount::Stereo;

    oboe::AudioApi mAudioApi = oboe::AudioApi::AAudio;
    oboe::AudioStream *mRecordingStream = nullptr;
    oboe::AudioStream *mPlaybackStream = nullptr;
    SoundRecording mSoundRecording;
    SndfileHandle sndfileHandle;

    void openRecordingStream();
//    //self function for above functinality
//    void self_openRecordingStream(oboe::AudioStreamBuilder* builder);
//    //
    void openPlaybackStreamFromRecordedStreamParameters();
//    void self_openPlaybackStreamFromRecordedStreamParameters(oboe::AudioStreamBuilder* builder);
    void openPlaybackStreamFromFileParameters();

    void startStream(oboe::AudioStream *stream);
    void stopStream(oboe::AudioStream *stream);
    void closeStream(oboe::AudioStream *stream);

    oboe::AudioStreamBuilder* setUpRecordingStreamParameters(oboe::AudioStreamBuilder* builder);
    oboe::AudioStreamBuilder* setUpPlaybackStreamParameters(oboe::AudioStreamBuilder *builder,
                                                            oboe::AudioApi audioApi, oboe::AudioFormat audioFormat, oboe::AudioStreamCallback *audioStreamCallback,
                                                            int32_t deviceId, int32_t sampleRate, int channelCount);

//    void setupPlaybackStreamParameters(oboe::AudioStreamBuilder *pBuilder);

//    oboe::DataCallbackResult process(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames);
};



AudioEngine::AudioEngine() {
    assert(mOutputChannelCount == mInputChannelCount);
}

AudioEngine::~AudioEngine() {
    stopStream(mRecordingStream);
    closeStream(mRecordingStream);

    stopStream(mPlaybackStream);
    closeStream(mPlaybackStream);
}

void AudioEngine::startRecording() {

    LOGD(TAG, "startRecording(): ");

    openRecordingStream();

    if (mRecordingStream) {
        startStream(mRecordingStream);
    } else {
        LOGE(TAG, "startRecording(): Failed to create recording (%p) stream", mRecordingStream);
        closeStream(mRecordingStream);
    }

}

void AudioEngine::stopRecording() {

    LOGD(TAG, "stopRecording(): ");

    stopStream(mRecordingStream);
    closeStream(mRecordingStream);

}

void AudioEngine::startPlayingRecordedStream() {

    LOGD(TAG, "startPlayingRecordedStream(): ");

    openPlaybackStreamFromRecordedStreamParameters();

    if (mPlaybackStream) {
        startStream(mPlaybackStream);
    } else {
        LOGE(TAG, "startPlayingRecordedStream(): Failed to create recording (%p) stream", mRecordingStream);
        closeStream(mPlaybackStream);
    }

}
//
//void AudioEngine::stopPlayingRecordedStream() {
//
//    LOGD(TAG, "stopPlayingRecordedStream(): ");
//
//    stopStream(mPlaybackStream);
//    closeStream(mPlaybackStream);
//    mSoundRecording.setReadPositionToStart();
//
//}
//
//void AudioEngine::startPlayingFromFile(const char* filePath) {
//
//    LOGD(TAG, "startPlayingFromFile(): ");
//
//    sndfileHandle = SndfileHandle(filePath);
//
//    openPlaybackStreamFromFileParameters();
//
//    if (mPlaybackStream) {
//        startStream(mPlaybackStream);
//    } else {
//        LOGE(TAG, "startPlayingFromFile(): Failed to create recording (%p) stream", mRecordingStream);
//        closeStream(mPlaybackStream);
//    }
//
//
//}
//
//void AudioEngine::stopPlayingFromFile() {
//
//    LOGD(TAG, "stopPlayingFromFile(): ");
//
//    stopStream(mPlaybackStream);
//    closeStream(mPlaybackStream);
//
//}
//
//void AudioEngine::writeToFile(const char* filePath) {
//
//    LOGD(TAG, "writeToFile(): ");
//
////    mSoundRecording.initiateWritingToFile(filePath, mOutputChannelCount, static_cast<int32_t>(mSampleRate * 0.465));
//    mSoundRecording.initiateWritingToFile(filePath, mOutputChannelCount, mSampleRate);
//
//}

void AudioEngine::openRecordingStream() {

    LOGD(TAG, "openRecordingStream(): ");

    oboe::AudioStreamBuilder builder;

    setUpRecordingStreamParameters(&builder);

    oboe::Result result = builder.openStream(&mRecordingStream);
    if (result == oboe::Result::OK && mRecordingStream) {
        assert(mRecordingStream->getChannelCount() == mInputChannelCount);
//        assert(mRecordingStream->getSampleRate() == mSampleRate);
//        assert(mRecordingStream->getFormat() == mFormat);

        mSampleRate = mRecordingStream->getSampleRate();
        mFormat = mRecordingStream->getFormat();
        LOGV(TAG, "openRecordingStream(): mSampleRate = ");
        LOGV(TAG, std::to_string(mSampleRate).c_str());

        LOGV(TAG, "openRecordingStream(): mFormat = ");
        LOGV(TAG, oboe::convertToText(mFormat));

    } else {
        LOGE(TAG, "Failed to create recording stream. Error: %s",
             oboe::convertToText(result));
    }

}

void AudioEngine::openPlaybackStreamFromRecordedStreamParameters() {

    LOGD(TAG, "openPlaybackStreamFromRecordedStreamParameters(): ");

    oboe::AudioStreamBuilder builder;

    setUpPlaybackStreamParameters(&builder, mAudioApi, mFormat, &playingCallback,
                                  mPlaybackDeviceId, mSampleRate, mOutputChannelCount);

    playingCallback.setPlaybackFromFile(false);

    oboe::Result result = builder.openStream(&mPlaybackStream);
    if (result == oboe::Result::OK && mPlaybackStream) {
        assert(mPlaybackStream->getChannelCount() == mOutputChannelCount);
//        assert(mPlaybackStream->getSampleRate() == mSampleRate);
        assert(mPlaybackStream->getFormat() == mFormat);

        mSampleRate = mPlaybackStream->getSampleRate();
        LOGV(TAG, "openPlaybackStreamFromRecordedStreamParameters(): mSampleRate = ");
        LOGV(TAG, std::to_string(mSampleRate).c_str());

        mFramesPerBurst = mPlaybackStream->getFramesPerBurst();
        LOGV(TAG, "openPlaybackStreamFromRecordedStreamParameters(): mFramesPerBurst = ");
        LOGV(TAG, std::to_string(mFramesPerBurst).c_str());

        // Set the buffer size to the burst size - this will give us the minimum possible latency
        mPlaybackStream->setBufferSizeInFrames(mFramesPerBurst);

    } else {
        LOGE(TAG, "openPlaybackStreamFromRecordedStreamParameters(): Failed to create recording stream. Error: %s",
             oboe::convertToText(result));
    }

}
//
//void AudioEngine::openPlaybackStreamFromFileParameters() {
//
//    LOGD(TAG, "openPlaybackStreamFromFileParameters(): ");
//
//    oboe::AudioStreamBuilder builder;
//
//    mSampleRate = sndfileHandle.samplerate();
//
//    int audioFileFormat = sndfileHandle.format();
//    LOGD(TAG, "openPlaybackStreamFromFileParameters(): audioFileFormat = ");
//    LOGD(TAG, std::to_string(audioFileFormat).c_str());
//
//    mOutputChannelCount = sndfileHandle.channels();
//
//    setUpPlaybackStreamParameters(&builder, mAudioApi, mFormat, &playingCallback,
//                                  mPlaybackDeviceId, mSampleRate, mOutputChannelCount);
//
//    playingCallback.setPlaybackFromFile(true);
//
//    oboe::Result result = builder.openStream(&mPlaybackStream);
//    if (result == oboe::Result::OK && mPlaybackStream) {
//        assert(mPlaybackStream->getChannelCount() == mOutputChannelCount);
////        assert(mPlaybackStream->getSampleRate() == mSampleRate);
//        assert(mPlaybackStream->getFormat() == mFormat);
//
//        mSampleRate = mPlaybackStream->getSampleRate();
//        LOGV(TAG, "openPlaybackStreamFromFileParameters(): mSampleRate = ");
//        LOGV(TAG, std::to_string(mSampleRate).c_str());
//
//        mFramesPerBurst = mPlaybackStream->getFramesPerBurst();
//        LOGV(TAG, "openPlaybackStreamFromFileParameters(): mFramesPerBurst = ");
//        LOGV(TAG, std::to_string(mFramesPerBurst).c_str());
//
//        // Set the buffer size to the burst size - this will give us the minimum possible latency
//        mPlaybackStream->setBufferSizeInFrames(mFramesPerBurst);
//
//    } else {
//        LOGE(TAG, "openPlaybackStreamFromFileParameters(): Failed to create recording stream. Error: %s",
//             oboe::convertToText(result));
//    }
//
//}

void AudioEngine::startStream(oboe::AudioStream *stream) {

    LOGD(TAG, "startStream(): ");

    assert(stream);
    if (stream) {
        oboe::Result result = stream->requestStart();
        if (result != oboe::Result::OK) {
            LOGE(TAG, "Error starting stream. %s", oboe::convertToText(result));
        }
    }

}

void AudioEngine::stopStream(oboe::AudioStream *stream) {

    LOGD(TAG, "stopStream(): ");

    if (stream) {
        oboe::Result result = stream->stop(0L);
        if (result != oboe::Result::OK) {
            LOGE(TAG, "Error stopping stream. %s", oboe::convertToText(result));
        }
        LOGW(TAG, "stopStream(): mTotalSamples = ");
        LOGW(TAG, std::to_string(mSoundRecording.getTotalSamples()).c_str());
    }

}

void AudioEngine::closeStream(oboe::AudioStream *stream) {

    LOGD(TAG, "closeStream(): ");

    if (stream) {
        oboe::Result result = stream->close();
        if (result != oboe::Result::OK) {
           // LOGE(TAG, "Error closing stream. %s", oboe::convertToText(result));
        } else {
            stream = nullptr;
        }

        LOGW(TAG, "closeStream(): mTotalSamples = ");
        LOGW(TAG, std::to_string(mSoundRecording.getTotalSamples()).c_str());
    }

}

oboe::AudioStreamBuilder *AudioEngine::setUpRecordingStreamParameters(oboe::AudioStreamBuilder *builder) {

    LOGD(TAG, "setUpRecordingStreamParameters(): ");

    builder->setAudioApi(mAudioApi)
            ->setFormat(mFormat)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setCallback(&recordingCallback)
            ->setDeviceId(mRecordingDeviceId)
            ->setDirection(oboe::Direction::Input)
//            ->setSampleRate(mSampleRate)
            ->setChannelCount(mInputChannelCount);
    return builder;
}

oboe::AudioStreamBuilder *AudioEngine::setUpPlaybackStreamParameters(oboe::AudioStreamBuilder *builder,
                                                                     oboe::AudioApi audioApi, oboe::AudioFormat audioFormat, oboe::AudioStreamCallback *audioStreamCallback,
                                                                     int32_t deviceId, int32_t sampleRate, int channelCount) {

    LOGD(TAG, "setUpPlaybackStreamParameters(): ");

    builder->setAudioApi(audioApi)
            ->setFormat(audioFormat)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setCallback(audioStreamCallback)
            ->setDeviceId(deviceId)
            ->setDirection(oboe::Direction::Output)
            ->setSampleRate(sampleRate)
            ->setChannelCount(channelCount);
    return builder;

}
//
//void AudioEngine::revertBack() {
//
//    LOGD(TAG, "revertBack(): ");
//
////    oboe::AudioStreamBuilder builder;
////    self_openRecordingStream(&builder);
////
////    //------------------ Recording Stream ------------------
////    //self_openPlaybackStreamFromRecordedStreamParameters(&builder);
////    if (mRecordingStream) {
////        startStream(mRecordingStream);
////    } else {
////        LOGE(TAG, "self startRecording(): Failed to create recording (%p) stream", mRecordingStream);
////        closeStream(mRecordingStream);
////    }
////
////    //------------------ Playing Stream ------------------
////    if (mPlaybackStream) {
////        startStream(mPlaybackStream);
////    } else {
////        LOGE(TAG, "self startPlayingRecordedStream(): Failed to create recording (%p) stream", mRecordingStream);
////        closeStream(mPlaybackStream);
////    }
////
////    //---------- Writing & Reading from buff
////    int16_t *rec_data = new int16_t [20]; int i = 0;
////    while( mPlaybackStream->isPlaying() && mRecordingStream->isPlaying() ){
////
////        mRecordingStream->write(rec_data,20,100);
////        mPlaybackStream->read(rec_data,20,100);
////
////        LOGE("revertBack(): loop", "loopy");
////        i++;
////    }
//
//}
//
//
//oboe::Result AudioEngine::openStreams() {
//
//    LOGD(TAG, "openStreams(): ");
////    oboe::AudioStreamBuilder inBuilder, outBuilder; //inBuilder is for recording, outBuilder is for playback
////    // Note: The order of stream creation is important. We create the playback
////    // stream first, then use properties from the playback stream
////    // (e.g. sample rate) to create the recording stream. By matching the
////    // properties we should get the lowest latency path
////    outBuilder
////        ->setErrorCallback(this)
////        ->setDeviceId(mPlaybackDeviceId)
////        ->setDirection(oboe::Direction::Output)
////        ->setChannelCount(mOutputChannelCount);
////
////    //open playback stream
////    oboe::Result result = outBuilder->openStream(&mPlaybackStream);
////    if(result != oboe::Result::OK){
////        LOGE(TAG, "Error opening playback stream. Error: %s", oboe::convertToText(result));
////        return result;
////    }
////    //get samples from playback stream
////    mSampleRate = mPlaybackStream->getSampleRate();
////
////    //setup the recording stream
////    inBuilder->setDataCallback(this)
////        ->setErrorCallback(this)
////        ->setDeviceId(mRecordingDeviceId)
////        ->setDirection(oboe::Direction::Input)
////        ->setChannelCount(mInputChannelCount)
////        ->setSampleRate(mSampleRate);
////
////    //open recording stream
////    result = inBuilder->openStream(&mRecordingStream);
////    if(result != oboe::Result::OK){
////        LOGE(TAG, "Error opening recording stream. Error: %s", oboe::convertToText(result));
////        return result;
////    }
////
//
//}
//
//
//void AudioEngine::self_openRecordingStream(oboe::AudioStreamBuilder* builder){
//    LOGD(TAG, "self_openRecordingStream(): ");
//
//    //recording side
//    setUpRecordingStreamParameters(builder);
//    oboe::Result result = builder->openStream(&mRecordingStream);
//
//    if (result == oboe::Result::OK && mRecordingStream) {
//        assert(mRecordingStream->getChannelCount() == mInputChannelCount);
//////        assert(mRecordingStream->getSampleRate() == mSampleRate);
//////        assert(mRecordingStream->getFormat() == mFormat);
////
////        mSampleRate = mRecordingStream->getSampleRate();
////        mFormat = mRecordingStream->getFormat();
////        LOGV(TAG, "openRecordingStream(): mSampleRate = ");
////        LOGV(TAG, std::to_string(mSampleRate).c_str());
////
////        LOGV(TAG, "openRecordingStream(): mFormat = ");
////        LOGV(TAG, oboe::convertToText(mFormat));
//
//    } else {
//        LOGE(TAG, "Failed to create recording stream. Error: %s",
//             oboe::convertToText(result));
//    }
//
//    //plaing side
//    setUpPlaybackStreamParameters(builder, mAudioApi, mFormat, &playingCallback,
//                                  mPlaybackDeviceId, mSampleRate, mOutputChannelCount);
//
//    playingCallback.setPlaybackFromFile(false);
//    result = builder->openStream(&mPlaybackStream);
//    if (result == oboe::Result::OK && mPlaybackStream) {
//        assert(mPlaybackStream->getChannelCount() == mOutputChannelCount);
////        assert(mPlaybackStream->getSampleRate() == mSampleRate);
//        assert(mPlaybackStream->getFormat() == mFormat);
//
//        mSampleRate = mPlaybackStream->getSampleRate();
//        LOGV(TAG, "openPlaybackStreamFromRecordedStreamParameters(): mSampleRate = ");
//        LOGV(TAG, std::to_string(mSampleRate).c_str());
//
//        mFramesPerBurst = mPlaybackStream->getFramesPerBurst();
//        LOGV(TAG, "openPlaybackStreamFromRecordedStreamParameters(): mFramesPerBurst = ");
//        LOGV(TAG, std::to_string(mFramesPerBurst).c_str());
//
//        // Set the buffer size to the burst size - this will give us the minimum possible latency
//        mPlaybackStream->setBufferSizeInFrames(mFramesPerBurst);
//
//    } else {
//        LOGE(TAG, "openPlaybackStreamFromRecordedStreamParameters(): Failed to create recording stream. Error: %s",
//             oboe::convertToText(result));
//    }
//
//
//}
//
//void AudioEngine::self_openPlaybackStreamFromRecordedStreamParameters(oboe::AudioStreamBuilder* builder) {
//    LOGD(TAG, "openPlaybackStreamFromRecordedStreamParameters(): ");
//
//    setUpPlaybackStreamParameters(builder, mAudioApi, mFormat, &playingCallback,
//                                  mPlaybackDeviceId, mSampleRate, mOutputChannelCount);
//
//    playingCallback.setPlaybackFromFile(false);
//
//    oboe::Result result = builder->openStream(&mPlaybackStream);
//    if (result == oboe::Result::OK && mPlaybackStream) {
//        assert(mPlaybackStream->getChannelCount() == mOutputChannelCount);
////        assert(mPlaybackStream->getSampleRate() == mSampleRate);
//        assert(mPlaybackStream->getFormat() == mFormat);
//
//        mSampleRate = mPlaybackStream->getSampleRate();
//        LOGV(TAG, "openPlaybackStreamFromRecordedStreamParameters(): mSampleRate = ");
//        LOGV(TAG, std::to_string(mSampleRate).c_str());
//
//        mFramesPerBurst = mPlaybackStream->getFramesPerBurst();
//        LOGV(TAG, "openPlaybackStreamFromRecordedStreamParameters(): mFramesPerBurst = ");
//        LOGV(TAG, std::to_string(mFramesPerBurst).c_str());
//
//        // Set the buffer size to the burst size - this will give us the minimum possible latency
//        mPlaybackStream->setBufferSizeInFrames(mFramesPerBurst);
//
//    } else {
//        LOGE(TAG, "openPlaybackStreamFromRecordedStreamParameters(): Failed to create recording stream. Error: %s",
//             oboe::convertToText(result));
//    }
//
//}
//
//void AudioEngine::setupPlaybackStreamParameters(oboe::AudioStreamBuilder *pBuilder) {
//
//}
//
//bool AudioEngine::setEffectOn(bool b) {
//
//    LOGD(TAG,"setEffectOn");
//
//    /* Builder for Recording Stream */
//    oboe::AudioStreamBuilder Inbuilder;
//
//    Inbuilder.setDirection(oboe::Direction::Input);
//    Inbuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
//
//    /* Recording Stream */
//    oboe::AudioStream *inStream;
//    oboe::Result r = Inbuilder.openStream(&inStream);
//
//    if(r!=oboe::Result::OK){
//        LOGE(TAG,"Error Opening Input stream");
//        LOGE(TAG, oboe::convertToText(r));
//    } else {LOGD(TAG,"Input stream"); }
//
//    r = inStream->requestStart();
//    if(r!=oboe::Result::OK){
//        LOGE(TAG,"Error Opening Request");
//    } else {LOGD(TAG,"Opening inStream Request");}
//
//
//    /* Builder for Playing Stream */
//    oboe::AudioStreamBuilder OutBuilder;
//    OutBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
//    OutBuilder.setSharingMode(oboe::SharingMode::Exclusive);
//    //not using the callback method
//    //OutBuilder.setCallback(this);
//
//    /* Playing Stream */
//    oboe::AudioStream *outStream;
//    oboe::Result r1 = OutBuilder.openStream(&outStream);
//
//    if(r1!=oboe::Result::OK){
//        LOGE(TAG,"Error Opening Output stream");
//        LOGE(TAG, oboe::convertToText(r1));
//    } else {LOGD(TAG,"Opening Output stream");}
//
//    r1 = outStream->requestStart();
//    if(r1!=oboe::Result::OK){
//        LOGE(TAG,"Error Opening Request");
//    } else {LOGD(TAG,"Opening outStream Request");}
//
//    //creating buffer for reading input data
//    const int32_t reqFrames = (int32_t)(5 * (inStream->getSampleRate() / oboe::kMillisPerSecond));
//    int16_t buffer[reqFrames];
//
//    int64_t nSec = 5 * oboe::kNanosPerMillisecond;
//
//    //read the frames for older values
////    int framesRead = 0;
////    do{
////        auto result = inStream->read(buffer, inStream->getBufferSizeInFrames(), 0);
////        if(result != oboe::Result::OK) break;
////        framesRead = result.value();
////    } while(framesRead !=0);
//
//    //read the frames for new values
//    while(stopStatus){
//        LOGD(TAG,"Started Reading");
//        auto result = inStream->read(buffer, reqFrames, 0);
//        if(result != oboe::Result::OK){
//            LOGE(TAG,"Error Reading");
//            inStream->close();
//        }
//        else{
//            LOGD(TAG,"Read frames", result.value());
////            for(int i=0; i<reqFrames; i++){
////                LOGD("BUFFER",buffer[i]);
////            }
//        }
//    }
//    LOGD(TAG,"Data Read Success");
//
//    /* Playing  */
////    int32_t numBytes = numFrames * oboeStream->getBytesPerFrame();
//    memset(buffer, 0 /* value */, reqFrames);
//
//    //write data to speaker from buffer
//    int32_t framesWritten = 0;
//    do {
//        auto result = outStream->write(buffer, reqFrames, nSec);
//        if (result != oboe::Result::OK) {
//            LOGE(TAG, "Failed to Write to Output Stream");
//            outStream->close();
//        }
//        framesWritten = result.value();
//        LOGD(TAG,"Frmes written in Output stream");
//        LOGD(TAG,framesWritten);
//    } while (!stopStatus);
//
//
//    inStream->close();
//    outStream->close();
//}
//
////oboe::DataCallbackResult
////AudioEngine::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
////    return process(oboeStream, audioData, numFrames);
////}
////
////oboe::DataCallbackResult AudioEngine::process(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
////    LOGD(TAG,"process()");
////    // Silence the output.
////    int32_t numBytes = numFrames * oboeStream->getBytesPerFrame();
////    memset(audioData, 0 /* value */, numBytes);
////
////    //write data to speaker from buffer
////    int32_t framesWritten = 0;
////    do {
////        auto result = oboeStream->write(audioData, numFrames, 0);
////        if (result != oboe::Result::OK) {
////            LOGE(TAG, "Failed to Write to Output Stream");
////            return oboe::DataCallbackResult::Stop;
////        }
////        framesWritten = result.value();
////    } while (framesWritten != 0);
////
////}
//

#endif //OBOE_RECORDER_AUDIOENGINE_H
