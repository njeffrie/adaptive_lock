#ifndef LOCK_H
#define LOCK_H

#include <ttslock.h>
#include <mcs_ticket_lock.h>
#include <mcs_queue_lock.h>
#include <mcs_hybrid_lock.h>

#define LOCK_TYPE mcs_hybrid_lock_t
#define LOCK_INIT INIT_HYBRID_LOCK
#define LOCK_PRE mcs_hybrid_
#define QNODE_TYPE hybrid_qnode_t

#define LOCK_DEF(lock) LOCK_TYPE lock
#define LOCK_INIT(lock) lock = LOCK_INIT

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

#define 

#endif /* LOCK_H */
