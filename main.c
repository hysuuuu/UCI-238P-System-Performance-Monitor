/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * main.c
 */

#include <signal.h>
#include "system.h"

/**
 * Needs:
 *   signal()
 */

static volatile int done;

static void
_signal_(int signum)
{
	assert( SIGINT == signum );

	done = 1;
}

void 
mem_util(void) {

	const char * const MEM_STAT = "/proc/meminfo";
	char line[1024];
	FILE *file; 
	unsigned long mem_total = 0;
	unsigned long mem_free = 0;

	if (!(file = fopen(MEM_STAT, "r"))) {
		TRACE("open meminfo failed");
	}
	
	while (fgets(line, sizeof(line), file)) {
		if (strncmp(line, "MemTotal:", 9) == 0) {
			sscanf(line + 9, "%lu", &mem_total);
		}
		if (strncmp(line, "MemFree:", 8) == 0) {
			sscanf(line + 8, "%lu", &mem_total);
		}
	}

	fclose(file);
	if (mem_total == 0 && mem_free == 0) {
		TRACE("mem scan failed");
	}
	printf(" | Memroy used: %5.1f%%", (double)((mem_total - mem_free) / mem_total) * 100.0) ;
}

void
net_stat() {

	const char * const NET_STAT = "/proc/net/dev";
	char line[1024];
	FILE *file; 

	if (!(file = fopen(NET_STAT, "r"))) {
		TRACE("open net failed");
	}

	
}

double
cpu_util(const char *s)
{
	static unsigned sum_, vector_[7];
	unsigned sum, vector[7];
	const char *p;
	double util;
	uint64_t i;

	/*
	  user
	  nice
	  system
	  idle
	  iowait
	  irq
	  softirq
	*/

	if (!(p = strstr(s, " ")) ||
	    (7 != sscanf(p,
			 "%u %u %u %u %u %u %u",
			 &vector[0],
			 &vector[1],
			 &vector[2],
			 &vector[3],
			 &vector[4],
			 &vector[5],
			 &vector[6]))) {
		return 0;
	}
	sum = 0.0;
	for (i=0; i<ARRAY_SIZE(vector); ++i) {
		sum += vector[i];
	}
	util = (1.0 - (vector[3] - vector_[3]) / (double)(sum - sum_)) * 100.0;
	sum_ = sum;
	for (i=0; i<ARRAY_SIZE(vector); ++i) {
		vector_[i] = vector[i];
	}
	return util;
}

int
main(int argc, char *argv[])
{
	const char * const PROC_STAT = "/proc/stat";
	char line[1024];
	FILE *file;

	UNUSED(argc);
	UNUSED(argv);

	if (SIG_ERR == signal(SIGINT, _signal_)) {
		TRACE("signal()");
		return -1;
	}
	while (!done) {
		if (!(file = fopen(PROC_STAT, "r"))) {
			TRACE("fopen()");
			return -1;
		}
		if (fgets(line, sizeof (line), file)) {
			printf("\r%5.1f%%", cpu_util(line));
			fflush(stdout);
		}
		us_sleep(500000);
		fclose(file);

		mem_util();
		net_stat();
	}
	printf("\rDone!   \n");
	return 0;
}
