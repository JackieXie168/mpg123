#include <out123.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

/*
	Simply pass a file through out123 to raw output, verifying that
	every byte comes through unharmed. Nothing with actual audio
	because that needs digital ears.
*/

const char *outfile = "out123_passthrough.out";

int main(int argc, char **argv)
{
	int in, out, ret;
	off_t inlen = 0, outlen = 0;
	ssize_t chunk;
	char inbuf[4096], outbuf[4096];
	out123_handle *ao;
	if(argc < 2) return -1;

	/* Going through without error checking to test robustness of things.
	   The test only is successful if the resulting output matches. */
	in = open(argv[1], O_RDONLY);
	ao = out123_new();
	unlink(outfile); // Ensure it does not exist if out123 fails.
	out123_open(ao, "raw", outfile);
	/* Format should not matter, but we stay true to the promise of
	   always feeding full PCM frames, single bytes in this case. */
	out123_start(ao, 1000, 1, MPG123_ENC_SIGNED_8);
	while( (chunk=read(in, inbuf, sizeof(inbuf))) > 0 )
	{
		inlen += chunk;
		out123_play(ao, inbuf, chunk);
	}
	out123_del(ao);

	fprintf(stderr, "wrote output, comparing\n");
	lseek(in, SEEK_SET, 0);
	out = open(outfile, O_RDONLY);

	/* Only coult output bytes that match. */
	while(
		read(out,outbuf,sizeof(outbuf)) == (chunk=read(in, inbuf, sizeof(inbuf)))
	&&	chunk > 0
	&&	!memcmp(outbuf, inbuf, chunk)
	)
		outlen += chunk;
	ret = inlen==outlen ? 0 : 1;

	close(out);
	close(in);
	printf("%s\n", ret ? "FAIL" : "PASS");
	return ret;
}
