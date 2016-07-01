/*
** Copyright (C) 2001 Erik de Castro Lopo <erikd AT mega-nerd DOT com>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<time.h>

#define	_ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1
#define	__USE_ISOC9X	1
#define	__USE_ISOC99	1
#include	<math.h>

#define	ARRAY_SIZE	(1<<8)

/* Prototypes. */

void float_cast_test (int) ;
void double_cast_test (int) ;

/* Read the Pentium instruction timer. */

typedef union 
{	long long	count ;
	long		value [2] ;
} TIMER_VALUE ;

#define RDTSC_TIMER(x) __asm__(".byte 0x0f,0x31" : "=a" ((x).value [0]), "=d" ((x).value [1]))

int
main (void)
{	int count ;

	printf ("GCC version : %d.%d\n", __GNUC__, __GNUC_MINOR__) ;

	/* Do this to make sure that the loop count is determined at run time.*/
	count = 100000 + time (NULL) % 10;
	
	float_cast_test (count) ;
	double_cast_test (count) ;
	
	return 0;
} /* main */

/*==============================================================================
*/

void
c_cast_float_array (int count, float *input, int *output)
{	while (count)
	{	count -- ;
		output [count] = (int) input [count] ;
		} ;
} /* c_cast_float_array */

void
c_int_pipe_float_array (int count, float *input, int *output)
{	int flt_int, out ;

	while (count)
	{	count -- ;
	
		if (input [count] == 0.0)
		{	output [count] = 0 ;
			continue ;
			} ;
		
		flt_int	= *((int *)&(input [count])) ;
	
		out = ((flt_int & 0x7FFFFF) + 0x800000) >> 
				(24 - (((flt_int >> 23) & 0xFF) - 126)) ;
	
		if (flt_int < 0)
			out = -out ;
	
		output [count] = out ;
		} ;

} /* c_int_pipe_float_array */

#define	FLOAT_TO_INT(in,out)		\
	__asm__ __volatile__ ("fistpl %0" : "=m" (out) : "t" (in) : "st") ;

void
macro_convert_float_array (int count, float *input, int *output)
{	while (count)
	{	count -- ;
		FLOAT_TO_INT (input [count], output [count]) ;
		} ;

} /* macro_convert_float_array */

void
lrintf_float_array (int count, float *input, int *output)
{	while (count)
	{	count -- ;
		output [count] = lrintf (input [count]) ;
		} ;
} /* lrintf_float_array */

void 
float_cast_test (int loop_count)
{	float	*input = malloc (ARRAY_SIZE * sizeof (float)) ;

	int	*cast_output   = malloc (ARRAY_SIZE * sizeof (int)) ;
	int	*int_pipe_output   = malloc (ARRAY_SIZE * sizeof (int)) ;
	int	*macro_output   = malloc (ARRAY_SIZE * sizeof (int)) ;
	int	*lrintf_output = malloc (ARRAY_SIZE * sizeof (int)) ;
	
	TIMER_VALUE	start_time, end_time ;
	double	cast_time, int_pipe_time, macro_time, lrintf_time ;
	int k ;

	printf ("\nTesting float -> int cast.\n") ;

	for (k = 0 ; k < ARRAY_SIZE ; k++)
		input [k] = 0.333333 * (k - (ARRAY_SIZE / 2)) ;
		
	RDTSC_TIMER (start_time) ;
	for (k = 0 ; k < loop_count ; k++)
		c_cast_float_array (ARRAY_SIZE, input, cast_output) ;
	RDTSC_TIMER (end_time) ;
	cast_time = end_time.count - start_time.count ;

	RDTSC_TIMER (start_time) ;
	for (k = 0 ; k < loop_count ; k++)
		c_int_pipe_float_array (ARRAY_SIZE, input, int_pipe_output) ;
	RDTSC_TIMER (end_time) ;
	int_pipe_time = end_time.count - start_time.count ;

	RDTSC_TIMER (start_time) ;
	for (k = 0 ; k < loop_count ; k++)
		lrintf_float_array (ARRAY_SIZE, input, lrintf_output) ;
	RDTSC_TIMER (end_time) ;
	lrintf_time = end_time.count - start_time.count ;
	
	RDTSC_TIMER (start_time) ;
	for (k = 0 ; k < loop_count ; k++)
		macro_convert_float_array (ARRAY_SIZE, input, macro_output) ;
	RDTSC_TIMER (end_time) ;
	macro_time = end_time.count - start_time.count ;
	
	for (k = 0 ; k < ARRAY_SIZE ; k++)
	{
		/*-printf ("% 10.2f -> (% 5d   % 5d   % 5d   % 5d)\n", input [k], cast_output [k], int_pipe_output [k], macro_output [k], lrintf_output [k]) ;-*/

		if (abs (macro_output [k] - cast_output [k]) > 1)
		{	printf ("\n\nError : macro_output mismatch at position %d\n", k) ;
			exit (1) ;
			} ;
		if (abs (int_pipe_output [k] - cast_output [k]) > 1)
		{	printf ("\n\nError : int_pipe_output mismatch at position %d\n", k) ;
			exit (1) ;
			} ;
		if (abs (lrintf_output [k] - cast_output [k]) > 1)
		{	printf ("\n\nError : lrintf_output mismatch at position %d\n", k) ;
			exit (1) ;
			} ;
		
		} ;

	printf ("    cast time                 = %7.3f\n", cast_time / cast_time) ;
	printf ("    cast time / int_pipe time = %7.3f\n", cast_time / int_pipe_time) ;
	printf ("    cast time / lrintf time   = %7.3f\n", cast_time / lrintf_time) ;
	printf ("    cast time / macro time    = %7.3f\n\n", cast_time / macro_time) ;

	return ;	
} /* float_cast_test */

/*==============================================================================
*/

void
c_cast_double_array (int count, double *input, int *output)
{	while (count)
	{	count -- ;
		output [count] = (int) input [count] ;
		} ;
} /* c_cast_double_array */

#define	DOUBLE_TO_INT(in,out)		\
	__asm__ __volatile__ ("fistpl %0" : "=m" (out) : "t" (in) : "st") ;

void
macro_convert_double_array (int count, double *input, int *output)
{	while (count)
	{	count -- ;
		DOUBLE_TO_INT (input [count], output [count]) ;
		} ;
} /* macro_convert_double_array */

void
lrint_double_array (int count, double *input, int *output)
{	while (count)
	{	count -- ;
		output [count] = lrint (input [count]) ;
		} ;
} /* macro_convert_double_array */

void 
double_cast_test (int loop_count)
{	double	*input = malloc (ARRAY_SIZE * sizeof (double)) ;

	int	*cast_output = malloc (ARRAY_SIZE * sizeof (int)) ;
	int	*macro_output = malloc (ARRAY_SIZE * sizeof (int)) ;
	int	*lrint_output = malloc (ARRAY_SIZE * sizeof (int)) ;

	TIMER_VALUE	start_time, end_time ;
	double	cast_time, macro_time, lrint_time ;
	int k ;

	printf ("\nTesting double -> int cast.\n") ;

	for (k = 0 ; k < ARRAY_SIZE ; k++)
		input [k] = 0.0666 * k - (ARRAY_SIZE / 2) ;
		
	RDTSC_TIMER (start_time) ;
	for (k = 0 ; k < loop_count ; k++)
		c_cast_double_array (ARRAY_SIZE, input, cast_output) ;
	RDTSC_TIMER (end_time) ;
	cast_time = end_time.count - start_time.count ;

	RDTSC_TIMER (start_time) ;
	for (k = 0 ; k < loop_count ; k++)
		lrint_double_array (ARRAY_SIZE, input, lrint_output) ;
	RDTSC_TIMER (end_time) ;
	lrint_time = end_time.count - start_time.count ;
	
	RDTSC_TIMER (start_time) ;
	for (k = 0 ; k < loop_count ; k++)
		macro_convert_double_array (ARRAY_SIZE, input, macro_output) ;
	RDTSC_TIMER (end_time) ;
	macro_time = end_time.count - start_time.count ;
	
	for (k = 0 ; k < ARRAY_SIZE ; k++)
	{	if (abs (macro_output [k] - cast_output [k]) > 1)
		{	printf ("\n\nError : macro_output mismatch at position %d\n", k) ;
			exit (1) ;
			} ;
		} ;

	printf ("    cast time                 = %7.3f\n", cast_time / cast_time) ;
	printf ("    cast time / lrint time    = %7.3f\n", cast_time / lrint_time) ;
	printf ("    cast time / macro time    = %7.3f\n\n", cast_time / macro_time) ;

	return ;	
} /* double_cast_test */

