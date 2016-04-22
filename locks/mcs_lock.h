/*
 * Implementation of MCS-Spinlock using GCC atomic built-ins
 */

#ifndef MCS_LOCK_H
#define MCS_LOCK_H

#include <stddef.h>
#include "atomics_x86.h"

typedef struct lock_qnode {
	struct lock_qnode *next;
	unsigned char wait;
} lock_qnode_t;

typedef lock_qnode_t *mcs_lock_t;

#define INIT_MCS_LOCK (mcs_lock_t)NULL

static inline void mcs_lock(mcs_lock_t *lock, lock_qnode_t *qnode) {
	qnode->next = NULL;

	// swap self with most recent locker (new lockers will see this qnode)
	lock_qnode_t *prev = lock_xchg_64(lock, qnode);
	if (prev) { // if !prev, no locker, take lock just with the atomic
		qnode->wait = 1;
		prev->next = qnode;

		// wait until previous lock holder acknowledges this thread
		while (qnode->wait);
	}
}

static inline void mcs_unlock(mcs_lock_t *lock, lock_qnode_t *qnode) {
	// check if there is a thread to hand the lock to
	if (!qnode->next) {
		// check if we are the final locker, change state to unlocked
		mcs_lock_t last = qnode;
		if (lock_cmpxchg_64(lock, qnode, NULL) == qnode)
			return;

		// synchronize with next thread
		while (!qnode->next);
	}
	
	// release next thread
	qnode->next->wait = 0;
}

#endif /* MCS_LOCK_H */
