ifeq ($(CC),)
CC=gcc
endif

bins=rmsdouble.bin rms16.bin s16_double.bin f32_double.bin s32_double.bin hex24_double.bin

.PHONY: all clean

all: $(bins) $(shell ls compliance/layer?/fl*.mpg | sed 's:mpg:bit:') $(shell ls compliance/layer?/fl*.pcm | sed 's:pcm:double:') compliance/layer3/compl.double

clean:
	rm -vf $(bins)

f32_double.bin: to_double.c
	$(CC) -DF32 -O -o $@ $< -lm

s32_double.bin: to_double.c
	$(CC) -DS32 -O -o $@ $< -lm

s16_double.bin: to_double.c
	$(CC) -DS16 -O -o $@ $< -lm

%.bin: %.c
	$(CC) -O -o $@ $< -lm

# That generic target does not work. Ignored. Why?
.c.bin:
	$(CC) -O -o $@ $< -lm

compliance/layer1/%.bit: compliance/layer1/%.mpg hex_bitstream.bin
	./hex_bitstream.bin < $< > $@

compliance/layer1/%.double: compliance/layer1/%.pcm hex24_double.bin
	./hex24_double.bin < $< > $@

compliance/layer2/%.bit: compliance/layer2/%.mpg hex_bitstream.bin
	./hex_bitstream.bin < $< > $@

compliance/layer2/%.double: compliance/layer2/%.pcm hex24_double.bin
	./hex24_double.bin < $< > $@

compliance/layer3/%.double: compliance/layer3/%.hex hex24_double.bin
	./hex24_double.bin < $< > $@

%.double: %.s16 s16_double.bin
	./s16_double.bin < $< > $@

%.double: %.s32 s32_double.bin
	./s32_double.bin < $< > $@

%.double: %.f32 f32_double.bin
	./f32_double.bin < $< > $@

%.double: %.f64
	cp $< $@
