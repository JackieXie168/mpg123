/* this is a demonstration of the incorrect threaded behavior of mpg123 lib.
 * an .mpg file is decoded rwice in parallel and the md5sum of the resulting
 * files are printed 
 * 
 * the expected behavior is that both outputs are identical but the actual 
 * behavior is that the two outputs are different.
 */


#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <mpg123.h>
#include <string.h>

pthread_mutex_t mutex;
char * condition;
char * decoder;
char * infile;
char outfile1[100];
char outfile2[100];
char mdsum1[32];
char mdsum2[32];
char pass[1];

FILE *fp;

void *thread_function (void *arg) {
	
	int ret;
	strcpy(outfile2,infile);
	strcat(outfile2, "_output2.wav");
	ret = mpg123_sample(infile, decoder,condition, outfile2, &mutex);
	return NULL;
	}

int main(int argc, char *inputarg[]) {
	
	pthread_t mythread;
	int ret;
	char systemcall1 [100];
	char systemcall2 [100];
	
	condition = inputarg[1];
	decoder = inputarg[2];
	infile = inputarg[3];

/* verify decoder is a valid decoder - valid decoders are 
 * 		SSE MMX i586 i386 generic*/
	
	ret = 0;
	if (strcmp (decoder, "SSE") == 0)ret = 1;
	if (strcmp (decoder, "MMX") == 0)ret = 1;
	if (strcmp (decoder, "i586") == 0)ret = 1;
	if (strcmp (decoder, "i386") == 0)ret = 1;
	if (strcmp (decoder, "generic") == 0)ret = 1;
	if (ret== 0){
	        printf("You entered an invalid decoder value %s\n", decoder);
			abort();
	   }

	ret = pthread_mutex_init(&mutex, NULL);
	if (strcmp(condition, "cond3") == 0) mpg123_init();
	
	if (pthread_create( &mythread, NULL, thread_function, NULL)){
			printf ("error creating thread");
			abort();
		}
	
	strcpy(outfile1,infile);
	strcat(outfile1, "_output1.wav");
	ret = mpg123_sample(infile, decoder,condition, outfile1, &mutex);
	
	if (pthread_join (mythread, NULL)){
			printf("error joining thread");
			abort();
	}

	strcpy(systemcall1,"md5sum ");
	strcat(systemcall1,outfile1);
	strcat(systemcall1, ">md51");
	system(systemcall1);
	

	
	strcpy(systemcall2,"md5sum ");
	strcat(systemcall2,outfile2);
	strcat(systemcall2, ">md52");
	system(systemcall2);
/********************
 * now we create a file with the results comma separated.
 * the file has the condition, the decoder, md5sum from output 1,
 * md5sum from output 2.
 * we append the results of this execution on to the results
 * of prior executions.
 */
	fp = fopen("md51", "r");
	fread(mdsum1,32,1,fp);
	fclose(fp);
	fp = fopen("md52", "r");
	fread(mdsum2,32,1,fp);
	fclose(fp);
	
	strcpy (pass, "P");
	if (strncmp(mdsum1, mdsum2, 32))strcpy (pass, "F");


	
	fp = fopen ("results.csv", "a");
//	fwrite ("condition, decoder, result\n", 1 , 1, fp);
	fwrite (condition, 1, strlen(condition), fp);
	fwrite (",",1,1,fp);
	fwrite (decoder, 1, strlen(decoder), fp);
	fwrite (", ",1,1,fp);
	fwrite (pass, 1,1, fp);
	fwrite ("\n", 1,1, fp);
	fclose (fp);
	
	printf("\n exit \n");
	return EXIT_SUCCESS;
}
