package wjt.camera.plugin;

public class PreviewMode
{
  private int width;
  private int height;
  private int fps;

  public PreviewMode(int width, int height, int fps)
  {
    this.width = width;
    this.height = height;
    this.fps = fps;
  }


  public int getWidth() {
    return this.width;
  }

  public int getHeight() {
    return this.height;
  }


  public int getFps() {
    return this.fps;
  }

  public String toString()
  {
    return "ImageStreamMode{width=" + this.width + ", height=" + this.height + ", fps=" + this.fps + '}';
  }
}
