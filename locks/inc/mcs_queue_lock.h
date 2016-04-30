#ifndef MCS_QUEUE_LOCK_H
#define MCS_QUEUE_LOCK_H

#include <stddef.h>

typedef struct lock_qnode {
	struct lock_qnode *next;
	unsigned char wait;
} lock_qnode_t;

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
