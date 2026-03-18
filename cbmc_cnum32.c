#include <linux/cnum.h>
#include <linux/limits.h>
#include <assert.h>

u64 nondet_u64(void);
u32 nondet_u32(void);
u16 nondet_u16(void);
u8 nondet_u8(void);

u64 nondet_s64(void);
u32 nondet_s32(void);
u16 nondet_s16(void);
u8 nondet_s8(void);

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

void check_cnum64_cnum32_intersect(void)
{
	struct cnum64 a = { nondet_u64(), nondet_u64() };
	struct cnum32 b = { nondet_u32(), nondet_u32() };
	struct cnum64 c;
	u64 v = nondet_u64();

	bool in_a = cnum64_contains(a, v);
	bool in_b = cnum32_contains(b, (u32)v);
	if (in_a && in_b) {
		bool isec = cnum64_cnum32_intersect(a, b, &c);
		assert(isec);
		assert(cnum64_contains(c, v));
	}
}

void check_range16_range8_intersect_uu(void)
{
	struct range16 a = { nondet_u16(), nondet_u16() };
	struct range8 b = { nondet_u8(), nondet_u8() };
	struct range16 c;
	u16 v = nondet_u16();

	__CPROVER_assume(a.min <= a.max);
	__CPROVER_assume(b.min <= b.max);

	bool in_a = a.min <= v && v <= a.max;
	bool in_b = b.min <= (u8)v && (u8)v <= b.max;
	if (in_a && in_b) {
		bool isec = range16_range8_intersect(a, b, &c);
		assert(isec);
		assert(c.min <= v && v <= c.max);
	}
}

void check_range16_range8_intersect_ss(void)
{
	struct range16 a = { nondet_s16(), nondet_s16() };
	struct range8 b = { nondet_s8(), nondet_s8() };
	struct range16 c;
	s16 v = nondet_s16();

	__CPROVER_assume((s16)a.min <= (s16)a.max);
	__CPROVER_assume((s8)b.min <= (s8)b.max);

	bool in_a = (s16)a.min <= v && v <= (s16)a.max;
	bool in_b = (s8)b.min <= (s8)v && (s8)v <= (s8)b.max;
	if (in_a && in_b) {
		bool isec = range16_range8_intersect(a, b, &c);
		assert(isec);
		assert((s16)c.min <= v && v <= (s16)c.max);
	}
}

void check_range16_range8_intersect_su(void)
{
	struct range16 a = { nondet_s16(), nondet_s16() };
	struct range8 b = { nondet_u8(), nondet_u8() };
	struct range16 c;
	u16 v = nondet_u16();

	__CPROVER_assume((s16)a.min <= (s16)a.max);
	__CPROVER_assume(b.min <= b.max);

	bool in_a = (s16)a.min <= (s16)v && (s16)v <= (s16)a.max;
	bool in_b = b.min <= (u8)v && (u8)v <= b.max;
	if (in_a && in_b) {
		bool isec = range16_range8_intersect(a, b, &c);
		assert(isec);
		assert((s16)c.min <= (s16)v && (s16)v <= (s16)c.max);
	}
}

void check_range16_range8_intersect_us(void)
{
	struct range16 a = { nondet_u16(), nondet_u16() };
	struct range8 b = { nondet_s8(), nondet_s8() };
	struct range16 c;
	u16 v = nondet_u16();

	__CPROVER_assume(a.min <= a.max);
	__CPROVER_assume((s8)b.min <= (s8)b.max);

	bool in_a = a.min <= v && v <= a.max;
	bool in_b = (s8)b.min <= (s8)v && (s8)v <= (s8)b.max;
	if (in_a && in_b) {
		bool isec = range16_range8_intersect(a, b, &c);
		assert(isec);
		assert(c.min <= v && v <= c.max);
	}
}

void check_range64_range32_intersect(void)
{
	struct range64 a = { nondet_u64(), nondet_u64() };
	struct range32 b = { nondet_u32(), nondet_u32() };
	struct range64 c;
	u64 v = nondet_u64();

        __CPROVER_assume(a.min <= a.max);
        __CPROVER_assume(b.min <= b.max);

	bool in_a = a.min <= v && v <= a.max;
	bool in_b = b.min <= (u32)v && (u32)v <= b.max;
	if (in_a && in_b) {
		bool isec = range64_range32_intersect(a, b, &c);
		assert(isec);
		assert(c.min <= v && v <= c.max);
	}
}

void check_range64_range32_intersect2(void)
{
	struct range64 a = { nondet_s64(), nondet_s64() };
	struct range32 b = { nondet_s32(), nondet_s32() };
	struct range64 c;
	s64 v = nondet_s64();

        __CPROVER_assume((s64)a.min <= (s64)a.max);
        __CPROVER_assume((s32)b.min <= (s32)b.max);

	bool in_a = (s64)a.min <= v && v <= (s64)a.max;
	bool in_b = (s32)b.min <= (s32)v && (s32)v <= (s32)b.max;
	if (in_a && in_b) {
		bool isec = range64_range32_intersect(a, b, &c);
		assert(isec);
		assert((s64)c.min <= v && v <= (s64)c.max);
	}
}

void check_range64_range32_intersect3(void)
{
	struct range64 a = { nondet_s64(), nondet_s64() };
	struct range32 b = { nondet_u32(), nondet_u32() };
	struct range64 c;
	u64 v = nondet_u64();

        __CPROVER_assume((s64)a.min <= (s64)a.max);
        __CPROVER_assume((u32)b.min <= (u32)b.max);

	bool in_a = (s64)a.min <= (s64)v && (s64)v <= (s64)a.max;
	bool in_b = (u32)b.min <= (u32)v && (u32)v <= (u32)b.max;
	if (in_a && in_b) {
		bool isec = range64_range32_intersect(a, b, &c);
		assert(isec);
		assert((s64)c.min <= (s64)v && (s64)v <= (s64)c.max);
	}
}

void check_range64_range32_intersect4(void)
{
	struct range64 a = { nondet_u64(), nondet_u64() };
	struct range32 b = { nondet_s32(), nondet_s32() };
	struct range64 c;
	u64 v = nondet_u64();

        __CPROVER_assume((u64)a.min <= (u64)a.max);
        __CPROVER_assume((s32)b.min <= (s32)b.max);

	bool in_a = (u64)a.min <= (u64)v && (u64)v <= (u64)a.max;
	bool in_b = (s32)b.min <= (s32)v && (s32)v <= (s32)b.max;
	if (in_a && in_b) {
		bool isec = range64_range32_intersect(a, b, &c);
		assert(isec);
		assert((u64)c.min <= (u64)v && (u64)v <= (u64)c.max);
	}
}

int main(void)
{
	check_from_urange();
	check_from_srange();
	check_umin_umax();
	check_smin_smax();
	check_intersect();
	check_cnum64_cnum32_intersect();
	check_range64_range32_intersect();
	check_range64_range32_intersect2();
	check_range64_range32_intersect3();
	check_range64_range32_intersect4();
	return 0;
}
