#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#ifdef S16
typedef int16_t sample_t;
/* funny: 32767 makes mpg123's s16 compare better with reference... but then, there is this whole business of different quantizations from/to int/float. */
#define TO_DOUBLE(x) (double) (x) / 32768.;
#endif

#ifdef S32
typedef int32_t sample_t;
#define TO_DOUBLE(x) (double)(x) / 2147483648.;
#endif

#ifdef F32
typedef float sample_t;
#define TO_DOUBLE(x) (double)(x)
#endif

int main()
{
	const size_t bufs = 1024;
	size_t got;
	sample_t f[bufs];
	double d[bufs];
	while( (got = fread(f, sizeof(sample_t), bufs, stdin)) )
	{
		size_t fi;
		for(fi=0; fi<got; ++fi)
		d[fi] = TO_DOUBLE(f[fi]);

		write(STDOUT_FILENO, d, sizeof(double)*got);
	}
	return 0;
}
