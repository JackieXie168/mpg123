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

/*
	With the original test file (non-public):
	This seek sequence passes: 8719191, 8318528
	This seek sequence fails: 8528399, 8547479

	With the default testfile (drum.mp3):
	528399, 547479
	(First guess and a hit the first time!)
*/

#define SEEK_1 528399
#define SEEK_2 547479

int main(int argc, char **argv)
{
	unsigned char buf[INBUFF];
	unsigned char *audio;
	FILE *in;
	mpg123_handle *m,*m_mpg123;
	int ret, mret;
	size_t bytes;
	off_t frame, inoffset, scanned, scanned_mpg123, decalc_mpg123, decoded_mpg123, decoded_calculated, seek_to;

	if(argc < 2){ fprintf(stderr, "Gimme a file!\n"); return -1; }

	in = fopen(argv[1], "rb");
	if(in == NULL)
	{
		fprintf(stderr,"Unable to open input file\n");
		return -1;
	}

	mpg123_init();

	init_handle(&m_mpg123, MPG123_MONO,  MPG123_ENC_FLOAT_32);
	mpg123_param(m_mpg123, MPG123_ADD_FLAGS, MPG123_IGNORE_STREAMLENGTH, 0.0);
	ret = mpg123_open(m_mpg123, argv[1]);
	fprintf(stderr, "\nPART 1: scan with mpg123\n\n");
	ret = mpg123_scan(m_mpg123);
	if(ret == MPG123_OK) {
		scanned_mpg123 = mpg123_length(m_mpg123);
	}
	fprintf(stderr,"scanning using mpg123_scan returned %d audio frames\n", scanned_mpg123);

	if(scanned_mpg123 <= SEEK_1 || scanned_mpg123 <= SEEK_2)
	{
		mpg123_close(m_mpg123);
		mpg123_exit();
		fprintf(stderr, "The file is too small for the configured offsets!\n");
		printf("FAIL\n");
		return -1;
	}

	fprintf(stderr, "\nPART 2: seek around and decode with mpg123\n\n");

	mret = mpg123_read(m_mpg123, buf, INBUFF, &bytes); /* that just gets new_format */

	fprintf(stderr, "Seek to %d\n", SEEK_1);
	mpg123_seek(m_mpg123, SEEK_1, SEEK_SET);
	fprintf(stderr, "Position: %"OFF_P"\n", (off_p)mpg123_tell(m_mpg123));
	/* Mimic the operation with the feeder: first decode a bit, then seek. */
	/* not giving full buffer here for better conparison with code below.. it decodes only one frame. */
	mret = mpg123_read(m_mpg123, buf, 2024, &bytes);
	fprintf(stderr, "Seek to %d\n", SEEK_2);
	mpg123_seek(m_mpg123, SEEK_2, SEEK_SET);
	decalc_mpg123 = mpg123_tell(m_mpg123);
	fprintf(stderr, "Position: %"OFF_P"\n", (off_p)decalc_mpg123);

	do
	{
		mret = mpg123_read(m_mpg123, buf, INBUFF, &bytes);
		decalc_mpg123 += bytes/4;
	} while(mret != MPG123_DONE && mret != MPG123_ERR);
	fprintf(stderr, "Last return: %i\n", mret);

	fprintf(stderr, "Length seek/decode with plain mpg123 read: %"OFF_P"\n", (off_p)decalc_mpg123);

	mpg123_close(m_mpg123);

	fprintf(stderr, "\nPART 3: scan with framebyframe / feeder\n\n");

	init_handle(&m, MPG123_MONO,  MPG123_ENC_FLOAT_32);
	mpg123_param(m, MPG123_ADD_FLAGS, MPG123_IGNORE_STREAMLENGTH, 0.0);

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
	fprintf(stderr, "scanning using mpg123_framebyframe_next returned %"OFF_P" audio frames\n", (off_p)scanned);

	fprintf(stderr, "\nPART 4: seek around and decode with feeder\n\n");

	seek_to = SEEK_1;
	fprintf(stderr, "Seek to %"OFF_P"\n", (off_p)seek_to);
	seek(m, seek_to, &frame, &inoffset, in, buf);
	fprintf(stderr, "Position: %"OFF_P"\n", (off_p)mpg123_tell(m));
	decode(m, in, buf, &audio, &bytes); /* Taking this line out results in a passed test */
	
	decoded_calculated = SEEK_2;
	fprintf(stderr, "Seek to %"OFF_P"\n", (off_p)decoded_calculated);
	seek(m, decoded_calculated, &frame, &inoffset, in, buf);
	fprintf(stderr, "Position: %"OFF_P"\n", (off_p)mpg123_tell(m));
	decoded_calculated = frame;
	while(decode(m, in, buf, &audio, &bytes))
	{
		/* mono float samples */
		decoded_calculated += (bytes / 4);
	}

	decoded_mpg123 = length(m);
	fprintf(stderr, "Length after decoding from mpg123_length: %"OFF_P"\n", (off_p)decoded_mpg123);

	fprintf(stderr, "Length after decoding calculated from decoded bytes: %"OFF_P"\n", (off_p)decoded_calculated);

	fclose(in);
	mpg123_delete(m);
	mpg123_exit();

	ret = 0;
	fprintf(stderr, "Summary:\n");
	fprintf(stderr, " mpg123 scan:         %"OFF_P"\n", (off_p)scanned_mpg123);
	fprintf(stderr, " mpg123 seekdec:      %"OFF_P"\n", (off_p)decalc_mpg123);
	fprintf(stderr, " f2f scan:            %"OFF_P"\n", (off_p)scanned);
	fprintf(stderr, " f2f seekdec:         %"OFF_P"\n", (off_p)decoded_calculated);
	fprintf(stderr, " f2f s/d mpg123 len:  %"OFF_P"\n", (off_p)decoded_mpg123);

	if(scanned != decoded_mpg123
		|| scanned != decalc_mpg123
		|| scanned != decoded_calculated
		|| scanned != scanned_mpg123)
		ret = 1;
	printf("%s\n", ret == 0 ? "PASS" : "FAIL");

	return ret;
}
