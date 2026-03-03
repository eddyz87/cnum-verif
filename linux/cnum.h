#ifndef CNUM_H
#define CNUM_H

#include <linux/types.h>

struct cnum8 {
	u8 base;
	u8 size;
};

struct cnum8 cnum8_from_urange(u8 min, u8 max);
struct cnum8 cnum8_from_srange(s8 min, s8 max);
u8 cnum8_umin(struct cnum8 cnum);
u8 cnum8_umax(struct cnum8 cnum);
s8 cnum8_smin(struct cnum8 cnum);
s8 cnum8_smax(struct cnum8 cnum);
bool cnum8_intersect(struct cnum8 a, struct cnum8 b, struct cnum8 *out);
bool cnum8_contains(struct cnum8 cnum, u8 v);

struct cnum32 {
	u32 base;
	u32 size;
};

struct cnum32 cnum32_from_urange(u32 min, u32 max);
struct cnum32 cnum32_from_srange(s32 min, s32 max);
u32 cnum32_umin(struct cnum32 cnum);
u32 cnum32_umax(struct cnum32 cnum);
s32 cnum32_smin(struct cnum32 cnum);
s32 cnum32_smax(struct cnum32 cnum);
bool cnum32_intersect(struct cnum32 a, struct cnum32 b, struct cnum32 *out);
bool cnum32_contains(struct cnum32 cnum, u32 v);

struct cnum64 {
	u64 base;
	u64 size;
};

struct cnum64 cnum64_from_urange(u64 min, u64 max);
struct cnum64 cnum64_from_srange(s64 min, s64 max);
u64 cnum64_umin(struct cnum64 cnum);
u64 cnum64_umax(struct cnum64 cnum);
s64 cnum64_smin(struct cnum64 cnum);
s64 cnum64_smax(struct cnum64 cnum);
bool cnum64_intersect(struct cnum64 a, struct cnum64 b, struct cnum64 *out);
bool cnum64_contains(struct cnum64 cnum, u64 v);

#endif /* CNUM_H */
