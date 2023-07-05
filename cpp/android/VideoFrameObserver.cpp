#include "VideoFrameObserver.h"

#include "VMUtil.h"

namespace agora {
VideoFrameObserver::VideoFrameObserver(JNIEnv *env, jobject jCaller,
                                       long long engineHandle)
    : jCallerRef(env->NewGlobalRef(jCaller)), engineHandle(engineHandle) {
  jclass jCallerClass = env->GetObjectClass(jCallerRef);
  jOnCaptureVideoFrame =
      env->GetMethodID(jCallerClass, "onCaptureVideoFrame",
                       "(ILio/agora/rtc/rawdata/base/VideoFrame;)Z");
  jOnRenderVideoFrame =
      env->GetMethodID(jCallerClass, "onRenderVideoFrame",
                       "(ILio/agora/rtc/rawdata/base/VideoFrame;)Z");
  jOnPreEncodeVideoFrame =
      env->GetMethodID(jCallerClass, "onPreEncodeVideoFrame",
                       "(ILio/agora/rtc/rawdata/base/VideoFrame;)Z");
  jGetVideoFormatPreference = env->GetMethodID(
      jCallerClass, "getVideoFormatPreference",
      "()Lio/agora/rtc/rawdata/base/VideoFrame$VideoFrameType;");
  jGetRotationApplied =
      env->GetMethodID(jCallerClass, "getRotationApplied", "()Z");
  jGetMirrorApplied = env->GetMethodID(jCallerClass, "getMirrorApplied", "()Z");
  jGetObservedFramePosition =
      env->GetMethodID(jCallerClass, "getObservedFramePosition", "()I");

  env->DeleteLocalRef(jCallerClass);

  jclass jVideoFrame = env->FindClass("io/agora/rtc/rawdata/base/VideoFrame");
  jVideoFrameClass = (jclass)env->NewGlobalRef(jVideoFrame);
  jVideoFrameInit =
      env->GetMethodID(jVideoFrameClass, "<init>", "(IIIIII[B[B[BII[FJI)V");
  jVideoFrameGetType =
      env->GetMethodID(jVideoFrameClass, "getType", "()I");
  jVideoFrameGetTextureId =
      env->GetMethodID(jVideoFrameClass, "getTextureId", "()I");
  jVideoFrameGetMatrix =
      env->GetMethodID(jVideoFrameClass, "getTextureMatrix", "()[F");
  env->DeleteLocalRef(jVideoFrame);

  jclass videoFrameType =
      env->FindClass("io/agora/rtc/rawdata/base/VideoFrame$VideoFrameType");
  jVideoFrameTypeClass = (jclass)env->NewGlobalRef(videoFrameType);
  jGetValue = env->GetMethodID(jVideoFrameTypeClass, "getValue", "()I");
  env->DeleteLocalRef(videoFrameType);

  env->GetJavaVM(&jvm);

  auto rtcEngine = reinterpret_cast<rtc::IRtcEngine *>(engineHandle);
  if (rtcEngine) {
    util::AutoPtr<media::IMediaEngine> mediaEngine;
    mediaEngine.queryInterface(rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
    if (mediaEngine) {
      mediaEngine->registerVideoFrameObserver(this);
    }
  }
}

VideoFrameObserver::~VideoFrameObserver() {
  auto rtcEngine = reinterpret_cast<rtc::IRtcEngine *>(engineHandle);
  if (rtcEngine) {
    util::AutoPtr<media::IMediaEngine> mediaEngine;
    mediaEngine.queryInterface(rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
    if (mediaEngine) {
      mediaEngine->registerVideoFrameObserver(nullptr);
    }
  }

  AttachThreadScoped ats(jvm);

  ats.env()->DeleteGlobalRef(jCallerRef);
  jOnCaptureVideoFrame = nullptr;
  jOnRenderVideoFrame = nullptr;
  jOnPreEncodeVideoFrame = nullptr;
  jGetVideoFormatPreference = nullptr;
  jGetRotationApplied = nullptr;
  jGetMirrorApplied = nullptr;
  jGetObservedFramePosition = nullptr;

  ats.env()->DeleteGlobalRef(jVideoFrameClass);
  jVideoFrameInit = nullptr;
  jVideoFrameGetType = nullptr;
  jVideoFrameGetTextureId = nullptr;
  jVideoFrameGetMatrix = nullptr;

  ats.env()->DeleteGlobalRef(jVideoFrameTypeClass);
  jGetValue = nullptr;
}

bool VideoFrameObserver::onCaptureVideoFrame(agora::rtc::VIDEO_SOURCE_TYPE type,
                                             VideoFrame &videoFrame) {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  std::vector<jbyteArray> j_array;
  jfloatArray j_matrix = nullptr;
  if (videoFrame.type == agora::media::base::VIDEO_TEXTURE_OES || videoFrame.type == agora::media::base::VIDEO_TEXTURE_2D) {
    j_matrix = env->NewFloatArray(16);
    env->SetFloatArrayRegion(j_matrix, 0, 16,
                           reinterpret_cast<const jfloat*>(&videoFrame.matrix[0]));
  } else {
    j_array = NativeToJavaByteArray(env, videoFrame);
  }

  jobject obj = NativeToJavaVideoFrame(env, videoFrame, j_array, j_matrix);
  jboolean ret =
      env->CallBooleanMethod(jCallerRef, jOnCaptureVideoFrame, type, obj);
  for (int i = 0; i < j_array.size(); ++i) {
    jbyteArray jByteArray = j_array[i];
    void *buffer = nullptr;
    if (i == 0) {
      buffer = videoFrame.yBuffer;
    } else if (i == 1) {
      buffer = videoFrame.uBuffer;
    } else if (i == 2) {
      buffer = videoFrame.vBuffer;
    }
    env->GetByteArrayRegion(jByteArray, 0, env->GetArrayLength(jByteArray),
                            static_cast<jbyte *>(buffer));
    env->DeleteLocalRef(jByteArray);
  }
  if (j_matrix) {
    jfloat* j_float = env->GetFloatArrayElements(j_matrix, nullptr);
    env->ReleaseFloatArrayElements(j_matrix, j_float, 0);
    env->DeleteLocalRef(j_matrix);
  }

  jint new_type = env->CallIntMethod(obj, jVideoFrameGetType);
  if (new_type == agora::media::base::VIDEO_TEXTURE_OES || new_type == agora::media::base::VIDEO_TEXTURE_2D) {
      videoFrame.type = static_cast<agora::media::base::VIDEO_PIXEL_FORMAT>(new_type);
      videoFrame.textureId = env->CallIntMethod(obj, jVideoFrameGetTextureId);
      jfloatArray new_matrix = static_cast<jfloatArray>(env->CallObjectMethod(obj, jVideoFrameGetMatrix));
      jfloat* new_float = env->GetFloatArrayElements(new_matrix, nullptr);
      memcpy(videoFrame.matrix, new_float, sizeof(videoFrame.matrix));
      env->ReleaseFloatArrayElements(new_matrix, new_float, 0);
      env->DeleteLocalRef(new_matrix);
  }

  env->DeleteLocalRef(obj);
  return ret;
}

bool VideoFrameObserver::onRenderVideoFrame(const char *channelId,
                                            rtc::uid_t remoteUid,
                                            VideoFrame &videoFrame) {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  std::vector<jbyteArray> arr = NativeToJavaByteArray(env, videoFrame);
  jobject obj = NativeToJavaVideoFrame(env, videoFrame, arr, nullptr);
  jboolean ret =
      env->CallBooleanMethod(jCallerRef, jOnRenderVideoFrame, remoteUid, obj);
  for (int i = 0; i < arr.size(); ++i) {
    jbyteArray jByteArray = arr[i];
    void *buffer = nullptr;
    if (i == 0) {
      buffer = videoFrame.yBuffer;
    } else if (i == 1) {
      buffer = videoFrame.uBuffer;
    } else if (i == 2) {
      buffer = videoFrame.vBuffer;
    }
    env->GetByteArrayRegion(jByteArray, 0, env->GetArrayLength(jByteArray),
                            static_cast<jbyte *>(buffer));
    env->DeleteLocalRef(jByteArray);
  }
  env->DeleteLocalRef(obj);
  return ret;
}

bool VideoFrameObserver::onPreEncodeVideoFrame(
    agora::rtc::VIDEO_SOURCE_TYPE type, VideoFrame &videoFrame) {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  std::vector<jbyteArray> arr = NativeToJavaByteArray(env, videoFrame);
  jobject obj = NativeToJavaVideoFrame(env, videoFrame, arr, nullptr);
  jboolean ret =
      env->CallBooleanMethod(jCallerRef, jOnPreEncodeVideoFrame, type, obj);
  for (int i = 0; i < arr.size(); ++i) {
    jbyteArray jByteArray = arr[i];
    void *buffer = nullptr;
    if (i == 0) {
      buffer = videoFrame.yBuffer;
    } else if (i == 1) {
      buffer = videoFrame.uBuffer;
    } else if (i == 2) {
      buffer = videoFrame.vBuffer;
    }
    env->GetByteArrayRegion(jByteArray, 0, env->GetArrayLength(jByteArray),
                            static_cast<jbyte *>(buffer));
    env->DeleteLocalRef(jByteArray);
  }
  env->DeleteLocalRef(obj);
  return ret;
}

media::base::VIDEO_PIXEL_FORMAT VideoFrameObserver::getVideoFormatPreference() {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  jobject obj = env->CallObjectMethod(jCallerRef, jGetVideoFormatPreference);
  jint ret = env->CallIntMethod(obj, jGetValue);
  env->DeleteLocalRef(obj);
  return (media::base::VIDEO_PIXEL_FORMAT)ret;
}

bool VideoFrameObserver::getRotationApplied() {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  jboolean ret = env->CallBooleanMethod(jCallerRef, jGetRotationApplied);
  return ret;
}

bool VideoFrameObserver::getMirrorApplied() {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  jboolean ret = env->CallBooleanMethod(jCallerRef, jGetMirrorApplied);
  return ret;
}

uint32_t VideoFrameObserver::getObservedFramePosition() {
  AttachThreadScoped ats(jvm);
  JNIEnv *env = ats.env();
  jint ret = env->CallIntMethod(jCallerRef, jGetObservedFramePosition);
  return ret;
}

std::vector<jbyteArray>
VideoFrameObserver::NativeToJavaByteArray(JNIEnv *env, VideoFrame &videoFrame) {
  int yLength, uLength, vLength;
  switch (videoFrame.type) {
  case agora::media::base::VIDEO_PIXEL_FORMAT::VIDEO_PIXEL_I420: {
    yLength = videoFrame.yStride * videoFrame.height;
    uLength = videoFrame.uStride * videoFrame.height / 2;
    vLength = videoFrame.vStride * videoFrame.height / 2;
    break;
  }
  case agora::media::base::VIDEO_PIXEL_FORMAT::VIDEO_PIXEL_I422: {
    yLength = videoFrame.yStride * videoFrame.height;
    uLength = videoFrame.uStride * videoFrame.height;
    vLength = videoFrame.vStride * videoFrame.height;
    break;
  }
  case agora::media::base::VIDEO_PIXEL_FORMAT::VIDEO_PIXEL_RGBA: {
    yLength = videoFrame.width * videoFrame.height * 4;
    uLength = 0;
    vLength = 0;
    break;
  }
  }

  std::vector<jbyteArray> vector;

  if (videoFrame.yBuffer && yLength > 0) {
    jbyteArray jYArray = env->NewByteArray(yLength);
    env->SetByteArrayRegion(
        jYArray, 0, yLength,
        reinterpret_cast<const jbyte *>(videoFrame.yBuffer));
    vector.push_back(jYArray);
  }
  if (videoFrame.uBuffer && uLength > 0) {
    jbyteArray jUArray = env->NewByteArray(uLength);
    env->SetByteArrayRegion(
        jUArray, 0, uLength,
        reinterpret_cast<const jbyte *>(videoFrame.uBuffer));
    vector.push_back(jUArray);
  }
  if (videoFrame.vBuffer && vLength > 0) {
    jbyteArray jVArray = env->NewByteArray(vLength);
    env->SetByteArrayRegion(
        jVArray, 0, vLength,
        reinterpret_cast<const jbyte *>(videoFrame.vBuffer));
    vector.push_back(jVArray);
  }

  return vector;
}

jobject VideoFrameObserver::NativeToJavaVideoFrame(
    JNIEnv *env, media::IVideoFrameObserver::VideoFrame &videoFrame,
    std::vector<jbyteArray> jByteArray, jfloatArray jMatrix) {
    jbyteArray jYArray = nullptr;
    jbyteArray jUArray = nullptr;
    jbyteArray jVArray = nullptr;
    if (jByteArray.size() == 3) {
        jYArray = jByteArray[0];
        jUArray = jByteArray[1];
        jVArray = jByteArray[2];
    }
    return env->NewObject(jVideoFrameClass, jVideoFrameInit, (int)videoFrame.type,
                        videoFrame.width, videoFrame.height, videoFrame.yStride,
                        videoFrame.uStride, videoFrame.vStride, jYArray,
                        jUArray, jVArray, videoFrame.rotation,
                        videoFrame.textureId, jMatrix,
                        videoFrame.renderTimeMs, videoFrame.avsync_type);
}

bool VideoFrameObserver::onMediaPlayerVideoFrame(
    media::IVideoFrameObserver::VideoFrame &videoFrame, int mediaPlayerId) {
  return false;
}

bool VideoFrameObserver::onTranscodedVideoFrame(
    media::IVideoFrameObserver::VideoFrame &videoFrame) {
  return false;
}

media::IVideoFrameObserver::VIDEO_FRAME_PROCESS_MODE
VideoFrameObserver::getVideoFrameProcessMode() {
  return PROCESS_MODE_READ_WRITE;
}
} // namespace agora
