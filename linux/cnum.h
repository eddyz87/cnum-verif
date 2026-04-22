/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2026 Meta Platforms, Inc. and affiliates. */

#ifndef _LINUX_CNUM_H
#define _LINUX_CNUM_H

#include <linux/types.h>

/* cnum8 is used locally for exhaustive testing */
struct cnum8 {
	u8 base;
	u8 size;
};

#define CNUM8_UNBOUNDED ((struct cnum8){ .base = 0, .size = U8_MAX })
#define CNUM8_EMPTY ((struct cnum8){ .base = U8_MAX, .size = U8_MAX })

struct cnum8 cnum8_from_urange(u8 min, u8 max);
struct cnum8 cnum8_from_srange(s8 min, s8 max);
u8 cnum8_umin(struct cnum8 cnum);
u8 cnum8_umax(struct cnum8 cnum);
s8 cnum8_smin(struct cnum8 cnum);
s8 cnum8_smax(struct cnum8 cnum);
struct cnum8 cnum8_intersect(struct cnum8 a, struct cnum8 b);
bool cnum8_contains(struct cnum8 cnum, u8 v);
bool cnum8_is_const(struct cnum8 cnum);
bool cnum8_is_empty(struct cnum8 cnum);
struct cnum8 cnum8_add(struct cnum8 a, struct cnum8 b);
struct cnum8 cnum8_normalize(struct cnum8 cnum);
int cnum8_cut(struct cnum8 a, struct cnum8 chunks[3]);
struct cnum8 cnum8_mul_chunk(struct cnum8 a, struct cnum8 b);
struct cnum8 cnum8_mul(struct cnum8 a, struct cnum8 b);
struct cnum8 cnum8_negate(struct cnum8 a);
bool cnum8_urange_overflow(struct cnum8 cnum);
bool cnum8_srange_overflow(struct cnum8 cnum);

/*
 * cnum32: a circular number.
 * A unified representation for signed and unsigned ranges.
 *
 * Assume that a 32-bit range is a circle, with 0 being in the 12 o'clock
 * position, numbers placed sequentially in clockwise order and U32_MAX
 * in the 11 o'clock position. Signed values map onto the same circle:
 * S32_MAX sits at 5 o'clock, S32_MIN sits at 6 o'clock (opposite 0),
 * negative values occupy the left half and positive values the right half.
 *
 * @cnum32 represents an arc on this circle drawn clockwise.
 * @base corresponds to the first value of the range.
 * @size corresponds to the number of integers in the range excluding @base.
 * (The @base is excluded to avoid integer overflow when representing the full
 *  0..U32_MAX range, which corresponds to 2^32, which can't be stored in u32).
 *
 * For example: {U32_MAX, 1} corresponds to signed range [-1, 0],
 *              {S32_MAX, 1} corresponds to unsigned range [S32_MAX, S32_MIN].
 */
struct cnum32 {
	u32 base;
	u32 size;
};

#define CNUM32_UNBOUNDED ((struct cnum32){ .base = 0, .size = U32_MAX })
#define CNUM32_EMPTY ((struct cnum32){ .base = U32_MAX, .size = U32_MAX })

struct cnum32 cnum32_from_urange(u32 min, u32 max);
struct cnum32 cnum32_from_srange(s32 min, s32 max);
u32 cnum32_umin(struct cnum32 cnum);
u32 cnum32_umax(struct cnum32 cnum);
s32 cnum32_smin(struct cnum32 cnum);
s32 cnum32_smax(struct cnum32 cnum);
struct cnum32 cnum32_intersect(struct cnum32 a, struct cnum32 b);
bool cnum32_contains(struct cnum32 cnum, u32 v);
bool cnum32_is_const(struct cnum32 cnum);
bool cnum32_is_empty(struct cnum32 cnum);
struct cnum32 cnum32_add(struct cnum32 a, struct cnum32 b);
struct cnum32 cnum32_normalize(struct cnum32 cnum);
int cnum32_cut(struct cnum32 a, struct cnum32 chunks[3]);
struct cnum32 cnum32_mul_chunk(struct cnum32 a, struct cnum32 b);
struct cnum32 cnum32_mul(struct cnum32 a, struct cnum32 b);
struct cnum32 cnum32_negate(struct cnum32 a);
bool cnum32_urange_overflow(struct cnum32 cnum);
bool cnum32_srange_overflow(struct cnum32 cnum);

/* Same as cnum32 but for 64-bit ranges */
struct cnum64 {
	u64 base;
	u64 size;
};

#define CNUM64_UNBOUNDED ((struct cnum64){ .base = 0, .size = U64_MAX })
#define CNUM64_EMPTY ((struct cnum64){ .base = U64_MAX, .size = U64_MAX })

struct cnum64 cnum64_from_urange(u64 min, u64 max);
struct cnum64 cnum64_from_srange(s64 min, s64 max);
u64 cnum64_umin(struct cnum64 cnum);
u64 cnum64_umax(struct cnum64 cnum);
s64 cnum64_smin(struct cnum64 cnum);
s64 cnum64_smax(struct cnum64 cnum);
struct cnum64 cnum64_intersect(struct cnum64 a, struct cnum64 b);
bool cnum64_contains(struct cnum64 cnum, u64 v);
bool cnum64_is_const(struct cnum64 cnum);
bool cnum64_is_empty(struct cnum64 cnum);
struct cnum64 cnum64_add(struct cnum64 a, struct cnum64 b);
struct cnum64 cnum64_normalize(struct cnum64 cnum);
int cnum64_cut(struct cnum64 a, struct cnum64 chunks[3]);
struct cnum64 cnum64_mul_chunk(struct cnum64 a, struct cnum64 b);
struct cnum64 cnum64_mul(struct cnum64 a, struct cnum64 b);
struct cnum64 cnum64_negate(struct cnum64 a);
bool cnum64_urange_overflow(struct cnum64 cnum);
bool cnum64_srange_overflow(struct cnum64 cnum);

struct cnum32 cnum32_from_cnum64(struct cnum64 cnum);
struct cnum64 cnum64_cnum32_intersect(struct cnum64 a, struct cnum32 b);

#endif /* _LINUX_CNUM_H */
