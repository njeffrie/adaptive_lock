/*
 * tests correctness for locks
 */

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <mcs_queue_lock.h>
#include <mcs_ticket_lock.h>
#include <mcs_hybrid_lock.h>
#include <tts_lock.h>
#include <assert.h>
#include <time.h>
#include "CycleTimer.h"
#include <omp.h>

//#define PTHREAD

#define LOOPS 1000
#define DELAY_LOOP 0
#define THREADS 59

using namespace std;

int shared;

ticketlock_t l = INIT_TICKETLOCK;
mcs_lock_t mcs = INIT_MCS_LOCK;
ttslock_t tts = INIT_TTSLOCK;  
mcs_hybrid_lock_t mcshybrid = INIT_HYBRID_LOCK;

inline void delay(int cycles){
	int temp = 0;
	for (int i=0; i<cycles; i++) temp++;
}

#define TEST_INTERNAL\
	do { \
		int var = shared;\
		/*delay(DELAY_LOOP);*/int i;\
		busy_wait(DELAY_LOOP, i);\
		shared = var + 1;\
	} while (0)

#define TEST_SETUP(fname, local_t, lock, unlock, l)\
void *fname(void *args){\
	for (int i=0; i<LOOPS; i++){\
		local_t loc;\
		lock(&l, &loc);\
		TEST_INTERNAL;\
		unlock(&l, &loc);\
		int j;\
		busy_wait(DELAY_LOOP, j);\
	}\
	return args;\
}

#define TEST_NOSETUP(fname, lock, unlock, l)\
void *fname(void *args){\
	for (int i=0; i<LOOPS; i++){\
		lock(&l);\
		TEST_INTERNAL;\
		unlock(&l);\
	}\
	return args;\
}

TEST_NOSETUP(test_func_tts, tts_lock, tts_unlock, tts)
TEST_NOSETUP(test_func_ticketlock, ticket_lock, ticket_unlock, l)
TEST_SETUP(test_func_mcslock, lock_qnode_t, mcs_lock, mcs_unlock, mcs)
TEST_SETUP(test_func_mcshybridlock, hybrid_qnode_t, mcs_hybrid_lock, mcs_hybrid_unlock, mcshybrid);

//uses omp critical
void *test_func_critical(void *arg){
	for (int i=0; i<LOOPS; i++){
		#pragma omp critical 
		{
			TEST_INTERNAL;
		}
	}
	return arg;
}

double launch_threads(void *(*fn)(void *), int threads){
	shared = 0;
	double start = CycleTimer::currentSeconds();
	#pragma omp parallel for num_threads(threads)
	for (int i=0; i<threads*4; i++){
		fn(NULL);
	}
	double dt = CycleTimer::currentSeconds() - start;

	if(shared != threads*LOOPS*4) {
		printf("locking failed\n");
		printf("observed:%d/%d\n", shared, 4*threads*LOOPS);
	}
	return dt;
}

int threadcounts[] = {1, 2, 4, 8, 16, 32, 48, 59};
void run_testes(){
	double start = CycleTimer::currentSeconds();
	printf("Threads\ttts\t\t\tticket\tmcs\t\t\thybrid\t(all relative to critical)\n");
	for (int i=0; i<sizeof(threadcounts)/sizeof(int); i++){
		int threads = threadcounts[i];
		//printf("running tests with %d threads\n", threads);
		double dt1, dt2, dt3, dt4, dt5;
		//printf("launched tts\n");
		dt1 = launch_threads(test_func_tts, threads);
		dt1 = launch_threads(test_func_tts, threads);
		//printf("launched ticket\n");
		dt2 = launch_threads(test_func_ticketlock, threads);
		//printf("launched mcs\n");
		dt3 = launch_threads(test_func_mcslock, threads);
		//printf("launched mcs hybrid\n");
		dt4 = launch_threads(test_func_mcshybridlock, threads);
		//printf("launched critical\n");
		dt5 = launch_threads(test_func_critical, threads);
		printf("%4d\t\t", threads);
		printf("%1.3f\t\t", dt1 / dt5);
		printf("%1.3f\t\t", dt2 / dt5);
		printf("%1.3f\t\t", dt3 / dt5);
		printf("%1.3f\n", dt4 / dt5);
	}
	printf("total runtime: %.4f\n", CycleTimer::currentSeconds() - start);
}

int main(int argc, char *argv[]){
	#pragma offload target(mic)
	{
		//yes... testes.
		run_testes();
	}
	return 0;
}
