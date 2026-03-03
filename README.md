# cnum-verif

Brute-force verification for the `cnum_intersect` algorithm.
The test exhaustively enumerates all cnum8 pairs (8-bit, 256-value circle) and
validates `cnum_intersect` results against brute-force containment checks.

## Build & run

```
make
./cnum-test
```

To run a single test case by hex ID:

```
./cnum-test 00ff00ff
```

## CBMC verification

Formal verification of cnum32 operations using the
[CBMC](https://www.cprover.org/cbmc/) model checker.
CBMC symbolically explores all possible 32-bit inputs.

```
make cbmc
```

Verified properties:

- for `c = cnum32_from_urange(lo, hi)`, `c` contains exactly the
  values in `[lo, hi]`
- for `c = cnum32_from_srange(lo, hi)`, `c` contains exactly the
  values in `[lo, hi]` (signed comparison)
- `cnum32_umin` / `cnum32_umax` -- for any value `v` in the cnum, `v >=
  cnum32_umin(c) && v <= cnum32_umax(c)`; also, when no unsigned range
  overflow, any `v` outside the cnum is outside `[umin, umax]`
- `cnum32_smin` / `cnum32_smax` -- for any value `v` in the cnum,
  `(s32)v >= cnum32_smin(c) && (s32)v <= cnum32_smax(c)`; also, when
  no signed range overflow, any `v` outside the cnum is outside
  `[smin, smax]`
- `cnum32_intersect(a, b)` -- if result is empty, no value is in both
  `a` and `b`; if non-empty, the result is a superset of the true
  intersection (may over-approximate when intersection is two disjoint
  arcs)
