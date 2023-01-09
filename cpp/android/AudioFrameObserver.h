#pragma once

#include "include/IAgoraMediaEngine.h"
#include "include/IAgoraRtcEngine.h"

#include <jni.h>

namespace agora {
class AudioFrameObserver : public media::IAudioFrameObserver {
public:
  AudioFrameObserver(JNIEnv *env, jobject jCaller, long long engineHandle);
  virtual ~AudioFrameObserver();

public:
    bool onRecordAudioFrame(const char* channelId, AudioFrame& audioFrame) override;
    bool onPlaybackAudioFrame(const char* channelId, AudioFrame& audioFrame) override;
    bool onMixedAudioFrame(const char* channelId, AudioFrame& audioFrame) override;
    bool onEarMonitoringAudioFrame(AudioFrame& audioFrame) override;
    bool onPlaybackAudioFrameBeforeMixing(const char* channelId, rtc::uid_t uid, AudioFrame& audioFrame) override;
    int getObservedAudioFramePosition() override;
    AudioParams getPlaybackAudioParams() override;
    AudioParams getRecordAudioParams() override;
    AudioParams getMixedAudioParams() override;
    AudioParams getEarMonitoringAudioParams() override;

private:
  jbyteArray NativeToJavaByteArray(JNIEnv *env, AudioFrame &audioFrame);
  jobject NativeToJavaAudioFrame(JNIEnv *env, AudioFrame &audioFrame,
                                 jbyteArray jByteArray);

private:
  JavaVM *jvm = nullptr;

  jobject jCallerRef;
  jmethodID jOnRecordAudioFrame;
  jmethodID jOnPlaybackAudioFrame;
  jmethodID jOnMixedAudioFrame;
  jmethodID jOnPlaybackAudioFrameBeforeMixing;

  jclass jAudioFrameClass;
  jmethodID jAudioFrameInit;

  long long engineHandle;
};
} // namespace agora
