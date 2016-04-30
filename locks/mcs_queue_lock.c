/*
 * Implementation of MCS-queue-based-spinlock
 */

#include <mcs_queue_lock.h>
#include <stddef.h>
#include <atomics_x86.h>

void mcs_lock(mcs_lock_t *lock, lock_qnode_t *qnode) {
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

void mcs_unlock(mcs_lock_t *lock, lock_qnode_t *qnode) {
	// check if there is a thread to hand the lock to
	if (!qnode->next) {
		// check if we are the final locker, change state to unlocked
		if (lock_cmpxchg_64(lock, qnode, NULL) == qnode)
			return;

		// synchronize with next thread
		while (!qnode->next);
	}
	
	// release next thread
	qnode->next->wait = 0;
}
