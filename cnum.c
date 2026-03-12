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

bool range64_range32_intersect(struct range64 a, struct range32 b, struct range64 *out)
{
	/*
	 * To simplify the reasoning, shift the intervals a -> a1, b -> b1,
	 * such that a1 starts at origin.
	 */
	struct range64 a1 = { 0, a.max - a.min };
	struct range32 b1 = { b.min - (u32)a.min, b.max - (u32)a.min };
	struct range64 t = a;
	u64 d;

	if (b1.max < b1.min) {
		if ((u32)a1.max > b1.max && (u32)a1.max < b1.min) {
			/*
			 * N*2^32                   (N+1)*2^32
			 * ||=====|------------|=====||=====|---------|---|=====||
			 *  |b1 ->|            |<- b1||b1 ->|         |   |<- b1|
			 *  |<----------------- a1 ------------------>|
			 *  |<----------- new t ----------->|<-- d -->|
			 *                                  ^
			 *                                b1.max
			 */
			d = (u32)a1.max - b1.max;
			t.max -= d;
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
		if (a1.max < b1.min)
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
		t.min += b1.min;
		d = 0;
		if ((u32)a1.max < b1.min)
			/*
			 * N*2^32                   (N+1)*2^32
			 * ||----------|===========|-||------|----|===========|-||
			 *  |          |<-- b1  -->|         |    |<-- b1  -->|
			 *  |<---------+---- a1 ---+-------->|
			 *             |<- new t ->|<-- d -->|
			 */
			d = (u32)a1.max + (BIT_ULL(32) - b1.max);
		else if ((u32)a1.max >= b1.max)
			/*
			 * N*2^32                   (N+1)*2^32
			 * ||--|========|------------||--|========|-------|-----||
			 *  |  |<- b1 ->|                |<- b1 ->|       |
			 *  |<-+------------------ a1 ------------+------>|
			 *     |<------------ new t ------------->|<- d ->|
			 */
			d = (u32)a1.max - b1.max;
		if (t.max - t.min < d)
			return false;
		t.max -= d;
	}
	*out = t;
	return true;
}
