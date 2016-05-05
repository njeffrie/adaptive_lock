#ifndef MCS_TICKET_LOCK_H
#define MCS_TICKET_LOCK_H

#include <stddef.h>

typedef struct ticketlock {
	uint64_t ticket __attribute__((aligned(64)));
	volatile uint64_t turn __attribute__((aligned(64)));
} ticketlock_t;

#define INIT_TICKETLOCK (ticketlock_t){0, 0}

#ifdef __cplusplus
extern "C" {
#endif

static inline void ticket_lock(ticketlock_t *lock) {
	// atomically fetch and increment ticket
	//uint64_t ticket = lock_xadd_64(&lock->ticket, 1);
	uint64_t ticket = __sync_fetch_and_add(&lock->ticket, 1);
	
	// wait for turn to match this thread's ticket
	uint64_t cycles;
	volatile uint64_t cnt;
	while ((cycles = ticket - lock->turn)) {
		// proportional back-off
		for (cnt = 0; cnt < cycles; cnt++);
	}
}

static inline void ticket_unlock(ticketlock_t *lock) {
	// increment turn
	lock->turn++;
}

#ifdef __cplusplus
}
#endif

#endif /* MCS_TICKET_LOCK_H */
