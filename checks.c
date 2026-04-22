#include <linux/bits.h>

#define T 8
#include "checks.h"
#undef T

#define T 32
#include "checks.h"
#undef T

#define T 64
#include "checks.h"
#undef T

void check_32_from_64(void)
{
	struct cnum64 a = { nondet_u64(), nondet_u64() };
	struct cnum32 b = cnum32_from_cnum64(a);
	u64 v = nondet_u64();

	__CPROVER_assume(!cnum64_is_empty(a));
	__CPROVER_assume(!cnum32_is_empty(b));

	bool in_a = cnum64_contains(a, v);
	bool in_b = cnum32_contains(b, (u32)v);
	if (in_a)
		assert(in_b);
}

void check_64_32_intersect(void)
{
	struct cnum64 a = { nondet_u64(), nondet_u64() };
	struct cnum32 b = { nondet_u32(), nondet_u32() };
	struct cnum64 c = cnum64_cnum32_intersect(a, b);
	u64 v = nondet_u64();

	__CPROVER_assume(!cnum64_is_empty(a));
	__CPROVER_assume(!cnum32_is_empty(b));
	__CPROVER_assume(cnum64_contains(a, v) && cnum32_contains(b, (u32)v));

	assert(cnum64_contains(c, v));
}

/*
 * mul_chunk: utt overflow bug for T>=32 (utt is same width as ut,
 * mk_mul_u intermediates need ut2 width).
 */
