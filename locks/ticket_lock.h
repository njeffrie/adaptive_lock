/*
 * Implementation of ticket spinlock using GCC atomic built-ins
 */

typedef struct ticketlock {
	unsigned int ticket;
	unsigned int turn;
} ticketlock_t;

#define INIT_TICKETLOCK (ticketlock_t){0, 0}

static inline void ticket_lock(ticketlock_t *lock) {
	// atomically fetch and increment ticket
	int ticket = __atomic_fetch_add(&lock->ticket, 1, __ATOMIC_ACQ_REL);

	// wait for turn to match this thread's ticket
	while (&lock->turn != ticket);
}

static inline void ticket_unlock(ticketlock_t *lock) {
	// increment turn
	lock->turn++;
}
