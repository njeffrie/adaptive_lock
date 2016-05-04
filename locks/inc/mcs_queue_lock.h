#ifndef MCS_QUEUE_LOCK_H
#define MCS_QUEUE_LOCK_H

#include <stddef.h>

struct lock_qnode {
	volatile struct lock_qnode *next;
	unsigned char wait;
} __attribute__((aligned(64)));

typedef volatile struct lock_qnode lock_qnode_t;
typedef lock_qnode_t *mcs_lock_t;

#define INIT_MCS_LOCK (mcs_lock_t)NULL

#ifdef __cplusplus
extern "C" {
#endif

void mcs_lock(mcs_lock_t *lock, lock_qnode_t *qnode);
void mcs_unlock(mcs_lock_t *lock, lock_qnode_t *qnode);

#ifdef __cplusplus
}
#endif

#endif /* MCS_QUEUE_LOCK_H */
