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
#include <CycleTimer.h>
#include <omp.h>

//#define PTHREAD

#define LOOPS 10000
#define DELAY_LOOP 10
#define THREADS 10

using namespace std;

int shared;
int first_access[THREADS] = {-1};

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
void fname(int threadId){\
	for (int i=0; i<LOOPS; i++){\
		local_t loc;\
		lock(&l, &loc);\
		if (first_access[threadId] == -1) first_access[threadId] = shared;\
		TEST_INTERNAL;\
		unlock(&l, &loc);\
	}\
}

#define TEST_NOSETUP(fname, lock, unlock, l)\
void fname(int threadId){\
	for (int i=0; i<LOOPS; i++){\
		lock(&l);\
		if (first_access[threadId] == -1) first_access[threadId] = shared;\
		TEST_INTERNAL;\
		unlock(&l);\
	}\
}

TEST_NOSETUP(test_func_tts, tts_lock, tts_unlock, tts)
TEST_NOSETUP(test_func_ticketlock, ticket_lock, ticket_unlock, l)
TEST_SETUP(test_func_mcslock, lock_qnode_t, mcs_lock, mcs_unlock, mcs)
TEST_SETUP(test_func_mcshybridlock, hybrid_qnode_t, mcs_hybrid_lock, mcs_hybrid_unlock, mcshybrid);

//uses omp critical
void test_func_critical(int threadId){
	for (int i=0; i<LOOPS; i++){
		#pragma omp critical 
		{
			if (first_access[threadId] == -1) first_access[threadId] = shared;\
			TEST_INTERNAL;
		}
	}
}

void print_first_accesses(){
	printf("first accesses: [");
	for (int i=0; i<THREADS; i++){
		printf("%d ", first_access[i]);
	}
	printf("]\n");
	for (int i=0; i<THREADS; i++){
		first_access[i] = -1;
	}
}

double launch_threads(void (*fn)(int)){
	shared = 0;
	double start = CycleTimer::currentSeconds();
	#pragma omp parallel for num_threads(THREADS)
	for (int i=0; i<THREADS; i++){
		fn(i);
	}
	double dt = CycleTimer::currentSeconds() - start;

	if(shared != THREADS*LOOPS) {
		printf("locking failed\n");
		printf("observed:%d/%d\n", shared, THREADS*LOOPS);
	}
	print_first_accesses();
	return dt;
}

int threadcounts[] = {2, 4, 8, 12, 20, 30, 40, 50, 60};
void run_testes(){
	printf("==================================================\n");
	printf("running tests with %d threads\n", THREADS);
	double dt1, dt2, dt3, dt4, dt5;
	print_first_accesses();
	printf("launched tts\n");
	dt1 = launch_threads(test_func_tts);
	dt1 = launch_threads(test_func_tts);
	printf("launched ticket\n");
	dt2 = launch_threads(test_func_ticketlock);
	printf("launched mcs\n");
	dt3 = launch_threads(test_func_mcslock);
	printf("launched mcs hybrid\n");
	dt4 = launch_threads(test_func_mcshybridlock);
	printf("launched critical\n");
	dt5 = launch_threads(test_func_critical);
	printf("Total Elapsed: %.4fs, results:\n", (dt1 + dt2 + dt3 + dt4 + dt5));
	printf("tts lock: %.4fs ", dt1);
	printf("ticket lock: %.4f (%.4fs) ", dt2 / dt1, dt2);
	printf("mcs lock: %.4f (%.4fs) ", dt3 / dt1, dt3);
	printf("mcs hybrid lock: %.4f (%.4fs) ", dt4 / dt1, dt4);
	printf("critical: %.4f (%.4fs)\n", dt5 / dt1, dt5);
	printf("==================================================\n\n");
}

int main(int argc, char *argv[]){
	#pragma offload target(mic)
	{
		//yes... testes.
		run_testes();
	}
	return 0;
}
