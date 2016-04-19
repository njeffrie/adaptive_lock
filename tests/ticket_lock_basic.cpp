/*
 * tests correctness for locks
 */

#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include "../locks/mcs_lock.h"
//#include "../locks/mcs_rwlock.h"
#include "../locks/ticket_lock.h"
//#include "../locks/ticket_rwlock.h"
#include <assert.h>
#include <time.h>
#include "CycleTimer.h"

using namespace std;

int testvar1 = 0;
int testvar2 = 0;

ticketlock_t l = INIT_TICKETLOCK;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
mcs_lock_t mcs = INIT_MCS_LOCK;

void *test_func_mutex(void *arg){
	for (int i=0; i<1600; i++){
		pthread_mutex_lock(&m);
		int var = testvar1;
		int temp = 0;
		for (int i=0; i<1000; i++){
			temp++;
		}
		if (!(i % 100)) { printf("."); fflush(stdout);}
		testvar1 = var + 1;
		pthread_mutex_unlock(&m);
	}
	return arg;
}

void *test_func_ticketlock(void *arg){
	for (int i=0; i<1600; i++){
		ticket_lock(&l);
		int var = testvar1;
		int temp = 0;
		for (int i=0; i<1000; i++){
			temp++;
		}
		if (!(i % 100)) { printf("."); fflush(stdout);}
		testvar1 = var + 1;
		ticket_unlock(&l);
	}
	return arg;
}

void *test_func_mcslock(void *arg){
	for (int i=0; i<1600; i++){
		lock_qnode_t node;
		mcs_lock(&mcs, &node);
		int var = testvar1;
		int temp = 0;
		for (int i=0; i<1000; i++){
			temp++;
		}
		if (!(i % 100)) { printf("."); fflush(stdout);}
		testvar1 = var + 1;
		mcs_unlock(&mcs, &node);
	}
	return arg;
}

void launch_threads(void *(*fn)(void *)){
	testvar1 = 0;
	pthread_t threads[10];
	for (int i=0; i<10; i++){
		pthread_create(&threads[i], NULL, fn, NULL);
	}
	for (int i=0; i<10; i++){
		pthread_join(threads[i], NULL);
	}
	if(testvar1 != 16000) printf("locking failed\n");
}

int main(int argc, char *argv[]){
	double start = CycleTimer::currentSeconds();
	launch_threads(test_func_mutex);
	double dt1 = CycleTimer::currentSeconds() - start;
	start = CycleTimer::currentSeconds();
	launch_threads(test_func_ticketlock);
	double dt2 = CycleTimer::currentSeconds() - start;
	start = CycleTimer::currentSeconds();
	launch_threads(test_func_mcslock);
	double dt3 = CycleTimer::currentSeconds() - start;
	printf("\nticket lock: %f\n", dt2 / dt1);
	printf("mcs lock: %f\n", dt3 / dt1);
	return 0;
}
