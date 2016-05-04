/*
 * Implementation of MCS ticket spinlock
 */

#include <stddef.h>
#include <atomics_x86.h>
#include <mcs_ticket_lock.h>

void ticket_lock(ticketlock_t *lock) {
	// atomically fetch and increment ticket
	//uint64_t ticket = lock_xadd_64(&lock->ticket, 1);
	uint64_t ticket = __sync_fetch_and_add(&lock->ticket, 1);
	
	// wait for turn to match this thread's ticket
	int64_t turn, cnt;
	while ((turn = lock->turn) != ticket) {
		// proportional back-off
		for (cnt = ticket; cnt < turn; cnt++);
	}
}

void ticket_unlock(ticketlock_t *lock) {
	// increment turn
	lock->turn++;
}
