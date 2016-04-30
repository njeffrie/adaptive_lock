/*
 * tests correctness for locks
 */

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <mcs_queue_lock.h>
#include <mcs_ticket_lock.h>
#include <assert.h>
#include <time.h>
#include "CycleTimer.h"
#include <omp.h>

#define LOOPS 10000
#define DELAY_LOOP 10
#define THREADS 600

using namespace std;

int testvar1 = 0;
int testvar2 = 0;

ticketlock_t l = INIT_TICKETLOCK;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
mcs_lock_t mcs = INIT_MCS_LOCK;

void *test_func_mutex(void *arg){
	for (int i=0; i<LOOPS; i++){
		pthread_mutex_lock(&m);
		int var = testvar1;
		int temp = 0;
		for (int j=0; j<DELAY_LOOP; j++){
			temp++;
		}
		testvar1 = var + 1;
		pthread_mutex_unlock(&m);
	}
	return arg;
}

void *test_func_critical(void *arg){
	for (int i=0; i<LOOPS; i++){
		#pragma omp critical 
		{
			int var = testvar1;
			int temp = 0;
			for (int j=0; j<DELAY_LOOP; j++){
				temp++;
			}
			testvar1 = var + 1;
		}
	}
	return arg;
}

void *test_func_ticketlock(void *arg){
	for (int i=0; i<LOOPS; i++){
		ticket_lock(&l);
		int var = testvar1;
		int temp = 0;
		for (int j=0; j<DELAY_LOOP; j++){
			temp++;
		}
		testvar1 = var + 1;
		ticket_unlock(&l);
	}
	return arg;
}

void *test_func_mcslock(void *arg){
	for (int i=0; i<LOOPS; i++){
		lock_qnode_t node;
		mcs_lock(&mcs, &node);
		int var = testvar1;
		int temp = 0;
		for (int j=0; j<DELAY_LOOP; j++){
			temp++;
		}
		testvar1 = var + 1;
		mcs_unlock(&mcs, &node);
	}
	return arg;
}

void launch_threads(void *(*fn)(void *)){
	testvar1 = 0;
	pthread_t threads[THREADS];
	#pragma omp parallel for
	for (int i=0; i<THREADS; i++){
		fn(NULL);
	}
	if(testvar1 != THREADS*LOOPS) printf("locking failed\n");
}

int main(int argc, char *argv[]){
	double start = CycleTimer::currentSeconds();
	launch_threads(test_func_mutex);
	double dt1 = CycleTimer::currentSeconds() - start;
	printf("finished mutex\n");
	/*start = CycleTimer::currentSeconds();
	launch_threads(test_func_ticketlock);
	double dt2 = CycleTimer::currentSeconds() - start;
	printf("finished ticketlock\n");
	start = CycleTimer::currentSeconds();
	launch_threads(test_func_mcslock);
	double dt3 = CycleTimer::currentSeconds() - start;
	printf("finished mcslock\n");
	*/start = CycleTimer::currentSeconds();
	launch_threads(test_func_critical);
	double dt4 = CycleTimer::currentSeconds() - start;
	printf("finished critical\n");
	//printf("ticket lock: %f\n", dt2 / dt1);
	//printf("mcs lock: %f\n", dt3 / dt1);
	printf("critical: %f\n", dt4 / dt1);
	return 0;
}
