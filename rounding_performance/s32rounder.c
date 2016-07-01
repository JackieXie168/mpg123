#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <math.h>

static int32_t rounder(float x)
{
	return x>0.0 ? (x+0.5) : (x-0.5);
}

static int32_t truncer(float x)
{
	return x;
}

static int32_t c99rounder(float x)
{
	return lrintf(x);
}

enum modes
{
	mode_round = 1
,	mode_trunc = 2
,	mode_c99   = 3
};

int main(int argc, char **argv)
{
	int mode = atoi(argv[1]);
	const size_t bufs = 1024;
	size_t got;
	float    in[bufs];
	int32_t out[bufs];

	while( (got = fread(in, sizeof(float), bufs, stdin)) )
	{
		size_t fi;
		for(fi=0; fi<got; ++fi)
			in[fi] *= INT32_MAX;
		switch(mode)
		{
			case mode_round:
				for(fi=0; fi<got; ++fi)
					out[fi] = rounder(in[fi]);
			break;
			case mode_trunc:
				for(fi=0; fi<got; ++fi)
					out[fi] = truncer(in[fi]);
			break;
			case mode_c99:
				for(fi=0; fi<got; ++fi)
					out[fi] = c99rounder(in[fi]);
			break;
			default:
				return 1;
		}
		fwrite(out, sizeof(int32_t), got, stdout);
	}
	return 0;
}
