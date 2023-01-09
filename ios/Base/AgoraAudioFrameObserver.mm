//
//  AgoraAudioFrameObserver.mm
//  react-native-agora-rawdata
//
//  Created by LXH on 2020/11/10.
//

#import "AgoraAudioFrameObserver.h"

#import <AgoraRtcKit/IAgoraMediaEngine.h>
#import <AgoraRtcKit/IAgoraRtcEngine.h>

namespace agora {
class AudioFrameObserver : public media::IAudioFrameObserver {
public:
  AudioFrameObserver(long long engineHandle, void *observer)
      : engineHandle(engineHandle), observer(observer) {
    auto rtcEngine = reinterpret_cast<rtc::IRtcEngine *>(engineHandle);
    if (rtcEngine) {
      util::AutoPtr<media::IMediaEngine> mediaEngine;
      mediaEngine.queryInterface(rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
      if (mediaEngine) {
        mediaEngine->registerAudioFrameObserver(this);
      }
    }
  }

  virtual ~AudioFrameObserver() {
    auto rtcEngine = reinterpret_cast<rtc::IRtcEngine *>(engineHandle);
    if (rtcEngine) {
      util::AutoPtr<media::IMediaEngine> mediaEngine;
      mediaEngine.queryInterface(rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
      if (mediaEngine) {
        mediaEngine->registerAudioFrameObserver(nullptr);
      }
    }
  }

public:
  bool onRecordAudioFrame(const char* channelId, AudioFrame& audioFrame) override {
    @autoreleasepool {
      AgoraAudioFrame *audioFrameApple = NativeToAppleAudioFrame(audioFrame);

      AgoraAudioFrameObserver *observerApple =
          (__bridge AgoraAudioFrameObserver *)observer;
      if (observerApple.delegate != nil &&
          [observerApple.delegate
              respondsToSelector:@selector(onRecordAudioFrame:)]) {
        return [observerApple.delegate onRecordAudioFrame:audioFrameApple];
      }
    }
    return true;
  }

  bool onPlaybackAudioFrame(const char* channelId, AudioFrame& audioFrame) override {
    @autoreleasepool {
      AgoraAudioFrame *audioFrameApple = NativeToAppleAudioFrame(audioFrame);

      AgoraAudioFrameObserver *observerApple =
          (__bridge AgoraAudioFrameObserver *)observer;
      if (observerApple.delegate != nil &&
          [observerApple.delegate
              respondsToSelector:@selector(onPlaybackAudioFrame:)]) {
        return [observerApple.delegate onPlaybackAudioFrame:audioFrameApple];
      }
    }
    return true;
  }

  bool onMixedAudioFrame(const char* channelId, AudioFrame& audioFrame) override {
    @autoreleasepool {
      AgoraAudioFrame *audioFrameApple = NativeToAppleAudioFrame(audioFrame);

      AgoraAudioFrameObserver *observerApple =
          (__bridge AgoraAudioFrameObserver *)observer;
      if (observerApple.delegate != nil &&
          [observerApple.delegate
              respondsToSelector:@selector(onMixedAudioFrame:)]) {
        return [observerApple.delegate onMixedAudioFrame:audioFrameApple];
      }
    }
    return true;
  }

  bool onPlaybackAudioFrameBeforeMixing(
                                        const char* channelId, rtc::uid_t uid, AudioFrame& audioFrame) override {
    @autoreleasepool {
      AgoraAudioFrame *audioFrameApple = NativeToAppleAudioFrame(audioFrame);

      AgoraAudioFrameObserver *observerApple =
          (__bridge AgoraAudioFrameObserver *)observer;
      if (observerApple.delegate != nil &&
          [observerApple.delegate respondsToSelector:@selector
                                  (onPlaybackAudioFrameBeforeMixing:uid:)]) {
        return [observerApple.delegate
            onPlaybackAudioFrameBeforeMixing:audioFrameApple
                                         uid:uid];
      }
    }
    return true;
  }

  bool onEarMonitoringAudioFrame(
            media::IAudioFrameObserverBase::AudioFrame &audioFrame) override {
    return false;
  }

  int getObservedAudioFramePosition() override {
    return 0;
  }

  media::IAudioFrameObserverBase::AudioParams getPlaybackAudioParams() override {
    return media::IAudioFrameObserverBase::AudioParams();
  }

  media::IAudioFrameObserverBase::AudioParams getRecordAudioParams() override {
    return media::IAudioFrameObserverBase::AudioParams();
  }

  media::IAudioFrameObserverBase::AudioParams getMixedAudioParams() override {
    return media::IAudioFrameObserverBase::AudioParams();
  }

  media::IAudioFrameObserverBase::AudioParams getEarMonitoringAudioParams() override {
    return media::IAudioFrameObserverBase::AudioParams();
  }

private:
  AgoraAudioFrame *NativeToAppleAudioFrame(AudioFrame &audioFrame) {
    AgoraAudioFrame *audioFrameApple = [[AgoraAudioFrame alloc] init];
    audioFrameApple.type = (AgoraAudioFrameType)audioFrame.type;
    audioFrameApple.samples = audioFrame.samplesPerChannel;
    audioFrameApple.bytesPerSample = audioFrame.bytesPerSample;
    audioFrameApple.channels = audioFrame.channels;
    audioFrameApple.samplesPerSec = audioFrame.samplesPerSec;
    audioFrameApple.buffer = audioFrame.buffer;
    audioFrameApple.renderTimeMs = audioFrame.renderTimeMs;
    audioFrameApple.avsync_type = audioFrame.avsync_type;
    return audioFrameApple;
  }

private:
  void *observer;
  long long engineHandle;
};
}

@interface AgoraAudioFrameObserver ()
@property(nonatomic) agora::AudioFrameObserver *observer;
@end

@implementation AgoraAudioFrameObserver

- (instancetype)initWithEngineHandle:(NSUInteger)engineHandle {
  if (self = [super init]) {
    self.engineHandle = engineHandle;
  }
  return self;
}

- (void)registerAudioFrameObserver {
  if (!_observer) {
    _observer =
        new agora::AudioFrameObserver(_engineHandle, (__bridge void *)self);
  }
}

- (void)unregisterAudioFrameObserver {
  if (_observer) {
    delete _observer;
    _observer = nullptr;
  }
}

@end
