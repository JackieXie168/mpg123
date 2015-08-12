/*
	basic test if parsing of a freeformat stream works

	copyright ... oh come on, trivial code, baby!
	initially written by Thomas Orgis
*/

#include <mpg123.h>
#include "helpers.h"

int main(int argc, char **argv)
{
	int ret = 0;
	char *filename;
	mpg123_handle *m = NULL;
	off_t samplen;

	mpg123_init();

	if(argc < 2)
	{
		/* Would at least need a file to use ... */
		fprintf(stderr, "\nUsage: %s <mpeg audio file>\n\n", argv[0]);
		ret = 2; goto end;
	}
	filename = argv[1];

	m = mpg123_new(NULL, NULL);

	if(mpg123_open(m, filename) != MPG123_OK)
	{
		fprintf(stderr, "Cannot open file.\n");
		ret = 2; goto end;
	}

	samplen = mpg123_length(m);
	fprintf(stderr, "mpg123_length() = %"OFF_P"\n", samplen);
	ret = (samplen > 0) ? 0 : 1;

end:
	mpg123_delete(m);
	mpg123_exit();
	printf("%s\n", samplen > 0 ? "PASS" : "FAIL");
	return ret;
}
