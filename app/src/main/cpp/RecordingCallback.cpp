
#include "RecordingCallback.h"



oboe::DataCallbackResult
RecordingCallback::onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) {
    return processRecordingFrames(audioStream, static_cast<int16_t *>(audioData), numFrames * audioStream->getChannelCount());
}

oboe::DataCallbackResult
RecordingCallback::processRecordingFrames(oboe::AudioStream *audioStream, int16_t *audioData, int32_t numFrames) {

    LOGD(TAG, "processRecordingFrames(): ");

    //convert int16_t to string without data loss
    std::string str = std::to_string(*audioData);
    LOGD(TAG, str.c_str());

    int32_t framesWritten = mSoundRecording->write(audioData, numFrames);

    return oboe::DataCallbackResult::Continue;

}

////check if revert back is on
//if(RT == Revert::revertBackOn) {
//
//LOGD(TAG, "RevertBackOn Reading Frames : ");
//onAudioReady(audioStream,audioData,numFrames);
//}
