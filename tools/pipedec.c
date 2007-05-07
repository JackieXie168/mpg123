/*
	Demonstration program for
	- forking a decoder (mpg123) for an audio file
	- reading all decoded samples from the pipe

	by Thomas Orgis in 2007, placed in the public domain.
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 16384

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <file.mp3>\n", argv[0]);
		return 1;
	}

	int dec_out[2];
	int pid, retval;
	char* file = argv[1];
	pipe(dec_out);
	pid=fork();
	if(pid == 0) /* decoder process */
	{
		close(dec_out[0]);
		dup2(dec_out[1],STDOUT_FILENO);
		execlp("mpg123", "mpg123", "-q", "-s", "-m", file, NULL);
		fprintf(stderr, "Start of decoder failed!\n");
		return 1;
	}

	/* back again in main process */
	if( waitpid(pid, &retval, WNOHANG) == 0 )
	{
		short buffer[BUFFER_SIZE];
		size_t  total = 0;
		FILE* stream;
		size_t  want = BUFFER_SIZE/sizeof(short);
		close(dec_out[1]);
		stream = fdopen(dec_out[0],"r");
		while(!feof(stream))
		{
			ssize_t count = fread(buffer, sizeof(short), want , stream);
			fprintf(stderr,"read %zi samples\n", count);
			total += count;
			/* do some funny stuff here... */
		}
		fclose(stream);
		fprintf(stderr, "Waiting for decoder process to end - do not want to breed zombies.\n");
		waitpid(pid, NULL, 0);
		fprintf(stderr, "Read %zu total samples.\n", total);
	}
	else
	{
		fprintf(stderr, "Decoder not alive??\n");
		return 1;
	}
	return 0;
}
