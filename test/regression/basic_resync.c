/*

	This shall test if mpg123 properly manages a bit of inserted zero bytes; skipping them via resync. The result is supposed to have unchanged length (unless I managed to cut right along a frame header) since the one broken frame is filled up and the next one properly found and decoded.

	Hm, just scanning might do the trick, too. It's only about the parser.

*/

#include <mpg123.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "helpers.h"

const int lead_in = 50000;
const int bad_bytes = 2000;

#define MIN(a,b) ((b)<(a) ? (b) : (a))

/* Bulk of the code is for preparing the damaged file. */

int maycutheader(unsigned char* buf, ssize_t bytes)
{
	if(bytes < 4) return 1;

	return (buf[bytes-1] == 0xff || buf[bytes-2] == 0xff || buf[bytes-3] == 0xff || buf[bytes-4] == 0xff);
}

int prepare_badfile(int goodfd, int badfd)
{
	off_t good_bytes;
	unsigned char buf[4096];
	off_t outpos = 0;
	off_t piece;
	unsigned long head;
	ssize_t n;
	int bad_bytes_left = bad_bytes;

	good_bytes = lseek(goodfd, 0, SEEK_END);
	lseek(goodfd, 0, SEEK_SET);

	if(good_bytes < lead_in+bad_bytes+100)
	{
		fprintf(stderr, "Too small input file.\n");
		return 0;
	}

	while(outpos < lead_in)
	{
		piece = MIN(sizeof(buf), lead_in-outpos);
		n = read(goodfd, buf, piece);
		if(n < 0 || write(badfd, buf, n) != n)
			return 0;
		outpos += n;
	}
	/* If buffer possibly happens to cut into a frame header, push some bytes. */
	while(maycutheader(buf, n))
	{
		fprintf(stderr, "Avoiding header cut.\n");
		n = read(goodfd, buf, 9);
		if(n < 0 || write(badfd, buf, n) != n)
			return 0;
		outpos += n;
	}
	/* Insert junk (zero bytes) */
	bzero(buf, sizeof(buf));
	while(bad_bytes_left)
	{
		n = write(badfd, buf, MIN(bad_bytes_left, sizeof(buf)));
		if(n < 0) return 0;

		bad_bytes_left -= n;
	}
	/* And write the rest of good data. */
	while((n=read(goodfd, buf, sizeof(buf))) > 0)
	{
		if(write(badfd, buf, n) != n) return 0;
	}
	lseek(badfd, SEEK_SET, 0);
	lseek(goodfd, SEEK_SET, 0);
	fprintf(stderr, "Successfully prepared bad file.\n");
	return 1;
}

int main(int argc, char **argv)
{
	mpg123_handle *mh;
	int ret = -1;
	int goodfd, badfd;
	char tempfile[] = "resynctest-XXXXXX";
	off_t goodsamples, badsamples;

	if(argc != 2) goto far_end;

	errno = 0;
	goodfd = open(argv[1], O_RDONLY);
	badfd = open("basic_resync-bad.mp3", O_TRUNC|O_RDWR|O_CREAT, 0666);
	fprintf(stderr, "fd: %i %i\n", goodfd, badfd);

	if(goodfd < 0 || badfd < 0)
	{
		fprintf(stderr, "cannot open files: %s\n", strerror(errno));
		goto far_end;
	}

	mpg123_init();
	mh = mpg123_new(NULL, NULL);
	mpg123_param(mh, MPG123_RESYNC_LIMIT, 3000, 0.);

	if(!prepare_badfile(goodfd, badfd)) goto end;

	mpg123_open_fd(mh, goodfd);
	mpg123_scan(mh);
	goodsamples = mpg123_length(mh);
	mpg123_open_fd(mh, badfd);
	mpg123_scan(mh);
	badsamples = mpg123_length(mh);
	mpg123_close(mh);

	fprintf(stderr, "good file samples: %"OFF_P"\n", (off_p)goodsamples);
	fprintf(stderr, "bad file  samples: %"OFF_P"\n", (off_p)badsamples);
	if(goodsamples > 0 && badsamples > 0 && goodsamples == badsamples)
		ret = 0;

end:
	close(goodfd);
	close(badfd);

	mpg123_delete(mh);
	mpg123_exit();

far_end:
	printf("%s\n", ret == 0 ? "PASS" : "FAIL");
	return ret;
}
