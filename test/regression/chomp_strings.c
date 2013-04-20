#include <mpg123.h>
#include <stdio.h>
#include <string.h>

/* The first implementation of mpg123_chomp_string() has a serious bug when used
   on strings of zero length: A loop runs wild. */

int chomptest(const char *input, const char *output, const char *desc)
{
	int err = 0;
	mpg123_string ms;
	fprintf(stderr, "chomping for '%s' (%s)\n", output, desc);
	mpg123_init_string(&ms);
	mpg123_set_string(&ms, input);
	mpg123_chomp_string(&ms);
	fprintf(stderr, "chomped      '%s'\n", ms.p);
	if(strcmp(ms.p, output)) ++err;

	mpg123_free_string(&ms);
	return err;
}

int main(int argc, char **argv)
{
	int i;
	int err = 0;

	err += chomptest("just line", "just line", "no line end");
	err += chomptest("cr line\r", "cr line", "text with cr");
	err += chomptest("lf line\n", "lf line", "text with lf");
	err += chomptest("crlf line\r\n", "crlf line", "text with crlf");
	err += chomptest("\r", "", "just cr");
	err += chomptest("\n", "", "just lf");
	err += chomptest("\r\n", "", "just crlf");
	err += chomptest("\r\n\r\n\r\n\r\n", "", "just many crlf");
	err += chomptest("", "", "empty");
	err += chomptest("crlf line\r\nwith more\r\n\n\n\r\r\n\n", "crlf line\r\nwith more", "two lines and lots of breaks");

	printf("%s\n", err ? "FAIL" : "PASS");
	return err ? -1 : 0;
}
