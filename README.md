# cnum-verif

Verification of circular number (cnum) arithmetic operations using
CBMC (bounded model checking) and exhaustive 8-bit brute-force testing.

## CBMC checks (`make cbmc`)

Symbolic verification for 32-bit and 64-bit domains using CBMC with
SAT solver. Each check uses nondeterministic inputs covering the full
input space.

| Function      | Bit-widths | Property verified                                                                 |
|---------------|------------|-----------------------------------------------------------------------------------|
| `from_urange` | 32, 64     | Result contains exactly the values in `[lo, hi]`                                  |
| `from_srange` | 32, 64     | Result contains exactly the signed values in `[lo, hi]`                           |
| `umin`/`umax` | 32, 64     | Contained values fall within `[umin, umax]`; exact when no unsigned wrap          |
| `smin`/`smax` | 32, 64     | Contained values fall within `[smin, smax]`; exact when no signed wrap            |
| `intersect`   | 32, 64     | Empty result implies no common values; non-empty result is a superset of the true intersection |
| `normalize`   | 32, 64     | Preserves containment; full-range arcs get base 0 or `ST_MAX`                    |
| `is_empty`    | 32, 64     | Only the `{UT_MAX, UT_MAX}` sentinel is empty                                    |
| `cut`         | 32, 64     | Chunks start/end match the input arc, are contiguous, and each chunk has same-sign bounds (split into 3 sub-checks by chunk count) |

### Not checked by CBMC

- `mul_chunk`: intermediate arithmetic in `mk_mul_u` uses `utt` which is
  the same width as `ut` for T=32 and T=64, causing overflow. Needs `ut2`
  width for correct intermediate products.
- `mul`, `union`, `gap`, `extend`, `bigger`, `complement`: these involve
  loops or complex compositions that exceed CBMC's solver capacity at
  32/64-bit. Verified by 8-bit brute-force instead.

## 8-bit brute-force (`make mul8-brute-force`)

Exhaustive testing of `mul` over all 8-bit cnum pairs. For every
`(a, b)` pair, computes `cnum8_mul(a, b)` and verifies that the result
contains all true products `v * w` for `v` in `a`, `w` in `b`.

This transitively exercises `cut`, `mul_chunk`, `mk_mul_u`, `intersect`,
`union`, `gap`, `extend`, `bigger`, and `complement`.

Run with: `./mul8-brute-force`

## Build

```
make                   # build brute-force binary + run CBMC checks
make cbmc              # CBMC checks only
make mul8-brute-force  # build brute-force binary only
make compile-check     # syntax check without CBMC
```

Requires: CBMC (tested with 6.9.0), GCC.
