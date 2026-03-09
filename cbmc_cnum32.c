#include <linux/cnum.h>
#include <linux/limits.h>
#include <assert.h>

u64 nondet_u64(void);
u32 nondet_u32(void);
s32 nondet_s32(void);

void check_from_urange(void)
{
	u32 lo = nondet_u32();
	u32 hi = nondet_u32();
	__CPROVER_assume(lo <= hi);

	struct cnum32 c = cnum32_from_urange(lo, hi);
	u32 v = nondet_u32();

	if (v >= lo && v <= hi)
		assert(cnum32_contains(c, v));
	else
		assert(!cnum32_contains(c, v));
}

void check_from_srange(void)
{
	s32 lo = nondet_s32();
	s32 hi = nondet_s32();
	__CPROVER_assume(lo <= hi);

	struct cnum32 c = cnum32_from_srange(lo, hi);
	u32 v = nondet_u32();

	if ((s32)v >= lo && (s32)v <= hi)
		assert(cnum32_contains(c, v));
	else
		assert(!cnum32_contains(c, v));
}

void check_umin_umax(void)
{
	struct cnum32 c = { nondet_u32(), nondet_u32() };
	u32 lo = cnum32_umin(c);
	u32 hi = cnum32_umax(c);
	u32 v = nondet_u32();
	bool urange_overflow = c.size > U32_MAX - c.base;

	/* any value contained in the cnum is within [umin, umax] */
	if (cnum32_contains(c, v))
		assert(v >= lo && v <= hi);

	/* when no urange overflow, [umin, umax] is exact */
	if (!urange_overflow && !cnum32_contains(c, v))
		assert(v < lo || v > hi);
}

void check_smin_smax(void)
{
	struct cnum32 c = { nondet_u32(), nondet_u32() };
	s32 lo = cnum32_smin(c);
	s32 hi = cnum32_smax(c);
	u32 v = nondet_u32();
	bool srange_overflow = cnum32_contains(c, (u32)S32_MAX) &&
			       cnum32_contains(c, (u32)S32_MIN);

	/* any value contained in the cnum is within [smin, smax] */
	if (cnum32_contains(c, v))
		assert((s32)v >= lo && (s32)v <= hi);

	/* when no srange overflow, [smin, smax] is exact */
	if (!srange_overflow && !cnum32_contains(c, v))
		assert((s32)v < lo || (s32)v > hi);
}

void check_intersect(void)
{
	struct cnum32 a = { nondet_u32(), nondet_u32() };
	struct cnum32 b = { nondet_u32(), nondet_u32() };
	struct cnum32 c;
	bool result = cnum32_intersect(a, b, &c);
	u32 v = nondet_u32();

	bool in_a = cnum32_contains(a, v);
	bool in_b = cnum32_contains(b, v);
	bool in_both = in_a && in_b;

	if (!result) {
		/* empty intersection => no value is in both */
		assert(!in_both);
	} else {
		/* non-empty => result is a superset of the true intersection */
		assert(!in_both || cnum32_contains(c, v));
	}
}

void check_32_from_64(void)
{
	struct cnum64 a = { nondet_u64(), nondet_u64() };
	struct cnum32 b = cnum32_from_cnum64(a);
	u64 v = nondet_u64();

	bool in_a = cnum64_contains(a, v);
	bool in_b = cnum32_contains(b, (u32)v);

	if (in_a)
		assert(in_b);
}

int main(void)
{
	check_from_urange();
	check_from_srange();
	check_umin_umax();
	check_smin_smax();
	check_intersect();
	return 0;
}
