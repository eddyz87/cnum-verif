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
static atomic_int progress;
static atomic_long total_result_size;
static atomic_long total_true_size;
static atomic_long total_pairs;

struct thread_arg {
	int a_base_start;
	int a_base_end;
};

static int abort_on_first;

static void *test_mul_thread(void *arg)
{
	struct thread_arg *ta = arg;
	int a_base, a_size, b_base, b_size, v, w;
	long local_result = 0, local_true = 0, local_pairs = 0;

	for (a_base = ta->a_base_start; a_base < ta->a_base_end; a_base++) {
		for (a_size = 0; a_size <= U8_MAX; a_size++) {
			struct cnum8 a = { a_base, a_size };
			if ((a.size > (u8)(U8_MAX - a.base)))
				continue;
			if (a_base <= S8_MAX && a_base + a_size >= (u8)S8_MIN)
				continue;
			for (b_base = 0; b_base <= U8_MAX; b_base++)
			for (b_size = 0; b_size <= U8_MAX; b_size++) {
				struct cnum8 b = { b_base, b_size };
				if ((b.size > (u8)(U8_MAX - b.base)))
					continue;
				if (b_base <= S8_MAX && b_base + b_size >= (u8)S8_MIN)
					continue;
				struct cnum8 r = cnum8_mul(a, b);
				u8 seen[256 / 8] = {};
				int true_cnt = 0;
				for (v = a_base; v <= a_base + a_size; v++)
				for (w = b_base; w <= b_base + b_size; w++) {
					u8 prod = (u8)(v * w);
					if (!(seen[prod / 8] & (1 << (prod % 8)))) {
						seen[prod / 8] |= (1 << (prod % 8));
						true_cnt++;
					}
					if (!cnum8_contains(r, prod)) {
						printf("FAIL: a={%d,%d} b={%d,%d} r={%d,%d} "
						       "v=%d w=%d prod=%d\n",
						       a_base, a_size, b_base, b_size,
						       r.base, r.size, v, w, prod);
						if (abort_on_first ||
						    atomic_fetch_add(&total_errors, 1) + 1 >= MAX_ERRORS)
							return NULL;
					}
				}
				local_result += (int)r.size + 1;
				local_true += true_cnt;
				local_pairs++;
			}
		}
		atomic_fetch_add(&progress, 1);
	}
	atomic_fetch_add(&total_result_size, local_result);
	atomic_fetch_add(&total_true_size, local_true);
	atomic_fetch_add(&total_pairs, local_pairs);
	return NULL;
}

int main(int argc, char **argv)
{
	int i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);

	if (argc == 4 && strcmp(argv[1], "--cut") == 0) {
		struct cnum8 a = { strtoul(argv[2], NULL, 0), strtoul(argv[3], NULL, 0) };
		struct cnum8 chunks[3];
		int n = cnum8_cut(a, chunks);

		printf("{%u,%u} => %d chunks:", a.base, a.size, n);
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
	struct thread_arg *args;
	int per, leftover, start;

	if (ncpus < 1)
		ncpus = 1;
	threads = calloc(ncpus, sizeof(*threads));
	args = calloc(ncpus, sizeof(*args));

	atomic_store(&total_errors, 0);
	atomic_store(&progress, 0);

	per = 256 / ncpus;
	leftover = 256 % ncpus;
	start = 0;
	for (i = 0; i < ncpus; i++) {
		int cnt = per + (i < leftover ? 1 : 0);
		args[i].a_base_start = start;
		args[i].a_base_end = start + cnt;
		start += cnt;
		pthread_create(&threads[i], NULL, test_mul_thread, &args[i]);
	}

	printf("testing mul on %d threads\n", ncpus);
	while (1) {
		int done = atomic_load(&progress);
		int errs = atomic_load(&total_errors);
		printf("\r%d / 256 (%d%%), %d errors", done, done * 100 / 256, errs);
		fflush(stdout);
		if (done >= 256 || errs >= MAX_ERRORS)
			break;
		usleep(200000);
	}
	printf("\n");

	for (i = 0; i < ncpus; i++)
		pthread_join(threads[i], NULL);

	free(threads);
	free(args);

	int errors = atomic_load(&total_errors);
	long res = atomic_load(&total_result_size);
	long tru = atomic_load(&total_true_size);
	long pairs = atomic_load(&total_pairs);
	printf("test_mul: %d errors\n", errors);
	printf("pairs: %ld, avg result: %.2f, avg true: %.2f, avg overapprox: %.2f (%.1f%%)\n",
	       pairs, (double)res / pairs, (double)tru / pairs,
	       (double)(res - tru) / pairs, (double)(res - tru) / tru * 100);
	return errors ? 1 : 0;
}
