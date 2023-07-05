package io.agora.agora_rtc_rawdata

import android.graphics.Matrix
import android.opengl.GLES20
import androidx.annotation.NonNull
import io.agora.base.TextureBufferHelper
import io.agora.base.internal.video.GlRectDrawer
import io.agora.base.internal.video.GlTextureFrameBuffer
import io.agora.base.internal.video.RendererCommon
import io.agora.rtc.rawdata.base.AudioFrame
import io.agora.rtc.rawdata.base.IAudioFrameObserver
import io.agora.rtc.rawdata.base.IVideoFrameObserver
import io.agora.rtc.rawdata.base.VideoFrame
import io.agora.rtc2.gl.EglBaseProvider
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import java.nio.ByteBuffer
import java.util.*
import java.util.concurrent.Callable

/** AgoraRtcRawdataPlugin */
class AgoraRtcRawdataPlugin : FlutterPlugin, MethodCallHandler {
  /// The MethodChannel that will the communication between Flutter and native Android
  ///
  /// This local reference serves to register the plugin with the Flutter Engine and unregister it
  /// when the Flutter Engine is detached from the Activity
  private lateinit var channel: MethodChannel

  private var audioObserver: IAudioFrameObserver? = null
  private var videoObserver: IVideoFrameObserver? = null
  private var textureBufferHelper: TextureBufferHelper? = null
  private var textureDrawer: GlRectDrawer? = null
  private var textureFrameBuffer: GlTextureFrameBuffer? = null

  override fun onAttachedToEngine(@NonNull flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
    channel = MethodChannel(flutterPluginBinding.binaryMessenger, "agora_rtc_rawdata")
    channel.setMethodCallHandler(this)
  }

  override fun onMethodCall(@NonNull call: MethodCall, @NonNull result: Result) {
    when (call.method) {
      "registerAudioFrameObserver" -> {
        if (audioObserver == null) {
          audioObserver = object : IAudioFrameObserver((call.arguments as Number).toLong()) {
            override fun onRecordAudioFrame(audioFrame: AudioFrame): Boolean {
              return true
            }

            override fun onPlaybackAudioFrame(audioFrame: AudioFrame): Boolean {
              return true
            }

            override fun onMixedAudioFrame(audioFrame: AudioFrame): Boolean {
              return true
            }

            override fun onPlaybackAudioFrameBeforeMixing(uid: Int, audioFrame: AudioFrame): Boolean {
              return true
            }
          }
        }
        audioObserver?.registerAudioFrameObserver()
        result.success(null)
      }
      "unregisterAudioFrameObserver" -> {
        audioObserver?.let {
          it.unregisterAudioFrameObserver()
          audioObserver = null
        }
        result.success(null)
      }
      "registerVideoFrameObserver" -> {
        if (videoObserver == null) {
          videoObserver = object : IVideoFrameObserver((call.arguments as Number).toLong()) {
            override fun onCaptureVideoFrame(sourceType: Int, videoFrame: VideoFrame): Boolean {
              if (videoFrame.type != VideoFrame.VideoFrameType.TEXTURE_OES.value) {
                return false
              }
              if (textureBufferHelper == null) {
                textureBufferHelper = TextureBufferHelper.create("agora_rawdata", EglBaseProvider.instance().rootEglBase.eglBaseContext);
                textureBufferHelper?.invoke(Callable {
                  GLES20.glViewport(
                    0,
                    0,
                    videoFrame.width,
                    videoFrame.height
                  )
                })
              }

              videoFrame.textureId = textureBufferHelper!!.invoke(Callable {
                return@Callable convertToRGBA(videoFrame.type, videoFrame.textureId, videoFrame.width, videoFrame.height,
                  videoFrame.rotation, videoFrame.textureMatrix, null)
              })
              videoFrame.setType(VideoFrame.VideoFrameType.TEXTURE_2D)
              videoFrame.textureMatrix = RendererCommon.convertMatrixFromAndroidGraphicsMatrix(Matrix())
              return true
            }
          }
        }
        videoObserver?.registerVideoFrameObserver()
        result.success(null)
      }
      "unregisterVideoFrameObserver" -> {
        videoObserver?.let {
          it.unregisterVideoFrameObserver()
          videoObserver = null
        }
        textureBufferHelper?.let {
          textureBufferHelper = null
          it.invoke {
            // release gl related resource
            textureDrawer?.release()
            textureFrameBuffer?.release()
            null
          }
          it.dispose()
        }

        result.success(null)
      }
      else -> result.notImplemented()
    }
  }

  fun convertToRGBA(
    textureType: Int,
    textureId: Int,
    frameWidth: Int,
    frameHeight: Int,
    rotation: Int,
    transformMatrix: FloatArray?,
    outBuffer: ByteBuffer?
  ): Int {
    if (textureType == VideoFrame.VideoFrameType.TEXTURE_2D.value) {
      return textureId
    }
    if (textureDrawer == null) {
      textureDrawer = GlRectDrawer()
    }
    if (textureFrameBuffer == null) {
      textureFrameBuffer =
        GlTextureFrameBuffer(
          GLES20.GL_RGBA
        )
    }
    textureFrameBuffer!!.setSize(
      frameWidth,
      frameHeight
    )
    // Bind the framebuffer.
    GLES20.glBindFramebuffer(
      GLES20.GL_FRAMEBUFFER,
      textureFrameBuffer!!.getFrameBufferId()
    )
    // Draw video frame without rotation.
    GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)
    val renderMatrix = Matrix()
    renderMatrix.preTranslate(0.5f, 0.5f)
    renderMatrix.preRotate(rotation.toFloat())
    renderMatrix.preTranslate(-0.5f, -0.5f)
    renderMatrix.postConcat(RendererCommon.convertMatrixToAndroidGraphicsMatrix(transformMatrix))
    val finalGlMatrix = RendererCommon.convertMatrixFromAndroidGraphicsMatrix(renderMatrix)
    textureDrawer?.drawOes(
      textureId, finalGlMatrix, frameWidth, frameHeight, 0, 0,
      frameWidth, frameHeight
    )
    if (outBuffer != null && outBuffer.capacity() >= frameWidth * frameHeight * 4) {
      GLES20.glReadPixels(
        0, 0, frameWidth,
        frameHeight, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, outBuffer
      )
    }
    // Unbind the framebuffer.
    GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0)
    GLES20.glFlush()
    return textureFrameBuffer!!.getTextureId()
  }

  override fun onDetachedFromEngine(@NonNull binding: FlutterPlugin.FlutterPluginBinding) {
    channel.setMethodCallHandler(null)
  }

  companion object {
    // Used to load the 'native-lib' library on application startup.
    init {
      System.loadLibrary("cpp")
    }
  }
}
