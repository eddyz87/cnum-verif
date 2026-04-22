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
	__CPROVER_assume(!FN(is_empty)(c));
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

void CHECK(contains)(void)
{
	struct cnum_t c = { nondet_ut(), nondet_ut() };
	ut v = nondet_ut();
	ut t = (ut)(v - c.base);

	__CPROVER_assume(!FN(is_empty)(c));
	assert(FN(contains)(c, v) == (t <= c.size));
}

void CHECK(add)(void)
{
	struct cnum_t a = { nondet_ut(), nondet_ut() };
	struct cnum_t b = { nondet_ut(), nondet_ut() };
	ut da = nondet_ut();
	ut db = nondet_ut();

	__CPROVER_assume(!FN(is_empty)(a));
	__CPROVER_assume(!FN(is_empty)(b));
	__CPROVER_assume(da <= a.size);
	__CPROVER_assume(db <= b.size);

	struct cnum_t r = FN(add)(a, b);
	assert(FN(contains)(r, (ut)(a.base + b.base + da + db)));
}

#undef cnum_t
#undef ut
#undef st
#undef UT_MAX
#undef ST_MAX
#undef ST_MIN
#undef FN
#undef CHECK
#undef nondet_ut
#undef nondet_st
