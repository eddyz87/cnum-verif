// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2026 Meta Platforms, Inc. and affiliates. */

#include <linux/bits.h>

#define T 8
#include "cnum_defs.h"
#undef T

#define T 16
#include "cnum_defs.h"
#undef T

#define T 32
#include "cnum_defs.h"
#undef T

#define T 64
#include "cnum_defs.h"
#undef T

struct cnum32 cnum32_from_cnum64(struct cnum64 cnum)
{
	if (cnum.size > U32_MAX)
		return (struct cnum32){ .base = 0, .size = U32_MAX };
	else
		return (struct cnum32){ .base = (u32)cnum.base, .size = cnum.size };
}

bool cnum16_cnum8_intersect(struct cnum16 a, struct cnum8 b, struct cnum16 *out)
{
	struct cnum8 b1 = { b.base - (u8)a.base, b.size };
	struct cnum16 t = a;
	u16 d, b1_max;

	if (!cnum8_urange_overflow(b1)) {
		if (t.size < b1.base)
			return false;
		t.base += b1.base;
		t.size -= b1.base;
		b1_max = b1.base + b1.size;
		d = 0;
		if ((u8)a.size < b1.base)
			d = (u8)a.size + U8_MAX - b1_max;
		else if ((u8)a.size >= b1_max)
			d = (u8)a.size - b1_max;
		if (t.size < d)
			return false;
		t.size -= d;
	}

	*out = t;
	return true;
}

/*
 * Suppose 'a' and 'b' are laid out as follows:
 *
 *                                                          64-bit number axis --->
 *
 * N*2^32                   (N+1)*2^32                (N+2)*2^32                (N+3)*2^32
 * ||------|---|=====|-------||----------|=====|-------||----------|=====|----|--||
 *         |   |< b >|                   |< b >|                   |< b >|    |
 *         |   |                                                         |    |
 *         |<--+--------------------------- a ---------------------------+--->|
 *	       |                                                         |
 *	       |<-------------------------- t -------------------------->|
 *
 * In such a case it is possible to infer a more tight representation t
 * such that ∀ v ∈ a, (u32)a ∈ b: v ∈ t.
 *
 * Note that trickier cases are possible, e.g.:
 *
 * N*2^32                   (N+1)*2^32                (N+2)*2^32                (N+3)*2^32
 * ||--|=====|----|----------||--|=====|---------------||--|=====|------------|--||
 *     |< b >|    |              |< b >|                   |< b >|            |
 *                |              |                               |            |
 *                |<-------------+--------- a -------------------|----------->|
 *	                         |                               |
 *	                         |<-------- t ------------------>|
 *
 * Or with b crossing the U32_MAX / 0 boundary:
 *
 * N*2^32                   (N+1)*2^32                (N+2)*2^32                (N+3)*2^32
 * ||===|---------|------|===||===|----------------|===||===|---------|------|===||
 *  |b >|         |      |< b||b >|                |< b||b >|         |      |< b|
 *                |      |                                  |         |
 *                |<-----+----------------- a --------------+-------->|
 *                       |                                  |
 *                       |<---------------- t ------------->|
 */
bool cnum64_cnum32_intersect(struct cnum64 a, struct cnum32 b, struct cnum64 *out)
{
	/*
	 * To simplify reasoning, rotate the circles so that [virtual] a1 starts
	 * at u32 boundary, b1 represents b in this new frame of reference.
	 */
	struct cnum32 b1 = { b.base - (u32)a.base, b.size };
	struct cnum64 t = a;
	u64 d, b1_max;

	if (cnum32_urange_overflow(b1)) {
		b1_max = (u32)b1.base + (u32)b1.size; /* overflow here is fine and necessary */
		if ((u32)a.size > b1_max && (u32)a.size < b1.base) {
			/*
			 * N*2^32                   (N+1)*2^32
			 * ||=====|------------|=====||=====|---------|---|=====||
			 *  |b1 ->|            |<- b1||b1 ->|         |   |<- b1|
			 *  |<----------------- a1 ------------------>|
			 *  |<----------- new t ----------->|<-- d -->|
			 *                                  ^
			 *                                b1_max
			 */
			d = (u32)a.size - b1_max;
			t.size -= d;
		} else {
			/*
			 * No adjustments possible in the following cases:
			 *
			 * ||=====|------------|=====||===|=|-------------|=|===||
			 *  |b1 ->|            |<- b1||b1 +>|             |<+ b1|
			 *  |<----------------- a1 ------>|                 |
			 *  |<----------------- (or) a1 ------------------->|
			 */
		}
	} else {
		if (t.size < b1.base)
			/*
			 * N*2^32                   (N+1)*2^32
			 * ||----------|--|=======|--||------>
			 *  |<-- a1 -->|  |<- b ->|
			 */
			return false;
		/*
		 * N*2^32                   (N+1)*2^32
		 * ||-------------|========|-||-----| -------|========|-||
		 *  |             |<- b1 ->|        |        |<- b1 ->|
		 *  |<------------+-- a1 --+------->|
		 *                |<---- new t ---->|
		 */
		t.base += b1.base;
		t.size -= b1.base;
		b1_max = b1.base + b1.size;
		d = 0;
		if ((u32)a.size < b1.base)
			/*
			 * N*2^32                   (N+1)*2^32
			 * ||----------|===========|-||------|----|===========|-||
			 *  |          |<-- b1  -->|         |    |<-- b1  -->|
			 *  |<---------+---- a1 ---+-------->|
			 *             |<- new t ->|<-- d -->|
			 */
			d = (u32)a.size + (BIT_ULL(32) - b1_max);
		else if ((u32)a.size >= b1_max)
			/*
			 * N*2^32                   (N+1)*2^32
			 * ||--|========|------------||--|========|-------|-----||
			 *  |  |<- b1 ->|                |<- b1 ->|       |
			 *  |<-+------------------ a1 ------------+------>|
			 *     |<------------ new t ------------->|<- d ->|
			 */
			d = (u32)a.size - b1_max;
		if (t.size < d)
			return false;
		t.size -= d;
	}
	*out = t;
	return true;
}

/* Replace low 8 bits of x with y, keeping upper 8 bits. */
static __always_inline u16 swap_low8(u16 x, u8 y)
{
	return (x & 0xff00) | y;
}

/* Move to the next / previous 2^8-aligned block. */
static __always_inline u16 next_u8_block(u16 x) { return x + (1U << 8); }
static __always_inline u16 prev_u8_block(u16 x) { return x - (1U << 8); }

/* Is v within the circular u16 range [base, base + len]? */
static __always_inline bool u16_range_contains(u16 v, u16 base, u16 len)
{
	return (u16)(v - base) <= len;
}

static __always_inline bool u8_range_contains(u8 v, u8 base, u8 len)
{
	return (u8)(v - base) <= len;
}

bool range16_range8_intersect(struct range16 a, struct range8 b, struct range16 *out)
{
        u16 b_len = (u8)(b.max - b.min);
	u16 a_len = a.max - a.min;
	u16 lo, hi;

	if (u8_range_contains((u8)a.min, (u8)b.min, b_len)) {
		lo = a.min;
	} else {
		lo = swap_low8(a.min, (u8)b.min);
		if (!u16_range_contains(lo, a.min, a_len))
			lo = next_u8_block(lo);
		if (!u16_range_contains(lo, a.min, a_len))
			return false;
	}
	if (u8_range_contains((u8)a.max, (u8)b.min, b_len)) {
		hi = a.max;
	} else {
		hi = swap_low8(a.max, (u8)b.max);
		if (!u16_range_contains(hi, a.min, a_len))
			hi = prev_u8_block(hi);
		if (!u16_range_contains(hi, a.min, a_len))
			return false;
	}
	*out = (struct range16){ lo, hi };
	return true;
}

/* Replace low 32 bits of x with y, keeping upper 32 bits. */
static __always_inline u64 swap_low32(u64 x, u32 y)
{
	return (x & 0xffffffff00000000ULL) | y;
}

/* Move to the next / previous 2^32-aligned block. */
static __always_inline u64 next_u32_block(u64 x) { return x + (1ULL << 32); }
static __always_inline u64 prev_u32_block(u64 x) { return x - (1ULL << 32); }

/* Is v within the circular u64 range [base, base + len]? */
static __always_inline bool u64_range_contains(u64 v, u64 base, u64 len)
{
	return v - base <= len;
}

/* Is v within the circular u32 range [base, base + len]? */
static __always_inline bool u32_range_contains(u32 v, u32 base, u32 len)
{
	return v - base <= len;
}

bool range64_range32_intersect(struct range64 a, struct range32 b, struct range64 *out)
{
	u64 b_len = (u32)(b.max - b.min);
	u64 a_len = a.max - a.min;
	u64 lo, hi;

	if (u32_range_contains((u32)a.min, (u32)b.min, b_len)) {
		lo = a.min;
	} else {
		lo = swap_low32(a.min, (u32)b.min);
		if (!u64_range_contains(lo, a.min, a_len))
			lo = next_u32_block(lo);
		if (!u64_range_contains(lo, a.min, a_len))
			return false;
	}
	if (u32_range_contains(a.max, (u32)b.min, b_len)) {
		hi = a.max;
	} else {
		hi = swap_low32(a.max, (u32)b.max);
		if (!u64_range_contains(hi, a.min, a_len))
			hi = prev_u32_block(hi);
		if (!u64_range_contains(hi, a.min, a_len))
			return false;
	}
	*out = (struct range64){ lo, hi };
	return true;
}
