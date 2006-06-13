
#include <jni.h>
#include "jmpg_audio_NativeAudio.h"

#include "audio.h"

#define MAX_AUDIO 16

static struct audio_info_struct *ais[MAX_AUDIO] = { NULL , };

JNIEXPORT jint JNICALL Java_jmpg_audio_NativeAudio_hplay (JNIEnv *env,
    jobject obj, jint handler,jbyteArray b, jint off, jint len)
{
   jsize blen = (*env)->GetArrayLength(env, b);
   jbyte *b1 = (*env)->GetByteArrayElements(env, b, 0);
   return audio_play_samples(ais[handler], b1+off,len);
}

/*
 * Class:     jmpg_audio_NativeAudio
 * Method:    hopen
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_jmpg_audio_NativeAudio_hopen (JNIEnv *env, jobject obj) 
{
  int i;
  int handler = -1;
  struct audio_info_struct *ai;

  for(i=0;i<MAX_AUDIO;i++) {
    if(ais[i] == NULL)  {
      ai = (struct audio_info_struct *) malloc(sizeof(struct audio_info_struct));
      if(!ai)
        break;
      ais[i] = ai;
      handler = i;
      break;
    }
  } 

  if(handler < 0)
    return handler;

  audio_info_struct_init(ai);

  ai->device = NULL;
  ai->rate = 44100;
  ai->channels = 2;
  ai->format = AUDIO_FORMAT_SIGNED_16;

  printf("Found handler %d\n",handler);

  if(audio_open(ai) < 0) {
    free(ais[handler]);
    ais[handler] = NULL;
    handler = -1;
  }

  printf("Opened %d %d\n",handler,ai->fn);

  return handler;
}

/*
 * Class:     jmpg_audio_NativeAudio
 * Method:    hclose
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_jmpg_audio_NativeAudio_hclose (JNIEnv *env, jobject obj, jint handler) 
{
  audio_close(ais[handler]);
  free(ais[handler]);
  ais[handler] = NULL;
}


