/*
 * Implementation of read-write ticketlock described in MCS paper
 */

#include <atomics_x86.h>
#include <stddef.h>
#include <mcs_ticket_rwlock.h>

void ticket_rlock(ticket_rwlock_t *lock) {
	// get ticket and increment read count
	//uint64_t prev_writers = lock_xadd_64(&lock->requests, RCNT_INC) & W_MASK;
	uint64_t prev_writers = __sync_fetch_and_add(&lock->requests, RCNT_INC) & W_MASK;

	// spin until the write completions match the ticket
	int wait = 0, cnt;
	while (prev_writers != (lock->completions & W_MASK)) {
		// proportional back-off
		for (cnt = 0; cnt < wait; cnt ++);
		wait++;
	}
}

void ticket_runlock(ticket_rwlock_t *lock) {
	// increment completed read count
	lock->completions += RCNT_INC;
}

void ticket_wlock(ticket_rwlock_t *lock) {
	// get ticket and increment write count
	//uint64_t prev_threads = lock_xadd_64(&lock->requests, WCNT_INC);
	uint64_t prev_threads = __sync_fetch_and_add(&lock->requests, WCNT_INC);

	// spin until the request completion matches ticket
	int wait = 0, cnt;
	while (prev_threads != lock->completions) {
		// proportional back-off
		for (cnt = 0; cnt < wait; cnt ++);
		wait++;
	}
}

void ticket_wunlock(ticket_rwlock_t *lock) {
	// increment completed read count
	lock->completions += WCNT_INC;
}
