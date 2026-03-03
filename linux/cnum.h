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

#endif /* CNUM_H */
