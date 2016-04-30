#ifndef MCS_QUEUE_RWLOCK_H
#define MCS_QUEUE_RWLOCK_H

#include <stddef.h>

typedef enum rwlock_type {
	RWLOCK_READ,
	RWLOCK_WRITE,
	RWLOCK_NIL
} rwlock_type_t;

typedef uint64_t rwlock_state_t;

typedef struct rwlock_qnode {
	struct rwlock_qnode *next;
	rwlock_state_t state;
	rwlock_type_t type;
} rwlock_qnode_t;

typedef struct mcs_rwlock {
	rwlock_qnode_t *tail;
	rwlock_qnode_t *next_wr;
	uint64_t num_read;
} mcs_rwlock_t;

#ifdef __cplusplus
extern "C" {
#endif

void mcs_rlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode);
void mcs_runlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode);
void mcs_wlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode);
void mcs_wunlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode);

#ifdef __cplusplus
}
#endif

#endif /* MCS_QUEUE_RWLOCK_H */
