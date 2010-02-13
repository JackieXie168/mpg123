#include <mpg123.h>
#include <stdio.h>
#include "helpers.h"

/*
	tulskiy writes:
	I have this code that opens the file and calls seek to a sample before
	last. If I try to decode the file after that, I get extra samples (garbage
	that should have been removed using enc_padding from Lame tag). So if my
	file is 29400 samples, I seek to sample 29300, and then decode the rest, i
	get 533 samples instead of only 100. Is the code wrong?

	Test code from the bug report, a bit of modification by Thomas.
*/

int check(const char *testfile)
{
	int cret = 0;
	mpg123_handle *m;
	unsigned char buf[5000];
	size_t done;
	int ret, channels, encoding;
	long rate;

	m = mpg123_new(NULL, NULL);

	mpg123_open(m, testfile);
	ret = mpg123_decode(m, NULL, 0, buf, sizeof(buf), &done);	
	if(ret == MPG123_NEW_FORMAT)
	mpg123_getformat(m, &rate, &channels, &encoding);

	off_t length = mpg123_length(m);

	/* Seek to 100 samples to the end. */
	off_t seek_to = mpg123_seek(m, 100, SEEK_END);
	fprintf(stderr, "Seeked to %"OFF_P" of %"OFF_P"\n", (off_p)seek_to, (off_p)length);

	off_t total = 0;

	while(1)
	{
		int ret = mpg123_decode(m, NULL, 0, buf, sizeof(buf), &done);
		/* Default is 16 bit samples, always. */
		total += done / (2 * channels);

		if(ret == MPG123_DONE)
		break;
	}

	printf("Length: %"OFF_P". Decoded: %"OFF_P". Diff: %"OFF_P"\n", (off_p)length, (off_p)total, (off_p)(length - seek_to - total));
	if(length - seek_to != total) cret = -1;

	mpg123_delete(m);

	printf("%s\n", cret == 0 ? "PASS" : "FAIL");
	return cret;
}

int main(int argc, char* argv[])
{
	mpg123_init();
	if(argc < 2){ fprintf(stderr, "Gimme the file!\n"); return -1; }

	return check(argv[1]);
}
