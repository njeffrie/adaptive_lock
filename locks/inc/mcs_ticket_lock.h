#ifndef MCS_TICKET_LOCK_H
#define MCS_TICKET_LOCK_H

#include <stddef.h>

typedef struct ticketlock {
	uint64_t ticket;
	uint64_t turn;
} ticketlock_t;

#define INIT_TICKETLOCK (ticketlock_t){0, 0}

#ifdef __cplusplus
extern "C" {
#endif

void ticket_lock(ticketlock_t *lock);
void ticket_unlock(ticketlock_t *lock);

#ifdef __cplusplus
}
#endif

#endif /* MCS_TICKET_LOCK_H */
