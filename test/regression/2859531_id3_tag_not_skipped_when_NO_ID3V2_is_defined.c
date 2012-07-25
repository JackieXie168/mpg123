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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mpg123.h"
#include "helpers.h"

#define INBUFF  16384 * 2 * 2

const off_t expected = 4926328;

int main(int argc, char* argv[])
{
	unsigned char buf[INBUFF];
	unsigned char *audio;
	FILE *in;
	mpg123_handle *m;
	int ret = 0;
	off_t len, num;
	size_t bytes;
	long rate;
	int channels, enc;
	off_t total = 0;

	if(argc < 2){ fprintf(stderr, "Gimme file!\n"); return -1; }

	in = fopen(argv[1], "rb");
	if(in == NULL)
	{
		fprintf(stderr,"Unable to open input file %s\n", argv[1]);
		return -1;
	}

	mpg123_init();

	init_handle(&m, MPG123_MONO | MPG123_STEREO,  MPG123_ENC_FLOAT_32);
	if(mpg123_param(m, MPG123_ADD_FLAGS, MPG123_GAPLESS, 0) != MPG123_OK)
	{
		fprintf(stderr, "Gapless code is not available, this test does not work that way. Letting it just PASS.\n");
		printf("PASS\n");
		return 0;
	}
	ret = mpg123_open_feed(m);
	if(ret != MPG123_OK)
	{
		fprintf(stderr,"Unable to open feed: %s\n", mpg123_strerror(m));
		return ret;
	}

	while(1)
	{
		len = fread(buf, sizeof(unsigned char), INBUFF, in);
		/* printf("read %d bytes\n", len); */
		if(len <= 0)
			break;
		ret = mpg123_feed(m, buf, len);
		/* printf("mpg123_feed %d\n", ret); */

		while(ret != MPG123_ERR && ret != MPG123_DONE && ret != MPG123_NEED_MORE)
		{
			ret = mpg123_decode_frame(m, &num, &audio, &bytes);
			total += bytes;
			/* printf("mpg123_decode_frame %d\n", ret); */
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

	fprintf(stderr, "total %"OFF_P":\n", (off_p)total);
	/* Newer MPG123 does not automatically cut decoder delay of 529 samples. */
	if(total != expected && total != expected+529*4*2)
	{
		fprintf(stderr, "Total byte count wrong, skipping of ID3v2 tag did not work out? (%"OFF_P" != %"OFF_P")\n", total, expected);
		ret = -1;
	}
	else ret = 0;

	printf("%s\n", ret == 0 ? "PASS" : "FAIL");

	return ret;
}
