/*
	bug201: combination of seek and resync triggering a buffer overflow on
	libmpg123 versions that fail to re-initialize the decoder structure
	appropriately

	copyright 2014 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

#include <mpg123.h>
#include "helpers.h"

const off_t frameskip = 10;

int main(int argc, char **argv)
{
	int ret = 0;
	char *filename;
	mpg123_handle *m = NULL;
	off_t num;
	unsigned char *audio;
	size_t bytes;

	mpg123_init();

	if(argc < 2)
	{
		/* Would at least need a file to use ... */
		fprintf(stderr, "\nUsage: %s <mpeg audio file>\n\n", argv[0]);
		ret = -1; goto end;
	}
	filename = argv[1];

	m = mpg123_new(NULL, NULL);

#if (defined MPG123_API_VERSION && MPG123_API_VERSION >= 18)
	if(mpg123_param(m, MPG123_PREFRAMES, 4, 0) != MPG123_OK)
	{
		fprintf(stderr, "Cannot set preframes to 4. Needed for this bug.\n");
		ret = -1; goto end;
	}
#endif

	if(mpg123_open(m, filename) != MPG123_OK)
	{
		fprintf(stderr, "Cannot open file.\n");
		ret = -1; goto end;
	}

	/* Now see if we crash. Failure to seek is also fine, since the test
	   data is supposed to contain junk. */
	mpg123_seek_frame(m, 10, SEEK_SET);
	mpg123_decode_frame(m, &num, &audio, &bytes);

	printf("Still here!\n");
end:
	mpg123_delete(m);
	mpg123_exit();
	printf("%s\n", ret == 0 ? "PASS" : "FAIL");
	return ret;
}
