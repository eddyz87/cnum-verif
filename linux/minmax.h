/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MINMAX_H
#define _LINUX_MINMAX_H

#define min(a, b) ({ typeof(a) ____a = (a); typeof(b) ____b = (b); ____a < ____b ? ____a : ____b; })
#define max(a, b) ({ typeof(a) ____a = (a); typeof(b) ____b = (b); ____a >= ____b ? ____a : ____b; })
#define swap(a, b) do { typeof(a) ____t = (a); a = (b); b = ____t; } while (0)

#endif /* _LINUX_MINMAX_H */
