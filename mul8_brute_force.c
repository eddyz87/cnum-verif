#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

#include <linux/cnum.h>
#include <linux/limits.h>

#define MAX_ERRORS 20

static atomic_int total_errors;
static atomic_int next_unit;
static atomic_int progress;

#define TOTAL_UNITS (256 * 256)

static int abort_on_first;

/* --- sanity3264 --- */

#define SANITY_MAX_SAMPLES 64

static int sample_chunk32(struct cnum32 c, u32 *out)
{
	int n = 0;

	if (c.size < SANITY_MAX_SAMPLES) {
		for (u32 i = 0; i <= c.size; i++)
			out[n++] = c.base + i;
	} else {
		out[n++] = c.base;
		out[n++] = c.base + 1;
		out[n++] = c.base + c.size / 2;
		out[n++] = c.base + c.size - 1;
		out[n++] = c.base + c.size;
	}
	return n;
}

static int sample_chunk64(struct cnum64 c, u64 *out)
{
	int n = 0;

	if (c.size < SANITY_MAX_SAMPLES) {
		for (u64 i = 0; i <= c.size; i++)
			out[n++] = c.base + i;
	} else {
		out[n++] = c.base;
		out[n++] = c.base + 1;
		out[n++] = c.base + c.size / 2;
		out[n++] = c.base + c.size - 1;
		out[n++] = c.base + c.size;
	}
	return n;
}

static int check_sanity32(struct cnum32 a, struct cnum32 b, const char *desc)
{
	struct cnum32 r = cnum32_mul(a, b);
	struct cnum32 ca[3], cb[3];
	int na, nb, errors = 0;
	u32 va[SANITY_MAX_SAMPLES], vb[SANITY_MAX_SAMPLES];

	if (cnum32_is_empty(a) || cnum32_is_empty(b)) {
		if (!cnum32_is_empty(r)) {
			printf("FAIL %s: expected empty\n", desc);
			return 1;
		}
		printf("PASS %s (empty)\n", desc);
		return 0;
	}

	na = cnum32_cut(cnum32_normalize(a), ca);
	nb = cnum32_cut(cnum32_normalize(b), cb);

	for (int i = 0; i < na; i++) {
		int nva = sample_chunk32(ca[i], va);
		for (int j = 0; j < nb; j++) {
			int nvb = sample_chunk32(cb[j], vb);
			for (int ai = 0; ai < nva; ai++)
				for (int bi = 0; bi < nvb; bi++) {
					u32 prod = va[ai] * vb[bi];
					if (!cnum32_contains(r, prod)) {
						printf("FAIL %s: a={%08x,%08x} b={%08x,%08x} "
						       "r={%08x,%08x} v=%08x w=%08x prod=%08x\n",
						       desc, a.base, a.size, b.base, b.size,
						       r.base, r.size, va[ai], vb[bi], prod);
						if (++errors >= 5)
							return errors;
					}
				}
		}
	}
	if (!errors)
		printf("PASS %s: r={%08x,%08x} (%d x %d cuts)\n",
		       desc, r.base, r.size, na, nb);
	return errors;
}

static int check_sanity64(struct cnum64 a, struct cnum64 b, const char *desc)
{
	struct cnum64 r = cnum64_mul(a, b);
	struct cnum64 ca[3], cb[3];
	int na, nb, errors = 0;
	u64 va[SANITY_MAX_SAMPLES], vb[SANITY_MAX_SAMPLES];

	if (cnum64_is_empty(a) || cnum64_is_empty(b)) {
		if (!cnum64_is_empty(r)) {
			printf("FAIL %s: expected empty\n", desc);
			return 1;
		}
		printf("PASS %s (empty)\n", desc);
		return 0;
	}

	na = cnum64_cut(cnum64_normalize(a), ca);
	nb = cnum64_cut(cnum64_normalize(b), cb);

	for (int i = 0; i < na; i++) {
		int nva = sample_chunk64(ca[i], va);
		for (int j = 0; j < nb; j++) {
			int nvb = sample_chunk64(cb[j], vb);
			for (int ai = 0; ai < nva; ai++)
				for (int bi = 0; bi < nvb; bi++) {
					u64 prod = va[ai] * vb[bi];
					if (!cnum64_contains(r, prod)) {
						printf("FAIL %s: a={%llx,%llx} b={%llx,%llx} "
						       "r={%llx,%llx} v=%llx w=%llx prod=%llx\n",
						       desc,
						       (unsigned long long)a.base, (unsigned long long)a.size,
						       (unsigned long long)b.base, (unsigned long long)b.size,
						       (unsigned long long)r.base, (unsigned long long)r.size,
						       (unsigned long long)va[ai], (unsigned long long)vb[bi],
						       (unsigned long long)prod);
						if (++errors >= 5)
							return errors;
					}
				}
		}
	}
	if (!errors)
		printf("PASS %s: r={%llx,%llx} (%d x %d cuts)\n",
		       desc,
		       (unsigned long long)r.base, (unsigned long long)r.size,
		       na, nb);
	return errors;
}

static int test_sanity3264(void)
{
	int errors = 0;

	printf("=== 32-bit multiplication sanity checks ===\n");

	errors += check_sanity32(
		(struct cnum32){5, 3}, (struct cnum32){7, 2},
		"32 pos x pos");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 3, 3}, (struct cnum32){2, 2},
		"32 pos(large) x pos");
	errors += check_sanity32(
		(struct cnum32){(u32)-4, 3}, (struct cnum32){(u32)-8, 3},
		"32 neg x neg");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MIN, 3}, (struct cnum32){(u32)S32_MIN, 3},
		"32 neg(large) x neg(large)");
	errors += check_sanity32(
		(struct cnum32){5, 3}, (struct cnum32){(u32)-4, 3},
		"32 pos x neg");
	errors += check_sanity32(
		(struct cnum32){(u32)-4, 3}, (struct cnum32){5, 3},
		"32 neg x pos");
	/* 2-cut: crosses S32_MAX/S32_MIN */
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 2, 5}, (struct cnum32){3, 2},
		"32 2-cut(signed) x pos");
	/* 2-cut: crosses U32_MAX/0 */
	errors += check_sanity32(
		(struct cnum32){(u32)-3, 5}, (struct cnum32){3, 2},
		"32 2-cut(unsigned) x pos");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 2, 5},
		(struct cnum32){(u32)-3, 5},
		"32 2-cut(signed) x 2-cut(unsigned)");
	/* 3-cut: crosses both boundaries */
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 2, (u32)S32_MAX + 5},
		(struct cnum32){2, 1},
		"32 3-cut x pos");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 2, (u32)S32_MAX + 5},
		(struct cnum32){(u32)-2, 1},
		"32 3-cut x neg");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 2, (u32)S32_MAX + 5},
		(struct cnum32){(u32)S32_MAX - 2, 5},
		"32 3-cut x 2-cut(signed)");

	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 3, 3}, (struct cnum32){0, 1},
		"32 pos(large) x {0,1}");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MIN, 3}, (struct cnum32){(u32)-1, 1},
		"32 neg(large) x {-1,0}");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 2, 5}, (struct cnum32){0, 1},
		"32 2-cut(signed) x {0,1}");
	errors += check_sanity32(
		(struct cnum32){(u32)-3, 5}, (struct cnum32){(u32)-1, 1},
		"32 2-cut(unsigned) x {-1,0}");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 2, (u32)S32_MAX + 5},
		(struct cnum32){0, 1},
		"32 3-cut x {0,1}");
	errors += check_sanity32(
		(struct cnum32){(u32)S32_MAX - 2, (u32)S32_MAX + 5},
		(struct cnum32){(u32)-1, 1},
		"32 3-cut x {-1,0}");

	printf("\n=== 64-bit multiplication sanity checks ===\n");

	errors += check_sanity64(
		(struct cnum64){5, 3}, (struct cnum64){7, 2},
		"64 pos x pos");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 3, 3}, (struct cnum64){2, 2},
		"64 pos(large) x pos");
	errors += check_sanity64(
		(struct cnum64){(u64)-4, 3}, (struct cnum64){(u64)-8, 3},
		"64 neg x neg");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MIN, 3}, (struct cnum64){(u64)S64_MIN, 3},
		"64 neg(large) x neg(large)");
	errors += check_sanity64(
		(struct cnum64){5, 3}, (struct cnum64){(u64)-4, 3},
		"64 pos x neg");
	errors += check_sanity64(
		(struct cnum64){(u64)-4, 3}, (struct cnum64){5, 3},
		"64 neg x pos");
	/* 2-cut: crosses S64_MAX/S64_MIN */
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 2, 5}, (struct cnum64){3, 2},
		"64 2-cut(signed) x pos");
	/* 2-cut: crosses U64_MAX/0 */
	errors += check_sanity64(
		(struct cnum64){(u64)-3, 5}, (struct cnum64){3, 2},
		"64 2-cut(unsigned) x pos");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 2, 5},
		(struct cnum64){(u64)-3, 5},
		"64 2-cut(signed) x 2-cut(unsigned)");
	/* 3-cut: crosses both boundaries */
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 2, (u64)S64_MAX + 5},
		(struct cnum64){2, 1},
		"64 3-cut x pos");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 2, (u64)S64_MAX + 5},
		(struct cnum64){(u64)-2, 1},
		"64 3-cut x neg");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 2, (u64)S64_MAX + 5},
		(struct cnum64){(u64)S64_MAX - 2, 5},
		"64 3-cut x 2-cut(signed)");

	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 3, 3}, (struct cnum64){0, 1},
		"64 pos(large) x {0,1}");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MIN, 3}, (struct cnum64){(u64)-1, 1},
		"64 neg(large) x {-1,0}");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 2, 5}, (struct cnum64){0, 1},
		"64 2-cut(signed) x {0,1}");
	errors += check_sanity64(
		(struct cnum64){(u64)-3, 5}, (struct cnum64){(u64)-1, 1},
		"64 2-cut(unsigned) x {-1,0}");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 2, (u64)S64_MAX + 5},
		(struct cnum64){0, 1},
		"64 3-cut x {0,1}");
	errors += check_sanity64(
		(struct cnum64){(u64)S64_MAX - 2, (u64)S64_MAX + 5},
		(struct cnum64){(u64)-1, 1},
		"64 3-cut x {-1,0}");

	printf("\n%d total errors\n", errors);
	return errors;
}

static int check_mul(struct cnum8 a, struct cnum8 b, struct cnum8 r)
{
	int v, w;

	if ((cnum8_is_empty(a) || cnum8_is_empty(b))) {
		if (!cnum8_is_empty(r))
			printf("FAIL: a={%d,%d} b={%d,%d} r={%d,%d} but should be empty\n",
			       a.base, a.size, b.base, b.size, r.base, r.size);
		return 0;
	}
	for (v = a.base; v <= a.base + a.size; v++)
		for (w = b.base; w <= b.base + b.size; w++) {
			u8 prod = (u8)(v * w);
			if (!cnum8_contains(r, prod)) {
				printf("FAIL: a={%d,%d} b={%d,%d} r={%d,%d} "
				       "v=%d w=%d prod=%d\n",
				       a.base, a.size, b.base, b.size,
				       r.base, r.size, v, w, prod);
				if (abort_on_first ||
				    atomic_fetch_add(&total_errors, 1) + 1 >= MAX_ERRORS)
					return -1;
			}
	}
	return 0;
}

static void *test_mul_thread(void *arg)
{
	int unit, a_base, a_size, b_base, b_size;

	while ((unit = atomic_fetch_add(&next_unit, 1)) < TOTAL_UNITS) {
		a_base = unit / 256;
		a_size = unit % 256;
		struct cnum8 a = { a_base, a_size };
		for (b_base = 0; b_base <= U8_MAX; b_base++)
			for (b_size = 0; b_size <= U8_MAX; b_size++) {
				struct cnum8 b = { b_base, b_size };
				struct cnum8 r = cnum8_mul(a, b);
				if (check_mul(a, b, r) < 0)
					return NULL;
			}
		atomic_fetch_add(&progress, 1);
	}
	return NULL;
}

int main(int argc, char **argv)
{
	int i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);

	if (argc == 4 && strcmp(argv[1], "--cut") == 0) {
		struct cnum8 a = { strtoul(argv[2], NULL, 0), strtoul(argv[3], NULL, 0) };
		struct cnum8 chunks[3];
		int n = cnum8_cut(a, chunks);

		printf("{%u,%u}[%u,%u] => %d chunks:",
		       a.base, a.size,
		       a.base, (u8)(a.base + a.size),
		       n);
		for (i = 0; i < n; i++)
			printf(" {%u,%u}[%u..%u]", chunks[i].base, chunks[i].size,
			       chunks[i].base, (u8)(chunks[i].base + chunks[i].size));
		printf("\n");
		return 0;
	}

	if (argc == 6 && strcmp(argv[1], "--mul") == 0) {
		struct cnum8 a = { strtoul(argv[2], NULL, 0), strtoul(argv[3], NULL, 0) };
		struct cnum8 b = { strtoul(argv[4], NULL, 0), strtoul(argv[5], NULL, 0) };
		struct cnum8 r = cnum8_mul(a, b);

		printf("{%u,%u} * {%u,%u} = {%u,%u} u:[%u..%u] s:[%d..%d]\n",
		       a.base, a.size, b.base, b.size,
		       r.base, r.size,
		       cnum8_umin(r), cnum8_umax(r),
		       cnum8_smin(r), cnum8_smax(r));
		return 0;
	}

	if (argc == 7 && strcmp(argv[1], "--mul32") == 0) {
		struct cnum32 a = { strtoul(argv[2], NULL, 0), strtoul(argv[3], NULL, 0) };
		struct cnum32 b = { strtoul(argv[4], NULL, 0), strtoul(argv[5], NULL, 0) };
		struct cnum32 r = cnum32_mul(a, b);
		u32 v = strtoul(argv[6], NULL, 0);

		printf("{%08x,%08x} * {%08x,%08x} = {%08x,%08x} u:[%08x..%08x] s:[%08x..%08x]\n",
		       a.base, a.size, b.base, b.size,
		       r.base, r.size,
		       cnum32_umin(r), cnum32_umax(r),
		       cnum32_smin(r), cnum32_smax(r));
		printf("contains(%08x) == %d\n", v, cnum32_contains(r, v));
		return 0;
	}

	if (argc > 1 && strcmp(argv[1], "--sanity3264") == 0)
		return test_sanity3264() ? 1 : 0;

	if (argc > 1 && strcmp(argv[1], "--single") == 0) {
		ncpus = 1;
		abort_on_first = 1;
	}
	pthread_t *threads;

	if (ncpus < 1)
		ncpus = 1;
	threads = calloc(ncpus, sizeof(*threads));

	atomic_store(&total_errors, 0);
	atomic_store(&next_unit, 0);
	atomic_store(&progress, 0);

	for (i = 0; i < ncpus; i++)
		pthread_create(&threads[i], NULL, test_mul_thread, NULL);

	printf("testing mul on %d threads\n", ncpus);
	while (1) {
		int done = atomic_load(&progress);
		int errs = atomic_load(&total_errors);
		printf("\r%d / %d (%d%%), %d errors",
		       done, TOTAL_UNITS, done * 100 / TOTAL_UNITS, errs);
		fflush(stdout);
		if (done >= TOTAL_UNITS || errs >= MAX_ERRORS)
			break;
		usleep(200000);
	}
	printf("\n");

	for (i = 0; i < ncpus; i++)
		pthread_join(threads[i], NULL);

	free(threads);

	int errors = atomic_load(&total_errors);
	printf("test_mul: %d errors\n", errors);
	return errors ? 1 : 0;
}
