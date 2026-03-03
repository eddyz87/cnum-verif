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
