/*
	The problem: mpg123_decoder() causes a segfault when called before a track is open.

	This has been diagnosed to an uninitialized fr->sampling_frequency in frame_freq() ... two causes:
	1. Sampling_frequency is not initialized to something non-dangerous.
	2. mpg123_decoder() calls decode_update() ... even if no track is open.

	Solution: Initialize sampling_frequency to 0 and prevent the premature call to decode_update.

	Note: It need not to get a segfault, but valgrind will complain about use of uninitialized values... -lduma could help, too.
*/

#include <mpg123.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

int main()
{
	mpg123_handle *mh = NULL;
	int errsum = 0;
	int err;
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	if(mh)
	{
		const char **dec = mpg123_supported_decoders();
		while(*dec != NULL)
		{
			fprintf(stderr, "switching decoder to %s\n", *dec);
			err = mpg123_decoder(mh, *dec);
			if(err != MPG123_OK)
			{
				fprintf(stderr, "Cannot switch decoder: %s\n", mpg123_strerror(mh));
				++errsum;
			}
			++dec;
		}
		mpg123_delete(mh);
	}
	else
	{
		fprintf(stderr, "Cannot create handle: %s\n", mpg123_plain_strerror(err));
		++errsum;
	}

	mpg123_exit();

	printf("%s\n", errsum ? "FAIL" : "PASS");
	return errsum;
}

