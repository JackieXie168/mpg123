/*
	Clive found strange behaviour in frame-based seeking:

	-----------------
	I am trying to seek to a frame and decode it.

	off_t fNum;
	size_t bytes;
	mpg123_seek_frame(hMp3, 300, SEEK_SET);
	mpg123_decode_frame(hMp3, &fNum, &bbuff, &bytes);

	This above works fine.

	If I then seek to frame 299 and decode it and then to frame 298 and
	decode that it does not work.
	&fNum shows I have actually just decoded frames 301 and 302.

	mpg123_seek_frame(hMp3, 299, SEEK_SET);
	mpg123_decode_frame(hMp3, &fNum, &bbuff, &bytes);

	mpg123_seek_frame(hMp3, 298, SEEK_SET);
	mpg123_decode_frame(hMp3, &fNum, &bbuff, &bytes);

	It appears that after the first seek the next two seeks are ignored.
	------------------

	For now I cannot reproduce that. Adding the test nontheless.
*/

#include <stdio.h>
#include <sys/types.h>
#include "mpg123.h"

#define CHECK(msg, call) \
fprintf(stderr, "%s\n", msg); \
if((ret = call) == MPG123_ERR) goto handleend;

int main(int argc, char **argv)
{
	mpg123_handle *mh;
	int ret;
	size_t bytes;
	off_t frame;

	if(argc < 2){ fprintf(stderr, "Gimme a file!\n"); return -1; }

	mpg123_init();
	mh = mpg123_new(NULL, &ret);
	if(mh == NULL)
	{
		fprintf(stderr, "cannot create handle: %s\n", mpg123_plain_strerror(ret));
		goto end; 
	}

	CHECK("open", mpg123_open(mh, argv[1]))

	fprintf(stderr, "initial seek\n");
	frame = mpg123_seek_frame(mh, 300, SEEK_SET);
	fprintf(stderr, "seek returned %li\n", (long)frame);
	if(frame < 0) goto handleend;

	CHECK("initial decode", mpg123_decode_frame(mh, &frame, NULL, NULL))
	if(frame != 300)
	{
		fprintf(stderr, "Got frame %li instead of 300!\n", (long)frame);
		goto handleend;
	}
	fprintf(stderr, "second seek\n");
	frame = mpg123_seek_frame(mh, 299, SEEK_SET);
	fprintf(stderr, "seek returned %li\n", (long)frame);
	if(frame < 0) goto handleend;

	CHECK("second decode", mpg123_decode_frame(mh, &frame, NULL, NULL))
	if(frame != 299)
	{
		fprintf(stderr, "Got frame %li instead of 299!\n", (long)frame);
		goto handleend;
	}

handleend:
	if(ret != MPG123_OK)
	{
		fprintf(stderr, "mpg123 error: %s", mpg123_strerror(mh));
	}
	mpg123_delete(mh);

end:
	mpg123_exit();
	printf("%s\n", ret == 0 ? "PASS" : "FAIL");

	return ret;
}
