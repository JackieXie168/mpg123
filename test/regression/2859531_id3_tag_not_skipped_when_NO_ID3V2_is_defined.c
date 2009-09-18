/*
	id3 tag not skipped when NO_ID3V2 is defined

	test code for sourceforge tracker bug id 2859531:
	https://sourceforge.net/tracker/?func=detail&atid=733194&aid=2859531&group_id=135704
	link this programm to libmpg123 compiled with NO_ID3V2 to trigger the regression 

	libmpg123: MPEG Audio Decoder library

	copyright 1995-2009 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

*/

/*
Code below takes a resyncing code path:

mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_NEED_MORE
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_DONE
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_DONE
...
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_NEW_FORMAT
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_OK
...

Remove ret != MPG123_DONE from the while loop below
to take an id3v2 skipping code path:

mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_NEED_MORE
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_DONE
mpg123_decode_frame MPG123_NEED_MORE
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_NEED_MORE
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_DONE
mpg123_decode_frame MPG123_DONE
mpg123_decode_frame MPG123_DONE
mpg123_decode_frame MPG123_NEED_MORE
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_NEW_FORMAT
mpg123_decode_frame MPG123_OK
...

mpg123_decode_frame in both code paths returns MPG123_DONE which does look incorrect feeder mode. 
I would expect the following:

mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_NEED_MORE
mpg123_feed MPG123_OK
mpg123_decode_frame MPG123_NEED_MORE
...
mpg123_decode_frame MPG123_NEW_FORMAT
mpg123_decode_frame MPG123_OK
...

And it should just skip the id3v2 not resynci. 

This is the case when this program is linked to a libmpg123 where the NO_ID3V2 define is removed.

*/

#include <stdio.h>
#include <io.h>
#include "mpg123.h"

#define INBUFF  16384 * 2 * 2

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

	ret = mpg123_param(*mh, MPG123_FLAGS, MPG123_FUZZY | MPG123_SEEKBUFFER | MPG123_GAPLESS, 0);
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

int main(int argc, char* argv[])
{
	unsigned char buf[INBUFF];
	unsigned char *audio;
	FILE *in;
	mpg123_handle *m;
	int ret;
	off_t len, num;
	size_t bytes;
	long rate;
	int channels, enc;

	in = fopen("2859531_id3_tag_not_skipped_when_NO_ID3V2_is_defined.mp3", "rb");
	if(in == NULL)
	{
		fprintf(stderr,"Unable to open input file %s\n", argv[1]);
		return -1;
	}

	mpg123_init();

	init_handle(&m, MPG123_MONO | MPG123_STEREO,  MPG123_ENC_FLOAT_32);

	ret = mpg123_open_feed(m);
	if(ret != MPG123_OK)
	{
		fprintf(stderr,"Unable to open feed: %s\n", mpg123_strerror(m));
		return ret;
	}

	while(1)
	{
		len = fread(buf, sizeof(unsigned char), INBUFF, in);
		printf("read %d bytes\n", len);
		if(len <= 0)
			break;
		ret = mpg123_feed(m, buf, len);
		printf("mpg123_feed %d\n", ret);

		while(ret != MPG123_ERR && ret != MPG123_DONE && ret != MPG123_NEED_MORE)
		{
			ret = mpg123_decode_frame(m, &num, &audio, &bytes);
			printf("mpg123_decode_frame %d\n", ret);
			if(ret == MPG123_NEW_FORMAT)
				mpg123_getformat(m, &rate, &channels, &enc);
		}

		if(ret == MPG123_ERR)
		{
			printf("Error: %s", mpg123_strerror(m));
			break; 
		}
	}

	fclose(in);
	mpg123_delete(m);
	mpg123_exit();

	return 0;
}
