# A crude Makefile to compile and run simple tests.

# A test is defined via a single .c source code file that is built and linked with libmpg123 (only).
# There may exist a corresponding .mp3 (or .mp2, or .mpg) file that is given as parameter to the resulting program to run the test. If there is not such .mp3, a generic test file is used, specified via TESTFILE variable.

.SUFFIXES:
.SUFFIXES: .c .h .o .bin

tests=$(patsubst %.c, %, $(filter-out helpers.c, $(wildcard *.c)))

ifeq ($(CC),)
  CC=cc
endif

# If there's a prefix set, make sure we use the libmpg123 from there.
ifneq ($(MPG123_PREFIX),)
  CPPFLAGS := $(CPPFLAGS) -I$(MPG123_PREFIX)/include
  LDFLAGS  := $(LDFLAGS) -L$(MPG123_PREFIX)/lib -Wl,-rpath $(MPG123_PREFIX)/lib
endif

ifeq ($(TESTFILE),)
  TESTFILE := drum.mp3
endif

# Rule to run all tests. If one test fails, the whole thing failed.

test: $(addprefix test-, $(tests))
	@echo "All tests apparently successful."

info:
	@echo "These are the tests: $(tests)"

clean:
	rm -vf $(addsuffix .bin, $(tests))

# Run specific tests.

test-%: %.bin %.mpg
	./$^

test-%: %.bin %.mp2
	./$^

test-%: %.bin %.mp3
	./$^

test-%: %.bin
	./$< $(TESTFILE)

# Build a test binary.

%.bin: %.c helpers.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lmpg123
