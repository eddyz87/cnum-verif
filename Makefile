CC = gcc
CFLAGS = -Wall -O2 -g -I. -flto
LDFLAGS = -pthread
TARGET = cnum-test
HEADERS = cnum_defs.h $(wildcard linux/*.h)

all: $(TARGET)

$(TARGET): main.c cnum.c $(HEADERS)
	bear -- $(CC) $(CFLAGS) $(LDFLAGS) -o $@ main.c cnum.c

cbmc: cbmc_cnum32.c cnum.c $(HEADERS)
	cbmc --smt2 -I. cbmc_cnum32.c cnum.c

cbmc-helpers: cbmc_helpers8.c $(HEADERS)
	cbmc --smt2 -I. cbmc_helpers8.c

CBMC_BIN = /home/ezingerman/cbmc/build/bin
GOTO_CC = $(CBMC_BIN)/goto-cc
GOTO_INSTRUMENT = $(CBMC_BIN)/goto-instrument
CBMC_BIN_CMD = $(CBMC_BIN)/cbmc

helpers8.gb: cbmc_helpers8.c $(HEADERS)
	$(GOTO_CC) -I. cbmc_helpers8.c -o helpers8.gb

helpers8-replaced-intersect.gb: helpers8.gb
	$(GOTO_INSTRUMENT) \
		--replace-call-with-contract cnum8_intersect \
		helpers8.gb helpers8-replaced-intersect.gb

helpers8-replaced-mul.gb: helpers8.gb
	$(GOTO_INSTRUMENT) \
		--replace-call-with-contract cnum8_intersect \
		--replace-call-with-contract cnum8_union \
		--replace-call-with-contract cnum8_mul_chunk \
		--replace-call-with-contract cnum8_cut \
		helpers8.gb helpers8-replaced-mul.gb

check-intersect: helpers8.gb
	$(CBMC_BIN_CMD) helpers8.gb --function check_intersect --smt2

check-union: helpers8.gb
	$(CBMC_BIN_CMD) helpers8.gb --function check_union --smt2

check-cut: helpers8.gb
	$(CBMC_BIN_CMD) helpers8.gb --function check_cut_1 --smt2
	$(CBMC_BIN_CMD) helpers8.gb --function check_cut_2 --smt2
	$(CBMC_BIN_CMD) helpers8.gb --function check_cut_3 --smt2

check-mul-chunk: helpers8-replaced-intersect.gb
	$(CBMC_BIN_CMD) helpers8-replaced-intersect.gb --function check_mul_chunk --smt2

check-mul: helpers8-replaced-mul.gb
	$(CBMC_BIN_CMD) helpers8-replaced-mul.gb --function check_mul --smt2 --unwind 9

clean:
	rm -f $(TARGET) compile_commands.json helpers8*.gb

.PHONY: all clean cbmc cbmc-helpers check-intersect check-union check-cut check-mul-chunk check-mul
