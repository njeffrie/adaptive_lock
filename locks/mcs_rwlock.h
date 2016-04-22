/*
 * Implementation of MCS-RWlock using GCC atomic built-ins
 */

typedef enum rwlock_type {
	RWLOCK_READ,
	RWLOCK_WRITE,
	RWLOCK_NIL
} rwlock_type_t;

typedef struct rwlock_state {
	unsigned char spinning;
	rwlock_type_t next_type;
} rwlock_state_t;

typedef struct rwlock_qnode {
	struct rwlock_qnode *next;
	rwlock_state_t state;
	rwlock_type_t type;
} rwlock_qnode_t;

typedef struct mcs_rwlock {
	rwlock_qnode_t *tail;
	rwlock_qnode_t *next_wr;
	unsigned long num_read;
} mcs_rwlock_t;

#define INIT_MCS_RWLOCK (mcs_rwlock_t){NULL, NULL, 0}
#define INIT_RLOCK_QNODE \
	(rwlock_qnode_t){NULL, (rwlock_state_t){1, RWLOCK_NIL}, RWLOCK_READ}
#define INIT_WLOCK_QNODE \
	(rwlock_qnode_t){NULL, (rwlock_state_t){1, RWLOCK_NIL}, RWLOCK_WRITE}

static inline void mcs_rlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	*qnode = INIT_RLOCK_QNODE;

	// set self as next locker (and try to take lock)
	rwlock_qnode_t *prev = 
		__atomic_exchange_n(lock->tail, qnode, __ATOMIC_ACQ_REL);

	// if no previous locker
	if (!prev) {
		__atomic_fetch_add(&lock->num_read, 1, __ATOMIC_ACQ_REL);
		qnode->state->spinning = 0;
	}
	// there is a previous locker
	else {
		rwlock_state_t state = (rwlock_state_t){1, RWLOCK_NIL};
		if ((prev->type == RWLOCK_WRITE) ||
				__atomic_compare_exchange_n(&prev->state, &state, 
					(rwlock_state_t){1, RWLOCK_READ}, false, __ATOMIC_ACQ_REL,
					__ATOMIC_ACQUIRE)) {
			// if predecessor is a writer || is waiting reader, wait to be released
			prev->next = qnode;
			while (qnode->state->spinning);
		}
		else {
			// otherwise, got read lock, increment count
			__atomic_fetch_add(&lock->num_read, 1, __ATOMIC_ACQ_REL);
			prev->next = qnode;
			qnode->state->spinning = 0;
		}
	}

	// check for next thread, if reader, allow to proceed
	if (qnode->state->next_type == RWLOCK_READ) {
		while (!qnode->next);
		__atomic_fetch_add(&lock->num_read, 1, __ATOMIC_ACQ_REL);
		qnode->next->state->spinning = 0;
	}
}

static inline void mcs_runlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	// check for next thread
	mcs_rwlock_t last = qnode;
	if (qnode->next || (__atomic_compare_exchange_n(lock, &last, NULL, false,
			__ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))) {
		// wait for next thread to identify itself
		while (!qnode->next);
		// if next is writer, set it as the 
		if (qnode->state->next_type == RWLOCK_WRITE)
			lock->next_wr = qnode->next;
	}

	// if last reader, wake any writer if it exists
	rwlock_qnode_t *writer;
	if ((__atomic_fetch_sub(&lock->num_read, 1, __ATOMIC_ACQ_REL) == 1) && 
			(writer = lock->next_wr) && !lock->num_read && 
			__atomic_compare_exchange_n(&lock->next_wr, &writer, NULL, false,
				__ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) 
		writer->state->spinning = 0;
}

static inline void mcs_wlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	*qnode = INIT_WLOCK_QNODE;

	// get previous thread (and try to set as owner)
	rwlock_qnode_t *prev = __atomic_exchange_n(lock, qnode, __ATOMIC_ACQ_REL);

	// check prev predecessor
	if (prev) {
		// identify self to predecessor and allow it to continue
		prev->state->next_type = RWLOCK_WRITE;
		prev->state->spinning = 0;
	}
	else {
		// otherwise, this is the next writer, wait for all readers to leave
		lock->next_wr = qnode;
		if (!lock->num_read && (__atomic_exchange_n(&lock->next_wr, NULL, 
				__ATOMIC_ACQ_REL) == qnode))
			qnode->state->spinning = 0;
	}

	// wait for predecessor or last reader to wake up this thread
	while (qnode->state->spinning);
}

static inline void mcs_wunlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	mcs_rwlock_t last = qnode;
	// check (and wait) for successor
	if (qnode->next || !(__atomic_compare_exchange_n(lock, &last, NULL, false,
			__ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))) {
		while (qnode->next);

		// increment read count if next is a reader
		if (qnode->next->type == RWLOCK_READ)
			__atomic_fetch_add(&lock->num_read, 1, __ATOMIC_ACQ_REL);
	
		// acknowledge next thread
		next->state->spinning = 0;
	}
}
