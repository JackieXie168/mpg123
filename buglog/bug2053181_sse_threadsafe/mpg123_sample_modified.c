/*
        mpg123_to_wav.c

        copyright 2007 by the mpg123 project - free software under the terms of the LGPL 2.1
        see COPYING and AUTHORS files in distribution or http://mpg123.org
        initially written by Nicholas Humfrey
*/


#include <stdio.h>
#include <strings.h>
#include <mpg123.h>
//#include "sndfile.h"
#include <pthread.h>
extern void init_waveout(FILE *fp);
extern void done_waveout(FILE *fp, const long pcmbytes, 
		const long freq, const short channels, const short bits);



void usage()
{
        printf("Usage: mpg123_to_wav <input> <output>\n");
        exit(99);
}

void cleanup(mpg123_handle *mh)
{
        /* It's really to late for error checks here;-) */
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
}

int mpg123_sample(char *infile, char *decoder, char *condition, 
		char *outfile, pthread_mutex_t *mutex )
{
/*******************modifications made ************
 *  modifed name of routine so that it could be called from main
*   modified calling sequence to add parameters 
* 		- input file
* 		- output file
* 		- condition of test
* 		- optimizer
* 		- mutex variable for sequencing 
*   mpg123_init
*   modified use of sndfile open, close, write routines to use standard
*   C fopen, fclose, fwrite since there was too much overhead in accessing
*   sndfile.c
**************************************************/

 //       SNDFILE* sndfile = NULL;
 		FILE *sndfile;
//       SF_INFO sfinfo;
        mpg123_handle *mh = NULL;
        unsigned char* buffer = NULL;
        size_t buffer_size = 0;
        size_t done = 0;
        int  channels = 0, encoding = 0;
        int ret;
        long rate = 0;
        int  err  = MPG123_OK;
        off_t samples = 0;
        if (strcmp(condition, "cond1") == 0)  ret = pthread_mutex_lock (mutex);

        printf( "condition: %s\n", condition);
        printf( "Input file: %s\n", infile);
        printf( "Output file: %s\n", outfile);
        printf( "decoder %s\n", decoder);
        
   /***********need to synchronize access the mpg_123 init call since it 
    * is not thread safe. We do this by locking and unlocking the mutex
    */     
 
        if (strcmp(condition, "cond2") == 0)pthread_mutex_lock (mutex);
        if (strcmp(condition, "cond3") != 0) err = mpg123_init();
        if (strcmp(condition, "cond2") == 0)pthread_mutex_unlock (mutex);
        
        if( err != MPG123_OK || (mh = mpg123_new(decoder, &err)) == NULL
            /* Let mpg123 work with the file, that excludes MPG123_NEED_MORE messages. */
            || mpg123_open(mh, infile) != MPG123_OK
            /* Peek into track and get first output format. */
            || mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK )
        {
                fprintf( stderr, "Trouble with mpg123: %s\n",
                         mh==NULL ? mpg123_plain_strerror(err) : mpg123_strerror(mh) );
                cleanup(mh);
                return -1;
        }

        if(encoding != MPG123_ENC_SIGNED_16)
        { /* Signed 16 is the default output format anyways; it would actually by only different if we forced it.
             So this check is here just for this explanation. */
                cleanup(mh);
                fprintf(stderr, "Bad encoding: 0x%x!\n", encoding);
                return -2;
        }
        /* Ensure that this output format will not change (it could, when we allow it). */
        mpg123_format_none(mh);
        mpg123_format(mh, rate, channels, encoding);

        /* Buffer could be almost any size here, mpg123_outblock() is just some recommendation.
           Important, especially for sndfile writing, is that the size is a multiple of sample size. */
        buffer_size = mpg123_outblock( mh );
        buffer = malloc( buffer_size );

//        bzero(&sfinfo, sizeof(sfinfo) );
//        sfinfo.samplerate = rate;
//        sfinfo.channels = channels;
//        sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;
        printf("Creating 16bit WAV with %i channels and %liHz.\n", channels, rate);

  /**********************
  *  using sndfile.c is overkill for our purposes so I replaced sndfile opens
  *  writes, and close by normal C language file I/O
        sndfile = sf_open(outfile, SFM_WRITE, &sfinfo);
        if(sndfile == NULL){ fprintf(stderr, "Cannot open output file!\n"); cleanup(mh); return -2; }
*************************/
		sndfile = fopen(outfile, "wb");
		init_waveout (sndfile);
        do
        {
                err = mpg123_read( mh, buffer, buffer_size, &done );
/********************
*	more removing calls to sndfile
                sf_write_short( sndfile, (short*)buffer, done/sizeof(short) );
*********************/
				
                fwrite(buffer,sizeof(short), done/sizeof(short), sndfile);

                samples += done/sizeof(short);
                /* We are not in feeder mode, so MPG123_OK, MPG123_ERR and MPG123_NEW_FORMAT are the only possibilities.
                   We do not handle a new format, MPG123_DONE is the end... so abort on anything not MPG123_OK. */
        } while (err==MPG123_OK);

        if(err != MPG123_DONE)
        fprintf( stderr, "Warning: Decoding ended prematurely because: %s\n",
                 err == MPG123_ERR ? mpg123_strerror(mh) : mpg123_plain_strerror(err) );
/****************
*		last sndfile reference
        sf_close( sndfile );
*****************/
        done_waveout(sndfile, (samples/channels*2), rate, channels, 16);
		fclose(sndfile);

        samples /= channels;
        printf("%li samples written.\n", (long)samples);
        cleanup(mh);
        if (strcmp(condition, "cond1") == 0) pthread_mutex_unlock (mutex);
        return 0;
}

