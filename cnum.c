// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2026 Meta Platforms, Inc. and affiliates. */

#define T 8
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
