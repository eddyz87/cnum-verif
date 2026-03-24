#include <linux/bits.h>

#define T 8
#include "cnum_defs.h"
#undef T

#include <assert.h>

u8 nondet_u8(void);

/* normalize preserves containment and produces canonical base when size == U8_MAX */
void check_normalize(void)
{
	struct cnum8 c = { nondet_u8(), nondet_u8() };
	struct cnum8 n = cnum8_normalize(c);
	u8 v = nondet_u8();

	/* normalize preserves containment */
	assert(cnum8_contains(c, v) == cnum8_contains(n, v));

	/* if size == U8_MAX, base must be 0 or S8_MAX */
	if (n.size == U8_MAX)
		assert(n.base == 0 || n.base == (u8)S8_MAX);
}

/* is_empty: only the EMPTY sentinel is empty */
void check_is_empty(void)
{
	struct cnum8 c = { nondet_u8(), nondet_u8() };

	if (cnum8_is_empty(c))
		assert(c.base == U8_MAX && c.size == U8_MAX);
	if (c.base != U8_MAX || c.size != U8_MAX)
		assert(!cnum8_is_empty(c));
}

/* complement: v is in complement(c) iff v is NOT in c */
void check_complement(void)
{
	struct cnum8 c = { nondet_u8(), nondet_u8() };
	struct cnum8 cc = cnum8_complement(c);
	u8 v = nondet_u8();

	if (cnum8_is_empty(c)) {
		assert(cc.base == 0 && cc.size == U8_MAX);
	} else {
		__CPROVER_assume(c.size != U8_MAX); /* exclude full range */
		assert(cnum8_contains(c, v) != cnum8_contains(cc, v));
	}
}

/* gap: values in the gap are not in a and not in b */
void check_gap(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };

	struct cnum8 g = cnum8_gap(a, b);
	u8 v = nondet_u8();

	if (cnum8_is_empty(a)) {
		assert(cnum8_is_empty(g));
	} else if (cnum8_is_empty(b)) {
		assert(cnum8_is_empty(g));
	} else if (cnum8_contains_n(g, v)) {
		assert(!cnum8_contains(a, v));
		assert(!cnum8_contains(b, v));
	}
}

/* b_min a_min b_max a_max => universum (b wraps in a's frame) */
void check_extend_1(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a) && !cnum8_urange_overflow(a));
	__CPROVER_assume(!cnum8_is_empty(b) && !cnum8_urange_overflow(b));
	u8 a_max = (u8)((u32)a.base + a.size);
	u8 b_max = (u8)((u32)b.base + b.size);
	__CPROVER_assume(b.base < a.base && a.base <= b_max && b_max <= a_max);

	struct cnum8 e = cnum8_extend(a, b);
	assert(e.size == U8_MAX);
}

/* b_min a_min a_max b_max => universum (b wraps in a's frame) */
void check_extend_2(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a) && !cnum8_urange_overflow(a));
	__CPROVER_assume(!cnum8_is_empty(b) && !cnum8_urange_overflow(b));
	u8 a_max = (u8)((u32)a.base + a.size);
	u8 b_max = (u8)((u32)b.base + b.size);
	__CPROVER_assume(b.base < a.base && a.base <= a_max && a_max <= b_max);

	struct cnum8 e = cnum8_extend(a, b);
	assert(e.size == U8_MAX);
}

/* a_min b_min a_max b_max => extends to {a.base, b_max - a.base} */
void check_extend_3(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a) && !cnum8_urange_overflow(a));
	__CPROVER_assume(!cnum8_is_empty(b) && !cnum8_urange_overflow(b));
	u8 a_max = (u8)((u32)a.base + a.size);
	u8 b_max = (u8)((u32)b.base + b.size);
	__CPROVER_assume(a.base <= b.base && b.base <= a_max && a_max <= b_max);

	struct cnum8 e = cnum8_extend(a, b);
	assert(e.base == a.base);
	assert(e.size == (u8)((u32)b_max - a.base));
}

/* b_min b_max a_min a_max => extends clockwise to b_max */
void check_extend_4(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a) && !cnum8_urange_overflow(a));
	__CPROVER_assume(!cnum8_is_empty(b) && !cnum8_urange_overflow(b));
	u8 a_max = (u8)((u32)a.base + a.size);
	u8 b_max = (u8)((u32)b.base + b.size);
	__CPROVER_assume(b.base <= b_max && b_max < a.base && a.base <= a_max);

	struct cnum8 e = cnum8_extend(a, b);
	u8 expected = (u8)((u32)b_max - a.base);
	if (expected == U8_MAX)
		assert(e.size == U8_MAX);
	else
		assert(e.base == a.base && e.size == expected);
}

/* a_min a_max b_min b_max => extends to {a.base, b_max - a.base} */
void check_extend_5(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a) && !cnum8_urange_overflow(a));
	__CPROVER_assume(!cnum8_is_empty(b) && !cnum8_urange_overflow(b));
	u8 a_max = (u8)((u32)a.base + a.size);
	u8 b_max = (u8)((u32)b.base + b.size);
	__CPROVER_assume(a.base <= a_max && a_max <= b.base && b.base <= b_max);

	struct cnum8 e = cnum8_extend(a, b);
	assert(e.base == a.base);
	assert(e.size == (u8)((u32)b_max - a.base));
}

/* a_min b_min b_max a_max => a contains b, keeps a */
void check_extend_6(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a) && !cnum8_urange_overflow(a));
	__CPROVER_assume(!cnum8_is_empty(b) && !cnum8_urange_overflow(b));
	u8 a_max = (u8)((u32)a.base + a.size);
	u8 b_max = (u8)((u32)b.base + b.size);
	__CPROVER_assume(a.base <= b.base && b.base <= b_max && b_max <= a_max);

	struct cnum8 e = cnum8_extend(a, b);
	assert(e.base == a.base && e.size == a.size);
}

/* contains_cnum: a contains b iff every value in b is in a */
void check_contains_cnum(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	u8 v = nondet_u8();

	if (cnum8_contains_cnum(a, b) && cnum8_contains_n(b, v))
		assert(cnum8_contains_n(a, v));
}

/* bigger: result is one of the inputs, and its size >= the other */
void check_bigger(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a));
	__CPROVER_assume(!cnum8_is_empty(b));

	struct cnum8 r = cnum8_bigger(a, b);

	assert((r.base == a.base && r.size == a.size) ||
	       (r.base == b.base && r.size == b.size));
	assert(r.size >= a.size && r.size >= b.size);
}

/* union of 2 arcs contains all values from both arcs */
void check_union_2(void)
{
	struct cnum8 arcs[2] = {
		{ nondet_u8(), nondet_u8() },
		{ nondet_u8(), nondet_u8() },
	};
	__CPROVER_assume(!cnum8_is_empty(arcs[0]));
	__CPROVER_assume(!cnum8_is_empty(arcs[1]));

	struct cnum8 u = cnum8_union(arcs, 2);
	u8 v = nondet_u8();

	if (cnum8_contains(arcs[0], v) || cnum8_contains(arcs[1], v))
		assert(cnum8_contains(u, v));
}

/* union of 3 arcs contains all values from all arcs */
void check_union_3(void)
{
	struct cnum8 arcs[3] = {
		{ nondet_u8(), nondet_u8() },
		{ nondet_u8(), nondet_u8() },
		{ nondet_u8(), nondet_u8() },
	};
	__CPROVER_assume(!cnum8_is_empty(arcs[0]));
	__CPROVER_assume(!cnum8_is_empty(arcs[1]));
	__CPROVER_assume(!cnum8_is_empty(arcs[2]));
	/* union requires sorted by base */
	__CPROVER_assume(arcs[0].base <= arcs[1].base);
	__CPROVER_assume(arcs[1].base <= arcs[2].base);

	struct cnum8 u = cnum8_union(arcs, 3);
	u8 v = nondet_u8();

	if (cnum8_contains(arcs[0], v) ||
	    cnum8_contains(arcs[1], v) ||
	    cnum8_contains(arcs[2], v))
		assert(cnum8_contains(u, v));
}

/* add: v+w is in the result for any v in a, w in b */
void check_add(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };

	struct cnum8 r = cnum8_add(a, b);
	u8 va = nondet_u8();
	u8 vb = nondet_u8();

	if (cnum8_contains(a, va) && cnum8_contains(b, vb))
		assert(cnum8_contains(r, (u8)((u32)va + vb)));
}

/* mul: v*w is in the result for any v in a, w in b */
void check_mul(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a));
	__CPROVER_assume(!cnum8_is_empty(b));

	struct cnum8 r = cnum8_mul(a, b);
	u8 va = nondet_u8();
	u8 vb = nondet_u8();

	if (cnum8_contains(a, va) && cnum8_contains(b, vb))
		assert(cnum8_contains(r, (u8)((u32)va * vb)));
}

static inline void verify_cut(struct cnum8 a, struct cnum8 *chunks, int n)
{
	int i;

	/* first chunk starts where 'a' starts */
	assert(chunks[0].base == a.base);
	/* last chunk ends where 'a' ends */
	assert((u8)(chunks[n-1].base + chunks[n-1].size) == (u8)(a.base + a.size));
	for (i = 0; i < n; i++) {
		/* each chunk has same sign for both bounds */
		s8 smin = (s8)chunks[i].base;
		s8 smax = (s8)(chunks[i].base + chunks[i].size);
		assert((smin >= 0 && smax >= 0) || (smin < 0 && smax < 0));
	}
	for (i = 0; i < n - 1; i++)
		/* chunks are contiguous */
		assert((u8)(chunks[i].base + chunks[i].size + 1) == chunks[i+1].base);
}

void check_cut_1(void)
{
	struct cnum8 a = cnum8_normalize((struct cnum8){ nondet_u8(), nondet_u8() });
	struct cnum8 chunks[3];
	int n = cnum8_cut(a, chunks);

	__CPROVER_assume(n == 1);
	verify_cut(a, chunks, n);
}

void check_cut_2(void)
{
	struct cnum8 a = cnum8_normalize((struct cnum8){ nondet_u8(), nondet_u8() });
	struct cnum8 chunks[3];
	int n = cnum8_cut(a, chunks);

	__CPROVER_assume(n == 2);
	verify_cut(a, chunks, n);
}

void check_cut_3(void)
{
	struct cnum8 a = cnum8_normalize((struct cnum8){ nondet_u8(), nondet_u8() });
	struct cnum8 chunks[3];
	int n = cnum8_cut(a, chunks);

	__CPROVER_assume(n == 3);
	verify_cut(a, chunks, n);
}

/* union: verify contract holds (used with --enforce-contract) */
void check_union(void)
{
	struct cnum8 arcs[3] = {
		{ nondet_u8(), nondet_u8() },
		{ nondet_u8(), nondet_u8() },
		{ nondet_u8(), nondet_u8() },
	};
	__CPROVER_assume(!cnum8_is_empty(arcs[0]));
	__CPROVER_assume(!cnum8_is_empty(arcs[1]));
	__CPROVER_assume(!cnum8_is_empty(arcs[2]));
	__CPROVER_assume(arcs[0].base <= arcs[1].base);
	__CPROVER_assume(arcs[1].base <= arcs[2].base);

	struct cnum8 u = cnum8_union(arcs, 3);
	(void)u;
}

/* intersect: verify contract holds (used with --enforce-contract) */
void check_intersect(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	struct cnum8 out;

	cnum8_intersect(a, b, &out);
}

/* mul_chunk: for inputs not crossing sign or unsigned boundaries,
 * v*w is in the result for any v in a, w in b
 */
void check_mul_chunk(void)
{
	struct cnum8 a = { nondet_u8(), nondet_u8() };
	struct cnum8 b = { nondet_u8(), nondet_u8() };
	__CPROVER_assume(!cnum8_is_empty(a));
	__CPROVER_assume(!cnum8_is_empty(b));
	__CPROVER_assume(a.size < S8_MAX);
	__CPROVER_assume(b.size < S8_MAX);
	s8 a_smin = (s8)a.base;
	s8 a_smax = (s8)(a.base + a.size);
	s8 b_smin = (s8)b.base;
	s8 b_smax = (s8)(b.base + b.size);
	__CPROVER_assume((a_smin >= 0 && a_smax >= 0) || (a_smin < 0 && a_smax < 0));
	__CPROVER_assume((b_smin >= 0 && b_smax >= 0) || (b_smin < 0 && b_smax < 0));

	struct cnum8 r = cnum8_mul_chunk(a, b);
	u8 va = nondet_u8();
	u8 vb = nondet_u8();

	if (cnum8_contains(a, va) && cnum8_contains(b, vb))
		assert(cnum8_contains(r, (u8)((u32)va * vb)));
}

int main(void)
{
	check_normalize();
	check_is_empty();
	check_complement();
	check_gap();
	check_extend_1();
	check_extend_2();
	check_extend_3();
	check_extend_4();
	check_extend_5();
	check_extend_6();
	check_contains_cnum();
	check_bigger();
	check_union_2();
	check_union_3();
	check_add();
	check_mul();
	check_mul_chunk();
	check_cut_1();
	check_cut_2();
	check_cut_3();
	return 0;
}
