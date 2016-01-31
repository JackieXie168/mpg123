#!/usr/bin/perl

my @lines = <>;

my $digits = int(log(scalar @lines)/log(10)) + 1;
my $linefmt = sprintf('%%0%dd', $digits); 
my $i;
for(@lines)
{
	s:<:\&lt;:g;
	s:>:\&gt;:g;
	print '<span class="num" data-num="'
	.	sprintf($linefmt, ++$i)
	.	'"></span>' # Self-closing doesn't work? Confuses Firefox inspector.
	.	"\t" # line numbers count for tab space, ensure consistency
	.	$_;
}
