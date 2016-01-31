#!/usr/bin/perl

# This filters <, >, and & out of text and prepends a special tag to each line,
# rendering non-selectable line-numbers with the matching CSS code.

my @lines = <>;

my $digits = int(log(scalar @lines)/log(10)) + 1;
my $linefmt = sprintf('%%0%dd', $digits); 
my $i;
for(@lines)
{
	# The three little replacements that make text harmless in HTML context.
	s:\&:\&amp;:g; # That first, naturally.
	s:<:\&lt;:g;
	s:>:\&gt;:g;
	print '<span class="num" data-num="'
	.	sprintf($linefmt, ++$i)
	.	'"></span>' # Self-closing doesn't work? Confuses Firefox inspector.
	.	"\t" # line numbers count for tab space, ensure consistency
	.	$_;
}
