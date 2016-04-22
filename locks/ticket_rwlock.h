/*
 * Implementation of read-write ticketlock described in MCS paper
 */

#ifndef TICKET_RWLOCK_H
#define TICKET_RWLOCK_H

#include <stddef.h>
#include "atomics_x86.h"

typedef uint64_t rwlock_count_t;

#define RCNT_INC 0x100000000L
#define WCNT_INC 0x1L
#define W_MASK 0xffffffffL

#define INIT_TICKET_RWLOCK (ticket_rwlock_t){0,0}

typedef struct ticket_rwlock {
	rwlock_count_t requests;
	rwlock_count_t completions;
} ticket_rwlock_t;

static inline void ticket_rlock(ticket_rwlock_t *lock) {
	// get ticket and increment read count
	uint64_t prev_writers = lock_xadd_64(&lock->requests, RCNT_INC) & W_MASK;

	// spin until the write completions match the ticket
	int wait = 0, cnt;
	while (prev_writers != (lock->completions & W_MASK)) {
		// proportional back-off
		for (cnt = 0; cnt < wait; cnt ++);
		wait++;
	}
}

static inline void ticket_runlock(ticket_rwlock_t *lock) {
	// increment completed read count
	lock->completions += RCNT_INC;
}

static inline void ticket_wlock(ticket_rwlock_t *lock) {
	// get ticket and increment write count
	uint64_t prev_threads = lock_xadd_64(&lock->requests, WCNT_INC);

	// spin until the request completion matches ticket
	int wait = 0, cnt;
	while (prev_threads != lock->completions) {
		// proportional back-off
		for (cnt = 0; cnt < wait; cnt ++);
		wait++;
	}
}

static inline void ticket_wunlock(ticket_rwlock_t *lock) {
	// increment completed read count
	lock->completions += WCNT_INC;
}

#endif /* TICKET_RWLOCK_H */
