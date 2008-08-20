/* 
 * simple support routines for writing WAV header
 */

struct WAVE_HDR {
	unsigned char  riff[4];   /* label                     */
	unsigned long  lenfbytes; /* len of file in bytes w/o header   */
	unsigned char  wavfmt[8]; /* 2 more 4 byte labels      */
	unsigned long  pcmhdr;    /* len in bytes of psm hdr   */
	unsigned short pcmflag;   /* pcm flag                  */
	unsigned short nchans;    /* number of channels        */
	unsigned long  freq;      /* sample freq in Hz         */
	unsigned long  bytesec;   /* bytes/sec freq*chan*bytes */
	unsigned short bytesamp;  /* bytes/sample chan*bytes   */
	unsigned short bitssamp;  /* bits/sample               */
	unsigned char  data[4];   /* data label                */
	unsigned long  lenpbytes; /* len in bytes of psm data  */
} WAVE_HDR;

#include <stdio.h>
#include <string.h>

void init_waveout (FILE *fp)
{
	if (!fseek(fp, sizeof(WAVE_HDR), SEEK_SET))
	{
		printf("seeked %d\n", sizeof(WAVE_HDR));
	}
	else
	{
		perror("init: fseek");
	}
}

#define	u_int	unsigned int

#define	swap_noop(data)	data

#define swap_word_bytes(data) \
   (u_int)((((u_int)data >> 8) & 0xFF) | \
           (((u_int)data & 0xFF) << 8) | \
           ((((u_int)data >> 16) & 0xFF) << 24) | \
           ((((u_int)data >> 24) & 0xFF) << 16))

#define swap_words(data) \
   (u_int)((((u_int)data >> 16) & 0xFFFF) | \
           (((u_int)data & 0xFFFF) << 16))

#define swap_lw_bytes(data) \
          (u_int)(((((((u_int)data >> 8) & 0xFF) | \
                     (((u_int)data & 0xFF) << 8)) << 16) | \
                   (((u_int)data >> 24) & 0xFF) | \
                   ((((u_int)data >> 16) & 0xFF) << 8)))

void done_waveout(FILE *fp, const long pcmbytes, const long freq,
	const short channels, const short bits)
{
	struct WAVE_HDR wave_hdr;
	int bytes = (bits+7)/8;
	int rc;

	if (!fseek(fp, 0l, SEEK_SET))
	{
		strncpy (wave_hdr.riff, "RIFF", 4);
		wave_hdr.lenfbytes = swap_noop(pcmbytes + sizeof(WAVE_HDR) - 8);
		strncpy (wave_hdr.wavfmt, "WAVEfmt ", 8);
		wave_hdr.pcmhdr = swap_noop(2+2+4+4+2+2);
		wave_hdr.pcmflag = swap_noop(1);
		wave_hdr.nchans = swap_noop(channels);
		wave_hdr.freq = swap_noop(freq);
		wave_hdr.bytesec = swap_noop(freq*channels*bytes);
		wave_hdr.bytesamp = swap_noop(channels*bytes);
		wave_hdr.bitssamp = swap_noop(bits);
		strncpy (wave_hdr.data, "data", 4);
		wave_hdr.lenpbytes = swap_noop(pcmbytes);
		rc = fwrite(&wave_hdr, 1, sizeof(WAVE_HDR), fp);
		if (ferror(fp))
		{
			perror("done: fwrite");
		}
		else
		{
			printf ("wrote %d, for %d, %d, %d, %d(0x%.8x)\n", rc, channels, freq, bits, pcmbytes, pcmbytes);
		}
	}
	else
	{
		perror("done: fseek");
	}
}
