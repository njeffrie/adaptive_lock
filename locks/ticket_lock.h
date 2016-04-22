/*
 * Implementation of ticket spinlock using GCC atomic built-ins
 */

#ifndef TICKET_LOCK_H
#define TICKET_LOCK_H

#include <stddef.h>
#include "atomics_x86.h"

typedef struct ticketlock {
	uint64_t ticket;
	uint64_t turn;
} ticketlock_t;

#define INIT_TICKETLOCK (ticketlock_t){0, 0}

static inline void ticket_lock(ticketlock_t *lock) {
	// atomically fetch and increment ticket
	uint64_t ticket = lock_xadd_64(&lock->ticket, 1);

	// wait for turn to match this thread's ticket
	int wait = 0, cnt;
	while (lock->turn != ticket) {
		// proportional back-off
		for (cnt = 0; cnt < wait; cnt ++);
		wait++;
	}
}

static inline void ticket_unlock(ticketlock_t *lock) {
	// increment turn
	lock->turn++;
}

#endif /* TICKET_LOCK_H */
