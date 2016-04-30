/*
 * tests correctness for read write locks
 */
#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <mcs_queue_lock.h>
#include <mcs_queue_rwlock.h>
#include <mcs_ticket_lock.h>
#include <mcs_ticket_rwlock.h>
#include <assert.h>
#include <time.h>
#include "CycleTimer.h"
#include <omp.h>

#define LOOPS 1000
#define READ_LOOPS 10
#define DELAY 10
#define THREADS 600

int shared = 0;

ticket_rwlock_t t = INIT_TICKET_RWLOCK;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
mcs_rwlock_t mcs = INIT_MCS_RWLOCK;

void wait_cycles(int cycles){
	int temp;
	for (int i=0; i<cycles; i++){
		temp++;
	}
}

void writer(){
	int temp = shared;
	wait_cycles(DELAY);
	shared = temp + 1;
}

int reader(){
	int temp = shared;
	wait_cycles(DELAY);
	return temp;
}

inline void test_func_mutex(){
	for (int i=0; i<LOOPS; i++){
		pthread_mutex_lock(&m);
		writer();
		pthread_mutex_unlock(&m);
		pthread_mutex_lock(&m);
		int val = reader();
		for (int i=0; i<READ_LOOPS; i++){
			assert(reader() == val);
		}
		pthread_mutex_unlock(&m);
	}
}

inline void test_func_ticketrwlock(){
	for (int i=0; i<LOOPS; i++){
		ticket_wlock(&t);	
		writer();
		ticket_wunlock(&t);
		ticket_rlock(&t);
		int val = reader();
		for (int i=0; i<READ_LOOPS; i++){
			assert(reader() == val);
		}
		ticket_runlock(&t);
	}
}

inline void test_func_mcsrwlock(){
	rwlock_qnode_t wnode = INIT_WLOCK_QNODE;
	rwlock_qnode_t rnode = INIT_RLOCK_QNODE;
	for (int i=0; i<LOOPS; i++){
		mcs_wlock(&mcs, &wnode);
		writer();
		mcs_wunlock(&mcs, &wnode);
		mcs_rlock(&mcs, &rnode);
		int val = reader();
		for (int i=0; i<READ_LOOPS; i++){
			assert(reader() == val);
		}
		mcs_runlock(&mcs, &rnode);
	}
}

void launch_threads(void (*fn)(void)){
	shared = 0;
	pthread_t threads[THREADS];
	#pragma omp parallel for
	for (int i=0; i<THREADS; i++){
		fn();
	}
	if(shared != THREADS*LOOPS) printf("locking failed\n");
}

int main(int argc, char *argv[]){
	double start = CycleTimer::currentSeconds();
	launch_threads(test_func_mutex);
	double dt1 = CycleTimer::currentSeconds() - start;
	start = CycleTimer::currentSeconds();
	launch_threads(test_func_ticketrwlock);
	double dt2 = CycleTimer::currentSeconds() - start;
	start = CycleTimer::currentSeconds();
	launch_threads(test_func_mcsrwlock);
	double dt3 = CycleTimer::currentSeconds() - start;
	
	printf("rwlock: %f\n", dt2 / dt1);
	printf("mcs rwlock: %f\n", dt3 / dt1);
	return 0;
}
