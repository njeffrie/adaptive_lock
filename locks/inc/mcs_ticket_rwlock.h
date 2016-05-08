#ifndef MCS_TICKET_RWLOCK_H
#define MCS_TICKET_RWLOCK_H

#include <stddef.h>
#include <mic_xeon_phi.h>

typedef uint64_t rwlock_count_t;

#define RCNT_INC 0x100000000L
#define WCNT_INC 0x1L
#define W_MASK 0xffffffffL

#define INIT_TICKET_RWLOCK (ticket_rwlock_t){0,0}

typedef struct ticket_rwlock {
	rwlock_count_t requests __attribute__((aligned(64)));
	volatile rwlock_count_t completions __attribute__((aligned(64)));
} ticket_rwlock_t;

#ifdef __cplusplus
extern "C" {
#endif

inline void ticket_rlock(ticket_rwlock_t *lock){
	// get ticket and increment read count
	//uint64_t prev_writers = lock_xadd_64(&lock->requests, RCNT_INC) & W_MASK;
	uint64_t prev_writers = __sync_fetch_and_add(&lock->requests, RCNT_INC) & W_MASK;

	// spin until the write completions match the ticket
	int wait = 0, cnt;
	while (wait = (prev_writers - (lock->completions & W_MASK))) {
		// proportional back-off
		busy_wait(wait, cnt);
	}
}

inline void ticket_runlock(ticket_rwlock_t *lock){
	__sync_synchronize();
	lock->completions += RCNT_INC;
}

inline void ticket_wlock(ticket_rwlock_t *lock){
	// get ticket and increment write count
	//uint64_t prev_threads = lock_xadd_64(&lock->requests, WCNT_INC);
	uint64_t prev_threads = __sync_fetch_and_add(&lock->requests, WCNT_INC);

	// spin until the request completion matches ticket
	uint64_t wait, cnt;
	while (wait = (prev_threads - lock->completions)) {
		int writers = wait & W_MASK;
		int readers = (wait >> 32) & W_MASK;
		// proportional back-off
		busy_wait(readers + writers, cnt);
	}
}

inline void ticket_wunlock(ticket_rwlock_t *lock){
	// increment completed read count
	__sync_synchronize();
	lock->completions += WCNT_INC;
}

#ifdef __cplusplus
}
#endif

#endif /* MCS_TICKET_RWLOCK_H */
