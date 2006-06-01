#!/usr/bin/perl

for($i = 1;$i <= $#ARGV; ++$i)
{
	system("(cd $ARGV[$i] && diff -x .svn -ruN ../$ARGV[$i-1] . > ../$ARGV[$i-1]_to_$ARGV[$i].diff)");
}
