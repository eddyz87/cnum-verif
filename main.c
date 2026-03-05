#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

#include <linux/cnum.h>
#include <linux/limits.h>

/* 1 (full circle) + 256*255 (all others) = 65281 */
static struct cnum8 all_cnums[65281];
static int cnums_cnt;

static void init_all_cnums(void)
{
	int i, j, k;

	k = 0;
	all_cnums[k++] = (struct cnum8){0, U8_MAX};
	for (i = 0; i <= U8_MAX; i++)
		for (j = 0; j <= U8_MAX - 1; j++)
			all_cnums[k++] = (struct cnum8){i, j};
	cnums_cnt = k;
}

static pthread_mutex_t report_lock = PTHREAD_MUTEX_INITIALIZER;

static int __attribute__((format(printf, 1, 2)))
report_error(const char *fmt, ...)
{
	va_list ap;

	pthread_mutex_lock(&report_lock);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	pthread_mutex_unlock(&report_lock);
	return 1;
}

static int test_intersect_once(struct cnum8 a, struct cnum8 b)
{
	unsigned int tid = (a.base << 24) | (a.size << 16) | (b.base << 8) | b.size;
	struct cnum8 c;
	bool has_isect = cnum8_intersect(a, b, &c);
	struct cnum8 smaller = a.size < b.size ? a : b;
	int v, sub_ranges = 0;
	bool prev_in;

	/* count how many disjoint intervals a and b intersect in */
	prev_in = cnum8_contains(a, U8_MAX) && cnum8_contains(b, U8_MAX);
	for (v = 0; v <= U8_MAX; v++) {
		bool cur_in = cnum8_contains(a, v) && cnum8_contains(b, v);
		if (cur_in && !prev_in)
			sub_ranges++;
		prev_in = cur_in;
	}
	if (sub_ranges == 0 && prev_in)
		sub_ranges = 1;

	switch (sub_ranges) {
	case 0:
		if (has_isect)
			return report_error("FAIL [%08x]: ({%02x,%02x}, {%02x,%02x}) "
					    "expected empty, got {%02x,%02x}\n",
					    tid, a.base, a.size, b.base, b.size,
					    c.base, c.size);
		break;
	case 1:
		if (!has_isect)
			return report_error("FAIL [%08x]: ({%02x,%02x}, {%02x,%02x}) "
					    "expected non-empty, got empty\n",
					    tid, a.base, a.size, b.base, b.size);
		for (v = 0; v <= U8_MAX; v++) {
			bool in_both = cnum8_contains(a, v) && cnum8_contains(b, v);
			if (in_both != cnum8_contains(c, v))
				return report_error("FAIL [%08x]: ({%02x,%02x}, {%02x,%02x}) = "
						    "{%02x,%02x} mismatch at %02x\n",
						    tid, a.base, a.size, b.base, b.size,
						    c.base, c.size, v);
		}
		break;
	default:
		if (!has_isect)
			return report_error("FAIL [%08x]: ({%02x,%02x}, {%02x,%02x}) "
					    "expected non-empty, got empty\n",
					    tid, a.base, a.size, b.base, b.size);
		if (c.base != smaller.base || c.size != smaller.size)
			return report_error("FAIL [%08x]: ({%02x,%02x}, {%02x,%02x}) = "
					    "{%02x,%02x} should be smaller {%02x,%02x}\n",
					    tid, a.base, a.size, b.base, b.size,
					    c.base, c.size, smaller.base, smaller.size);
		break;
	}
	return 0;
}

#define MAX_ERRORS 20

static atomic_int progress;
static atomic_int total_errors;

struct thread_arg {
	int start;
	int end;
};

static void *test_intersect_thread(void *arg)
{
	struct thread_arg *ta = arg;
	int i, j, errors = 0, done = 0;

	for (i = ta->start; i < ta->end; i++) {
		for (j = i; j < cnums_cnt; j++) {
			errors += test_intersect_once(all_cnums[i], all_cnums[j]);
			if (atomic_load(&total_errors) + errors >= MAX_ERRORS)
				goto out;
		}
		if (++done % 100 == 0)
			atomic_fetch_add(&progress, 100);
	}
out:
	atomic_fetch_add(&progress, done % 100);
	atomic_fetch_add(&total_errors, errors);
	return NULL;
}

static int test_intersect(void)
{
	int ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t *threads;
	struct thread_arg *args;
	int i, rows_per_thread, leftover;

	if (ncpus < 1)
		ncpus = 1;
	threads = calloc(ncpus, sizeof(*threads));
	args = calloc(ncpus, sizeof(*args));

	atomic_store(&progress, 0);
	atomic_store(&total_errors, 0);

	printf("running on %d threads\n", ncpus);

	rows_per_thread = cnums_cnt / ncpus;
	leftover = cnums_cnt % ncpus;
	int start = 0;
	for (i = 0; i < ncpus; i++) {
		int count = rows_per_thread + (i < leftover ? 1 : 0);
		args[i].start = start;
		args[i].end = start + count;
		start += count;
		pthread_create(&threads[i], NULL, test_intersect_thread, &args[i]);
	}

	/* progress reporting from main thread */
	while (1) {
		int done = atomic_load(&progress);
		int errs = atomic_load(&total_errors);
		printf("\r%d / %d (%d%%), %d errors", done, cnums_cnt, done * 100 / cnums_cnt, errs);
		fflush(stdout);
		if (done >= cnums_cnt || errs >= MAX_ERRORS)
			break;
		usleep(50000);
	}
	printf("\n");

	for (i = 0; i < ncpus; i++)
		pthread_join(threads[i], NULL);

	free(threads);
	free(args);
	return atomic_load(&total_errors);
}

int main(int argc, char **argv)
{
	int errors;

	if (argc == 6 && strcmp(argv[1], "--intersect32") == 0) {
		u32 a = strtoul(argv[2], NULL, 0);
		u32 b = strtoul(argv[3], NULL, 0);
		u32 c = strtoul(argv[4], NULL, 0);
		u32 d = strtoul(argv[5], NULL, 0);
		struct cnum32 r1 = cnum32_from_urange(a, b);
		struct cnum32 r2 = cnum32_from_urange(c, d);
		struct cnum32 out;
		bool has = cnum32_intersect(r1, r2, &out);

		printf("[%u,%u] & [%u,%u] = ", a, b, c, d);
		if (has)
			printf("{base=%u, size=%u} => u:[%u,%u] s:[%d,%d]\n",
			       out.base, out.size,
			       cnum32_umin(out), cnum32_umax(out),
			       cnum32_smin(out), cnum32_smax(out));
		else
			printf("empty\n");
		return 0;
	} else if (argc > 1) {
		unsigned int tid = strtoul(argv[1], NULL, 16);
		struct cnum8 a = { tid >> 24, tid >> 16 };
		struct cnum8 b = { tid >> 8, tid };

		errors = test_intersect_once(a, b);
	} else {
		init_all_cnums();
		printf("all_cnums: %d entries\n", cnums_cnt);

		errors = test_intersect();
		printf("test_intersect: %d errors\n", errors);
	}

	return errors ? 1 : 0;
}
