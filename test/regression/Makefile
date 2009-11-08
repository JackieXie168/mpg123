# A crude Makefile to compile and run simple tests.

# A test is defined via a single .c source code file that is built and linked with libmpg123 (only).
# There may exist a corresponding .mp3 (or .mp2, or .mpg) file that is given as parameter to the resulting program to run the test. If there is not such .mp3, a generic test file is used, specified via TESTFILE variable.

.SUFFIXES:
.SUFFIXES: .c .h .o .bin

tests=$(patsubst %.c, %, $(wildcard *.c))

ifeq ($(CC),)
  CC=cc
endif

ifeq ($(TESTFILE),)
  $(error I need some TESTFILE with MPEG audio to work on.)
endif

# Rule to run all tests. If one test fails, the whole thing failed.

test: $(addprefix test-, $(tests))
	@echo "All tests apparently successful."

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

%.bin: %.c
	$(CC) $(CFLCAGS) -o $@ $< -lmpg123
