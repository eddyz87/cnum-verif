CC = gcc
CFLAGS = -Wall -Werror -O2 -g -I. -flto
HEADERS = cnum_defs.h checks.h $(wildcard linux/*.h)
SRCS = checks.c cnum.c

all: mul8-brute-force cbmc

mul8-brute-force: mul8_brute_force.c cnum.c $(HEADERS)
	$(CC) $(CFLAGS) -pthread -o $@ mul8_brute_force.c cnum.c

cbmc: $(SRCS) $(HEADERS)
	cbmc -I. $(SRCS)

compile-check: $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -fsyntax-only checks.c

clean:
	rm -f compile_commands.json mul8-brute-force

.PHONY: all clean cbmc compile-check
