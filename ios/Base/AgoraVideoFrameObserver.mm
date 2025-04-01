//
//  AgoraVideoFrameObserver.mm
//  react-native-agora-rawdata
//
//  Created by LXH on 2020/11/10.
//

#import "AgoraVideoFrameObserver.h"

#import <AgoraRtcKit/IAgoraMediaEngine.h>
#import <AgoraRtcKit/IAgoraRtcEngine.h>

namespace agora {
class VideoFrameObserver : public media::IVideoFrameObserver {
public:
  VideoFrameObserver(long long engineHandle, void *observer)
      : engineHandle(engineHandle),
        observer((__bridge AgoraVideoFrameObserver *)observer) {
    auto rtcEngine = reinterpret_cast<rtc::IRtcEngine *>(engineHandle);
    if (rtcEngine) {
      util::AutoPtr<media::IMediaEngine> mediaEngine;
      mediaEngine.queryInterface(rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
      if (mediaEngine) {
        mediaEngine->registerVideoFrameObserver(this);
      }
    }
  }

  virtual ~VideoFrameObserver() {
    auto rtcEngine = reinterpret_cast<rtc::IRtcEngine *>(engineHandle);
    if (rtcEngine) {
      util::AutoPtr<media::IMediaEngine> mediaEngine;
      mediaEngine.queryInterface(rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
      if (mediaEngine) {
        mediaEngine->registerVideoFrameObserver(nullptr);
      }
    }
  }

public:
  bool onCaptureVideoFrame(agora::rtc::VIDEO_SOURCE_TYPE type,
                           VideoFrame &videoFrame) override {
    @autoreleasepool {
      AgoraVideoFrameObserver *strongObserverApple = observer;
      if (strongObserverApple) {
        AgoraVideoFrame *videoFrameApple = NativeToAppleVideoFrame(videoFrame);

        if (strongObserverApple.delegate != nil &&
            [strongObserverApple.delegate
                respondsToSelector:@selector(onCaptureVideoFrame:frame:)]) {
          return [strongObserverApple.delegate
              onCaptureVideoFrame:type
                            frame:videoFrameApple];
        }
      }
    }
    return true;
  }

  bool onRenderVideoFrame(const char *channelId, rtc::uid_t remoteUid,
                          VideoFrame &videoFrame) override {
    @autoreleasepool {
      AgoraVideoFrameObserver *strongObserverApple = observer;
      if (strongObserverApple) {
        AgoraVideoFrame *videoFrameApple = NativeToAppleVideoFrame(videoFrame);

        if (strongObserverApple.delegate != nil &&
            [strongObserverApple.delegate
                respondsToSelector:@selector(onRenderVideoFrame:uid:)]) {
          return
              [strongObserverApple.delegate onRenderVideoFrame:videoFrameApple
                                                           uid:remoteUid];
        }
      }
    }
    return true;
  }

  bool onPreEncodeVideoFrame(agora::rtc::VIDEO_SOURCE_TYPE type,
                             VideoFrame &videoFrame) override {
    @autoreleasepool {
      AgoraVideoFrameObserver *strongObserverApple = observer;
      if (strongObserverApple) {
        AgoraVideoFrame *videoFrameApple = NativeToAppleVideoFrame(videoFrame);

        if (strongObserverApple.delegate != nil &&
            [strongObserverApple.delegate
                respondsToSelector:@selector(onPreEncodeVideoFrame:frame:)]) {
          return [strongObserverApple.delegate
              onPreEncodeVideoFrame:type
                              frame:videoFrameApple];
        }
      }
    }

    return false;
  }

  media::base::VIDEO_PIXEL_FORMAT getVideoFormatPreference() override {
    @autoreleasepool {
      AgoraVideoFrameObserver *strongObserverApple = observer;
      if (strongObserverApple) {
        if (strongObserverApple.delegate != nil &&
            [strongObserverApple.delegate
                respondsToSelector:@selector(getVideoFormatPreference)]) {
          return (
              media::base::VIDEO_PIXEL_FORMAT)[strongObserverApple.delegate
                                                   getVideoFormatPreference];
        }
      }
    }
    return IVideoFrameObserver::getVideoFormatPreference();
  }

  bool getRotationApplied() override {
    @autoreleasepool {
      AgoraVideoFrameObserver *strongObserverApple = observer;
      if (strongObserverApple) {
        if (strongObserverApple.delegate != nil &&
            [strongObserverApple.delegate
                respondsToSelector:@selector(getRotationApplied)]) {
          return [strongObserverApple.delegate getRotationApplied];
        }
      }
    }
    return IVideoFrameObserver::getRotationApplied();
  }

  bool getMirrorApplied() override {
    @autoreleasepool {
      AgoraVideoFrameObserver *strongObserverApple = observer;
      if (strongObserverApple) {
        if (strongObserverApple.delegate != nil &&
            [strongObserverApple.delegate
                respondsToSelector:@selector(getMirrorApplied)]) {
          return [strongObserverApple.delegate getMirrorApplied];
        }
      }
    }
    return IVideoFrameObserver::getMirrorApplied();
  }

  uint32_t getObservedFramePosition() override {
    @autoreleasepool {
      AgoraVideoFrameObserver *strongObserverApple = observer;
      if (strongObserverApple) {
        if (strongObserverApple.delegate != nil &&
            [strongObserverApple.delegate
                respondsToSelector:@selector(getObservedFramePosition)]) {
          return [strongObserverApple.delegate getObservedFramePosition];
        }
      }
    }
    return IVideoFrameObserver::getObservedFramePosition();
  }

  bool
  onMediaPlayerVideoFrame(media::IVideoFrameObserver::VideoFrame &videoFrame,
                          int mediaPlayerId) override {
    return false;
  }

  bool onTranscodedVideoFrame(
      media::IVideoFrameObserver::VideoFrame &videoFrame) override {
    return false;
  }

  media::IVideoFrameObserver::VIDEO_FRAME_PROCESS_MODE
  getVideoFrameProcessMode() override {
    return PROCESS_MODE_READ_WRITE;
  }

private:
  AgoraVideoFrame *NativeToAppleVideoFrame(VideoFrame &videoFrame) {
    AgoraVideoFrame *videoFrameApple = [[AgoraVideoFrame alloc] init];
    // Only support VIDEO_PIXEL_I420/VIDEO_PIXEL_RGBA/VIDEO_PIXEL_I422 for
    // demostration purpose. If you need more format, please check the value of
    // type of `VIDEO_PIXEL_FORMAT`
    videoFrameApple.type = (AgoraVideoFrameType)videoFrame.type;
    videoFrameApple.width = videoFrame.width;
    videoFrameApple.height = videoFrame.height;
    videoFrameApple.yStride = videoFrame.yStride;
    videoFrameApple.uStride = videoFrame.uStride;
    videoFrameApple.vStride = videoFrame.vStride;
    videoFrameApple.yBuffer = videoFrame.yBuffer;
    videoFrameApple.uBuffer = videoFrame.uBuffer;
    videoFrameApple.vBuffer = videoFrame.vBuffer;
    videoFrameApple.rotation = videoFrame.rotation;
    videoFrameApple.renderTimeMs = videoFrame.renderTimeMs;
    videoFrameApple.avsync_type = videoFrame.avsync_type;
    return videoFrameApple;
  }

private:
  __weak AgoraVideoFrameObserver *observer;
  long long engineHandle;
};
} // namespace agora

@interface AgoraVideoFrameObserver ()
@property(nonatomic, assign) agora::VideoFrameObserver *observer;
@end

@implementation AgoraVideoFrameObserver
- (instancetype)initWithEngineHandle:(NSUInteger)engineHandle {
  if (self = [super init]) {
    self.engineHandle = engineHandle;
  }
  return self;
}

- (void)registerVideoFrameObserver {
  if (!_observer) {
    _observer =
        new agora::VideoFrameObserver(_engineHandle, (__bridge void *)self);
  }
}

- (void)unregisterVideoFrameObserver {
  if (_observer) {
    delete _observer;
    _observer = nullptr;
  }
}
@end
