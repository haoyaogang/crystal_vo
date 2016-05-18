 package tv.danmaku.ijk.media.publisher;
 
 import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.os.Build;
import android.os.Build.VERSION;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;

 import java.io.IOException;
import java.util.List;
 
 public class VideoRecorder
   implements SurfaceHolder.Callback, Camera.PreviewCallback
 {
   private final String subTAG = "Crystal.VideoRecorder";
   private Camera mCamera = null;
   private SurfaceHolder mSurfaceHolder = null;
   private int mCameraNum = 0;
   private int mCameraId = 0;
 
   private int mUIOrientation = 0;
   private byte[] mPreviewBuffer;
   public int preWidth = 1280;
   public int preHeight = 720;
   private boolean[] cameraAutoFocus;
   private boolean[] cameraFlashModes;
   boolean isCameraInfoDetected = false;
   boolean isPause = false;
 
   public int startVideoRecorder(SurfaceView preview, int uiOrientation, int camId) {
     switch (uiOrientation) {
     case 0:
       this.mUIOrientation = 90;
       break;
     case 1:
       this.mUIOrientation = 0;
       break;
     case 2:
       this.mUIOrientation = 270;
       break;
     case 3:
       this.mUIOrientation = 180;
     }
 
     if (!this.isCameraInfoDetected)
     {
       this.mCameraNum = Camera.getNumberOfCameras();
       if (this.mCameraNum == 0) {
         Log.e("Crystal.VideoRecorder", "Get number of cameras error mCameraNum:" + this.mCameraNum);
         return -1;
       }
       this.cameraAutoFocus = new boolean[this.mCameraNum];
       this.cameraFlashModes = new boolean[this.mCameraNum];
 
       for (int i = 0; i < this.mCameraNum; i++) {
         this.cameraAutoFocus[i] = true;
         this.cameraFlashModes[i] = false;
         try
         {
           this.mCamera = Camera.open(i);
         } catch (Exception e) {
           Log.e("Crystal.VideoRecorder", "Camera id:" + i + " open error.");
           continue;
         }
 
         try
         {
           Camera.Parameters parameters = this.mCamera.getParameters();
           parameters.setPreviewSize(1280, 720);
           this.mCamera.setParameters(parameters);
           Log.i("Crystal.VideoRecorder", "Camera id:" + i + " supported 720p preview");
         } catch (Exception e) {
           this.preWidth = 640;
           this.preHeight = 480;
           Log.w("Crystal.VideoRecorder", "Camera id:" + i + " unsupported 720p preview,all using VGA");
         }
 
         try
         {
           if (Build.VERSION.SDK_INT < 14) {
             throw new Exception("Unsupported API version for FOCUS_MODE_CONTINUOUS_VIDEO");
           }
           Camera.Parameters parameters = this.mCamera.getParameters();
           parameters.setFocusMode("continuous-video");
           this.mCamera.setParameters(parameters);
           Log.i("Crystal.VideoRecorder", "Camera id:" + i + " supported FOCUS_MODE_CONTINUOUS_VIDEO");
         }
         catch (Exception e) {
           this.cameraAutoFocus[i] = false;
           Log.w("Crystal.VideoRecorder", "Camera id:" + i + " unsupported FOCUS_MODE_CONTINUOUS_VIDEO");
         }
 
         try
         {
           Camera.Parameters parameters = this.mCamera.getParameters();
           List flashModes = parameters.getSupportedFlashModes();
           if ((flashModes.contains("torch")) && (flashModes.contains("off"))) {
             this.cameraFlashModes[i] = true;
           }
           Log.i("Crystal.VideoRecorder", "Camera id:" + i + " supported set flash mode.");
         } catch (Exception e) {
           Log.w("Crystal.VideoRecorder", "Camera id:" + i + " unsupported set flash mode.");
         }
 
         this.mCamera.release();
         this.mCamera = null;
         this.isCameraInfoDetected = true;
       }
 
       if (!this.isCameraInfoDetected) {
         Log.e("Crystal.VideoRecorder", "VideoRecorder 启动错误，可能是权限未开启");
         return -1;
       }
 
     }
 
     releaseCamera();
     if (openCamera(camId) != 0) {
       return -1;
     }
     try
     {
       preview.getHolder().addCallback(this);
       preview.getHolder().setKeepScreenOn(true);
     } catch (Exception e) {
       return -1;
     }
     this.isPause = false;
     return 0;
   }
 
   public void pause() {
     this.isPause = true;
   }
 
   public void resume() {
     this.isPause = false;
   }
 
   public int switchCamera() {
     if (this.mCameraNum == 1) {
       return 0;
     }
     releaseCamera();
     if (openCamera(this.mCameraId == 0 ? 1 : 0) != 0)
       return -1;
     try
     {
       this.mCamera.setPreviewCallbackWithBuffer(this);
       this.mCamera.setPreviewDisplay(this.mSurfaceHolder);
       this.mCamera.startPreview();
     } catch (IOException e) {
       e.printStackTrace();
       return -1;
     }
 
     return this.mCameraId;
   }
 
   public int setFlashEnable(boolean flashEnable) {
     int ret = -1;
     if (this.cameraFlashModes[this.mCameraId] != false) {
       Camera.Parameters parameters = this.mCamera.getParameters();
       if (flashEnable) {
         parameters.setFlashMode("torch");
         ret = 1;
       } else {
         parameters.setFlashMode("off");
         ret = 0;
       }
       this.mCamera.setParameters(parameters);
     }
     return ret;
   }
 
   public int stopVideoRecorder() {
     releaseCamera();
     return 0;
   }
 
   public int setCameraOrientation(int interfaceOrientation) {
     switch (interfaceOrientation) {
     case 0:
       this.mUIOrientation = 90;
       break;
     case 1:
       this.mUIOrientation = 0;
       break;
     case 2:
       this.mUIOrientation = 270;
       break;
     case 3:
       this.mUIOrientation = 180;
     }
 
     this.mCamera.setDisplayOrientation(this.mUIOrientation);
     return 0;
   }
 
   private int openCamera(int id)
   {
     try
     {
       this.mCamera = Camera.open(id);
     } catch (Exception e) {
       Log.e("Crystal.VideoRecorder", "Camera id:" + id + " open error:" + e.getMessage());
       return -1;
     }
     this.mCameraId = id;
     Camera.Parameters parameters = this.mCamera.getParameters();
     parameters.setPreviewFormat(ImageFormat.NV21);
 
     int previewBufferSize = this.preWidth * this.preHeight * 3 / 2;
     this.mPreviewBuffer = new byte[previewBufferSize];
     Log.i("Crystal.VideoRecorder", "Camera Preview set to width:" + this.preWidth + " height:" + this.preHeight + " PreviewBufferSize:" + previewBufferSize);
     parameters.setPreviewSize(this.preWidth, this.preHeight);
     if (this.cameraAutoFocus[id] != false) {
       parameters.setFocusMode("continuous-video");
     }
     this.mCamera.setParameters(parameters);
     this.mCamera.addCallbackBuffer(this.mPreviewBuffer);
     this.mCamera.setDisplayOrientation(this.mUIOrientation);
     return 0;
   }
 
   private void releaseCamera() {
     if (this.mCamera != null) {
       this.mCamera.stopPreview();
       this.mCamera.release();
       this.mCamera = null;
     }
   }
 
   public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3)
   {
   }
 
   public void surfaceCreated(SurfaceHolder arg0)
   {
     if (this.mCamera != null)
       try {
         this.mCamera.stopPreview();
         this.mCamera.setPreviewCallbackWithBuffer(this);
         this.mCamera.setPreviewDisplay(arg0);
         this.mCamera.startPreview();
         this.mSurfaceHolder = arg0;
       } catch (IOException e) {
         e.printStackTrace();
       }
   }
 
   public void surfaceDestroyed(SurfaceHolder arg0)
   {
     if (this.mCamera != null)
       try {
         this.mCamera.stopPreview();
         this.mCamera.setPreviewDisplay(null);
       } catch (IOException e) {
         e.printStackTrace();
       }
   }
 
   @Override
   public void onPreviewFrame(byte[] data, Camera camera)
   {
     if (!this.isPause) {
    	 LivePublisherJni.getInstance().putVideoData(data, data.length);
     }
     camera.addCallbackBuffer(data);
   }
 }
