## Makefile.am: produce Makefile.in from this

## copyright by the mpg123 project - free software under the terms of the LGPL 2.1
## see COPYING and AUTHORS files in distribution or http://mpg123.de
## initially written by Nicholas J. Humfrey

mpg123_LDADD = @CPU_TYPE_LIB@
mpg123_DEPENDENCIES = @CPU_TYPE_LIB@

EXTRA_DIST = Makefile.legacy config.h.legacy


bin_PROGRAMS = mpg123
EXTRA_PROGRAMS = testcpu

testcpu_SOURCES = \
	testcpu.c \
	getcpuflags.S

mpg123_SOURCES = \
	audio.c \
	audio.h \
	buffer.c \
	buffer.h \
	common.c \
	common.h \
	control_generic.c \
	debug.h \
	decode_2to1.c \
	decode_4to1.c \
	decode_ntom.c \
	equalizer.c \
	getbits.c \
	getbits.h \
	getlopt.c \
	getlopt.h \
	httpget.c \
	huffman.h \
	icy.c \
	icy.h \
	id3.c \
	id3.h \
	genre.h \
	l2tables.h \
	layer1.c \
	layer2.c \
	layer3.c \
	layer3.h \
	mangle.h \
	mpg123.c \
	mpg123.h \
	optimize.h \
	optimize.c \
	playlist.c \
	playlist.h \
	readers.c \
	tabinit.c \
	sfifo.c \
	sfifo.h \
	stringbuf.c \
	stringbuf.h \
	term.c \
	term.h \
	wav.c \
	xfermem.c \
	xfermem.h


## Plugins

lib_LTLIBRARIES = @MODULE_LIST@

EXTRA_LTLIBRARIES = \
	output_alsa.la \
	output_coreaudio.la \
	output_dummy.la \
	output_esound.la \
	output_jack.la \
	output_portaudio.la \
	output_sdl.la


## Output Plugins

output_alsa_la_SOURCES = audio_alsa.c
output_alsa_la_LDFLAGS = -module
output_alsa_la_LIBADD = -lasound

output_coreaudio_la_SOURCES = audio_coreaudio.c
output_coreaudio_la_LDFLAGS = -module -framework AudioToolbox -framework AudioUnit -framework CoreServices

output_dummy_la_SOURCES = audio_dummy.c
output_dummy_la_LDFLAGS = -module

output_esound_la_SOURCES = audio_esd.c
output_esound_la_CFLAGS = @ESOUND_CFLAGS@
output_esound_la_LDFLAGS = -module @ESOUND_LDFLAGS@
output_esound_la_LIBADD = @ESOUND_LIBS@

output_jack_la_SOURCES = audio_jack.c
output_jack_la_CFLAGS = @JACK_CFLAGS@
output_jack_la_LDFLAGS = -module @JACK_LDFLAGS@
output_jack_la_LIBADD = @JACK_LIBS@

output_portaudio_la_SOURCES = audio_portaudio.c
output_portaudio_la_LDFLAGS = -module -lportaudio

output_sdl_la_SOURCES = audio_sdl.c
output_sdl_la_CFLAGS = @SDL_CFLAGS@
output_sdl_la_LDFLAGS = -module @SDL_LDFLAGS@
output_sdl_la_LIBADD = @SDL_LIBS@




#EXTRA_mpg123_SOURCES = \
#	audio_aix.c \
#	audio_alib.c \
#	audio_alsa.c \
#	audio_dummy.c \
#	audio_esd.c \
#	audio_hp.c \
#	audio_jack.c \
#	audio_macosx.c \
#	audio_mint.c \
#	audio_nas.c \
#	audio_os2.c \
#	audio_oss.c \
#	audio_portaudio.c \
#	audio_sdl.c \
#	audio_sgi.c \
#	audio_sun.c \
#	audio_win32.c \
#	getcpuflags.h


EXTRA_mpg123_SOURCES = getcpuflags.h


# Processor dependent decoder code
EXTRA_LIBRARIES = \
	libcpu_generic.a \
	libcpu_generic_fpu.a \
	libcpu_generic_nofpu.a \
	libcpu_altivec.a \
	libcpu_i386.a \
	libcpu_i386_fpu.a \
	libcpu_i386_nofpu.a \
	libcpu_i486.a \
	libcpu_i586.a \
	libcpu_i586_dither.a \
	libcpu_3dnow.a \
	libcpu_3dnowext.a \
	libcpu_mmx.a \
	libcpu_sse.a \
	libcpu_x86.a

libcpu_generic_a_SOURCES = \
	decode.c \
	dct64.c

libcpu_generic_fpu_a_SOURCES = \
	decode.c \
	dct64.c

libcpu_generic_nofpu_a_SOURCES = \
	decode.c \
	dct64.c

libcpu_altivec_a_SOURCES = \
	decode_altivec.c \
	dct64_altivec.c

libcpu_i386_a_SOURCES = \
	decode_i386.c \
	dct64_i386.c

libcpu_i386_fpu_a_SOURCES = \
	decode_i386.c \
	dct64_i386.c

libcpu_i386_nofpu_a_SOURCES = \
	decode_i386.c \
	dct64_i386.c

libcpu_i486_a_SOURCES = \
	decode_i386.c \
	dct64_i386.c \
	decode_i486.c \
	dct64_i486.c

libcpu_i586_a_SOURCES = \
	decode_i386.c \
	dct64_i386.c \
	decode_i586.S

libcpu_i586_dither_a_SOURCES = \
	decode_i386.c \
	dct64_i386.c \
	decode_i586_dither.S

libcpu_3dnow_a_SOURCES = \
	decode_i386.c \
	decode_3dnow.S \
	decode_i586.S \
	dct64_i386.c \
	dct64_3dnow.S \
	dct36_3dnow.S \
	equalizer_3dnow.S \
	getcpuflags.S

libcpu_mmx_a_SOURCES = \
	decode_i386.c \
	dct64_mmx.S \
	tabinit_mmx.S \
	decode_mmx.S

libcpu_sse_a_SOURCES = \
	decode_i386.c \
	dct64_sse.c \
	dct64_mmx.S \
	tabinit_mmx.S \
	decode_mmxsse.c

libcpu_3dnowext_a_SOURCES = \
	decode_i386.c \
	dct64_3dnowext.c \
	dct36_3dnowext.c \
	dct64_mmx.S \
	tabinit_mmx.S \
	decode_mmxsse.c

libcpu_x86_a_SOURCES = \
	decode.c \
	decode_i386.c \
	decode_i586.S \
	decode_i586_dither.S \
	decode_3dnow.S \
	decode_mmx.S \
	decode_mmxsse.c \
	dct64.c \
	dct64_i386.c \
	dct64_sse.c \
	dct64_mmx.S \
	dct64_3dnow.S \
	dct36_3dnow.S \
	dct64_3dnowext.c \
	dct36_3dnowext.c \
	tabinit_mmx.S \
	equalizer_3dnow.S \
	getcpuflags.S
