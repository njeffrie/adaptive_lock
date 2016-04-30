#ifndef MCS_TICKET_RWLOCK_H
#define MCS_TICKET_RWLOCK_H

#include <stddef.h>

typedef uint64_t rwlock_count_t;

#define RCNT_INC 0x100000000L
#define WCNT_INC 0x1L
#define W_MASK 0xffffffffL

#define INIT_TICKET_RWLOCK (ticket_rwlock_t){0,0}

typedef struct ticket_rwlock {
	rwlock_count_t requests;
	rwlock_count_t completions;
} ticket_rwlock_t;

void ticket_rlock(ticket_rwlock_t *lock);
void ticket_runlock(ticket_rwlock_t *lock);
void ticket_wlock(ticket_rwlock_t *lock);
void ticket_wunlock(ticket_rwlock_t *lock);

#endif /* MCS_TICKET_RWLOCK_H */