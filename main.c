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

static int check_mul(struct cnum8 a, struct cnum8 b, struct cnum8 r)
{
	int v, w;

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
