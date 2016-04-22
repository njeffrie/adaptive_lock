/*
 * Implementation of MCS-RWlock using GCC atomic built-ins
 */

#ifndef MCS_RWLOCK_H
#define MCS_RWLOCK_H

#include <stddef.h>
#include "atomics_x86.h"

typedef enum rwlock_type {
	RWLOCK_READ,
	RWLOCK_WRITE,
	RWLOCK_NIL
} rwlock_type_t;

typedef union __attribute__((packed)) rwlock_state {
	uint64_t bits;
	struct __attribute((packed)) state {
		uint32_t spinning;
		rwlock_type_t next_type;
	} state;
} rwlock_state_t;

typedef struct rwlock_qnode {
	struct rwlock_qnode *next;
	rwlock_state_t state;
	rwlock_type_t type;
} rwlock_qnode_t;

typedef struct mcs_rwlock {
	rwlock_qnode_t *tail;
	rwlock_qnode_t *next_wr;
	uint64_t num_read;
} mcs_rwlock_t;

#define STATE_WAIT_NIL ((RWLOCK_NIL << 32) | 1)
#define STATE_WAIT_READ ((RWLOCK_READ << 32) | 1)
#define STATE_WAIT_WRITE ((RWLOCK_WRITE << 32) | 1)
#define STATE_NOWAIT_NIL ((RWLOCK_NIL << 32) | 0)
#define STATE_NOWAIT_READ ((RWLOCK_READ << 32) | 0)
#define STATE_NOWAIT_WRITE ((RWLOCK_WRITE << 32) | 0)
#define STATE(next_type, wait) (((next_type) << 32) | (wait))

#define INIT_MCS_RWLOCK (mcs_rwlock_t){NULL, NULL, 0}
#define INIT_RLOCK_QNODE \
	(rwlock_qnode_t){NULL, STATE_WAIT_NIL, RWLOCK_READ}
#define INIT_WLOCK_QNODE \
	(rwlock_qnode_t){NULL, STATE_WAIT_NIL, RWLOCK_WRITE}

static inline void mcs_rlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	*qnode = INIT_RLOCK_QNODE;

	// set self as next locker (and try to take lock)
	rwlock_qnode_t *prev = lock_xchg_64(lock->tail, qnode);

	// if no previous locker
	if (!prev) {
		lock_xadd_64(&lock->num_read, 1);
		qnode->state.state.spinning = 0;
	}
	// there is a previous locker
	else {
		if ((prev->type == RWLOCK_WRITE) ||
				(lock_cmpxchg_64(&prev->state.bits, STATE_WAIT_NIL, STATE_WAIT_READ) 
					== STATE_WAIT_NIL)) {
			// if predecessor is a writer || is waiting reader, wait to be released
			prev->next = qnode;
			while (qnode->state.state.spinning);
		}
		else {
			// otherwise, got read lock, increment count
			lock_xadd_64(&lock->num_read, 1);
			prev->next = qnode;
			qnode->state.state.spinning = 0;
		}
	}

	// check for next thread, if reader, allow to proceed
	if (qnode->state.state.next_type == RWLOCK_READ) {
		while (!qnode->next);
		lock_xadd_64(&lock->num_read, 1);
		qnode->next->state.state.spinning = 0;
	}
}

static inline void mcs_runlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	// check for next thread
	if (qnode->next || (lock_cmpxchg_64(lock, qnode, NULL) == qnode)) {
		// wait for next thread to identify itself
		while (!qnode->next);
		// if next is writer, set it as the 
		if (qnode->state.state.next_type == RWLOCK_WRITE)
			lock->next_wr = qnode->next;
	}

	// if last reader, wake any writer if it exists
	rwlock_qnode_t *writer;
	if ((lock_xadd_64(&lock->num_read, -1) == 1) && 
			(writer = lock->next_wr) && !lock->num_read && 
			(lock_cmpxchg_64(&lock->next_wr, writer, NULL) == writer))
		writer->state.state.spinning = 0;
}

static inline void mcs_wlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	*qnode = INIT_WLOCK_QNODE;

	// get previous thread (and try to set as owner)
	rwlock_qnode_t *prev = lock_xchg_64(lock, qnode);

	// check prev predecessor
	if (prev) 
		// identify self to predecessor and allow it to continue
		prev->state.bits = STATE_NOWAIT_WRITE;
	else {
		// otherwise, this is the next writer, wait for all readers to leave
		lock->next_wr = qnode;
		if (!lock->num_read && (lock_xchg_64(&lock->next_wr, NULL) == qnode))
			qnode->state.state.spinning = 0;
	}

	// wait for predecessor or last reader to wake up this thread
	while (qnode->state.state.spinning);
}

static inline void mcs_wunlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	// check (and wait) for successor
	if (qnode->next || !(lock_cmpxchg_64(lock, qnode, NULL) == qnode)) {
		while (qnode->next);

		// increment read count if next is a reader
		if (qnode->next->type == RWLOCK_READ)
			lock_xadd_64(&lock->num_read, 1);
	
		// acknowledge next thread
		next->state.state.spinning = 0;
	}
}

#endif /* MCS_RWLOCK_H */
