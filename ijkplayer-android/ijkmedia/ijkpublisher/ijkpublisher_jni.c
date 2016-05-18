#include "ijkpublisher_jni.h"
#include "androidlog.h"
#include "jni.h"
#include "rtmp_publisher.h"

/*
 * Class:     tv_danmaku_ijk_media_publisher_LivePublisherJni
 * Method:    initRtmp
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_tv_danmaku_ijk_media_publisher_LivePublisherJni_initRtmp
  (JNIEnv *env , jobject clazz)
{
    LOGI("initRtmp");
    ffmpeg_init();
}

/*
 * Class:     tv_danmaku_ijk_media_publisher_LivePublisherJni
 * Method:    closeRtmp
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_tv_danmaku_ijk_media_publisher_LivePublisherJni_closeRtmp
  (JNIEnv *env, jobject  clazz)
{
    LOGI("closeRtmp");
    ffmpeg_uninit();
}

/*
 * Class:     tv_danmaku_ijk_media_publisher_LivePublisherJni
 * Method:    putAudioData
 * Signature: ([BI)V
 */
JNIEXPORT void JNICALL Java_tv_danmaku_ijk_media_publisher_LivePublisherJni_putAudioData
(JNIEnv *env , jobject clazz, jbyteArray bytearray, jint size)
{
//    LOGI("putAudioData");
    jbyte* bBuffer = (*env)->GetByteArrayElements(env,bytearray,0);
    ffmpeg_push_pcm((char*)bBuffer,size);
}


/*
 * Class:     tv_danmaku_ijk_media_publisher_LivePublisherJni
 * Method:    setCameraSize
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_tv_danmaku_ijk_media_publisher_LivePublisherJni_setCameraSize
(JNIEnv *env, jobject obj, jint width, jint height)
{
    LOGI("setCameraSize");
    ffmpeg_set_Camera_size(width,height);
}

/*
 * Class:     tv_danmaku_ijk_media_publisher_LivePublisherJni
 * Method:    setCameraId
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_tv_danmaku_ijk_media_publisher_LivePublisherJni_setCameraId
(JNIEnv *env, jobject clazz , jint cid)
{
    LOGI("setCameraId");
    ffmpeg_set_camera_id(cid);
}

/*
 * Class:     tv_danmaku_ijk_media_publisher_LivePublisherJni
 * Method:    putVideoData
 * Signature: ([BI)V
 */
JNIEXPORT void JNICALL Java_tv_danmaku_ijk_media_publisher_LivePublisherJni_putVideoData
(JNIEnv *env , jobject clazz , jbyteArray bytearray, jint size)
{
//    LOGI("putVideoData");
    jbyte* bBuffer = (*env)->GetByteArrayElements(env,bytearray,0);
    ffmpeg_push_yuv((char*)bBuffer,size);
}

/*
 * Class:     tv_danmaku_ijk_media_publisher_LivePublisherJni
 * Method:    startAVPublisher
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_tv_danmaku_ijk_media_publisher_LivePublisherJni_startAVPublisher
(JNIEnv *env, jobject clazz, jstring rtmpurl)
{
    LOGI("startAVPublisher");
    const char *url = (*env)->GetStringUTFChars(env, rtmpurl, 0);
    ffmpeg_start_av_publisher(url);
    (*env)->ReleaseStringUTFChars(env, rtmpurl, url);

}
