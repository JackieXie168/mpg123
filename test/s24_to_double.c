#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

/* Reading 24bit signed data in native order into 32 bit integers, stuffing in the missing lowest byte. */
#define TO_DOUBLE(x) (double)(x) / 2147483648.;

int main()
{
	const size_t bufs = 1024;
	size_t got;
	unsigned char in[bufs*3];
	double d[bufs];
	while( (got = fread(in, 3, bufs, stdin)) )
	{
		size_t fi;
		for(fi=0; fi<got; ++fi)
		{
			int i;
			const int byteshift[3] =
#ifdef BIG_ENDIAN
			{ 3, 2, 1 };
#else
# ifdef LITTLE_ENDIAN
			{ 1, 2 ,3 };
# else
#  error "bad endianess"
# endif
#endif
			unsigned char *bytes = in+3*fi;
			union { int32_t s; uint32_t u; } sample;
			sample.u = 0;
			for(i=0; i<3; ++i) sample.u |= (bytes[i]<<(byteshift[i]*8));

			d[fi] = TO_DOUBLE(sample.s);
		}

		write(STDOUT_FILENO, d, sizeof(double)*got);
	}
	return 0;
}
