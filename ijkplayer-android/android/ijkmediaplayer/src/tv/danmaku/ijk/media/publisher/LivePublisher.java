 package tv.danmaku.ijk.media.publisher;
 
 import android.content.Context;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.os.Handler;
import android.util.Log;
import android.view.SurfaceView;
 
 public class LivePublisher
 {
   private static final String subTAG = "NodeMedia.LivePublisher";
   private static final String TAG = LivePublisher.class.getSimpleName();
   private static LivePublisher sInstance;
   private LivePublishDelegate mLivePublishDelegate = null;
   private Context mContext;
 
   private AudioRecorder mAudioRecorder = null;
   private VideoRecorder mVideoRecorder = null;
   private AudioManager am = null;
   public static final int AAC_PROFILE_LC = 0;
   public static final int AAC_PROFILE_HE = 1;
   public static final int AVC_PROFILE_BASELINE = 0;
   public static final int AVC_PROFILE_MAIN = 1;
   public static final int CAMERA_BACK = 0;
   public static final int CAMERA_FRONT = 1;
   public static final int VIDEO_ORI_PORTRAIT = 0;
   public static final int VIDEO_ORI_LANDSCAPE = 1;
   public static final int VIDEO_ORI_PORTRAIT_REVERSE = 2;
   public static final int VIDEO_ORI_LANDSCAPE_REVERSE = 3;
 
   //audio configure
//   44100, 1, 1024
   private final int AUDIO_SAMPLE_RATE = 44100;
   private final int AUDIO_CHANNEL = 2;
   
   private final int AUDIO_FREAME_SIZE = 1024;
   /**
    * @return
    */
   public static LivePublisher getInstance()
   {
	   if (sInstance == null)
	   {
		   sInstance = new LivePublisher();
	   }
	   return sInstance;
   }
   
   /**
    *int rtmp
    * @param ctx
    */
   public  void initCore(Context ctx)
   {
	   mContext = ctx;
	   LivePublisherJni.getInstance().initRtmp();
   }
   /**
    * close rtmp
    */
   public void uninitCore()
   {
	   LivePublisherJni.getInstance().closeRtmp();
   }
   /**
    */
   public void initAudioRecorder()
   {
	   if(mContext == null)
	   {
		   Log.e(TAG, "crystal:  initAudioRecorder  ERROR because of mContext == null");
		   return;
	   }
	   
		sInstance.mAudioRecorder = new AudioRecorder();
		sInstance.am = ((AudioManager) mContext
				.getSystemService(Context.AUDIO_SERVICE));
		sInstance.am.requestAudioFocus(
				new AudioManager.OnAudioFocusChangeListener() {
					public void onAudioFocusChange(int focusChange) {
						Log.i(TAG, "onAudioFocusChange:"
								+ focusChange);
						if (focusChange == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT){
							LivePublisher.sInstance.mAudioRecorder.pause();
							LivePublisher.sInstance.mVideoRecorder.pause();
						} else if (focusChange == AudioManager.AUDIOFOCUS_GAIN) {
							new Handler().postDelayed(new Runnable() {
								public void run() {
									LivePublisher.sInstance.mAudioRecorder
											.resume();
								}
							}, 500L);
						}
					}
				}, 3, 1);
   }
   
   /**
    * 鍙戝竷audio鏁版嵁鍒皉tmp server
    * @param rtmpurl
    */
   public void startAudioRecorder()
   {
	   int audioRet = sInstance.mAudioRecorder.startAudioRecoder(AUDIO_SAMPLE_RATE, AUDIO_CHANNEL, AUDIO_FREAME_SIZE);
   }
   @Deprecated
   public void startAudioPublisher(String rtmpurl)
   {
	   int audioRet = sInstance.mAudioRecorder.startAudioRecoder(AUDIO_SAMPLE_RATE, AUDIO_CHANNEL, AUDIO_FREAME_SIZE);
	   
   }
   /**
    */
   public void stopAudioPublisher()
   {
	   sInstance.mAudioRecorder.releaseAudioRecorder();
   }
   public void pauseAudioPublisher()
   {
	   sInstance.mAudioRecorder.pause();
   }
   public void resumeAudioPublisher()
   {
	   sInstance.mAudioRecorder.resume();
   }
   /**
    */
   public void initVideoPublisher()
   {
	   sInstance.mVideoRecorder = new VideoRecorder();
	   
   }
   /**
    * @param rtmpurl
    */
   public void startAVPublisher(String rtmpurl)
   {
	   LivePublisherJni.getInstance().startAVPublisher(rtmpurl);
   }
   public void setCameraSize(int width,int height)
   {
	   LivePublisherJni.getInstance().setCameraSize(width, height);
   }
   public void startVideoPreview(SurfaceView cameraPreview, int interfaceOrientation, int camId)
   {
	   int videoRet = sInstance.mVideoRecorder.startVideoRecorder(cameraPreview, interfaceOrientation, camId);
	   
   }
   public void stopVideoPreview()
   {
	   sInstance.mVideoRecorder.stopVideoRecorder();
   }
   
   public static int init(Context ctx) {
     if (sInstance == null) {
       sInstance = new LivePublisher();
       sInstance.mAudioRecorder = new AudioRecorder();
       sInstance.mVideoRecorder = new VideoRecorder();
       sInstance.am = ((AudioManager)ctx.getSystemService("audio"));
       sInstance.am.requestAudioFocus(new AudioManager.OnAudioFocusChangeListener()
       {
         public void onAudioFocusChange(int focusChange) {
           Log.i("NodeMedia.LivePublisher", "onAudioFocusChange:" + focusChange);
           if (focusChange == -2) {
             LivePublisher.sInstance.mAudioRecorder.pause();
             LivePublisher.sInstance.mVideoRecorder.pause();
           } else if (focusChange == 1) {
             new Handler().postDelayed(new Runnable()
             {
               public void run()
               {
                 LivePublisher.sInstance.mAudioRecorder.resume();
                 LivePublisher.sInstance.mVideoRecorder.resume();
               }
             }
             , 500L);
           }
         }
       }
       , 3, 1);
 
       return sInstance.jniInit(ctx);
     }
     return 0;
   }
 
   public static int startPreview(SurfaceView cameraPreview, int interfaceOrientation, int camId) {
     int ret = 0;
     int audioRet = sInstance.mAudioRecorder.startAudioRecoder(44100, 1, 1024);
     int videoRet = sInstance.mVideoRecorder.startVideoRecorder(cameraPreview, interfaceOrientation, camId);
     if ((audioRet == -1) && (videoRet == -1)) {
       ret = -1;
       Log.e("NodeMedia.LivePublisher", "Microphone and Camera cannot be open. preview Error.");
     } else if (audioRet == -1) {
       Log.w("NodeMedia.LivePublisher", "Microphone cannot be open.");
       setAudioParam(0, 0);
     } else if (videoRet == -1) {
       Log.w("NodeMedia.LivePublisher", "Camera cannot be open.");
       setVideoParam(0, 0, 0, 0, 0);
     } else if (videoRet == 0) {
       setCameraParm(sInstance.mVideoRecorder.preWidth, sInstance.mVideoRecorder.preHeight, camId);
       setVideoOrientation(interfaceOrientation);
     }
     return ret;
   }
 
   public static int stopPreview() {
     sInstance.mVideoRecorder.stopVideoRecorder();
     sInstance.mAudioRecorder.releaseAudioRecorder();
     return 0;
   }
 
   public static int switchCamera() {
     int ret = sInstance.mVideoRecorder.switchCamera();
     if (ret != -1) {
       setCameraParm(sInstance.mVideoRecorder.preWidth, sInstance.mVideoRecorder.preHeight, ret);
     }
     return ret;
   }
 
   public static int setFlashEnable(boolean flashEnable) {
     return sInstance.mVideoRecorder.setFlashEnable(flashEnable);
   }
 
   public static int setCameraOrientation(int interfaceOrientation) {
     return sInstance.mVideoRecorder.setCameraOrientation(interfaceOrientation);
   }
 
   public static int startPublish(String rtmpUrl) {
     return startPublish(rtmpUrl, "", "");
   }
 
   public static int startPublish(String rtmpUrl, String pageUrl, String swfUrl) {
     return jniStartPublish(rtmpUrl, pageUrl, swfUrl);
   }
 
   public static void setDelegate(LivePublishDelegate delegate)
   {
     sInstance.mLivePublishDelegate = delegate;
   }
 
   private void onEvent(int event, String msg) {
     if (this.mLivePublishDelegate != null) {
       Log.d("NodeMedia.JAVA", "event:" + event + "  msg:" + msg);
       this.mLivePublishDelegate.onEventCallback(event, msg);
     }
   }
 
   private native int jniInit(Object paramObject);
 
   public static native int setAudioParam(int paramInt1, int paramInt2);
 
   public static native int setVideoParam(int paramInt1, int paramInt2, int paramInt3, int paramInt4, int paramInt5);
 
   private static native int setCameraParm(int paramInt1, int paramInt2, int paramInt3);
 
   public static native int setVideoOrientation(int paramInt);
 
   private static native int jniStartPublish(String paramString1, String paramString2, String paramString3);
 
   public static native int stopPublish();
 
   public static native int setDenoiseEnable(boolean paramBoolean);
 
   public static native int setMicEnable(boolean paramBoolean);
 
   public static native int setCamEnable(boolean paramBoolean);
 
   public static native int putVideoData(byte[] paramArrayOfByte, int paramInt);
 
 
   public static abstract interface LivePublishDelegate
   {
     public abstract void onEventCallback(int paramInt, String paramString);
   }
   
   
  
 }
