#!/usr/bin/perl -w

use strict;

# just picking samples from the host of available bitstreams
use FindBin qw($Bin);
use File::Basename;

die "Give me the mpg123 command to test\n" unless @ARGV;

my @files=
(
	 "$Bin/compliance/layer1/fl*.mpg"
	,"$Bin/compliance/layer2/fl*.mpg"
	,"$Bin/compliance/layer3/compl.bit"
);

my $rms="$Bin/rmsdouble.bin";
my $f32conv="$Bin/f32_double.bin";
my $s16conv="$Bin/s16_double.bin";

die "Binaries missing, run make in $Bin\n" unless (-e $rms and -e $f32conv and -e $s16conv);

my $nogap='';
my $floater=0;
{
	open(DAT, "@ARGV --longhelp 2>&1|");
	while(<DAT>)
	{
		$nogap='--no-gapless' if /no-gapless/;
		$floater=1 if /f32/;
	}
	close(DAT);
}

for(my $lay=1; $lay<=3; ++$lay)
{
	print "\n";
	print "==== Layer $lay ====\n";

	print "--> 16 bit signed integer output\n";
	tester($files[$lay-1], '', $s16conv);

	next unless $floater;

	print "--> 32 bit floating point output\n";
	tester($files[$lay-1], '-e f32', $f32conv);
}

sub tester
{
	my $pattern = shift;
	my $enc = shift;
	my $conv = shift;
	foreach my $f (glob($pattern))
	{
		my $bit = $f;
		$bit =~ s:mpg$:bit:;
		my $double = $f;
		$double =~ s:(mpg|bit)$:double:;
		die "Please make some files!\n" unless (-e $bit and -e $double);
		print basename($bit).":\t";
		my $commandline = "@ARGV $nogap $enc -q  -s ".quotemeta($bit)." | $conv | $rms ".quotemeta($double)." 2>/dev/null";
		system($commandline);
	}
}
