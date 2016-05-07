#ifndef MCS_TICKET_LOCK_H
#define MCS_TICKET_LOCK_H

#include <stddef.h>
#include <mic_xeon_phi.h>

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
	uint64_t ticket = __sync_fetch_and_add(&lock->ticket, 1);
	uint64_t i, cycles;
	while ((cycles = ticket - lock->turn))
		// proportional back-off
		busy_wait(cycles, i);
}

static inline void ticket_unlock(ticketlock_t *lock) {
	// prevent load reordering, no load fence on xeon phi
	#ifndef __MIC__
	__sync_synchronize();
	#endif /* __MIC__ */

	// increment turn
	lock->turn++;
}

#ifdef __cplusplus
}
#endif

#endif /* MCS_TICKET_LOCK_H */
