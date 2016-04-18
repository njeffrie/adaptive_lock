/*
 * Implementation of MCS-Spinlock using GCC atomic built-ins
 */

typedef struct lock_qnode {
	struct lock_qnode *next;
	unsigned char wait;
} lock_qnode_t;

typedef lock_qnode_t *mcs_lock_t;

#define INIT_MCS_LOCK (mcs_lock_t)NULL

static inline void mcs_lock(mcs_lock_t *lock, lock_qnode_t *qnode) {
	qnode->next = NULL;

	// swap self with most recent locker (new lockers will see this qnode)
	lock_qnode_t *prev = __atomic_exchange_n(lock, qnode, __ATOMIC_ACQ_REL);
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
		if (__atomic_compare_exchange_n(lock, &last, NULL, false, 
				__ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)
			return;

		// synchronize with next thread
		while (!qnode->next);
	}
	
	// release next thread
	qnode->next->wait = 0;
}
