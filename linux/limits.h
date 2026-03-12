/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_LIMITS_H
#define _LINUX_LIMITS_H

#define U8_MAX   ((u8)0xFF)
#define S8_MAX   ((s8)0x7F)
#define S8_MIN   ((s8)0x80)

#define U16_MAX   ((u16)0xFFFF)
#define S16_MAX   ((s16)0x7FFF)
#define S16_MIN   ((s16)0x8000)

#define U32_MAX  ((u32)0xFFFFFFFF)
#define S32_MAX  ((s32)0x7FFFFFFF)
#define S32_MIN  ((s32)0x80000000)

#define U64_MAX  ((u64)0xFFFFFFFFFFFFFFFFULL)
#define S64_MAX  ((s64)0x7FFFFFFFFFFFFFFFLL)
#define S64_MIN  ((s64)0x8000000000000000LL)

#endif /* _LINUX_LIMITS_H */
