package tv.danmaku.ijk.media.demo;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import tv.danmaku.ijk.media.publisher.LivePublisher;
import android.app.Activity;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Toast;


public class VideoRecorderActivity extends Activity {

	LivePublisher livePublisher;
	SurfaceView surfaceView;
	private final String RTMP_URL = "rtmp://s1.weiqu168.com/live/66/72";
	private final int mCamereWidth = 1280;
	private final int mCameraHeight = 720;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.video_recorder_layout);
		
		//1:initialize live publisher
		livePublisher = LivePublisher.getInstance();
		livePublisher.initCore(this);
		//to find views
		findviews();
		//2:publish
		startPublish();
	}
	/**
	 * initialize views
	 */
	public void findviews()
	{
		View v = findViewById(R.id.btn_test);
		v.setOnClickListener(new View.OnClickListener(){

			@Override
			public void onClick(View v) {
				testAudio();
				Toast.makeText(VideoRecorderActivity.this, "testAudio",
						Toast.LENGTH_SHORT).show();
			}
		});
		View player = findViewById(R.id.btn_player);
		player.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
//				Intent intent = new Intent(VideoRecorderActivity.this,PlayerTestActivity.class);
//				startActivity(intent);
//				finish();
			}
		});
		
		View btn_pcm = findViewById(R.id.btn_pcm);
		btn_pcm.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				playPCM();
			}
		});
		
		View btn_sendvideo = findViewById(R.id.btn_sendvideo);
		
		btn_sendvideo.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
			}
		});
		
		surfaceView = (SurfaceView) findViewById(R.id.surface_camera);
	}
	
	public void testAudio()
	{
		new Thread(new Runnable() {
			@Override
			public void run() {
				livePublisher.initAudioRecorder();
				livePublisher.startAudioPublisher("rtmp://s1.weiqu168.com/live/100/100");
			}
		}).start();
	}
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		livePublisher.stopVideoPreview();
		livePublisher.stopAudioPublisher();
		livePublisher.uninitCore();
	}
	
	public void startPublish()
	{
		//1:initailize audio and video
		livePublisher.initAudioRecorder();
		livePublisher.initVideoPublisher();
		livePublisher.setCameraSize(mCamereWidth, mCameraHeight);
		//2:start rtmp 
		livePublisher.startAVPublisher(RTMP_URL);
		//3: open video and audio device to grab data 
		livePublisher.startVideoPreview(surfaceView, 0, 0);
		livePublisher.startAudioRecorder();
	}
	
	/////////////////////////////////////////////
	//
	// PCM test
	/////////////////////////////////////////////
	MyAudioTrack myAudioTrack;
	boolean isPlaying = false;
	public void playPCM()
	{
		if(isPlaying)return;
		isPlaying = true;
		if(myAudioTrack == null)
		myAudioTrack = new MyAudioTrack(44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
		myAudioTrack.init();
		new Thread(new Runnable() {
			@Override
			public void run() {
				Log.i("crystal_rtmp_tag", "Runnable");
				try
				{
					File file = new File("/sdcard/audio.pcm");
					FileInputStream fis = new FileInputStream(file);
					
					int len = 0;
					byte[] buffer = new byte[100];
					while((len = fis.read(buffer)) != -1)
					{
						Log.i("crystal_rtmp_tag", "read..."+len);
						byte[] tempbuffer = new byte[len];
						System.arraycopy(buffer, 0, tempbuffer, 0, len);
						myAudioTrack.playAudioTrack(tempbuffer,0,len);
					}	
				}catch(IOException ex)
				{
					Log.i("crystal_rtmp_tag", "catch exception..."+ex.getMessage());
					Toast.makeText(VideoRecorderActivity.this, "ddd", Toast.LENGTH_SHORT).show();
				}finally
				{
					isPlaying = false;
					Log.i("crystal_rtmp_tag", "read...OK");
				}
			}
		}).start();
		
	}
	
	
	public class MyAudioTrack {
		int mFrequency;// 采样率
		int mChannel;// 声道
		int mSampBit;// 采样精度
		AudioTrack mAudioTrack;

		public MyAudioTrack(int frequency, int channel, int sampbit) {
			mFrequency = frequency;
			mChannel = channel;
			mSampBit = sampbit;
		}

		public void init() {
			if (mAudioTrack != null) {
				release();
			}
			int minBufSize = AudioTrack.getMinBufferSize(mFrequency, mChannel,
					mSampBit);
			mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, mFrequency,
					mChannel, mSampBit, minBufSize, AudioTrack.MODE_STREAM);
			mAudioTrack.play();
			Toast.makeText(VideoRecorderActivity.this, "mAudioTrack.play()", Toast.LENGTH_SHORT).show();
		}

		public void release() {
			if (mAudioTrack != null) {
				mAudioTrack.stop();
				mAudioTrack.release();
			}
		}

		public void playAudioTrack(byte[] data, int offset, int length) {
			if (data == null || data.length == 0) {
				return;
			}
			try {
				mAudioTrack.write(data, offset, length);
				Log.i("crystal_rtmp_tag", "mAudioTrack.write size ="+length);
			} catch (Exception e) {
				Log.i("crystal_rtmp_tag", "catch exception...");
			}
		}

		public int getPrimePlaySize() {
			int minBufSize = AudioTrack.getMinBufferSize(mFrequency, mChannel,
					mSampBit);
			return minBufSize * 2;
		}
	}
	
}
