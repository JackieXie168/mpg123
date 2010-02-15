/*
	helpers: support routines for testing code
	libmpg123: MPEG Audio Decoder library

	copyright 1995-2009 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

*/

#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include "mpg123.h"

#ifndef _MSC_VER
#include <inttypes.h>
#endif

/* Wanna print offsets. */
#define OFF_P PRIiMAX
typedef intmax_t off_p;

#define INBUFF  16384 * 2 * 2

int init_handle(mpg123_handle **mh, int channels, int encodings);
void get_output_format(int encoding, unsigned short *wav_format, unsigned short *bits_per_sample);
int feed(mpg123_handle *mh, FILE *in, void *buf);
int scan(mpg123_handle *mh, FILE *in, void *buf);
off_t length(mpg123_handle *mh);
int validate_offset(mpg123_handle *mh, off_t input_offset);
int seek(mpg123_handle *mh, off_t frameNumber, off_t *newFrameNumber, off_t *newInputOffset, FILE *in, void *buf);
int decode(mpg123_handle *mh, FILE *in, void *buf, unsigned char **out, size_t *count);
int copy_seek_index(mpg123_handle *source, mpg123_handle *target);

#endif
