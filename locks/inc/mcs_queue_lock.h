#ifndef MCS_QUEUE_LOCK_H
#define MCS_QUEUE_LOCK_H

#include <stddef.h>
#include <atomics_x86.h>

struct lock_qnode {
	volatile struct lock_qnode *next;
	unsigned char wait;
} __attribute__((aligned(64)));

typedef volatile struct lock_qnode lock_qnode_t;
typedef lock_qnode_t * mcs_lock_t;

#define INIT_MCS_LOCK (mcs_lock_t)NULL

#ifdef __cplusplus
extern "C" {
#endif

static inline void mcs_lock(mcs_lock_t *lock, lock_qnode_t *qnode) {
	qnode->next = NULL;

	// swap self with most recent locker (new lockers will see this qnode)
	//lock_qnode_t *prev = lock_xchg_64(lock, qnode);
	lock_qnode_t *prev = __sync_lock_test_and_set(lock, qnode);
	if (prev) { // if !prev, no locker, take lock just with the atomic
		qnode->wait = 1;
		prev->next = qnode;

		// wait until previous lock holder acknowledges this thread
		while (qnode->wait);
	}
}

static inline void mcs_unlock(mcs_lock_t *lock, lock_qnode_t *qnode) {
	// TSO so no need for full barrier, prevent load reordering
	_mm_lfence();

	// check if there is a thread to hand the lock to
	if (!qnode->next) {
		// check if we are the final locker, change state to unlocked
		//if (lock_cmpxchg_64(lock, qnode, NULL) == qnode)
		if (__sync_bool_compare_and_swap(lock, qnode, NULL))
			return;

		// synchronize with next thread
		while (!qnode->next);
	}
	
	// release next thread
	qnode->next->wait = 0;
}

#ifdef __cplusplus
}
#endif

#endif /* MCS_QUEUE_LOCK_H */
