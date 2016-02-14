#!/usr/bin/perl

print "Content-type: text/html\n\n";

if(open(NEWS,"news.dat"))
{
	if($ENV{QUERY_STRING} eq "top")
	{
	my %head;
	my $data = 0;
	my $end = 0;
	print "<div class=\"newsblock\">\n";
	while(<NEWS>)
	{
		if($_ =~ /^([:\.])(.*)$/)
		{
		if($1 eq '.')
		{
			unless($data)
			{
			$data = 1;
			NewsHead(\%head);
			print "<div class=\"newsbody\">\n";
			}
			print $2."\n";
		}
		else
		{
			if($data)
			{
			print "</div>\n";
			$end = 1;
			last;
			}
			if($2  =~ /^(title|date|by|release)\s+(.*\S)$/)
			{
			$head{$1} = $2;
			}
		}
		}
	}
	close_news(\%head);
	print "</div>\n" unless $end;
	print "<p>For older news see the <a href=\"$ENV{SCRIPT_NAME}\">news archive</a></p>";
	}
	else
	{
	my %yearmark;
	while(<NEWS>)
	{
		$yearmark{$2} = $1
			if(/^:date\s+((\d+)-\d+-\d+)$/);
	}
	seek(NEWS, 0, SEEK_SET);

	Put("../header.html");
	print "<title>mpg123: news archive</title>\n</head>\n<body>\n";
	Put("../linkbar.html");
	print "<h1>News archive</h1>\n";
	print "<div class=\"yearlinks\">\n";
	for my $y (sort {$b <=> $a} keys %yearmark)
	{
		print "<a href=\"#$yearmark{$y}\">$y</a>\n";
	}
	print "</div>\n";
	my %head;
	my $data = 0;
	my $block = 0;
	while(<NEWS>)
	{
		if($_ =~ /^([:\.])(.*)$/)
		{
		if($1 eq '.')
		{
			unless($data)
			{
			$data = 1;
			print "</div>\n"
				if $block;
			$block = 1;
			print "<div class=\"newsblock\">\n";
			NewsHead(\%head);
			print "<div class=\"newsbody\">\n";
			}
			print $2."\n";
		}
		else
		{
			if($data)
			{
				close_news(\%head);
			$data = 0;
			delete $head{title};
			delete $head{date};
			delete $head{by};
			delete $head{release};
			}
			if($2  =~ /^(title|date|by|release)\s+(.*\S)$/)
			{
			$head{$1} = $2;
			}
		}
		}
	}
	close_news(\%head) if $data;
	print "</div>\n" if $block;
	print "</body>\n</html>\n";
	}
	close(NEWS);
}
else
{
	print "<p>No news available.</p>\n";
}

sub Put
{
	my $file = shift;
	if(open(DAT, $file))
	{
	while(<DAT>)
	{
		print;
		}
		close(DAT);
	}	
	else{ print "<p class=\"error\">cannot open $file!</p>\n"; }
}

sub NewsHead
{
	my $head = shift;
	my $release = defined $head->{release}
		? "Releasing mpg123 version $head->{release}: "
		: "";
	print "<div class=\"newshead\"><a id=\"$head->{date}\"></a><a href=\"#\"><span class=\"newsdate\">$head->{date}</span> ";
	print "<span class=\"newsby\">$head->{by}:</span> " if $head->{by};
	print "<span class=\"newstitle\">$release$head->{title}</span></a></div>\n";
}

sub close_news
{
	my $head = shift;
	if(defined $head->{release})
	{
		print '<p>Head over to the <a href="/download.shtml">download section</a>'
		.	' for getting your hands on the release.</p>'."\n";
	}
	print "</div>\n";
}

