#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <math.h>

static int16_t ieeerounder(float x)
{
	union
	{
		float f;
		int32_t i;
	} u_fi;
	u_fi.f = x + 12582912.0f; /* Magic Number: 2^23 + 2^22 */
	return (int16_t)u_fi.i;
}

static int16_t rounder(float x)
{
	return x>0.0 ? (x+0.5) : (x-0.5);
}

static int16_t truncer(float x)
{
	return x;
}

static int16_t c99rounder(float x)
{
	return lrintf(x);
}

enum modes
{
	mode_round = 1
,	mode_trunc = 2
,	mode_c99   = 3
,	mode_ieee  = 4
};

int main(int argc, char **argv)
{
	int mode = atoi(argv[1]);
	const size_t bufs = 1024;
	size_t got;
	float    in[bufs];
	int16_t out[bufs];

	while( (got = fread(in, sizeof(float), bufs, stdin)) )
	{
		size_t fi;
		for(fi=0; fi<got; ++fi)
			in[fi] *= INT16_MAX;
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
			case mode_ieee:
				for(fi=0; fi<got; ++fi)
					out[fi] = ieeerounder(in[fi]);
			break;
			default:
				return 1;
		}
		fwrite(out, sizeof(int16_t), got, stdout);
	}
	return 0;
}
