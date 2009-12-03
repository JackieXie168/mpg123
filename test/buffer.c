#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

enum modes
{
	 mode_pass     = 1
	,mode_count    = 2
	,mode_progress = 4
};

int main(int argc, char **argv)
{
	int mode = 0;
	int ai;
	size_t count = 0;
	for(ai=1; ai<argc; ++ai)
	{
		if(!strcmp(argv[ai], "pass"))     mode |= mode_pass;
		else
		if(!strcmp(argv[ai], "count"))    mode |= mode_count;
		else
		if(!strcmp(argv[ai], "progress")) mode |= mode_progress;
		else
		fprintf(stderr, "buffer: unknown mode switch: %s\n", argv[ai]);
	}
	/* Loop over stdin, write out buffer if full. */
	while(1)
	{
		char buf[4096]; /* yeah, one could tune that */
		size_t fill  = 0;
		ssize_t got  = 0;
		/* fill buffer */
		while(fill < sizeof(buf))
		{
			errno = 0;
			got = read(STDIN_FILENO, buf+fill, sizeof(buf)-fill);
			if(got < 0)
			{
				fprintf(stderr, "buffer: error reading! (%s)\n", strerror(errno));
				return -1;
			}
			else
			{
				fill += got;
				if(got == 0) break;
			}
		}
		count += fill;
		if(mode & mode_progress)
		fprintf(stderr, "buffer: %zu B\n", count);
		/* flush buffer */
		if(mode & mode_pass)
		{
			write(STDOUT_FILENO, buf, fill);
		}
		if(got == 0) break; /* EOF */
	}
	if(mode & mode_count)
	{
		fprintf(stderr, "buffer: %zu B total (%zu K, %zu M)\n", count, count>>10, count>>20);
	}
	return 0;
}
