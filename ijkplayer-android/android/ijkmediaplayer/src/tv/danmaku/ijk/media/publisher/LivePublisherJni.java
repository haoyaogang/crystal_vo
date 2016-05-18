package tv.danmaku.ijk.media.publisher;

public class LivePublisherJni {
	
	private static LivePublisherJni livepublisher;
	
	static
	{
		//load ffmpeg
		System.loadLibrary("ijkffmpeg");
		//publisher
		System.loadLibrary("ijkpublisher");
	}
 
	private LivePublisherJni(){}
	
	public static LivePublisherJni getInstance()
	{
		if(livepublisher == null)
		{
			livepublisher = new LivePublisherJni();
		}
		return livepublisher;
	}
	
	 /***************************************************************
	    * 
	    * Native Methods
	    * 
	    ***************************************************************/
	   
	   public  native void initRtmp(); //init ffmpeg rtmp
	   public  native void closeRtmp();
	   //320 bytes aac encode
	   public  native void putAudioData(byte[] bytes, int bytesize);
	   //video h264
	   public  native void setCameraSize(int width,int height);
	   public  native void setCameraId(int cid);
	   public  native void putVideoData(byte[] bytes, int bytesize);
	   // merge audio and video to publish
	   public  native void startAVPublisher(String rtmpurl);
}
