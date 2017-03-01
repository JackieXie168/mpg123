#!/usr/bin/perl

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
++$mon;
$year += 1900;
print sprintf(":date %04d-%02d-%02d\n", $year, $mon, $mday);
print ":by $ENV{USER}\n";

while(<STDIN>)
{
	if(/(^\d+\.\d+\.\d+)$/)
	{
		last
			if $version;
		$version = $1;
		print ":release $version\n";
	}
	elsif(/^(-+)\s+(\S.*)$/)
	{
		my $lev = length($1);
		if($lev <= $level)
		{
			print ".</li></ul>\n" x ($level-$lev);
			print ".</li>\n";
		}
		if($lev > $level)
		{
			print ".<ul>" x ($lev-$level);
		}
		$level = $lev;
		print ".<li>\n\t$2\n";
	}
	elsif(/^\s+(\S.*)$/)
	{
		print ".\t$1\n";
	}
}

print ".</li></ul>\n" x $level;
