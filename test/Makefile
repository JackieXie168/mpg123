ifeq ($(CC),)
CC=gcc
endif

# Change that to exe or whatever... but it should be non-empty.
BIN_SUFFIX=bin

bins=rmsdouble.$(BIN_SUFFIX) rms16.$(BIN_SUFFIX) s16_double.$(BIN_SUFFIX) f32_double.$(BIN_SUFFIX) s32_double.$(BIN_SUFFIX) s24_double.$(BIN_SUFFIX) hex24_double.$(BIN_SUFFIX) buffer.$(BIN_SUFFIX)

.PHONY: all clean

all: binsuffix $(bins) $(shell ls compliance/layer?/fl*.mpg | sed 's:mpg:bit:') $(shell ls compliance/layer?/fl*.pcm | sed 's:pcm:double:') compliance/layer3/compl.double

clean:
	rm -vf $(bins)

binsuffix: $(bins)
	echo $(BIN_SUFFIX) > $@

f32_double.$(BIN_SUFFIX): to_double.c
	$(CC) -DF32 -O -o $@ $< -lm

s32_double.$(BIN_SUFFIX): to_double.c
	$(CC) -DS32 -O -o $@ $< -lm

s24_double.$(BIN_SUFFIX): s24_to_double.c endian.txt
	$(CC) -D$(shell cat endian.txt) -o $@ $< -lm

s16_double.$(BIN_SUFFIX): to_double.c
	$(CC) -DS16 -O -o $@ $< -lm

endian.txt: endian.$(BIN_SUFFIX)
	./$< > $@

%.$(BIN_SUFFIX): %.c
	$(CC) -O -o $@ $< -lm

# That generic target does not work. Ignored. Why?
.c.$(BIN_SUFFIX):
	$(CC) -O -o $@ $< -lm

compliance/layer1/%.bit: compliance/layer1/%.mpg hex_bitstream.$(BIN_SUFFIX)
	./hex_bitstream.$(BIN_SUFFIX) < $< > $@

compliance/layer1/%.double: compliance/layer1/%.pcm hex24_double.$(BIN_SUFFIX)
	./hex24_double.$(BIN_SUFFIX) < $< > $@

compliance/layer2/%.bit: compliance/layer2/%.mpg hex_bitstream.$(BIN_SUFFIX)
	./hex_bitstream.$(BIN_SUFFIX) < $< > $@

compliance/layer2/%.double: compliance/layer2/%.pcm hex24_double.$(BIN_SUFFIX)
	./hex24_double.$(BIN_SUFFIX) < $< > $@

compliance/layer3/%.double: compliance/layer3/%.hex hex24_double.$(BIN_SUFFIX)
	./hex24_double.$(BIN_SUFFIX) < $< > $@

%.double: %.s16 s16_double.$(BIN_SUFFIX)
	./s16_double.$(BIN_SUFFIX) < $< > $@

%.double: %.s24 s24_double.$(BIN_SUFFIX)
	./s24_double.$(BIN_SUFFIX) < $< > $@

%.double: %.s32 s32_double.$(BIN_SUFFIX)
	./s32_double.$(BIN_SUFFIX) < $< > $@

%.double: %.f32 f32_double.$(BIN_SUFFIX)
	./f32_double.$(BIN_SUFFIX) < $< > $@

%.double: %.f64
	cp $< $@
