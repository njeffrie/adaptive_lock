#ifndef MCS_HYBRID_LOCK_H
#define MCS_HYBRID_LOCK_H

#include <stddef.h>

typedef volatile struct hybrid_qnode {
	volatile struct hybrid_qnode *next;
	uint64_t ticket;
	unsigned char wait;
	unsigned char valid;
} __attribute__((aligned(64))) hybrid_qnode_t;

typedef struct mcs_hybrid_lock {
	hybrid_qnode_t *tail __attribute__((aligned(64)));
	volatile uint64_t lock_turn __attribute__((aligned(64)));
	volatile uint64_t unlock_turn __attribute__((aligned(64)));
} mcs_hybrid_lock_t;

#define INIT_HYBRID_LOCK (mcs_hybrid_lock_t){NULL, 0, 0}
#define INIT_HYBRID_QNODE (struct hybrid_qnode){NULL, 0, 0, 0}

// threshold for transition between ticketlock and queuelock
#define THRESH 5L

#ifdef __cplusplus
extern "C" {
#endif

static inline void mcs_hybrid_lock(mcs_hybrid_lock_t *lock, hybrid_qnode_t *qnode) {
	*qnode = INIT_HYBRID_QNODE;

	// swap self with most recent locker (new lockers will see this qnode)
	hybrid_qnode_t *prev = __sync_lock_test_and_set(&lock->tail, qnode);

	// if prev is non-NULL, there are preceding lockers
	if (prev) {
		qnode->wait = 1;
		// wait for prev to become valid
		while (!prev->valid);
		qnode->ticket = prev->ticket + 1;
		qnode->valid = 1;
		prev->next = qnode;
	
		// check if should be scaling to queue behavior
		if (qnode->ticket - lock->lock_turn > THRESH) 
			while (qnode->wait);

		// wait for this thread to be made head
		uint64_t turn, cnt;
		while ((turn = lock->lock_turn) != (cnt = qnode->ticket)) {
			// proportional back-off
			for ( ; cnt < turn; cnt++);
		}
	}
	// if prev is NULL, xchg got this thread the lock
	else {
		qnode->ticket = lock->lock_turn;
		qnode->valid = 1;
	}
}

static inline void mcs_hybrid_unlock(mcs_hybrid_lock_t *lock, hybrid_qnode_t *qnode) {
	// release next locking thread
	lock->lock_turn++;

	// wait for any predecessors to finish unlocking completely
	uint64_t turn, cnt;
	while ((turn = lock->unlock_turn) != (cnt = qnode->ticket)) {
		// proportional back-off
		for ( ; cnt < turn; cnt++);
	}

	// check if there is a thread to hand the lock to
	if (!qnode->next) {
		// check if we are the final locker, change state to unlocked
		if (__sync_bool_compare_and_swap(&lock->tail, qnode, NULL)) {
			// release any new unlocking thread
			lock->unlock_turn++;
			return;
		}

		// synchronize with next thread
		while (!qnode->next);
	}

	qnode->next->wait = 0;

	// release next unlocking thread
	lock->unlock_turn++;
}

#ifdef __cplusplus
}
#endif

#endif /* MCS_HYBRID_LOCK_H */
