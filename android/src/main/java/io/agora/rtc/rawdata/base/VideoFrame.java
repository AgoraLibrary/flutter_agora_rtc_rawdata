package io.agora.rtc.rawdata.base;

public class VideoFrame {
  public enum VideoFrameType {
    YUV420(1),
    YUV422(16),
    RGBA(4),
    TEXTURE_2D(10),
    TEXTURE_OES(11);

    private final int value;

    VideoFrameType(int value) { this.value = value; }

    public int getValue() { return value; }
  }

  private VideoFrameType type;
  private int width;
  private int height;
  private int yStride;
  private int uStride;
  private int vStride;
  private byte[] yBuffer;
  private byte[] uBuffer;
  private byte[] vBuffer;
  private int rotation;
  private int textureId;
  private float[] textureMatrix;
  private long renderTimeMs;
  private int avsync_type;

  public VideoFrame(int type, int width, int height, int yStride, int uStride,
                    int vStride, byte[] yBuffer, byte[] uBuffer, byte[] vBuffer,
                    int rotation, int textureId, float[] matrix,
                    long renderTimeMs, int avsync_type) {
    // Only support VIDEO_PIXEL_I420/VIDEO_PIXEL_RGBA/VIDEO_PIXEL_I422 for
    // demostration purpose. If you need more format, please check the value of
    // type of `VIDEO_PIXEL_FORMAT`(locate in header
    // cpp/android/include/AgoraMediaBase.h)
    switch (type) {
    case 1: // VIDEO_PIXEL_I420
      this.type = VideoFrameType.YUV420;
      break;
    case 4: // VIDEO_PIXEL_RGBA
      this.type = VideoFrameType.RGBA;
      break;
    case 16: // VIDEO_PIXEL_I422
      this.type = VideoFrameType.YUV422;
      break;
    case 10:
      this.type = VideoFrameType.TEXTURE_2D;
      break;
    case 11:
      this.type = VideoFrameType.TEXTURE_OES;
      break;
    default:
      throw new IllegalArgumentException(
          "Only VIDEO_PIXEL_I420/VIDEO_PIXEL_I422/VIDEO_PIXEL_RGBA/Texture supported for demostration purpose.");
    }

    this.width = width;
    this.height = height;
    this.yStride = yStride;
    this.uStride = uStride;
    this.vStride = vStride;
    this.yBuffer = yBuffer;
    this.uBuffer = uBuffer;
    this.vBuffer = vBuffer;
    this.rotation = rotation;
    this.textureId = textureId;
    this.textureMatrix = matrix;
    this.renderTimeMs = renderTimeMs;
    this.avsync_type = avsync_type;
  }

  public int getType() { return type.getValue(); }

  public void setType(VideoFrameType type) { this.type = type; }

  public int getWidth() { return width; }

  public void setWidth(int width) { this.width = width; }

  public int getHeight() { return height; }

  public void setHeight(int height) { this.height = height; }

  public int getyStride() { return yStride; }

  public void setyStride(int yStride) { this.yStride = yStride; }

  public int getuStride() { return uStride; }

  public void setuStride(int uStride) { this.uStride = uStride; }

  public int getvStride() { return vStride; }

  public void setvStride(int vStride) { this.vStride = vStride; }

  public byte[] getyBuffer() { return yBuffer; }

  public void setyBuffer(byte[] yBuffer) { this.yBuffer = yBuffer; }

  public byte[] getuBuffer() { return uBuffer; }

  public void setuBuffer(byte[] uBuffer) { this.uBuffer = uBuffer; }

  public byte[] getvBuffer() { return vBuffer; }

  public void setvBuffer(byte[] vBuffer) { this.vBuffer = vBuffer; }

  public int getRotation() { return rotation; }

  public void setRotation(int rotation) { this.rotation = rotation; }

  public int getTextureId() { return textureId; }

  public void setTextureId(int textureId) { this.textureId = textureId; }

  public float[] getTextureMatrix() { return textureMatrix; }

  public void setTextureMatrix(float[] matrix) { this.textureMatrix = matrix; }

  public long getRenderTimeMs() { return renderTimeMs; }

  public void setRenderTimeMs(long renderTimeMs) {
    this.renderTimeMs = renderTimeMs;
  }

  public int getAvsync_type() { return avsync_type; }

  public void setAvsync_type(int avsync_type) {
    this.avsync_type = avsync_type;
  }
}
