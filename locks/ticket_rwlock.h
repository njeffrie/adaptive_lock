/*
 * Implementation of read-write ticketlock described in MCS paper
 */

typedef unsigned long long rwlock_count_t;

#define RCNT_INC 0x100000000
#define WCNT_INC 0x1
#define W_MASK 0xffffffff

typedef struct ticket_rwlock {
	rwlock_count_t requests;
	rwlock_count_t completions;
} ticket_rwlock_t;

static inline void ticket_rlock(ticket_rwlock_t *lock) {
	// get ticket and increment read count
	unsigned int prev_writers = __atomic_fetch_add(&lock->requests, 
		RCNT_INC, __ATOMIC_ACQ_REL) & W_MASK;

	// spin until the write completions match the ticket
	while (prev_writers != (lock->completions & W_MASK));
}

static inline void ticket_runlock(ticket_rwlock_t *lock) {
	// increment completed read count
	lock->completions += RCNT_INC;
}

static inline void ticket_wlock(ticket_rwlock_t *lock) {
	// get ticket and increment write count
	unsigned int prev_threads = __atomic_fetch_add(&lock->requests, 
		WCNT_INC, __ATOMIC_ACQ_REL);

	// spin until the request completion matches ticket
	while (prev_threads != lock->completions);
}

static inline void ticket_wunlock(ticket_rwlock_t *lock) {
	// increment completed read count
	lock->completions += WCNT_INC;
}
