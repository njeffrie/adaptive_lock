/*
 * tests correctness for locks
 */

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <mcs_queue_lock.h>
#include <mcs_ticket_lock.h>
#include <tts_lock.h>
#include <assert.h>
#include <time.h>
#include "CycleTimer.h"
#include <omp.h>

#define LOOPS 10
#define DELAY_LOOP 10
#define THREADS 2

using namespace std;

int shared;

ticketlock_t l = INIT_TICKETLOCK;
mcs_lock_t mcs = INIT_MCS_LOCK;
ttslock_t tts = INIT_TTSLOCK;  

inline void test_fn(){
	int var = shared;
	int temp = 0;
	for (int j=0; j<DELAY_LOOP; j++){
		temp++;
	}
	shared = var + 1;
}

void *test_func_tts(void *arg){
	for (int i=0; i<LOOPS; i++){
		tts_lock(&tts);
		test_fn();
		tts_unlock(&tts);
	}
	return arg;
}

void *test_func_critical(void *arg){
	for (int i=0; i<LOOPS; i++){
		#pragma omp critical 
		{
			test_fn();
		}
	}
	return arg;
}

void *test_func_ticketlock(void *arg){
	for (int i=0; i<LOOPS; i++){
		ticket_lock(&l);
		test_fn();
		ticket_unlock(&l);
	}
	return arg;
}

void *test_func_mcslock(void *arg){
	for (int i=0; i<LOOPS; i++){
		lock_qnode_t node;
		mcs_lock(&mcs, &node);
		test_fn();
		mcs_unlock(&mcs, &node);
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
		//fn(NULL);
	}
	for (int i=0; i<THREADS; i++){
		pthread_join(threads[i], NULL);
		//fn(NULL);
	}
#else
	#pragma omp parallel for num_threads(THREADS)
	for (int i=0; i<THREADS; i++){
		fn(NULL);
	}

#endif
	if(shared != THREADS*LOOPS) printf("locking failed\n");
	return CycleTimer::currentSeconds() - start;
}

int main(int argc, char *argv[]){
	double dt1, dt2, dt3, dt4;
	dt1 = launch_threads(test_func_tts);
	printf("finished tts\n");
	dt2 = launch_threads(test_func_ticketlock);
	printf("finished ticketlock\n");
	dt3 = launch_threads(test_func_mcslock);
	printf("finished mcslock\n");
	dt4 = launch_threads(test_func_critical);
	printf("finished tts\n");
	printf("ticket lock: %f\n", dt2 / dt1);
	printf("mcs lock: %f\n", dt3 / dt1);
	printf("critical: %f\n", dt4 / dt1);
	return 0;
}
