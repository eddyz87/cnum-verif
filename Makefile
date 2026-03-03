CC = gcc
CFLAGS = -Wall -Werror -O2 -g -I.
LDFLAGS = -pthread
TARGET = cnum-test

all: $(TARGET)

$(TARGET): main.c cnum.c cnum_defs.h $(wildcard linux/*.h)
	bear -- $(CC) $(CFLAGS) $(LDFLAGS) -o $@ main.c cnum.c

clean:
	rm -f $(TARGET) compile_commands.json

.PHONY: all clean
