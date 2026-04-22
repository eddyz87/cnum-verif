# cnum-verif

Verification of circular number (cnum) arithmetic operations using
CBMC (bounded model checking).

## CBMC checks (`make cbmc`)

Symbolic verification using CBMC with SAT solver. Each check uses
nondeterministic inputs covering the full input space. Checks are
instantiated for 32-bit and 64-bit widths.

| Function      | Property verified                                                                 |
|---------------|-----------------------------------------------------------------------------------|
| `from_urange` | Result contains exactly the values in `[lo, hi]`                                  |
| `from_srange` | Result contains exactly the signed values in `[lo, hi]`                           |
| `umin`/`umax` | Contained values fall within `[umin, umax]`; exact when no unsigned wrap          |
| `smin`/`smax` | Contained values fall within `[smin, smax]`; exact when no signed wrap            |
| `intersect`   | If `v` is in both `a` and `b`, then `v` is in `intersect(a, b)`                  |
| `normalize`   | Preserves containment; full-range arcs get base 0 or `ST_MAX`                    |
| `is_empty`    | Only the `{UT_MAX, UT_MAX}` sentinel is empty                                    |
| `contains`    | `contains(c, v)` iff `(ut)(v - c.base) <= c.size`                                |
| `add`         | If `va` is in `a` and `vb` is in `b`, then `va + vb` is in `add(a, b)`           |

### Cross-width checks

| Function                  | Property verified                                                     |
|---------------------------|-----------------------------------------------------------------------|
| `cnum32_from_cnum64`      | If `v` is in the 64-bit cnum, then `(u32)v` is in the 32-bit result  |
| `cnum64_cnum32_intersect` | If `v` is in both `a` (64-bit) and `(u32)v` is in `b` (32-bit), then `v` is in the result |

### Not checked

- `negate`, `is_const`: no CBMC checks.

## Build

```
make                   # run CBMC checks
make cbmc              # same
make compile-check     # syntax check without CBMC
```

Requires: CBMC (tested with 6.9.0), GCC.
