CC=gcc
CFLAGS=-O3 -std=c99
# This selects intrinsics (?)... not sure how you get at the supposed
# inline definitions from glibc bits/mathinline.h.
FASTLRINT_FLAGS=-fno-math-errno
time=/usr/bin/time

iterations=10

binaries=s16rounder-fastlrint s16rounder s32rounder s32rounder-fastlrint

test: testdata.f32 $(binaries)
	for i in s??rounder s??rounder-fastlrint; do \
	  maxmode=3; echo $$i | grep -q s16 && maxmode=4; \
	  for m in $$(seq 1 $$maxmode); do \
	    echo -n "testing $$i mode $$m: "; \
	    n=0; while test $$n -lt $(iterations); do \
	      let n=$$n+1; \
	      cat $<; \
	    done \
	    | $(time) ./$$i $$m 2>&1 > /dev/null \
	    | perl -ne 'print $$1 if /([\d\.]+)user/'; \
	    echo " s"; \
	  done; \
	done

clean:
	rm -f $(binaries)
	rm testdata.f32

# Do what you please here to get some floatinf point input.
testdata.f32: testdata.mp3
	mpg123 -q -e f32 -s $< > $@

s%rounder: s%rounder.c
	$(CC) $(CFLAGS) $< -o $@ -lm

s%rounder-fastlrint: s%rounder.c
	$(CC) $(CFLAGS) $(FASTLRINT_FLAGS) $< -o $@
