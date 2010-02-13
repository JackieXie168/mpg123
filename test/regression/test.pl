#!/usr/bin/perl

use File::Basename;

my $log  = 'test.log';
my $good = 'PASS';
my $bad  = 'FAIL';

open(LOG, ">$log") or die "Cannot open log file $log! ($!)\n";

print <<EOT;
Starting tests, look into test.log for messages.
$good is good, $bad is bad.

EOT
print LOG "Beginning tests.\n";
close(LOG);

my $fail = 0;

my $proglen = 0;
foreach my $prog (@ARGV)
{
	my $base = basename($prog);
	$proglen = length($base) if length($base) > $proglen;
}

foreach my $prog (@ARGV)
{
	my $base = strip_bin(basename($prog));
	my $command = $prog;
	$command .= ' '.quotemeta(find_testfile($prog));
	print STDOUT "$base: ";
	# Open the log, append a line, close again so that program output goes there in an orderly fashion.
	open(LOG, ">>$log") or die "Cannot re-open log file $log! ($!)\n";
	print LOG "\nTEST: Output of $command:\n";
	close(LOG);

	my $locfail = -1;
	if(open(PROG, "$command 2>>$log |"))
	{
		while(<PROG>)
		{
			$locfail = 1 if /$bad/o;
			$locfail = 0 if /$good/o;
		}
		close(PROG);
	}
	$fail = 1 if $locfail;
	print ' ' x ($proglen-length($base)), ($locfail ? $bad : $good), ($locfail < 0 ? ' (execute FAIL)' : ''), "\n";
}

print STDOUT "\n", $fail ? "FAIL" : "PASS", "\n";

exit $fail;

sub strip_bin
{
	my $bin = shift;
	$bin =~ s:\.bin$::;
	return $bin;
}

# Find a file to give to the test program.
# Either there is a file with matching name or we use the TESTFILE from environment.
sub find_testfile
{
	my $name = strip_bin(shift);
	foreach my $e qw(mpg mp1 mp2 mp3 mpeg)
	{
		if(-f "$name.$e")
		{
			return "$name.$e";
		}
	}
	return $ENV{TESTFILE};
}
