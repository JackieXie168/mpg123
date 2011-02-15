#!/usr/bin/env perl

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

open(SUFDAT, "$Bin/binsuffix") or die "Hm, binsuffix file does not exist, did you run make in $Bin?";
my $bsuf = <SUFDAT>;
chomp($bsuf);
close(SUFDAT);

my $rms="$Bin/rmsdouble.$bsuf";
my $f32conv="$Bin/f32_double.$bsuf";
my $s32conv="$Bin/s32_double.$bsuf";
my $s24conv="$Bin/s24_double.$bsuf";
my $s16conv="$Bin/s16_double.$bsuf";

die "Binaries missing, run make in $Bin\n" unless (-e $rms and -e $f32conv and -e $s16conv);

my $nogap='';
my $floater=0;
my $int32er=0;
my $int24er=0;
# Crude hack to detect the raw decoder test binary.
my $rawdec = 1;

# Store STDERR descriptor.
open(my $the_stderr, '>&', \*STDERR);

{
	stderr_null();
	open(DAT, "-|", @ARGV, '--longhelp');
	while(<DAT>)
	{
		# If we see the greeting from mpg123, we don't have the raw decoder at hand.
		$rawdec = 0 if /^High Performance MPEG/;
		$nogap='--no-gapless' if /no-gapless/;
	}
	close(DAT);
	stderr_normal();
	$int32er = test_encoding('s32');
	$int24er = test_encoding('s24');
	$floater = test_encoding('f32');
}

for(my $lay=1; $lay<=3; ++$lay)
{
	print "\n";
	print "==== Layer $lay ====\n";

	print "--> 16 bit signed integer output\n";
	tester($files[$lay-1], 's16', $s16conv);

	if($int32er)
	{
		print "--> 32 bit integer output\n";
		tester($files[$lay-1], 's32', $s32conv);
	}

	if($int24er)
	{
		print "--> 24 bit integer output\n";
		tester($files[$lay-1], 's24', $s24conv);
	}

	if($floater)
	{
		print "--> 32 bit floating point output\n";
		tester($files[$lay-1], 'f32', $f32conv);
	}
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
		# This needs to be reworked to be save with funny stuff in @ARGV.
		# Relying on the shell is dangerous.
		my @com = @ARGV;
		if($rawdec)
		{
			push(@com, $enc, $bit);
			
		}
		else
		{
			push(@com, '-e', $enc) unless($enc eq 's16');
			push(@com, '-q', '-s');
			push(@com, $nogap) unless($nogap eq '');
			push(@com, $bit);
		}
		my $commandline;
		for(@com)
		{
			$commandline .= ' ' if $commandline ne '';
			$commandline .= quotemeta($_);
		}
		$commandline .= " 2>/dev/null | $conv | $rms ".quotemeta($double)." 2>/dev/null";
		system($commandline);
	}
}

sub test_encoding
{
	my $enc = shift;
	# We only support the raw decoder supporting all encodings, for now.
	if($rawdec)
	{
		print STDERR "Warning: Hacked use of the raw decoder test binary, assuming format support.\n";
		return 1;
	}
	my $supported = 0;
	my @testfiles = glob($files[2]);
	my $testfile = $testfiles[0];
	stderr_null();
	open(DAT, '-|', @ARGV, '-q', '-s', '-e', $enc, $testfile);
	my $tmpbuf;
	if(read(DAT, $tmpbuf, 1024) == 1024)
	{
		$supported = 1;
	}
	close(DAT);
	stderr_normal();
	return $supported;
}

sub stderr_null
{
	close(STDERR);
}

sub stderr_normal
{
	open STDERR, '>&', $the_stderr;
}
