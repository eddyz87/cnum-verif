#include "linux/compiler_types.h"
#include <linux/bits.h>

#define T 32
#include "checks.h"
#undef T

#define T 64
#include "checks.h"
#undef T

u16 nondet_u16(void);

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

static struct cnum64 deduce_bounds_64_from_step(struct cnum64 in, u16 base, u16 step)
{
	u64 end = in.base + in.size;
	u64 start = in.base;
	u64 size = in.size;
	u64 d;

	d = (step - (start - base) % step) % step;
	d = min(d, base - start);
	start += d;
	if (d > size)
		return CNUM64_EMPTY;
	size -= d;
	d = (end - base) % step;
	d = min(d, base - start);
	end -= d;
	if (d > size)
		return CNUM64_EMPTY;

	return (struct cnum64) {
		.base = start,
		.size = end - start,
	};
}

void check_64_from_step(void)
{
	struct cnum64 a = { nondet_u64(), nondet_u64() };
        u16 base = nondet_u16();
	u16 step = nondet_u16();
	u64 v = nondet_u64();

	__CPROVER_assume(step > 0);
	__CPROVER_assume(base < step);
	__CPROVER_assume(cnum64_contains(a, v));
	__CPROVER_assume((v - base) % step == 0);

        struct cnum64 b = deduce_bounds_64_from_step(a, base, step);
	assert(cnum64_contains(b, v));
}
