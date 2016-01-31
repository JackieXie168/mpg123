.PHONY: all

all: examplecode.html

%.html: %.c
	perl scripts/codeblock.pl < $<  > $@
