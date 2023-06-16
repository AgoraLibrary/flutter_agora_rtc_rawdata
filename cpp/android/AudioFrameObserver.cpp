#include "AudioFrameObserver.h"

#include "VMUtil.h"

namespace agora {
AudioFrameObserver::AudioFrameObserver(JNIEnv *env, jobject jCaller,
                                       long long engineHandle)
    : jCallerRef(env->NewGlobalRef(jCaller)), engineHandle(engineHandle) {
  jclass jCallerClass = env->GetObjectClass(jCallerRef);
  jOnRecordAudioFrame =
      env->GetMethodID(jCallerClass, "onRecordAudioFrame",
                       "(Lio/agora/rtc/rawdata/base/AudioFrame;)Z");
  jOnPlaybackAudioFrame =
      env->GetMethodID(jCallerClass, "onPlaybackAudioFrame",
                       "(Lio/agora/rtc/rawdata/base/AudioFrame;)Z");
  jOnMixedAudioFrame =
      env->GetMethodID(jCallerClass, "onMixedAudioFrame",
                       "(Lio/agora/rtc/rawdata/base/AudioFrame;)Z");
  jOnPlaybackAudioFrameBeforeMixing =
      env->GetMethodID(jCallerClass, "onPlaybackAudioFrameBeforeMixing",
                       "(ILio/agora/rtc/rawdata/base/AudioFrame;)Z");

  env->DeleteLocalRef(jCallerClass);

  jclass jAudioFrame = env->FindClass("io/agora/rtc/rawdata/base/AudioFrame");
  jAudioFrameClass = (jclass)env->NewGlobalRef(jAudioFrame);
  jAudioFrameInit =
      env->GetMethodID(jAudioFrameClass, "<init>", "(IIIII[BJI)V");
  env->DeleteLocalRef(jAudioFrame);

  env->GetJavaVM(&jvm);

  auto rtcEngine = reinterpret_cast<rtc::IRtcEngine *>(engineHandle);
  if (rtcEngine) {
    util::AutoPtr<media::IMediaEngine> mediaEngine;
    mediaEngine.queryInterface(rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
    if (mediaEngine) {
      mediaEngine->registerAudioFrameObserver(this);
    }
  }
}

AudioFrameObserver::~AudioFrameObserver() {
  auto rtcEngine = reinterpret_cast<rtc::IRtcEngine *>(engineHandle);
  if (rtcEngine) {
    util::AutoPtr<media::IMediaEngine> mediaEngine;
    mediaEngine.queryInterface(rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
    if (mediaEngine) {
      mediaEngine->registerAudioFrameObserver(nullptr);
    }
  }

  AttachThreadScoped ats(jvm);

  ats.env()->DeleteGlobalRef(jCallerRef);
  jOnRecordAudioFrame = nullptr;
  jOnPlaybackAudioFrame = nullptr;
  jOnMixedAudioFrame = nullptr;
  jOnPlaybackAudioFrameBeforeMixing = nullptr;

  ats.env()->DeleteGlobalRef(jAudioFrameClass);
  jAudioFrameInit = nullptr;
}

bool AudioFrameObserver::onRecordAudioFrame(const char *channelId,
                                            AudioFrame &audioFrame) {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  jbyteArray arr = NativeToJavaByteArray(env, audioFrame);
  jobject obj = NativeToJavaAudioFrame(env, audioFrame, arr);
  jboolean ret = env->CallBooleanMethod(jCallerRef, jOnRecordAudioFrame, obj);
  env->GetByteArrayRegion(arr, 0, env->GetArrayLength(arr),
                          static_cast<jbyte *>(audioFrame.buffer));
  env->DeleteLocalRef(arr);
  env->DeleteLocalRef(obj);
  return ret;
}

bool AudioFrameObserver::onPlaybackAudioFrame(const char *channelId,
                                              AudioFrame &audioFrame) {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  jbyteArray arr = NativeToJavaByteArray(env, audioFrame);
  jobject obj = NativeToJavaAudioFrame(env, audioFrame, arr);
  jboolean ret = env->CallBooleanMethod(jCallerRef, jOnPlaybackAudioFrame, obj);
  env->GetByteArrayRegion(arr, 0, env->GetArrayLength(arr),
                          static_cast<jbyte *>(audioFrame.buffer));
  env->DeleteLocalRef(arr);
  env->DeleteLocalRef(obj);
  return ret;
}

bool AudioFrameObserver::onMixedAudioFrame(const char *channelId,
                                           AudioFrame &audioFrame) {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  jbyteArray arr = NativeToJavaByteArray(env, audioFrame);
  jobject obj = NativeToJavaAudioFrame(env, audioFrame, arr);
  jboolean ret = env->CallBooleanMethod(jCallerRef, jOnMixedAudioFrame, obj);
  env->GetByteArrayRegion(arr, 0, env->GetArrayLength(arr),
                          static_cast<jbyte *>(audioFrame.buffer));
  env->DeleteLocalRef(arr);
  env->DeleteLocalRef(obj);
  return ret;
}

bool AudioFrameObserver::onPlaybackAudioFrameBeforeMixing(
    const char *channelId, rtc::uid_t uid, AudioFrame &audioFrame) {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  jbyteArray arr = NativeToJavaByteArray(env, audioFrame);
  jobject obj = NativeToJavaAudioFrame(env, audioFrame, arr);
  jboolean ret = env->CallBooleanMethod(
      jCallerRef, jOnPlaybackAudioFrameBeforeMixing, uid, obj);
  env->GetByteArrayRegion(arr, 0, env->GetArrayLength(arr),
                          static_cast<jbyte *>(audioFrame.buffer));
  env->DeleteLocalRef(arr);
  env->DeleteLocalRef(obj);
  return ret;
}

jbyteArray AudioFrameObserver::NativeToJavaByteArray(JNIEnv *env,
                                                     AudioFrame &audioFrame) {
  int length = audioFrame.samplesPerChannel * audioFrame.channels *
               audioFrame.bytesPerSample;

  jbyteArray jByteArray = env->NewByteArray(length);

  if (audioFrame.buffer) {
    env->SetByteArrayRegion(jByteArray, 0, length,
                            static_cast<const jbyte *>(audioFrame.buffer));
  }
  return jByteArray;
}

jobject AudioFrameObserver::NativeToJavaAudioFrame(JNIEnv *env,
                                                   AudioFrame &audioFrame,
                                                   jbyteArray jByteArray) {
  return env->NewObject(jAudioFrameClass, jAudioFrameInit, (int)audioFrame.type,
                        audioFrame.samplesPerChannel,
                        (int)audioFrame.bytesPerSample, audioFrame.channels,
                        audioFrame.samplesPerSec, jByteArray,
                        audioFrame.renderTimeMs, audioFrame.avsync_type);
}

bool AudioFrameObserver::onEarMonitoringAudioFrame(
    media::IAudioFrameObserverBase::AudioFrame &audioFrame) {
  return false;
}

int AudioFrameObserver::getObservedAudioFramePosition() { return 0; }

media::IAudioFrameObserverBase::AudioParams
AudioFrameObserver::getPlaybackAudioParams() {
  return media::IAudioFrameObserverBase::AudioParams();
}

media::IAudioFrameObserverBase::AudioParams
AudioFrameObserver::getRecordAudioParams() {
  return media::IAudioFrameObserverBase::AudioParams();
}

media::IAudioFrameObserverBase::AudioParams
AudioFrameObserver::getMixedAudioParams() {
  return media::IAudioFrameObserverBase::AudioParams();
}

media::IAudioFrameObserverBase::AudioParams
AudioFrameObserver::getEarMonitoringAudioParams() {
  return media::IAudioFrameObserverBase::AudioParams();
}
} // namespace agora
