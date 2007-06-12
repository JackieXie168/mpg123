/*
	decode_i386.c: decode for i386 (really faster?)

	copyright 1995-2006 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp

	slighlty optimized for machines without autoincrement/decrement.
	The performance is highly compiler dependend. Maybe
	the decode.c version for 'normal' processor may be faster
	even for Intel processors.
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "mpg123.h"

#if 1
 /* old WRITE_SAMPLE */
#define WRITE_SAMPLE(samples,sum,clip) \
  if( (sum) > 32767.0) { *(samples) = 0x7fff; (clip)++; } \
  else if( (sum) < -32768.0) { *(samples) = -0x8000; (clip)++; } \
  else { *(samples) = sum; }
#else
 /* new WRITE_SAMPLE */
 /* keep in mind that we are on known little-endian i386 here and special tricks are allowed... */
#define WRITE_SAMPLE(samples,sum,clip) { \
  double dtemp; int v; /* sizeof(int) == 4 */ \
  dtemp = ((((65536.0 * 65536.0 * 16)+(65536.0 * 0.5))* 65536.0)) + (sum);  \
  v = ((*(int *)&dtemp) - 0x80000000); \
  if( v > 32767) { *(samples) = 0x7fff; (clip)++; } \
  else if( v < -32768) { *(samples) = -0x8000; (clip)++; } \
  else { *(samples) = v; }  \
}
#endif

int synth_1to1_8bit_i386(real *bandPtr,int channel, struct frame *fr, int final)
{
  short samples_tmp[64];
  short *tmp1 = samples_tmp + channel;
  int i,ret;

  unsigned char *samples = fr->buffer.data;
  int pnt = fr->buffer.fill;
  fr->buffer.data = (unsigned char*) samples_tmp;
  fr->buffer.fill = 0;
  ret = opt_synth_1to1(fr)(bandPtr, channel, fr , 0);
  fr->buffer.data = samples;

  samples += channel + pnt;
  for(i=0;i<32;i++) {
    *samples = conv16to8[*tmp1>>AUSHIFT];
    samples += 2;
    tmp1 += 2;
  }
  fr->buffer.fill = pnt + (final ? 64 : 0 );

  return ret;
}

int synth_1to1_8bit_mono_i386(real *bandPtr, struct frame *fr)
{
  short samples_tmp[64];
  short *tmp1 = samples_tmp;
  int i,ret;
 
  unsigned char *samples = fr->buffer.data;
  int pnt = fr->buffer.fill;
  fr->buffer.data = (unsigned char*) samples_tmp;
  fr->buffer.fill = 0;
  ret = opt_synth_1to1(fr)(bandPtr, 0, fr, 0);
  fr->buffer.data = samples;

  samples += pnt;
  for(i=0;i<32;i++) {
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    tmp1+=2;
  }
  fr->buffer.fill = pnt + 32;

  return ret;
}

int synth_1to1_8bit_mono2stereo_i386(real *bandPtr, struct frame *fr)
{
  short samples_tmp[64];
  short *tmp1 = samples_tmp;
  int i,ret;

  unsigned char *samples = fr->buffer.data;
  int pnt = fr->buffer.fill;
  fr->buffer.data = (unsigned char*) samples_tmp;
  fr->buffer.fill = 0;
  ret = opt_synth_1to1(fr)(bandPtr, 0, fr, 0);
  fr->buffer.data = samples;

  samples += pnt;
  for(i=0;i<32;i++) {
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    tmp1 += 2;
  }
  fr->buffer.fill = pnt + 64;

  return ret;
}

int synth_1to1_mono_i386(real *bandPtr, struct frame *fr)
{
  short samples_tmp[64];
  short *tmp1 = samples_tmp;
  int i,ret;

  unsigned char *samples = fr->buffer.data;
  int pnt = fr->buffer.fill;
  fr->buffer.data = (unsigned char*) samples_tmp;
  fr->buffer.fill = 0;
  ret = opt_synth_1to1(fr)(bandPtr, 0, fr, 0);
  fr->buffer.data = samples;

  samples += pnt;
  for(i=0;i<32;i++) {
    *( (short *) samples) = *tmp1;
    samples += 2;
    tmp1 += 2;
  }
  fr->buffer.fill = pnt + 64;

  return ret;
}


int synth_1to1_mono2stereo_i386(real *bandPtr, struct frame *fr)
{
  int i,ret;
  unsigned char *samples = fr->buffer.data;

  ret = opt_synth_1to1(fr)(bandPtr, 0, fr, 1);
  samples += fr->buffer.fill - 128;

  for(i=0;i<32;i++) {
    ((short *)samples)[1] = ((short *)samples)[0];
    samples+=4;
  }

  return ret;
}

/* needed for i386, i486 */
#ifdef OPT_I386
int synth_1to1_i386(real *bandPtr,int channel, struct frame *fr, int final)
{
  static const int step = 2;
  short *samples = (short *) (fr->buffer.data + fr->buffer.fill);

  real *b0, **buf;
  int clip = 0; 
  int bo1;

  if(have_eq_settings)
	do_equalizer(bandPtr,channel);

  if(!channel) {
    fr->bo--;
    fr->bo &= 0xf;
    buf = fr->real_buffs[0];
  }
  else {
    samples++;
    buf = fr->real_buffs[1];
  }

  if(fr->bo & 0x1) {
    b0 = buf[0];
    bo1 = fr->bo;
    dct64_i386(buf[1]+((fr->bo+1)&0xf),buf[0]+fr->bo,bandPtr);
  }
  else {
    b0 = buf[1];
    bo1 = fr->bo+1;
    dct64_i386(buf[0]+fr->bo,buf[1]+fr->bo+1,bandPtr);
  }
  
  {
    register int j;
    real *window = opt_decwin(fr) + 16 - bo1;

    for (j=16;j;j--,b0+=0x10,window+=0x20,samples+=step)
    {
      real sum;
      sum  = window[0x0] * b0[0x0];
      sum -= window[0x1] * b0[0x1];
      sum += window[0x2] * b0[0x2];
      sum -= window[0x3] * b0[0x3];
      sum += window[0x4] * b0[0x4];
      sum -= window[0x5] * b0[0x5];
      sum += window[0x6] * b0[0x6];
      sum -= window[0x7] * b0[0x7];
      sum += window[0x8] * b0[0x8];
      sum -= window[0x9] * b0[0x9];
      sum += window[0xA] * b0[0xA];
      sum -= window[0xB] * b0[0xB];
      sum += window[0xC] * b0[0xC];
      sum -= window[0xD] * b0[0xD];
      sum += window[0xE] * b0[0xE];
      sum -= window[0xF] * b0[0xF];

      WRITE_SAMPLE(samples,sum,clip);
    }

    {
      real sum;
      sum  = window[0x0] * b0[0x0];
      sum += window[0x2] * b0[0x2];
      sum += window[0x4] * b0[0x4];
      sum += window[0x6] * b0[0x6];
      sum += window[0x8] * b0[0x8];
      sum += window[0xA] * b0[0xA];
      sum += window[0xC] * b0[0xC];
      sum += window[0xE] * b0[0xE];
      WRITE_SAMPLE(samples,sum,clip);
      b0-=0x10,window-=0x20,samples+=step;
    }
    window += bo1<<1;

    for (j=15;j;j--,b0-=0x10,window-=0x20,samples+=step)
    {
      real sum;
      sum = -window[-0x1] * b0[0x0];
      sum -= window[-0x2] * b0[0x1];
      sum -= window[-0x3] * b0[0x2];
      sum -= window[-0x4] * b0[0x3];
      sum -= window[-0x5] * b0[0x4];
      sum -= window[-0x6] * b0[0x5];
      sum -= window[-0x7] * b0[0x6];
      sum -= window[-0x8] * b0[0x7];
      sum -= window[-0x9] * b0[0x8];
      sum -= window[-0xA] * b0[0x9];
      sum -= window[-0xB] * b0[0xA];
      sum -= window[-0xC] * b0[0xB];
      sum -= window[-0xD] * b0[0xC];
      sum -= window[-0xE] * b0[0xD];
      sum -= window[-0xF] * b0[0xE];
      sum -= window[-0x0] * b0[0xF];

      WRITE_SAMPLE(samples,sum,clip);
    }
  }
  if(final) fr->buffer.fill += 128;

  return clip;
}
#endif

#ifdef OPT_PENTIUM
int synth_1to1_i586(real *bandPtr,int channel, struct frame *fr, int final)
{
	int ret;
	if(have_eq_settings) do_equalizer(bandPtr,channel);

	/* this is in asm, can be dither or not */
	/* uh, is this return from pointer correct? */ 
	TODO: get buffs out of the game,
	ret = (int) opt_synth_1to1_i586_asm(fr)(bandPtr, channel, fr->buffer.data+fr->buffer.fill, (short *) fr->rawbuffs, &fr->bo);
	if(final) fr->buffer.fill += 128;
	return ret;
}
#endif

#ifdef OPT_3DNOW
int synth_1to1_3dnow(real *bandPtr,int channel, struct frame *fr, int final)
{
	int ret;
	if(have_eq_settings) do_equalizer(bandPtr,channel);

	/* this is in asm, can be dither or not */
	/* uh, is this return from pointer correct? */ 
	TODO: get buffs out of the game,
	ret = (int) synth_1to1_3dnow_asm(bandPtr, channel, fr->buffer.data+fr->buffer.fill, (short *) fr->rawbuffs, &fr->bo);
	if(final) fr->buffer.fill += 128;
	return ret;
}
#endif

#ifdef OPT_MMX
/* wrapper for da interface */
int synth_1to1_mmx(real *bandPtr, int channel, struct frame *fr, int final)
{
	if(have_eq_settings) do_equalizer(bandPtr,channel);

	/* in asm */
	synth_1to1_MMX(bandPtr, channel, (short*) (fr->buffer.data+fr->buffer.fill), (short *) fr->rawbuffs, &fr->bo); 
	if(final) fr->buffer.fill += 128;
	return 0;
}
#endif

#ifdef OPT_MPLAYER
void synth_1to1_sse_s(real *bandPtr, int channel, short *samples, short *buffs, int *bo);
int synth_1to1_sse(real *bandPtr, int channel, struct frame *fr, int final)
{
	if(have_eq_settings) do_equalizer(bandPtr,channel);

	synth_1to1_sse_s(bandPtr, channel, (short*) (fr->buffer.data+fr->buffer.fill), (short *) fr->rawbuffs, &fr->bo); 
	if(final) fr->buffer.fill += 128;
	return 0;
}
#endif
