CC = gcc
CFLAGS = -Wall -Werror -O2 -g -I. -flto
HEADERS = cnum_defs.h checks.h $(wildcard linux/*.h)
SRCS = checks.c cnum.c
CBMC = cbmc -I. $(SRCS)

all: compile-check cbmc

cbmc: $(SRCS) $(HEADERS)
	parallel $(CBMC) --function ::: \
		 check_32_from_urange \
	 	 check_32_from_srange \
	 	 check_32_umin_umax \
	 	 check_32_smin_smax \
	 	 check_32_intersect \
	 	 check_32_normalize \
	 	 check_32_is_empty \
	 	 check_32_contains \
	 	 check_64_from_urange \
	 	 check_64_from_srange \
	 	 check_64_umin_umax \
	 	 check_64_smin_smax \
	 	 check_64_intersect \
	 	 check_64_normalize \
	 	 check_64_is_empty \
	 	 check_64_contains \
	 	 check_32_add \
	 	 check_64_add \
	 	 check_32_from_64 \
	 	 check_64_32_intersect

compile-check: $(SRCS) $(HEADERS)
	bear -- $(CC) $(CFLAGS) -fsyntax-only checks.c

clean:
	rm -f compile_commands.json mul8-brute-force

.PHONY: all clean cbmc compile-check
