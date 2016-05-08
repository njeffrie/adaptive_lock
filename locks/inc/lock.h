#ifndef LOCK_H
#define LOCK_H

#include <ttslock.h>
#include <mcs_ticket_lock.h>
#include <mcs_queue_lock.h>
#include <mcs_hybrid_lock.h>

#define LOCK_TYPE mcs_hybrid_lock_t
#define LOCK_INIT_VAL INIT_HYBRID_LOCK
#define LOCK_PRE mcs_hybrid_
#define QNODE_TYPE hybrid_qnode_t

#define LOCK_DEF(lock) LOCK_TYPE lock
#define LOCK_INIT(lock) lock = LOCK_INIT_VAL

#ifdef LOCK_QNODE

#define LOCK_LOCK(lock, qnode) \
	LOCK_PRE ## lock(lock, qnode)
#define LOCK_UNLOCK(lock, qnode) \
	LOCK_PRE ## unlock(lock, qnode)

#else

#define LOCK_LOCK(lock, qnode) \
	do { \
		(void)qnode; \
		LOCK_PRE ## lock(lock); \
	} while (0)
#define LOCK_UNLOCK(lock, qnode) \
	do { \
		(void)qnode; \
		LOCK_PRE ## unlock(lock); \
	} while (0)

#endif 

#define ALOCK_DEF(lock, num_locks) LOCK_TYPE lock[num_locks]
#define ALOCK_INIT(lock, num_locks) \
	do { \
		int i; \
		for (i = 0; i < (num_locks); i++) \
			(lock)[i] = LOCK_INIT_VAL; \
	} while (0)
#define ALOCK_LOCK(lock, idx, qnode) LOCK_LOCK(lock + idx, qnode)
#define ALOCK_UNLOCK(lock, idx, qnode) LOCK_UNLOCK(lock + idx, qnode)

#endif /* LOCK_H */
