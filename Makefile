CC = gcc
CFLAGS = -Wall -Werror -O2 -g -I. -flto
LDFLAGS = -pthread
TARGET = cnum-test
HEADERS = cnum_defs.h $(wildcard linux/*.h)

all: $(TARGET)

$(TARGET): main.c cnum.c $(HEADERS)
	bear -- $(CC) $(CFLAGS) $(LDFLAGS) -o $@ main.c cnum.c

clean:
	rm -f $(TARGET) compile_commands.json

cbmc: cbmc_cnum32.c cnum.c $(HEADERS)
	cbmc -I. cbmc_cnum32.c cnum.c

.PHONY: all clean cbmc
