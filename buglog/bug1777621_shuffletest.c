#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static int *limit;

static double square(double d)
{
	return d*d;
}

static void init_rando(int n)
{
	int i;
	limit = malloc(n*sizeof(int));
	for(i=0;i<n;++i)
	{
		limit[i] = RAND_MAX;
		while( (limit[i] % (i+1)) != i ) --limit[i];
	}
}

static int rando(int n)
{
	int ran;
	while( (ran = rand()%n) > limit[n-1] ){};
	return ran;
}

static void init(int* l, int n)
{
	int i;
	for(i=0;i<n;++i) l[i] = i;
}

static void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

static void doubleshuffle(int *l, int n)
{
	int i;
	for(i=0; i<n; ++i) swap(l+i, l+rando(n));
}

static void durstenfeld(int *l, int n)
{
	int i;
	for(i=0; i<n; ++i) swap(l+i, l+i+rando(n-i));
}

int main(int argc, char **argv)
{
	if(argc < 4){ printf("gimme: method(0/1) n repeat\n"); return -1; }
	int method = atoi(argv[1]);
	int n = atoi(argv[2]);
	int r = atoi(argv[3]);
	int i,j;
	int *l = calloc(n,sizeof(int));
	int *e = calloc(n*n,sizeof(int)); /* all zero */
	init_rando(n);
	for(i=0;i<r;++i)
	{
		init(l,n);
		switch(method)
		{
			case 0: doubleshuffle(l,n); break;
			case 1: durstenfeld(l,n);   break;
		}
		for(j=0;j<n;++j) ++e[j*n + l[j]];
	}
	/* Computation of mean is a bit unnecessary, but it is a hint for consistency of my program. */
	double mean = 0;
	double stddev = 0;
	for(i=0;i<n*n;++i) mean += e[i];

	mean /= (n*n);
	for(i=0;i<n*n;++i) stddev += square(e[i]-mean);

	stddev /= n*n;
	stddev = sqrt(stddev);
	printf("# histogram for method=%i n=%i r=%i\n", method, n, r);
	printf("# mean=%f stddev=%f\n", mean, stddev);
	printf("#source\tdest\tcount\n");
	for(i=0;i<n;++i)
	{
		for(j=0;j<n;++j) printf("%i\t%i\t%i\n", i, j, e[i*n + j]);
		printf("\n");
	}
	return 0;
}

