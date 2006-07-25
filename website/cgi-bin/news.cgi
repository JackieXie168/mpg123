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
		    if($2  =~ /^(title|date|by)\s+(.*\S)$/)
		    {
			$head{$1} = $2;
		    }
		}
	    }
	}
	print "</div>\n";
	print "</div>\n" unless $end;
	print "<p>For older news see the <a href=\"$ENV{SCRIPT_NAME}\">news archive</a></p>";
    }
    else
    {
	Put("../header.html");
	print "<title>mpg123: news archive</title>\n</head>\n<body>\n";
	Put("../linkbar.html");
	print "<h1>News archive</h1>\n";
	my %head;
	my $data = 0;
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
			print "</div><br />\n";
			$data = 0;
			delete $head{title};
			delete $head{date};
			delete $head{by};
		    }
		    if($2  =~ /^(title|date|by)\s+(.*\S)$/)
		    {
			$head{$1} = $2;
		    }
		}
	    }
	}
	print "</div>\n";
	print "</div>\n" if $data;
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
    print "<div class=\"newshead\"><span class=\"newsdate\">$head->{date}</span> ";
    print "<span class=\"newsby\">$head->{by}:</span> " if $head->{by};
    print "<span class=\"newstitle\">$head->{title}</span></div>\n";
}