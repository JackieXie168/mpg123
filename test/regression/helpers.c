/*
	helpers: support routines for testing code
	libmpg123: MPEG Audio Decoder library

	copyright 1995-2009 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

*/

#include "helpers.h"

#ifndef WAVE_FORMAT_IEEE_FLOAT
	#define WAVE_FORMAT_IEEE_FLOAT 0x0003 /* 32-bit floating-point */
#endif

#ifndef WAVE_FORMAT_PCM
	#define WAVE_FORMAT_PCM 1
#endif

int init_handle(mpg123_handle **mh, int channels, int encodings)
{
	size_t i, nrates;
	const long *rates;
	int ret;

	*mh = mpg123_new(NULL, &ret);
	if(*mh == NULL)
		goto initerror;

	ret = mpg123_param(*mh, MPG123_VERBOSE, 4, 0);
	if(ret != MPG123_OK)
		goto initerror;

	ret = mpg123_param(*mh, MPG123_FLAGS, MPG123_SEEKBUFFER | MPG123_GAPLESS | MPG123_QUIET, 0);
	if(ret != MPG123_OK)
		goto initerror;

	/* Let the seek index auto-grow and contain an entry for every frame */
	ret = mpg123_param(*mh, MPG123_INDEX_SIZE, -1, 0);
	if(ret != MPG123_OK)
		goto initerror;

	/* Look at the whole stream while looking for sync */
	ret = mpg123_param(*mh, MPG123_RESYNC_LIMIT, -1, 0);
	if(ret != MPG123_OK)
		goto initerror;

	/* Setup output format: float output, all supported samplerates */
	ret = mpg123_format_none(*mh);
	if(ret != MPG123_OK)
		goto initerror;

	rates = NULL;
	nrates = 0;
	mpg123_rates(&rates, &nrates);
	for(i=0; i<nrates; i++)
	{
		ret = mpg123_format(*mh, rates[i], channels,  encodings);
		if(ret != MPG123_OK)
			goto initerror;
	}

	return ret;

initerror:
	fprintf(stderr,"Error initializing handle: %s\n", mpg123_strerror(*mh));
	return ret;
}

void get_output_format(int encoding, unsigned short *wav_format, unsigned short *bits_per_sample)
{
	if(encoding & MPG123_ENC_FLOAT_64)
	{
		*bits_per_sample = 64;
		*wav_format = WAVE_FORMAT_IEEE_FLOAT;
	}
	else if(encoding & MPG123_ENC_FLOAT_32)
	{
		*bits_per_sample = 32;
		*wav_format = WAVE_FORMAT_IEEE_FLOAT;
	}
	else if(encoding & MPG123_ENC_16)
	{
		*bits_per_sample = 16;
		*wav_format = WAVE_FORMAT_PCM;
	}
	else
	{
		*bits_per_sample = 8;
		*wav_format = WAVE_FORMAT_PCM;
	}
}

int feed(mpg123_handle *mh, FILE *in, void *buf)
{
	int ret;
	size_t len;

	len = fread(buf, sizeof(unsigned char), INBUFF, in);
	if(len <= 0)
		return 0;
	
	ret = mpg123_feed(mh, (unsigned char *) buf, len);
	if(ret != MPG123_OK)
	{
		fprintf(stderr,"Error feeding data: %s\n", mpg123_strerror(mh));
		return 0;
	}

	return 1;
}

int scan(mpg123_handle *mh, FILE *in, void *buf)
{
	int ret = mpg123_framebyframe_next(mh);
	while(ret == MPG123_NEED_MORE)
	{
		if(!feed(mh, in, buf))
			return 0;
		ret = mpg123_framebyframe_next(mh);
	}

	/* if(ret == MPG123_NEW_FORMAT)
	{
		GetOutputFormat(mh, &_SampleRate, &_NumberOfChannels, &_WavFormat, &_BitsPerSample);
		_FormatDetected = true;
	} */

	if(ret == MPG123_OK || ret == MPG123_NEW_FORMAT)
		return 1;

	fprintf(stderr,"Unknown return value: %d\n", ret);
	return 0;
}

off_t length(mpg123_handle *mh)
{
	off_t len = mpg123_length(mh);
	if(len < 0)
	{
		fprintf(stderr,"Error calling mpg123_length: %s\n", mpg123_strerror(mh));
		return 0;
	}

	return len;
}

int validate_offset(mpg123_handle *mh, off_t input_offset)
{
	off_t* offsets;
	off_t step;
	size_t fill,i;
	int ret;

	offsets = NULL;
	step = 0;
	fill = 0;
	ret = 0;

	ret = mpg123_index(mh, &offsets, &step, &fill);
	if(ret != MPG123_OK)
	{
		fprintf(stderr,"Error calling mpg123_index: %s\n", mpg123_strerror(mh));
		return 0;
	}

	for(i=0; i<fill; i++)
	{
		if(offsets[i]==input_offset)
			return 1;
	}

	return 0;
}

int seek(mpg123_handle *mh, off_t frameNumber, off_t *newFrameNumber, off_t *newInputOffset, FILE *in, void *buf)
{
	off_t ret;
	ret = 0;

	while((*newFrameNumber = mpg123_feedseek(mh, (off_t) frameNumber, SEEK_SET, newInputOffset)) == MPG123_NEED_MORE)
	{
		if (!feed(mh, in, buf))
			return 0;
	}

	if(*newFrameNumber == MPG123_ERR)
	{
		fprintf(stderr,"Error calling mpg123_feedseek: %s\n", mpg123_strerror(mh));
		return 0;
	}

	if(fseek(in, *newInputOffset, SEEK_SET) != 0)
	{
		fprintf(stderr,"Error calling fseek\n");
		return 0;
	}

	return 1;
}

int decode(mpg123_handle *mh, FILE *in, void *buf, unsigned char **out, size_t *count)
{
	int ret;

	*count = 0;

	ret = mpg123_framebyframe_next(mh);
	while(ret == MPG123_NEED_MORE)
	{
		if(!feed(mh, in, buf))
			return 0;
		ret = mpg123_framebyframe_next(mh);
	}

	/* if(ret == MPG123_NEW_FORMAT)
	{
		GetOutputFormat(_MH, &_SampleRate, &_NumberOfChannels, &_WavFormat, &_BitsPerSample);
		_FormatDetected = true;
	} */

	if(ret == MPG123_OK || ret == MPG123_NEW_FORMAT)
	{
		ret = mpg123_framebyframe_decode(mh, NULL, out, count);
		if(ret != MPG123_OK)
		{
			fprintf(stderr,"Error calling mpg123_framebyframe_decode: %s\n", mpg123_strerror(mh));
			return 0;
		}
		
		return 1;
	}
	
	fprintf(stderr,"Unknown return value: %d\n", ret);
	return 0;
}

int copy_seek_index(mpg123_handle *source, mpg123_handle *target)
{
	off_t* offsets;
	off_t step;
	size_t fill;
	int ret;

	offsets = NULL;
	step = 0;
	fill = 0;
	ret = 0;

	ret = mpg123_index(source, &offsets, &step, &fill);
	if(ret != MPG123_OK)
	{
		fprintf(stderr,"Error calling mpg123_index: %s\n", mpg123_strerror(source));
		return 0;
	}

	mpg123_set_index(target, offsets, step, fill);
	if(ret != MPG123_OK)
	{
		fprintf(stderr,"Error calling mpg123_set_index: %s\n", mpg123_strerror(target));
		return 0;
	}

	return 1;
}
