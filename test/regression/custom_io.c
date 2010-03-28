/* Check if custom I/O really brings identical results. */

#include <mpg123.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "helpers.h"

/*
	I/O variants:
	1. internal I/O, mpg123_open()
	2. internal I/O with my file descriptor, mpg123_open_fd()
	3. replaced I/O on my file descriptor
	   This could possibly use mpg123_open() or mpg123_open_fd(), I restrict it to the latter.
	4. replaced I/O on my handle
*/
#define iovariants 4
#define piece 100
#define positions 2

int read_stuff(mpg123_handle *mh, unsigned char buf[positions][piece], off_t pos[positions]);

int read_calls = 0;
int seek_calls = 0;

int hread_calls = 0;
int hseek_calls = 0;
int cleanup_calls = 0;

ssize_t my_read(int fd, void *buf, size_t count);
off_t my_seek(int fd, off_t offset, int whence);

ssize_t my_h_read(void *handle, void *buf, size_t count);
off_t my_h_seek(void *handle, off_t offset, int whence);
void my_h_cleanup(void *handle);

struct my_handle
{
	char garbage[1337];
	int fd;
};

int main(int argc, char **argv)
{
	const char *filename;
	off_t pos[positions] = {0, 50}; /* percent of whole file length */
	/* Decoded bytes reference and current method, each seek position. */
	unsigned char buf[2][positions][piece];
	mpg123_handle *mh;
	off_t length;
	int i;
	int ret = 0;
	int fd;
	struct my_handle* ioh;

	if(argc < 2){ fprintf(stderr, "Gimme a file!\n"); ret = 1; goto realend; }

	filename = argv[1];

	if(mpg123_init() != MPG123_OK)
	{
		fprintf(stderr, "mpg123_init failed\n");
		ret = 2; goto realend;
	}

	mh = mpg123_new(NULL, NULL);
	if(mh == NULL)
	{
		fprintf(stderr, "handle creation failed\n");
		ret = 3; goto end;
	}

	/* Variant 1: Internal I/O. */

	if(mpg123_open(mh, filename) != MPG123_OK)
	{
		fprintf(stderr, "cannot open\n");
		ret = 4; goto end;
	}

	length = mpg123_length(mh);

	for(i=0; i<positions; ++i)
	pos[i] = pos[i]*length/100;

	if(read_stuff(mh, buf[0], pos) != MPG123_OK)
	{
		fprintf(stderr, "reading failed\n");
		ret = 5; goto end;
	}

	mpg123_close(mh);

	/* Variant 2: Internal I/O with my fd. */
	fd = open(filename, O_RDONLY);
	if(mpg123_open_fd(mh, fd) != MPG123_OK)
	{
		fprintf(stderr, "cannot open descriptor\n");
		ret = 6; goto end;
	}

	if(read_stuff(mh, buf[1], pos) != MPG123_OK)
	{
		fprintf(stderr, "reading failed\n");
		ret = 7; goto end;
	}

	mpg123_close(mh);
	close(fd);

	if(memcmp(buf[0], buf[1], positions*piece) != 0)
	{
		fprintf(stderr, "internal I/O with my fd bad\n");
		ret = 8; goto end;
	}

	/* Variant 3: Replaced I/O with my fd. */

	seek_calls = 0;
	read_calls = 0;
	fd = open(filename, O_RDONLY);
	mpg123_replace_reader(mh, my_read, my_seek);

	if(mpg123_open_fd(mh, fd) != MPG123_OK)
	{
		fprintf(stderr, "cannot open descriptor\n");
		ret = 9; goto end;
	}

	if(read_stuff(mh, buf[1], pos) != MPG123_OK)
	{
		fprintf(stderr, "reading failed\n");
		ret = 10; goto end;
	}

	mpg123_close(mh);
	mpg123_replace_reader(mh, NULL, NULL);
	close(fd);

	if(seek_calls == 0 || read_calls == 0)
	{
		fprintf(stderr, "callbacks not called\n");
		ret = 11; goto end;
	}

	if(memcmp(buf[0], buf[1], positions*piece) != 0)
	{
		fprintf(stderr, "replaced I/O with my fd bad\n");
		ret = 12; goto end;
	}

	/* Variant 4: Replaced I/O with my handle. */
#if (defined MPG123_API_VERSION) && (MPG123_API_VERSION >= 24)
	hseek_calls = 0;
	hread_calls = 0;
	ioh = malloc(sizeof(struct my_handle));
	fd = -1; /* Just for clarity, we're not operating on that fd now. */
	ioh->fd = open(filename, O_RDONLY);
	mpg123_replace_reader_handle(mh, my_h_read, my_h_seek, my_h_cleanup);

	if(mpg123_open_handle(mh, ioh) != MPG123_OK)
	{
		fprintf(stderr, "cannot open handle\n");
		ret = 13; goto end;
	}

	if(read_stuff(mh, buf[1], pos) != MPG123_OK)
	{
		fprintf(stderr, "reading failed\n");
		ret = 14; goto end;
	}

	mpg123_close(mh);
	/* The  cleanup function should have closed the fd now. */
	mpg123_replace_reader(mh, NULL, NULL);

	if(hseek_calls == 0 || hread_calls == 0 || cleanup_calls == 0)
	{
		fprintf(stderr, "callbacks not called\n");
		ret = 15; goto end;
	}

	if(memcmp(buf[0], buf[1], positions*piece) != 0)
	{
		fprintf(stderr, "replaced I/O with my handle bad\n");
		ret = 16; goto end;
	}
	/* Make sure mpg123 does not do funny things when I don't hand in a cleanup handler. */
	ioh = malloc(sizeof(struct my_handle));
	fd = -1; /* Just for clarity, we're not operating on that fd now. */
	ioh->fd = open(filename, O_RDONLY);
	fprintf(stderr, "fd for testing non-cleanup: %i\n", ioh->fd);
	mpg123_replace_reader_handle(mh, my_h_read, my_h_seek, NULL);

	if(mpg123_open_handle(mh, ioh) != MPG123_OK)
	{
		fprintf(stderr, "cannot open handle again\n");
		ret = 17; goto end;
	}

	mpg123_close(mh);
	fprintf(stderr, "fd after mpg123_close: %i\n", ioh->fd);
	/* Now clean up explictly... */
	my_h_cleanup(ioh);
#else
	fprintf(stderr, "I/O with handle not supported in this API version.\n");
#endif

end:
	mpg123_delete(mh);
	mpg123_exit();
realend:
	printf("%s\n", ret == 0 ? "PASS" : "FAIL");
	return ret;
}

int read_stuff(mpg123_handle *mh, unsigned char buf[positions][piece], off_t pos[positions])
{
	int err;
	int i;
	int enc, chan;
	long rate;

	memset(buf, 0, positions*piece);
	mpg123_getformat(mh, &rate, &chan, &enc);
	mpg123_format_none(mh);
	mpg123_format(mh, rate, chan, enc);
	for(i=0; i<positions; ++i)
	{
		size_t done = 0;
		if(   mpg123_seek(mh, pos[i], SEEK_SET) != pos[i]
		   || mpg123_read(mh, buf[i], piece, &done) != MPG123_OK
		   || done != piece )
		{
			fprintf(stderr, "seeking/reading failed, %zu/%i (%s)\n", done, piece, mpg123_strerror(mh));
			return MPG123_ERR;
		}
	}
	return MPG123_OK;
}

ssize_t my_read(int fd, void *buf, size_t count)
{
	++read_calls;
	fprintf(stderr, "my_read: %zu bytes\n", count);
	return read(fd, buf, count);
}

off_t my_seek(int fd, off_t offset, int whence)
{
	++seek_calls;
	fprintf(stderr, "my_seek: offset %"OFF_P" (%i)\n", (off_p)offset, whence);
	return lseek(fd, offset, whence);
}

ssize_t my_h_read(void *handle, void *buf, size_t count)
{
	struct my_handle *ioh = (struct my_handle*)handle;
	++hread_calls;
	fprintf(stderr, "my_h_read: %zu bytes\n", count);
	return read(ioh->fd, buf, count);
}

off_t my_h_seek(void *handle, off_t offset, int whence)
{
	struct my_handle *ioh = (struct my_handle*)handle;
	++hseek_calls;
	fprintf(stderr, "my_h_seek: offset %"OFF_P" (%i)\n", (off_p)offset, whence);
	return lseek(ioh->fd, offset, whence);
}

void my_h_cleanup(void *handle)
{
	struct my_handle *ioh = (struct my_handle*)handle;
	++cleanup_calls;
	fprintf(stderr, "cleanup handle with fd %i\n", ioh->fd);
	close(ioh->fd);
	free(ioh);
}
