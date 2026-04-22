#ifndef T
#error "Define T (bit width: 32, 64) before including checks.h"
#endif

#include <linux/cnum.h>
#include <linux/limits.h>
#include <linux/minmax.h>
#include <linux/compiler_types.h>
#include <assert.h>

#ifndef __CPROVER
void __CPROVER_assume(_Bool);
#endif

#define cnum_t  __PASTE(cnum, T)
#define ut      __PASTE(u, T)
#define st      __PASTE(s, T)
#define UT_MAX  __PASTE(__PASTE(U, T), _MAX)
#define ST_MAX  __PASTE(__PASTE(S, T), _MAX)
#define ST_MIN  __PASTE(__PASTE(S, T), _MIN)
#define FN(name) __PASTE(__PASTE(cnum, T), __PASTE(_, name))
#define CHECK(name) __PASTE(__PASTE(check_, T), __PASTE(_, name))

#if T == 8
#define utt u32
#define stt s32
#elif T == 32
#define utt u32
#define stt s32
#elif T == 64
#define utt u64
#define stt s64
#endif

#define nondet_ut __PASTE(nondet_u, T)
#define nondet_st __PASTE(nondet_s, T)
ut nondet_ut(void);
st nondet_st(void);

void CHECK(from_urange)(void)
{
	ut lo = nondet_ut();
	ut hi = nondet_ut();
	__CPROVER_assume(lo <= hi);

	struct cnum_t c = FN(from_urange)(lo, hi);
	ut v = nondet_ut();

	if (v >= lo && v <= hi)
		assert(FN(contains)(c, v));
	else
		assert(!FN(contains)(c, v));
}

void CHECK(from_srange)(void)
{
	st lo = nondet_st();
	st hi = nondet_st();
	__CPROVER_assume(lo <= hi);

	struct cnum_t c = FN(from_srange)(lo, hi);
	ut v = nondet_ut();

	if ((st)v >= lo && (st)v <= hi)
		assert(FN(contains)(c, v));
	else
		assert(!FN(contains)(c, v));
}

void CHECK(umin_umax)(void)
{
	struct cnum_t c = { nondet_ut(), nondet_ut() };
	ut lo = FN(umin)(c);
	ut hi = FN(umax)(c);
	ut v = nondet_ut();
	bool uof = FN(urange_overflow)(c);

	if (FN(contains)(c, v))
		assert(v >= lo && v <= hi);

	if (!uof && !FN(contains)(c, v))
		assert(v < lo || v > hi);
}

void CHECK(smin_smax)(void)
{
	struct cnum_t c = { nondet_ut(), nondet_ut() };
	__CPROVER_assume(!FN(is_empty)(c));
	st lo = FN(smin)(c);
	st hi = FN(smax)(c);
	ut v = nondet_ut();
	bool sof = FN(contains)(c, (ut)ST_MAX) && FN(contains)(c, (ut)ST_MIN);

	if (FN(contains)(c, v))
		assert((st)v >= lo && (st)v <= hi);

	if (!sof && !FN(contains)(c, v))
		assert((st)v < lo || (st)v > hi);
}

void CHECK(intersect)(void)
{
	struct cnum_t a = { nondet_ut(), nondet_ut() };
	struct cnum_t b = { nondet_ut(), nondet_ut() };
	struct cnum_t c = FN(intersect)(a, b);
	ut v = nondet_ut();

	__CPROVER_assume(!FN(is_empty)(a));
	__CPROVER_assume(!FN(is_empty)(b));
	__CPROVER_assume(FN(contains)(a, v) && FN(contains)(b, v));

	assert(FN(contains)(c, v));
}

void CHECK(normalize)(void)
{
	struct cnum_t c = { nondet_ut(), nondet_ut() };
	struct cnum_t n = FN(normalize)(c);
	ut v = nondet_ut();

	__CPROVER_assume(!FN(is_empty)(c));
	assert(FN(contains)(c, v) == FN(contains)(n, v));

	if (n.size == UT_MAX)
		assert(n.base == 0 || n.base == (ut)ST_MAX);
}

void CHECK(is_empty)(void)
{
	struct cnum_t c = { nondet_ut(), nondet_ut() };

	if (FN(is_empty)(c))
		assert(c.base == UT_MAX && c.size == UT_MAX);
	if (c.base != UT_MAX || c.size != UT_MAX)
		assert(!FN(is_empty)(c));
}

static inline void __PASTE(verify_cut_, T)(struct cnum_t a, struct cnum_t *chunks, int n)
{
	int i;

	assert(chunks[0].base == a.base);
	assert((ut)((utt)chunks[n-1].base + chunks[n-1].size) == (ut)((utt)a.base + a.size));
	for (i = 0; i < n; i++) {
		st smin = (st)chunks[i].base;
		st smax = (st)((utt)chunks[i].base + chunks[i].size);
		assert((smin >= 0 && smax >= 0) || (smin < 0 && smax < 0));
	}
	for (i = 0; i < n - 1; i++)
		assert((ut)((utt)chunks[i].base + chunks[i].size + 1) == chunks[i+1].base);
}

void CHECK(cut_1)(void)
{
	struct cnum_t a = FN(normalize)((struct cnum_t){ nondet_ut(), nondet_ut() });
	struct cnum_t chunks[3];
	int n = FN(cut)(a, chunks);

	__CPROVER_assume(n == 1);
	__PASTE(verify_cut_, T)(a, chunks, n);
}

void CHECK(cut_2)(void)
{
	struct cnum_t a = FN(normalize)((struct cnum_t){ nondet_ut(), nondet_ut() });
	struct cnum_t chunks[3];
	int n = FN(cut)(a, chunks);

	__CPROVER_assume(n == 2);
	__PASTE(verify_cut_, T)(a, chunks, n);
}

void CHECK(cut_3)(void)
{
	struct cnum_t a = FN(normalize)((struct cnum_t){ nondet_ut(), nondet_ut() });
	struct cnum_t chunks[3];
	int n = FN(cut)(a, chunks);

	__CPROVER_assume(n == 3);
	__PASTE(verify_cut_, T)(a, chunks, n);
}

void CHECK(mul_chunk)(void)
{
	struct cnum_t a = { nondet_ut(), nondet_ut() };
	struct cnum_t b = { nondet_ut(), nondet_ut() };
	__CPROVER_assume(a.size <= ST_MAX);
	__CPROVER_assume(b.size <= ST_MAX);
	st a_smin = (st)a.base;
	st a_smax = (st)((utt)a.base + a.size);
	st b_smin = (st)b.base;
	st b_smax = (st)((utt)b.base + b.size);
	__CPROVER_assume((a_smin >= 0 && a_smax >= 0) || (a_smin < 0 && a_smax < 0));
	__CPROVER_assume((b_smin >= 0 && b_smax >= 0) || (b_smin < 0 && b_smax < 0));
	ut va = nondet_ut();
	ut vb = nondet_ut();
	__CPROVER_assume(FN(contains)(a, va) && FN(contains)(b, vb));
	ut vv = va * vb;

	struct cnum_t r = FN(mul_chunk)(a, b);
	assert(FN(contains)(r, vv));
}

#undef cnum_t
#undef ut
#undef st
#undef UT_MAX
#undef ST_MAX
#undef ST_MIN
#undef FN
#undef CHECK
#undef utt
#undef stt
#undef nondet_ut
#undef nondet_st
