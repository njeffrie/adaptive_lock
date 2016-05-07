#include <stdio.h>
#include <stdlib.h>
#include "CycleTimer.h"

#define THREADS 60
#define ITERS 5000000

volatile int shared = 1;


void test_loop_xadd(){
	for (int i=0; i<ITERS; i++){
		__sync_fetch_and_add(&shared, 1);
	}
}

void test_loop_xchg(){
	for (int i=0; i<ITERS; i++){
		__sync_lock_test_and_set(&shared, 1);
	}
}

void test_loop_cmpxchg_success(){
	for (int i=0; i<ITERS; i++){
		__sync_bool_compare_and_swap(&shared, 1, 1);	
	}
}

void test_loop_cmpxchg_failure(){
	for (int i=0; i<ITERS; i++){
		__sync_bool_compare_and_swap(&shared, 1, 1);
	}
}

//n readers
void test_readers(){
	int accum = 0;
	for (int i=0; i<ITERS; i++){
		accum += shared;
	}
}

//n-1 readers and 1 writer
void test_invalidations(int thread){
	if (thread){
		int accum = 0;
		for (int i=0; i<ITERS; i++){
			accum += shared;
		}
	}
	else{
		for (int i=0; i<ITERS; i++){
			shared = i;
		}
	}
}

double launch_threads(void (*fn)(void)){
	shared = 0;
	double start = CycleTimer::currentSeconds();
	#pragma omp parallel for num_threads(THREADS)
	for (int i=0; i<THREADS; i++){
		fn();
	}
	return CycleTimer::currentSeconds() - start;
}

void run_testes(){
	printf("==================================================\n");
	printf("running tests with %d threads\n", THREADS);
	double dt1, dt2, dt3, dt4, dt5, dt6;
	dt1 = launch_threads(test_loop_xchg);
	dt1 = launch_threads(test_loop_xchg);
	dt2 = launch_threads(test_loop_xadd);
	dt3 = launch_threads(test_loop_cmpxchg_success);
	dt4 = launch_threads(test_loop_cmpxchg_failure);
	printf("Total Elapsed: %.4f ms, results:\n", (dt1 + dt2 + dt3 + dt4));
	printf("xchg: %.4fs\n", dt1);
	printf("xadd: %.4f (%.4fs) ", dt2 / dt1, dt2);
	printf("cmpxchg_success: %.4f (%.4fs) ", dt3 / dt1, dt3);
	printf("cmpxchg_failure: %.4f (%.4fs) ", dt4 / dt1, dt4);
}

int main(int argc, char *argv[]){
	#pragma offload target(mic)
	{
			
		#pragma omp parallel for num_threads(THREADS)
		for (int i=0; i<THREADS*100; i++){
			test_readers();
		}
		double start = CycleTimer::currentSeconds();
		#pragma omp parallel for num_threads(THREADS)
		for (int i=0; i<THREADS*100; i++){
			test_readers();
		}
		double dt1 = CycleTimer::currentSeconds() - start;
		start = CycleTimer::currentSeconds();
		#pragma omp parallel for num_threads(THREADS)
		for (int i=0; i<THREADS*100; i++){
			test_invalidations(i);
		}
		double dt2 = CycleTimer::currentSeconds() - start;
		start = CycleTimer::currentSeconds();
		#pragma omp parallel for num_threads(THREADS)
		for (int i=0; i<THREADS*100; i++){
			test_invalidations(i);
		}
		dt2 += CycleTimer::currentSeconds() - start;
		start = CycleTimer::currentSeconds();
		#pragma omp parallel for num_threads(THREADS)
		for (int i=0; i<THREADS*100; i++){
			test_readers();
		}
		dt1 += CycleTimer::currentSeconds() - start;
		printf("invalidations are %fx slower than reads with %d threads\n", dt2/dt1, THREADS);	
		printf("total time:%f\n", dt1 + dt2);
		//yes... testes.
		//run_testes();
	}
	return 0;
}
