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

#define LOOPS 100000
#define DELAY_LOOP 10
#define THREADS 60

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
	int var = shared;\
	delay(DELAY_LOOP);\
	shared = var + 1\

#define TEST_SETUP(fname, local_t, lock, unlock, l)\
void *fname(void *args){\
	for (int i=0; i<LOOPS; i++){\
		local_t loc;\
		lock(&l, &loc);\
		TEST_INTERNAL;\
		unlock(&l, &loc);\
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

double launch_threads(void *(*fn)(void *)){
	shared = 0;
	double start = CycleTimer::currentSeconds();
#ifdef PTHREAD
	pthread_t threads[THREADS];
	for (int i=0; i<THREADS; i++){
		pthread_create(&threads[i], NULL, fn, NULL);
	}
	for (int i=0; i<THREADS; i++){
		pthread_join(threads[i], NULL);
	}
#else
	#pragma omp parallel for num_threads(THREADS)
	for (int i=0; i<THREADS; i++){
		fn(NULL);
	}

#endif
	if(shared != THREADS*LOOPS) {
		printf("locking failed\n");
		printf("observed:%d/%d\n", shared, THREADS*LOOPS);
	}
	return CycleTimer::currentSeconds() - start;
}

void run_testes(){
	printf("==================================================\n");
	double dt1, dt2, dt3, dt4, dt5;
	printf("launched tts\n");
	dt1 = launch_threads(test_func_tts);
	printf("launched ticket\n");
	dt2 = launch_threads(test_func_ticketlock);
	printf("launched mcs\n");
	dt3 = launch_threads(test_func_mcslock);
	printf("launched mcs hybrid\n");
	dt4 = launch_threads(test_func_mcshybridlock);
	printf("launched critical\n");
	dt5 = launch_threads(test_func_critical);
	printf("Total Elapsed: %.4f ms, results:\n", (dt1 + dt2 + dt3 + dt4 + dt5));
	printf("ticket lock: %.4f (%.4f) ", dt2 / dt1, dt2);
	printf("mcs lock: %.4f (%.4f) ", dt3 / dt1, dt3);
	printf("mcs hybrid lock: %.4f (%.4f) ", dt4 / dt1, dt4);
	printf("critical: %.4f (%.4f)\n", dt5 / dt1, dt5);
	printf("==================================================\n\n");
}

int main(int argc, char *argv[]){
	#pragma offload optional target(mic)
	{
		//yes... testes.
		run_testes();
	}
	return 0;
}
