#ifndef MCS_QUEUE_RWLOCK_H
#define MCS_QUEUE_RWLOCK_H

#include <stddef.h>

#define STATE_SET_NEXT(state, next) (((state) & 0xffffffff) | ((next) << 32))
#define STATE_GET_NEXT(state) ((state) >> 32)
#define STATE_GET_SPIN(state) ((state) & 0x1)
#define STATE_SET_SPIN(state, spin) (((state) & ~0x1) | (spin))
#define STATE_WAIT_NIL (((uint64_t)RWLOCK_NIL << 32) | 1)
#define STATE_WAIT_READ (((uint64_t)RWLOCK_READ << 32) | 1)
#define STATE_WAIT_WRITE (((uint64_t)RWLOCK_WRITE << 32) | 1)
#define STATE_NOWAIT_NIL (((uint64_t)RWLOCK_NIL << 32) | 0)
#define STATE_NOWAIT_READ (((uint64_t)RWLOCK_READ << 32) | 0)
#define STATE_NOWAIT_WRITE (((uint64_t)RWLOCK_WRITE << 32) | 0)
#define STATE(next_type, wait) (((next_type) << 32) | (wait))

#define INIT_MCS_RWLOCK (mcs_rwlock_t){NULL, NULL, 0}
#define INIT_RLOCK_QNODE \
	(rwlock_qnode_t){NULL, STATE_WAIT_NIL, RWLOCK_READ}
#define INIT_WLOCK_QNODE \
	(rwlock_qnode_t){NULL, STATE_WAIT_NIL, RWLOCK_WRITE}

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
} __attribute__((aligned(64))) rwlock_qnode_t;

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
