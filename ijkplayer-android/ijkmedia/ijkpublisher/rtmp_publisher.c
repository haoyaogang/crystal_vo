#include "rtmp_publisher.h"
#include "x264test.h"

#define TEST_H264 0

int mCameraW = 320,mCamerah = 240;
int mCameraID = -1;
const char* mUrl;
int isStartSuccess = 0;

void ffmpeg_init()
{
    isStartSuccess = 0;
    LOGI("ffmpeg_init");
    av_log_set_callback(my_logoutput);
    av_register_all();
    //Network
    avformat_network_init();
    ffmpeg_init_data();
    //test h264
    if(TEST_H264)
    ffmpeg_start_x264test_thread();

}
void ffmpeg_uninit()
{
    isStartSuccess = 0;
    LOGI("ffmpeg_uninit");
}
void ffmpeg_set_Camera_size(int w,int h)
{
    LOGI("ffmpeg_set_Camera_size w=%d, h=%d",w,h);
    mCameraW = w;
    mCamerah = h;
}

void ffmpeg_set_camera_id(int id)
{
    LOGI("ffmpeg_set_camera_id id = %d",id);
    mCameraID = id;
}
void ffmpeg_set_camera_framerate(int framerate)
{
    LOGI("ffmpeg_set_camera_framerate framerate = %d",framerate);
    m_iFrameRate = framerate;
}

void ffmpeg_push_pcm(char* bytes, int size)
{
    //LOGI("ffmpeg_push_pcm size= %d",size);

        uint8_t *samples;
        int channels = 2;
        AVFrame* pFrame = av_frame_alloc();

        pFrame->nb_samples = size;
        pFrame->format = AV_SAMPLE_FMT_S16;
        pFrame->channel_layout = AV_CH_LAYOUT_STEREO;


        int framesize = av_samples_get_buffer_size(NULL, channels, size, AV_SAMPLE_FMT_S16, 1);
        samples = av_malloc(framesize);
        avcodec_fill_audio_frame(pFrame, channels, AV_SAMPLE_FMT_S16, (const uint8_t *)samples, framesize, 1);

        memcpy(samples,bytes,framesize);

        if (NULL == m_pStreamInfo->m_pAudioFifo) {
            m_pStreamInfo->m_pAudioFifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_S16 ,
                channels, 10 * pFrame->nb_samples);
            LOGI("ffmpeg_push_pcm --->av_audio_fifo_alloc");
        }
        int buf_space = av_audio_fifo_space(m_pStreamInfo->m_pAudioFifo);
        if (buf_space >= pFrame->nb_samples) {
            av_audio_fifo_write(m_pStreamInfo->m_pAudioFifo, (void **) pFrame->data, pFrame->nb_samples);
            LOGI("ffmpeg_push_pcm --> av_audio_fifo_write av_audio_fifo_write");
        }
        else if (buf_space > 0) {
            av_audio_fifo_write(m_pStreamInfo->m_pAudioFifo, (void **) pFrame->data, buf_space);
            LOGI("ffmpeg_push_pcm ->  av_audio_fifo_write av_audio_fifo_write");
        }
}

void ffmpeg_push_yuv(char* bytes, int size)
{
//    if(!isStartSuccess)return;

    if (m_pStreamInfo->m_pVideoFifo != NULL && av_fifo_space(m_pStreamInfo->m_pVideoFifo) >= size)
    {
//        int ret = av_fifo_generic_write(m_pStreamInfo->m_pVideoFifo,bytes, size, NULL);
//        LOGI("ffmpeg_push_yuv size = %d", ret);
    }

}
void ffmpeg_start_av_publisher(const char* url)
{
    LOGI("ffmpeg_start_av_publisher url=%s",url);
    mUrl = url;
//    pthread_t pt;
//    pthread_create(&pt, NULL, &ffmpeg_start_thread, NULL);
    ffmpeg_start_thread();
}
void ffmpeg_start_thread()
{
    int ret = ffmpeg_OpenRtmpAddr();
    if (ret)
    {
        isStartSuccess = 0;
        LOGI("ffmpeg_start_av_publisher ffmpeg_OpenRtmpAddr fail");
        return;
    }
    else
    {
        LOGI("ffmpeg_start_av_publisher ffmpeg_OpenRtmpAddr OK");
        isStartSuccess = 1;
    }
    ffmpeg_start_rtmp_thread();
}
int ffmpeg_OpenRtmpAddr()
{
    int iRet = -1;
    AVOutputFormat      *pStreamOutfmt = NULL;
    LOGI("ffmpeg_OpenRtmpAddr url=%s",mUrl);
    avformat_alloc_output_context2(&m_pFmtRtmpCtx, NULL, "flv", mUrl);
    if (NULL == m_pFmtRtmpCtx) {
        LOGE("ffmpeg_OpenRtmpAddr NULL == m_pFmtRtmpCtx\n");
        return iRet;
    }

    pStreamOutfmt = m_pFmtRtmpCtx->oformat;
    if (NULL == pStreamOutfmt) {
        LOGE("ffmpeg_OpenRtmpAddr NULL == pStreamOutfmt\n");
        return iRet;
    }
    if (ffmpeg_init_rtmp_codec() < 0) {
        LOGE("ffmpeg_OpenRtmpAddr OpenRtmpUrl err\n");
        return iRet;
    }
    //写头
    if (!(pStreamOutfmt->flags & AVFMT_NOFILE)) {
        iRet = avio_open(&m_pFmtRtmpCtx->pb, mUrl, AVIO_FLAG_WRITE);
        if (iRet < 0) {
            LOGE("ffmpeg_OpenRtmpAddr Could not open output URL '%s'", mUrl);
            return iRet;
        }
    }

    iRet = avformat_write_header(m_pFmtRtmpCtx, NULL);
    if (iRet < 0) {
        LOGE("ffmpeg_OpenRtmpAddr Error occurred when opening output URL\n");
        return iRet;
    }

    return iRet;
}
int ffmpeg_init_rtmp_codec()
{
    int iRet = -1;
    //视频推流信息
    //if (NULL != m_pFmtVideoCtx)
    {
        m_pStreamInfo->m_pVideoStream = avformat_new_stream(m_pFmtRtmpCtx, NULL);
        if (!m_pStreamInfo->m_pVideoStream) {
            LOGE(" ffmpeg_init_rtmp_codec Failed allocating output stream\n");
            goto END;
        }
        AVCodec* pvCodec = avcodec_find_encoder(AV_CODEC_ID_H264);//AV_CODEC_ID_FLV1
        if(pvCodec)
        {
            m_pStreamInfo->m_pVideoStream->codec->codec = pvCodec;
            LOGI("ffmpeg_init_rtmp_codec AV_CODEC_ID_H264  ok");
        }
        else
        {
            LOGI("ffmpeg_init_rtmp_codec AV_CODEC_ID_H264  fail");
        }
        m_pStreamInfo->m_pVideoStream->codec->codec_tag = 0;
        m_pStreamInfo->m_pVideoStream->codec->height = mCamerah;
        m_pStreamInfo->m_pVideoStream->codec->width = mCameraW;
        m_pStreamInfo->m_pVideoStream->codec->time_base.den = m_iFrameRate;//25
        m_pStreamInfo->m_pVideoStream->codec->time_base.num = 1;
       // m_pStreamInfo->m_pVideoStream->codec->sample_aspect_ratio = m_pCodecVideoCtx->sample_aspect_ratio;
        m_pStreamInfo->m_pVideoStream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
        // take first format from list of supported formats
        m_pStreamInfo->m_pVideoStream->codec->bit_rate = 900000;
        m_pStreamInfo->m_pVideoStream->codec->rc_max_rate = 900000;
        m_pStreamInfo->m_pVideoStream->codec->rc_min_rate = 900000;
        m_pStreamInfo->m_pVideoStream->codec->gop_size = 250;
        m_pStreamInfo->m_pVideoStream->codec->qmin = 5;
        m_pStreamInfo->m_pVideoStream->codec->qmax = 51;
        //m_pStreamInfo->m_pVideoStream->codec->max_b_frames = m_pCodecVideoCtx->max_b_frames;
        //m_pStreamInfo->m_pVideoStream->r_frame_rate = m_pFmtVideoCtx->streams[m_iVideoIndex]->r_frame_rate;
        m_iVideoOutIndex = m_pStreamInfo->m_pVideoStream->index;

        if (m_pFmtRtmpCtx->oformat->flags & AVFMT_GLOBALHEADER)
            m_pStreamInfo->m_pVideoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

        m_pStreamInfo->m_pVideoStream->codec->profile = FF_PROFILE_H264_HIGH;
//        ((X264Context*)(m_pStreamInfo->m_pVideoStream->codec->priv_data))->preset = "superfast";
//        ((X264Context*)(m_pStreamInfo->m_pVideoStream->codec->priv_data))->tune = "zerolatency";

        //打开视频编码器
        if ((avcodec_open2(m_pStreamInfo->m_pVideoStream->codec, pvCodec, NULL)) < 0) {
            LOGE("ffmpeg_init_rtmp_codec can not open the video encoder\n");
            //goto END;
        }

        m_pStreamInfo->m_pVideoFifo = av_fifo_alloc(30 * avpicture_get_size(AV_PIX_FMT_YUV420P, m_pStreamInfo->m_pVideoStream->codec->width, m_pStreamInfo->m_pVideoStream->codec->height));
    }

    //音频推流信息
    //if (NULL != m_pFmtAudioCtx)
    {
        m_pStreamInfo->m_pAudioStream = avformat_new_stream(m_pFmtRtmpCtx, NULL);
        if (NULL == m_pStreamInfo->m_pAudioStream) {
            LOGE("ffmpeg_init_rtmp_codec NULL == m_pStreamInfo->m_pAudioStream");
            goto END;
        }
        AVCodec* pCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if(pCodec)
        {
            m_pStreamInfo->m_pAudioStream->codec->codec = pCodec;
            LOGI("ffmpeg_init_rtmp_codec AV_CODEC_ID_AAC  ok");
        }
        else
        {
            LOGI("ffmpeg_init_rtmp_codec AV_CODEC_ID_AAC  fail");
        }
        m_pStreamInfo->m_pAudioStream->codec->sample_rate = 44100;//m_pCodecAudioCtx->sample_rate;
        m_pStreamInfo->m_pAudioStream->codec->channel_layout = AV_CH_LAYOUT_STEREO;//m_pFmtRtmpCtx->streams[m_iAudioIndex]->codec->channel_layout;
        m_pStreamInfo->m_pAudioStream->codec->channels = av_get_channel_layout_nb_channels(m_pStreamInfo->m_pAudioStream->codec->channel_layout);
        if (m_pStreamInfo->m_pAudioStream->codec->channel_layout == 0) {
            m_pStreamInfo->m_pAudioStream->codec->channel_layout = AV_CH_LAYOUT_STEREO;
            m_pStreamInfo->m_pAudioStream->codec->channels = av_get_channel_layout_nb_channels(m_pStreamInfo->m_pAudioStream->codec->channel_layout);
        }
        m_pStreamInfo->m_pAudioStream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
        m_pStreamInfo->m_pAudioStream->codec->sample_fmt = m_pStreamInfo->m_pAudioStream->codec->codec->sample_fmts[0];
        AVRational time_base = { 1, m_pStreamInfo->m_pAudioStream->codec->sample_rate };
        m_pStreamInfo->m_pAudioStream->time_base = time_base;
        m_iAudioOutIndex = m_pStreamInfo->m_pAudioStream->index;

        m_pStreamInfo->m_pAudioStream->codec->codec_tag = 0;
        if (m_pFmtRtmpCtx->oformat->flags & AVFMT_GLOBALHEADER)
            m_pStreamInfo->m_pAudioStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

        //打开音频编码器
        av_opt_set(m_pStreamInfo->m_pAudioStream->codec,"strict","experimental",0);
        if (avcodec_open2(m_pStreamInfo->m_pAudioStream->codec, m_pStreamInfo->m_pAudioStream->codec->codec, NULL) < 0) {
            LOGE("ffmpeg_init_rtmp_codec can not open the audio encoder\n");
            goto END;
        }
    }

    iRet = 0;
    END:
        return iRet;
}
void ffmpeg_init_data()
{
    m_pStreamInfo = (struct_stream_info *) calloc(1, sizeof(struct_stream_info));
    if (NULL == m_pStreamInfo)
    {
        LOGE("m_streamstate is NULL!\n");
        return;
    }
    //初始化流媒体
    m_pStreamInfo->m_pFormatCtx = avformat_alloc_context();
    if (NULL == m_pStreamInfo->m_pFormatCtx) {
        LOGE("NULL == m_pStreamInfo->m_pFormatCtx\n");
        return;
    }

    m_pStreamInfo->m_width = 0;
    m_pStreamInfo->m_height = 0;
    m_pStreamInfo->m_pAudioStream = NULL;
    m_pStreamInfo->m_pAudioFifo = NULL;
    m_pStreamInfo->m_pVideoFifo = NULL;
    m_pStreamInfo->m_pVideoStream = NULL;
    m_pStreamInfo->m_pAudioFrame = NULL;
    m_pStreamInfo->m_pAudioBuf = NULL;
    m_pStreamInfo->m_iAudioBufSize = 0;
    m_pStreamInfo->m_iAudioBufIndex = 0;
    m_pStreamInfo->m_iAduioPktSize = 0;
    m_pStreamInfo->m_iSampleArrayIndex = 0;
    m_pStreamInfo->m_iAbortRequest = 0;
    m_pStreamInfo->m_iRefresh = 0;

    //
    m_iFrameRate = 25;//video frame rate
}
void ffmpeg_uninit_data()
{
    if (NULL != m_pStreamInfo) {
        m_pStreamInfo->m_iAbortRequest = 1;

        if (m_pStreamInfo->m_pAudioFifo) {
            av_audio_fifo_free(m_pStreamInfo->m_pAudioFifo);
            m_pStreamInfo->m_pAudioFifo = NULL;
        }
        if (m_pStreamInfo->m_pVideoFifo) {
            av_fifo_free(m_pStreamInfo->m_pVideoFifo);
            m_pStreamInfo->m_pVideoFifo = NULL;
        }

        if (m_pStreamInfo->m_pPushPicSize) {
            av_freep(&m_pStreamInfo->m_pPushPicSize);
        }

        if (m_pStreamInfo->m_pVideoStream) {
            if (m_pStreamInfo->m_pVideoStream->codec) {
                avcodec_close(m_pStreamInfo->m_pVideoStream->codec);
                m_pStreamInfo->m_pVideoStream->codec = NULL;
            }
        }

        if (m_pStreamInfo->m_pAudioStream) {
            if (m_pStreamInfo->m_pAudioStream->codec) {
                avcodec_close(m_pStreamInfo->m_pAudioStream->codec);
                m_pStreamInfo->m_pAudioStream->codec = NULL;
            }
        }
        m_pStreamInfo->m_iAbortRequest = 0;
    }
}

int ffmpeg_push_thr(struct_stream_info *stream_info)
{
//    if(!isStartSuccess)return;
    int iRet = -1;
    int64_t cur_pts_v = 0, cur_pts_a = 0;
    int frame_video_index = 0;
    int frame_audio_index = 0;
    struct_stream_info* pStrctStreamInfo = stream_info;
    AVFrame *picture = av_frame_alloc();
    int size = avpicture_get_size(AV_PIX_FMT_YUV420P,
        pStrctStreamInfo->m_pVideoStream->codec->width,
        pStrctStreamInfo->m_pVideoStream->codec->height);
    pStrctStreamInfo->m_pPushPicSize = (uint8_t*) av_malloc(size);
//    LOGE("ffmpeg_push_thr avpicture_get_size == %d",size); //1382400
    AVPacket pkt;    //视频包
    AVPacket pkt_out_audio;//声音包
    AVFrame *frame;  //声音帧

    if(m_pFmtRtmpCtx == NULL)
    {
        LOGE("ffmpeg_push_thr m_pFmtRtmpCtx == NULL");
        return iRet;
    }
    while(1)
    {
        if (pStrctStreamInfo->m_iAbortRequest) {
            break;
        }
        //Audio
        if (NULL == pStrctStreamInfo->m_pAudioFifo) {
            LOGI("ffmpeg_push_thr no init  fifo");
            continue;    //还未初始化fifo
        }
#if 0

        if (av_compare_ts(cur_pts_v, m_pFmtRtmpCtx->streams[m_iVideoOutIndex]->time_base,
            cur_pts_a, m_pFmtRtmpCtx->streams[m_iAudioOutIndex]->time_base) <= 0) {

            int varraysize = av_fifo_size(pStrctStreamInfo->m_pVideoFifo);
            LOGI("ffmpeg_push_thr av_compare_ts video varraysize=%d",varraysize);
            //视频处理
            if (varraysize >= size) {
                int retsize = av_fifo_generic_read(pStrctStreamInfo->m_pVideoFifo, pStrctStreamInfo->m_pPushPicSize, size, NULL);
                avpicture_fill((AVPicture *) picture, pStrctStreamInfo->m_pPushPicSize,
                    m_pFmtRtmpCtx->streams[m_iVideoOutIndex]->codec->pix_fmt,
                    m_pFmtRtmpCtx->streams[m_iVideoOutIndex]->codec->width,
                    m_pFmtRtmpCtx->streams[m_iVideoOutIndex]->codec->height);

                //picture->pts = frame_video_index * ((m_pFmtRtmpCtx->streams[m_iVideoOutIndex]->time_base.den / m_pFmtRtmpCtx->streams[m_iVideoOutIndex]->time_base.num) / m_iFrameRate);
                picture->pts = frame_video_index++;

                int got_picture = 0;
                av_init_packet(&pkt);
                pkt.data = NULL;
                pkt.size = 0;
                iRet = avcodec_encode_video2(m_pFmtRtmpCtx->streams[m_iVideoOutIndex]->codec, &pkt, picture, &got_picture);
                LOGI("ffmpeg_push_thr............... ..VIDEO.avcodec_encode_video2  iRet = %d got_picture= %d",iRet,got_picture);
                if (iRet < 0) {
                    LOGE("ffmpeg_push_thr can not encode a video frame");
                    av_free_packet(&pkt);
                    continue;
                }
                if (got_picture == 1) {

                    pkt.pts = av_rescale_q(pkt.pts,m_pStreamInfo->m_pVideoStream->codec->time_base,
                        m_pStreamInfo->m_pVideoStream->time_base);
                    pkt.dts = av_rescale_q(pkt.dts,m_pStreamInfo->m_pVideoStream->codec->time_base,
                                            m_pStreamInfo->m_pVideoStream->time_base);
                    pkt.stream_index = m_iVideoOutIndex;
                    if (m_pFmtRtmpCtx->streams[m_iVideoOutIndex]->codec->coded_frame->key_frame) {
                        pkt.flags |= AV_PKT_FLAG_KEY;
                    }

                    cur_pts_v = pkt.pts;
                    if (av_interleaved_write_frame(m_pFmtRtmpCtx, &pkt) < 0) {
                        LOGE("av_interleaved_write_frame video err!\n");
                    }
                    av_free_packet(&pkt);
                    frame_video_index++;
                }
            }
        }
        else if (av_audio_fifo_size(pStrctStreamInfo->m_pAudioFifo) >=
                        (m_pFmtRtmpCtx->streams[m_iAudioOutIndex]->codec->frame_size > 0 ? m_pFmtRtmpCtx->streams[m_iAudioOutIndex]->codec->frame_size : 1024))
#endif
        {
            int aArraysize = av_audio_fifo_size(pStrctStreamInfo->m_pAudioFifo);
            if(aArraysize >= 1024)
            {
                LOGI("ffmpeg_push_thr av_compare_ts audio aArraysize=%d",aArraysize);
                LOGI("ffmpeg_push_thr av_compare_ts audio framesize=%d",m_pFmtRtmpCtx->streams[m_iAudioOutIndex]->codec->frame_size);
                int nb_samples = m_pFmtRtmpCtx->streams[m_iAudioOutIndex]->codec->frame_size > 0 ? m_pFmtRtmpCtx->streams[m_iAudioOutIndex]->codec->frame_size : 1024;
                frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO,44100, nb_samples);
                int size = av_audio_fifo_read(pStrctStreamInfo->m_pAudioFifo, (void **) frame->data, nb_samples);
                LOGI("ffmpeg_push_thr av_compare_ts av_audio_fifo_read size = %d",size);

                av_init_packet(&pkt_out_audio);
                int got_picture = -1;
                pkt_out_audio.data = NULL;
                pkt_out_audio.size = 0;

                frame->pts = frame_audio_index++;//frame_audio_index * m_pFmtRtmpCtx->streams[m_iAudioOutIndex]->codec->frame_size;
                if (avcodec_encode_audio2(m_pStreamInfo->m_pAudioStream->codec, &pkt_out_audio, frame, &got_picture) < 0) {
                    LOGE("ffmpeg_push_thr can not encode a frame");
                    av_free_packet(&pkt_out_audio);
                    continue;
                }
                if (NULL == pkt_out_audio.data) {
                    LOGE(" ffmpeg_push_thr NULL == pkt_out_audio.data");
                    av_free_packet(&pkt_out_audio);
                    continue;
                }
                if (got_picture == 1) {
                    pkt_out_audio.stream_index = m_iAudioOutIndex;
                    pkt_out_audio.pts = av_rescale_q(pkt_out_audio.pts,m_pStreamInfo->m_pAudioStream->codec->time_base,
                        m_pStreamInfo->m_pAudioStream->time_base);
                    pkt_out_audio.dts = av_rescale_q(pkt_out_audio.dts,m_pStreamInfo->m_pAudioStream->codec->time_base,
                        m_pStreamInfo->m_pAudioStream->time_base);

                    cur_pts_a = pkt_out_audio.pts;
                    if (av_interleaved_write_frame(m_pFmtRtmpCtx, &pkt_out_audio) < 0) {
                        LOGE("av_interleaved_write_frame audio err!\n");
                    }
                    LOGI(" av_interleaved_write_frame ..AUDIO....");
                    av_free_packet(&pkt_out_audio);
                    frame_audio_index++;
                }
                av_frame_free(&frame);
            }
        }
    }
    //推送结束
    av_write_trailer(m_pFmtRtmpCtx);
    av_frame_free(&picture);

    return iRet;
}
void ffmpeg_start_rtmp_thread()
{
    pthread_t pt;
    pthread_create(&pt, NULL, &ffmpeg_push_thr, m_pStreamInfo);
}
void my_logoutput(void* ptr, int level, const char* fmt, va_list vl)
{
    char log[1024];
     vsnprintf(log,sizeof(log),fmt,vl);
     LOGI("my_logoutput  = %s",log);
}


/********************************************
 *
 *
 *  X264 TEST
 *
 */
void ffmpeg_start_x264test_thread()
{
    pthread_t pt;
    pthread_create(&pt, NULL, &ffmpeg_x2645test_thread, NULL);
}
void ffmpeg_x2645test_thread()
{
    LOGI("testYUVToH264 begin");
    testYUVToH264();
    LOGI("testYUVToH264 end");
}
