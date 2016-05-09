/*
 * tests lock performance over a varying workload
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

#define LOOPS 1000
#define DELAY_LOOP 100
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
		busy_wait(DELAY_LOOP, j);\
		shared = var + 1;\
	} while (0)

#define TEST_SETUP(fname, local_t, lock, unlock, l)\
void fname(int cont){\
	int j;\
	for (int i=0; i<LOOPS; i++){\
		local_t loc;\
		lock(&l, &loc);\
		TEST_INTERNAL;\
		unlock(&l, &loc);\
		int j;\
		busy_wait(cont, j);\
	}\
}

#define TEST_NOSETUP(fname, lock, unlock, l)\
void fname(int cont){\
	int j;\
	for (int i=0; i<LOOPS; i++){\
		lock(&l);\
		TEST_INTERNAL;\
		unlock(&l);\
		busy_wait(cont, j);\
	}\
}

TEST_NOSETUP(test_func_tts, tts_lock, tts_unlock, tts)
TEST_NOSETUP(test_func_ticketlock, ticket_lock, ticket_unlock, l)
TEST_SETUP(test_func_mcslock, lock_qnode_t, mcs_lock, mcs_unlock, mcs)
TEST_SETUP(test_func_mcshybridlock, hybrid_qnode_t, mcs_hybrid_lock, mcs_hybrid_unlock, mcshybrid);

//uses omp critical
void test_func_critical(int cont){
	int j;
	for (int i=0; i<LOOPS; i++){
		#pragma omp critical 
		{
			TEST_INTERNAL;
		}
		busy_wait(cont, j);
	}
}

double launch_threads(void (*fn)(int), int threads, int cont){
	shared = 0;
	double start = CycleTimer::currentSeconds();
	#pragma omp parallel for num_threads(threads)
	for (int i=0; i<threads*4; i++){
		fn(cont);
	}
	double dt = CycleTimer::currentSeconds() - start;

	if(shared != threads*LOOPS*4) {
		printf("locking failed\n");
		printf("observed:%d/%d\n", shared, 4*threads*LOOPS);
	}
	return dt;
}

#define MIN 800
#define MAX 1600
//#define CSV
int threadcounts[] = {1, 2, 4, 8, 16, 32, 48, 59, 64, 128, 192, 236};
void run_testes(){
	double start = CycleTimer::currentSeconds();
#ifdef CSV
	printf("Threads,tts,ticket,mcs,hybrid,critical\n");
#else
	printf("Threads\ttts\t\t\tticket\tmcs\t\t\thybrid\t\tcritical\n");
#endif
	//for (int i=0; i<sizeof(threadcounts)/sizeof(int); i++){
	int offset = 1;
	int cont = 10;
	int threads = 4;
	for (int threads = 3; threads < 7; threads++){
	//int threads = threadcounts[i];
		//printf("running tests with %d threads\n", threads);
		double dt1=0;
		double dt2=0;
		double dt3=0;
		double dt4=0;
		double dt5=0;
		
		launch_threads(test_func_tts, threads, cont);
		int dc=200;
		for (int i=0; i<50; i++){
			for (cont=MIN; cont<=MAX && cont >= MIN; cont+=dc){
				//printf("launched tts\n");
				dt1 += launch_threads(test_func_tts, threads, cont);
				//printf("launched ticket\n");
				dt2 += launch_threads(test_func_ticketlock, threads, cont);
				//printf("launched mcs\n");
				dt3 += launch_threads(test_func_mcslock, threads, cont);
				//printf("launched mcs hybrid\n");
				dt4 += launch_threads(test_func_mcshybridlock, threads, cont);
				//printf("launched critical\n");
				dt5 += launch_threads(test_func_critical, threads, cont);
			}
			dc = -1 * dc;
		}
#ifdef CSV
		printf("%4d,%2.3f,%2.3f,%2.3f,%2.3f,%2.3f\n", threads, dt1, dt2, dt3, dt4, dt5);
		/*if (threads / 4 == offset * offset)
				offset *= 2;
				*/
	}
#else
		printf("%4d\t\t%2.3f\t\t%2.3f\t\t%2.3f\t\t%2.3f\t\t%2.3f\n", threads, dt1, dt2, dt3, dt4, dt5);
		/*
		if (threads / 4 == offset * offset)
				offset *= 2;*/
	}
	printf("total runtime: %.4f\n", CycleTimer::currentSeconds() - start);
#endif
}

int main(int argc, char *argv[]){
	#pragma offload target(mic)
	{
		//yes... testes.
		run_testes();
	}
	return 0;
}
