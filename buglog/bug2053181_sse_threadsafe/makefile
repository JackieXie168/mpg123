#MPG=libmpg123
#MPGFLAGS=-mno-cygwin

MPG=../trunk/test
MPGFLAGS=

#
# for cygwin
#EXTRALIB=
#
# for linux/posix
EXTRALIB=-lpthread -lm

MPGLIBPATH=$(MPG)/lib
MPGINCPATH=$(MPG)/include

all: mpglib-ljb

clean: main.exe
	rm main.exe

mpglib-ljb: main.c support.c mpg123_sample_modified.c
	gcc $(MPGFLAGS) -o main main.c support.c mpg123_sample_modified.c  -L $(MPGLIBPATH) -l mpg123 $(EXTRALIB) -I $(MPGINCPATH)
