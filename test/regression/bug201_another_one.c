/* gcc -g mpg123_bug_test.c -lmpg123 -o mpg123_bug_test */
#include <stdio.h>
#include <math.h>
#include <mpg123.h>
#include <assert.h>
 
#define AST( X ) assert(MPG123_OK == (X) )
 
 
int process(const char* path, int scan_flag, off_t start_frame)
{
int err = MPG123_OK;
mpg123_handle* mh = NULL;
off_t length, pos;
long rate;
int channels;
int encoding;
size_t bufferSize;
unsigned char* buffer=NULL;
 
mh = mpg123_new(NULL, &err );
mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_QUIET, 0);
mpg123_param(mh, MPG123_RESYNC_LIMIT, -1, 0);
 
AST( mpg123_open(mh, path) );
AST( mpg123_getformat(mh, &rate, &channels, &encoding) );
AST( mpg123_format(mh, rate, channels, MPG123_ENC_SIGNED_16) );
bufferSize = mpg123_outblock(mh);
buffer = (unsigned char*) malloc(bufferSize);
size_t done;
 
printf("MPG123_API_VERSION: %d, scan_flag: %d, start_frame: %d\n", MPG123_API_VERSION, scan_flag, (int)start_frame);
if (scan_flag) {
mpg123_scan(mh); // may be a bug here
}
if (start_frame != 0) {
mpg123_seek_frame(mh, start_frame, SEEK_SET);
}
 
do {
err = mpg123_read(mh, buffer, bufferSize, &done);
} while (err!=MPG123_DONE);
 
mpg123_close(mh);
mpg123_delete(mh);
 
if (buffer) {
free(buffer);
buffer = NULL;
}
return (pos == length) ? 0 : -1;
}
 
 
int main(int argc, char **argv)
{
if(argc < 2)
{
printf("Gimme a MPEG file name...\n");
return 0;
}
const char* audio_path = argv[1];
mpg123_init();
int scan_flag = 0;
off_t start_frame = 0;
 
/**
* condition when the bug should be reproduced:
* scan_flag = 1 and start_frame = 0
*
* condition when the bug may not be reproduced:
* scan_flag = 0
* scan_flag = 1 and start_frame = 1+
**/
 
scan_flag = 1;
start_frame = 0;
process(audio_path, scan_flag, start_frame);
 
mpg123_exit();
printf("PASS\n");
return 0;
}
