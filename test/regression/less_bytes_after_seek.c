/*

	When seeking and decoding several times in a file and then continuing to decode to the end of the file
	the return value of mpg123_length is to small. The number of actually decoded bytes is also to small.
	This happens only for certain combinations of places to seek to followed by decoding.

	libmpg123: MPEG Audio Decoder library

	copyright 1995-2009 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mpg123.h"
#include "helpers.h"

int main(int argc, char **argv)
{
	unsigned char buf[INBUFF];
	unsigned char *audio;
	FILE *in;
	mpg123_handle *m,*m_mpg123;
	int ret;
	size_t bytes;
	off_t frame, inoffset, scanned, scanned_mpg123, decoded_mpg123, decoded_calculated, seek_to;

	if(argc < 2){ fprintf(stderr, "Gimme a file!\n"); return -1; }

	in = fopen(argv[1], "rb");
	if(in == NULL)
	{
		fprintf(stderr,"Unable to open input file\n");
		return -1;
	}

	mpg123_init();

	init_handle(&m_mpg123, MPG123_MONO | MPG123_STEREO,  MPG123_ENC_FLOAT_32);
	ret = mpg123_open(m_mpg123, argv[1]);
	ret = mpg123_scan(m_mpg123);
	if(ret == MPG123_OK) {
		scanned_mpg123 = mpg123_length(m_mpg123);
	}
	mpg123_close(m_mpg123);
	printf("scanning using mpg123_scan returned %d frames\n", scanned_mpg123);

	init_handle(&m, MPG123_MONO | MPG123_STEREO,  MPG123_ENC_FLOAT_32);

	ret = mpg123_open_feed(m);
	if(ret != MPG123_OK)
	{
		fprintf(stderr,"Unable to open feed: %s\n", mpg123_strerror(m));
		return ret;
	}

	while(scan(m, in, buf)) {
		;
	}
	
	scanned = length(m);
	fprintf(stdout, "scanning using mpg123_framebyframe_next returned %d frames\n", scanned);
	
	/* This seek sequence passes: 8719191, 8318528 */
	/* This seek sequence fails: 8528399, 8547479 */

	seek_to = 100000;
	fprintf(stdout, "Seek to %d\n", seek_to);
	seek(m, seek_to, &frame, &inoffset, in, buf);
	decode(m, in, buf, &audio, &bytes); /* Taking this line out results in a passed test */
	
	decoded_calculated = 0;
	fprintf(stdout, "Seek to %d\n", decoded_calculated);
	seek(m, decoded_calculated, &frame, &inoffset, in, buf);
	while(decode(m, in, buf, &audio, &bytes)) {
		decoded_calculated += (bytes / 2 / 4);
	}

	decoded_mpg123 = length(m);
	fprintf(stdout, "Length after decoding from mpg123_length: %d\n", decoded_mpg123);

	fprintf(stdout, "Length after decoding calculated from decoded bytes: %d\n", decoded_calculated);

	fclose(in);
	mpg123_delete(m);
	mpg123_exit();

	ret = 0;
	if(scanned != decoded_mpg123
		|| scanned != decoded_calculated
		|| scanned != scanned_mpg123) ret = 1;
	printf("%s\n", ret == 0 ? "PASS" : "FAIL");

	return ret;
}
