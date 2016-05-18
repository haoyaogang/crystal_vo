#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "androidlog.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/mathematics.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "x264.h"
#include "x264_config.h"


#define AUDIO_BUF_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
#define SAMPLE_ARRAY_SIZE (8 * 65536)


enum DeviceType{
    n_Video = 0,        //视频
    n_Audio = 1     //音频
};
typedef struct AudioParams {
    int freq;
    int channels;
    int channel_layout;
    enum AVSampleFormat fmt;
} AudioParams;

typedef struct stream_info{
    AVFormatContext     *m_pFormatCtx;
    int                  m_xLeft;               //显示窗体的坐标及大小
    int                  m_yTop;
    int                  m_width;
    int                  m_height;
    int                  m_iAbortRequest;       //退出标记
    int                  m_iRefresh;                //刷新标记
    int                  m_iShowMode;           //显示模式
    int                  m_iPaused;             //暂停标记
    /************************音频相关参数-start*********************/
    AVStream                *m_pAudioStream;        //音频流
    AVFrame             *m_pAudioFrame;     //音频帧
    AVAudioFifo         *m_pAudioFifo;
    AudioParams          m_AudioPrm;
    char*             *m_pAudioBuf;
    int                  m_iAudioBufSize;
    int                  m_iAudioBufIndex;
    int                  m_iAduioPktSize;
    int                  m_iAudioWriteBufSize;
    int                  m_iAudioLastStart;
    char               m_uSilenceBuf[AUDIO_BUF_SIZE];
    short              m_iSampleArray[SAMPLE_ARRAY_SIZE];
    int                  m_iSampleArrayIndex;
    /************************音频相关参数-end***********************/

    /************************视频相关参数-satrt*********************/
    AVFifoBuffer            *m_pVideoFifo;
    AVStream                *m_pVideoStream;            //视频流
    char                    *m_pPushPicSize;            //推送Pic大小
    //SwsContext               *m_pVideoSwsCtx;            //视频变化ctx

    /************************视频相关参数-end***********************/
}struct_stream_info;


struct_stream_info*                     m_pStreamInfo;  //音视频全局结构体
//AVFormatContext                        *m_pFmtVideoCtx; //视频采集format 视频设备
//AVFormatContext                        *m_pFmtAudioCtx; //音频采集format 音频设备
AVFormatContext                        *m_pFmtRtmpCtx;  //rtmp推送format
//AVCodecContext                         *m_pCodecVideoCtx;//视频采集解码器信息
//AVCodecContext                         *m_pCodecAudioCtx;//音频采集解码器信息

int                                     m_iVideoIndex;  //视频采集解码器索引
int                                     m_iAudioIndex;  //音频采集解码器索引
int                                     m_iVideoOutIndex;//推送视频解码器索引
int                                     m_iAudioOutIndex;//推送音频解码器索引
int                                     m_iFrameRate;   //帧率


void ffmpeg_init();
void ffmpeg_uninit();
void ffmpeg_set_Camera_size(int w,int h);
void ffmpeg_set_camera_id(int id);
void ffmpeg_set_camera_framerate(int framerate);
void ffmpeg_push_pcm(char* bytes, int size);
void ffmpeg_push_yuv(char* bytes, int size);
void ffmpeg_start_av_publisher(const char* url);
void ffmpeg_start_thread();
//rtmp
void ffmpeg_init_data();
void ffmpeg_uninit_data();
int ffmpeg_OpenRtmpAddr();
int ffmpeg_init_rtmp_codec();
int ffmpeg_push_thr(struct_stream_info *stream_info);
void ffmpeg_start_rtmp_thread();
void my_logoutput(void* ptr, int level, const char* fmt, va_list vl);
//x264 test
void ffmpeg_start_x264test_thread();
void ffmpeg_x2645test_thread();

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
    uint64_t channel_layout,
    int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        LOGE("Error allocating an audio frame\n");
        return NULL;
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            LOGE("Error allocating an audio buffer\n");
            return NULL;
        }
    }
    return frame;
}
typedef struct X264Context {
    AVClass        *class;
    x264_param_t    params;
    x264_t         *enc;
    x264_picture_t  pic;
    uint8_t        *sei;
    int             sei_size;
    char *preset;
    char *tune;
    char *profile;
    char *level;
    int fastfirstpass;
    char *wpredp;
    char *x264opts;
    float crf;
    float crf_max;
    int cqp;
    int aq_mode;
    float aq_strength;
    char *psy_rd;
    int psy;
    int rc_lookahead;
    int weightp;
    int weightb;
    int ssim;
    int intra_refresh;
    int bluray_compat;
    int b_bias;
    int b_pyramid;
    int mixed_refs;
    int dct8x8;
    int fast_pskip;
    int aud;
    int mbtree;
    char *deblock;
    float cplxblur;
    char *partitions;
    int direct_pred;
    int slice_max_size;
    char *stats;
    int nal_hrd;
    int avcintra_class;
    char *x264_params;
} X264Context;
