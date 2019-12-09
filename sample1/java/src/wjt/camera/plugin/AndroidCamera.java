package wjt.camera.plugin;

import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.util.Log;
import java.io.IOException;
import java.util.List;

public class AndroidCamera implements Camera.PreviewCallback {
	private static final String TAG = AndroidCamera.class.getSimpleName();
	private Camera camera;
	private Camera.Parameters cameraParams;
	private boolean isStreaming = false;
	private byte[] dataBuffer;
	private static AndroidCamera androidCameraInstance;
	private static final int MAGIC_TEXTURE_ID = 6;
	private SurfaceTexture gSurfaceTexture;

	private AndroidCamera() {
		Log.d(TAG, "Constructor");
	}

	public static synchronized AndroidCamera getInstance() {
		if (androidCameraInstance == null) {
			androidCameraInstance = new AndroidCamera();
		}
		return androidCameraInstance;
	}

	public boolean openCamera() {
		Log.d(TAG, "openCamera()");

		if (this.camera == null) {
			int numOfCameras = Camera.getNumberOfCameras();
			Log.v(TAG, "openCamera(): Number of Cameras " + numOfCameras);

			for (int i = 0; i < numOfCameras; i++) {
				try {
					this.camera = Camera.open(i);
				} catch (Exception e) {
					Log.d(TAG, "openCamera(): Exception while opening camera #" + i + ": " + e.toString());
				}
			}
		}

		if (this.camera == null) {
			return false;
		}

		cameraOpened();

		return true;
	}

	public boolean closeCamera() {
		Log.d(TAG, "closeCamera()");

		if (this.camera != null) {
			try {
				this.camera.setPreviewTexture(null);
			} catch (IOException e) {
				e.printStackTrace();
			}
			this.camera.setPreviewCallback(null);
			this.camera.stopPreview();
			this.isStreaming = false;
			this.camera.release();
			this.camera = null;
			this.dataBuffer = null;
		}

		cameraClosed();

		return true;
	}

	public boolean startCamera() {
		Log.d(TAG, "startCamera()");

		if (this.camera != null) {
			this.cameraParams = this.camera.getParameters();

			boolean foundNV21 = false;
			List<Integer> formats = this.cameraParams.getSupportedPreviewFormats();
			Log.d(TAG, "Preview format supported count: " + formats.size());
			for (int i = 0; i < formats.size(); i++) {
				int format = ((Integer) formats.get(i)).intValue();
				Log.d(TAG, "startCamera: Preview format supported: " + format + " bits per pixel: "
						+ ImageFormat.getBitsPerPixel(format));
				if (format == ImageFormat.NV21) {
					/*ImageFormat.NV21 == 17*/
					this.cameraParams.setPreviewFormat(ImageFormat.NV21);
					foundNV21 = true;
//					break;
				}
			}

			if (!foundNV21) {
				Log.d(TAG, "startCamera: Camera doesn't support ImageFormat.NV21. Can't use it.");
				return false;
			}

			List<int[]> fpsRange = this.cameraParams.getSupportedPreviewFpsRange();
			for (int i = 0; i < fpsRange.size(); i++) {
				int[] fps = (int[]) fpsRange.get(i);
				String fpsString = "startCamera: Supported preview FPS";
				for (int j = 0; j < fps.length; j++) {
					fpsString = fpsString + " " + fps[j];
				}
				Log.d(TAG, fpsString);
			}

			this.camera.setParameters(this.cameraParams);

			Camera.Size previewSize = this.cameraParams.getPreviewSize();

			int bytesPerPixel = 3;

			this.dataBuffer = new byte[previewSize.width * previewSize.height * bytesPerPixel / 2];
			this.camera.addCallbackBuffer(this.dataBuffer);

			if (this.gSurfaceTexture == null) {
				this.gSurfaceTexture = new SurfaceTexture(MAGIC_TEXTURE_ID);
			}
			try {
				this.camera.setPreviewTexture(this.gSurfaceTexture);
			} catch (IOException e) {
				e.printStackTrace();
			}

			this.camera.setPreviewCallback(this);
			this.camera.startPreview();

			this.isStreaming = true;

			this.cameraParams = this.camera.getParameters();

			int fps = cameraParams.getPreviewFrameRate();
			Log.i(TAG, "startCamera: PreviewSize With = " + previewSize.width
					+ " Height = " + previewSize.height + " fps:" + fps);

			cameraStarted(previewSize.width, previewSize.height, fps);
			return true;
		}

		return false;
	}

	public boolean stopCamera() {
		Log.d(TAG, "stopCamera()");

		if (this.camera != null) {
			this.camera.setPreviewCallback(null);
			this.camera.stopPreview();
			this.isStreaming = false;
		}
		cameraStopped();
		return true;
	}

	public PreviewMode[] getAvailableCameraModes() {
		Log.d(TAG, "getAvailableCameraModes()");

		this.cameraParams = this.camera.getParameters();

		List<Integer> formats = this.cameraParams.getSupportedPreviewFormats();
		Log.d(TAG, "Preview format supported count: " + formats.size());
		for (int i = 0; i < formats.size(); i++) {
			int format = ((Integer) formats.get(i)).intValue();
			Log.d(TAG, "Preview format supported: " + format + " bits per pixel: " + ImageFormat.getBitsPerPixel(format));
		}

		List<int[]> fpsRange = this.cameraParams.getSupportedPreviewFpsRange();
		for (int i = 0; i < fpsRange.size(); i++) {
			int[] fps = (int[]) fpsRange.get(i);
			String fpsString = "Supported preview FPS";
			for (int j = 0; j < fps.length; j++) {
				fpsString = fpsString + " " + fps[j];
			}
			Log.d(TAG, fpsString);
		}

		int fps = this.cameraParams.getPreviewFrameRate();
		List<Size> supportedPreviewSizes = this.cameraParams.getSupportedPreviewSizes();
		if ((supportedPreviewSizes == null) || (supportedPreviewSizes.size() <= 0)) {
			PreviewMode[] modes = new PreviewMode[1];
			Camera.Size previewSize = this.cameraParams.getPreviewSize();
			String sizeString = "Supported preview size ";
			sizeString = sizeString + "width:" + previewSize.width + " height:" + previewSize.height;
			Log.d(TAG, sizeString);
			modes[0] = new PreviewMode( previewSize.width, previewSize.height, fps);
			return modes;
		}

		PreviewMode[] modes = new PreviewMode[supportedPreviewSizes.size()];

		for (int i = 0; i < supportedPreviewSizes.size(); i++) {
			Camera.Size size = (Camera.Size) supportedPreviewSizes.get(i);
			String sizeString = "Supported preview size ";
			sizeString = sizeString + "width:" + size.width + " height:" + size.height; 
			Log.d(TAG, sizeString);
			modes[i] = new PreviewMode(size.width, size.height, fps);
		}

		return modes;
	}

	public PreviewMode getCameraMode() {
		Log.d(TAG, "getCameraMode()");
		Camera.Parameters cameraParams = this.camera.getParameters();
		Camera.Size previewSize = cameraParams.getPreviewSize();
		int fps = cameraParams.getPreviewFrameRate();
		return new PreviewMode(previewSize.width, previewSize.height, fps);
	}

	public void setCameraMode(PreviewMode mode) {
		Log.d(TAG, String.format("setCameraMode(%d,%d,%d)",mode.getWidth(),mode.getHeight(),mode.getFps()));
		this.cameraParams.setPreviewSize(mode.getWidth(), mode.getHeight());
		this.camera.setParameters(this.cameraParams);
		if (this.isStreaming) {
			stopCamera();
			startCamera();
		}
	}

	public void onPreviewFrame(byte[] data, Camera myCamera) {
		Camera.Parameters parameters = this.camera.getParameters();
		if ((data != null) && (parameters.getPreviewFormat() == ImageFormat.NV21)) {
			int width = parameters.getPreviewSize().width;
			int height = parameters.getPreviewSize().height;

			processCameraFrame(width, height, data);
		}

		this.camera.addCallbackBuffer(this.dataBuffer);
	}

	private native void cameraStarted(int width, int height, int fps);

	private native void cameraStopped();

	private native void cameraOpened();

	private native void cameraClosed();

	private native void processCameraFrame(int width, int height, byte[] imgdata);
}
