package tv.danmaku.ijk.media.publisher;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Process;
import android.util.Log;

public class AudioRecorder {
	private static final String subTAG = "Crystal.AudioRecorder";
	private AudioRecord mAudioRecord = null;
	private RecordAudioThread mRecordAudioThread = null;
	private boolean mRecordThreadExitFlag = false;
	private boolean mAudioRecordReleased = true;
	private int mFrameBufferSize;
	private boolean mIsPause = false;

	/**
	 * 
	 * @param sampleRate
	 *            采样频率44100
	 * @param channels
	 *            采样通道 2
	 * @param frameSize
	 *            帧大小
	 * @return
	 */
	public int startAudioRecoder(int sampleRate, int channels, int frameSize) {
		releaseAudioRecorder();
		this.mFrameBufferSize = (frameSize * 2);
		int channel;
		if (channels == 1)// 单通道
			channel = AudioFormat.CHANNEL_IN_MONO;
		else {// 双通道
			channel = AudioFormat.CHANNEL_IN_STEREO;
		}

		// 采样位数16
		int samplebit = AudioFormat.ENCODING_PCM_16BIT;
		try {
			int minRecordBufSize = AudioRecord.getMinBufferSize(sampleRate,
					channel, samplebit);
			if (minRecordBufSize < 2048) {
				minRecordBufSize = 2048;
			}
			mFrameBufferSize = minRecordBufSize;
			this.mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
					sampleRate, channel, samplebit, minRecordBufSize);
			this.mAudioRecord.startRecording();
//			byte[] data = new byte[this.mFrameBufferSize];
			//测试权限问题
//			int ret = this.mAudioRecord.read(data, 0, this.mFrameBufferSize);
//			if ((ret == AudioRecord.ERROR_INVALID_OPERATION) || (ret == AudioRecord.ERROR_BAD_VALUE))
//				throw new Exception();
		} catch (Exception e) {
			Log.e("Crystal.AudioRecorder", "AudioRecorder 启动失败,可能是权限未开启.");
			return -1;
		}
		if (this.mRecordAudioThread == null) {
			this.mAudioRecordReleased = false;
			this.mRecordThreadExitFlag = false;
			this.mRecordAudioThread = new RecordAudioThread();
			this.mRecordAudioThread.start();
		}
		this.mIsPause = false;

		return 0;
	}

	public void pause() {
		this.mIsPause = true;
	}

	public void resume() {
		this.mIsPause = false;
	}

	public void releaseAudioRecorder() {
		if (this.mAudioRecordReleased) {
			return;
		}
		if (this.mRecordAudioThread != null) {
			this.mRecordThreadExitFlag = true;
			this.mRecordAudioThread = null;
		}
		if (this.mAudioRecord != null) {
			this.mAudioRecord.stop();
			this.mAudioRecord.release();
			this.mAudioRecord = null;
		}
		this.mAudioRecordReleased = true;
	}

	class RecordAudioThread extends Thread {
		RecordAudioThread() {
		}

		public void run() {
			try {
				Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
			} catch (Exception e) {
				Log.e("Crystal.AudioRecorder",
						"Set record thread priority failed: " + e.getMessage());
			}
//			byte[] data = new byte[AudioRecorder.this.mFrameBufferSize];
			int tmpsize = 320;
			byte[] data = new byte[tmpsize];
			while (!AudioRecorder.this.mRecordThreadExitFlag) {
				int ret = AudioRecorder.this.mAudioRecord.read(data, 0,
						tmpsize);
				if ((ret == AudioRecord.ERROR_INVALID_OPERATION) || (ret == AudioRecord.ERROR_BAD_VALUE)) {
					break;
				}
				if (!AudioRecorder.this.mIsPause) {
					LivePublisherJni.getInstance().putAudioData(data,
							ret);
				}
			}
			//释放内存占用
			data = (byte[]) null;
			System.gc();
		}
	}
}
