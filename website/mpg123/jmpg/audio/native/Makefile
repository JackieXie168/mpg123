
CC=gcc
CFLAGS=-I/usr/java1.1/include/ -I/usr/java1.1/include/solaris


all: jmpg_audio_NativeAudio.h

jmpg_audio_NativeAudio.h: ../NativeAudio.java
	javah -jni jmpg.audio.NativeAudio

solaris: jmpg_audio_NativeAudio.h
	cc -G $(CFLAGS) audio_java.c audio.c audio_sun.c -o libnativeaudio.so 

linux: jmpg_audio_NativeAudio.h
	gcc -DLINUX -shared -I/usr/lib/java/include/ -I/usr/lib/java/include/genunix/ \
		audio_java.c audio.c audio_oss.c -o libnativeaudio.so

clean:
	rm libnativeaudio.so 

