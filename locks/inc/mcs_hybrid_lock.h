/**
 * @file mcs_hybrid_lock.h
 * @brief Hybrid of MCS queue spinlock and MCS ticket spinlock
 *
 * Spinlock based on MCS queue-based spinlock and MCS ticket spinlock.
 * The objective of this implementation is to observe the lower latencies
 * of the ticket lock in situations of lower contention while transitioning
 * to the queue lock behavior in situations of higher contention in order
 * to achieve good scaling. This is done by altering the standard queue lock
 * implementation such that under low contention, the overhead of the 
 * additional atomic operation on unlocking is removed from the critical path.
 *
 * @author Madhav Iyengar
 */

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
	volatile uint64_t turn __attribute__((aligned(64)));
} mcs_hybrid_lock_t;

#define INIT_HYBRID_LOCK (mcs_hybrid_lock_t){NULL, 0}
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
		if (qnode->ticket - lock->turn > THRESH) 
			while (qnode->wait);

		// wait for this thread to be made head
		uint64_t cycles;
		volatile uint64_t cnt;
		while ((cycles = qnode->ticket - lock->turn)) {
			// proportional back-off
			for (cnt = 0 ; cnt < cycles; cnt++);
		}
	}
	// if prev is NULL, xchg got this thread the lock
	else {
		qnode->ticket = lock->turn;
		qnode->valid = 1;
	}
}

static inline void mcs_hybrid_unlock(mcs_hybrid_lock_t *lock, hybrid_qnode_t *qnode) {
	// prevent load reordering, no load fence on xeon phi
	__sync_synchronize();

	// release next locking thread
	lock->turn++;

	// wait for predecessor if bypassed queue wait on locking
	while (qnode->wait);

	// check if there is a thread to hand the lock to
	if (!qnode->next) {
		// check if we are the final locker, change state to unlocked
		if (__sync_bool_compare_and_swap(&lock->tail, qnode, NULL))
			return;

		// synchronize with next thread
		while (!qnode->next);
	}

	// completely release next thread
	qnode->next->wait = 0;
}

#ifdef __cplusplus
}
#endif

#endif /* MCS_HYBRID_LOCK_H */
