/*
 * Implementation of MCS-RWlock 
 */

#include <stddef.h>
#include <atomics_x86.h>
#include <mcs_queue_rwlock.h>

void mcs_rlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	*qnode = INIT_RLOCK_QNODE;

	// set self as next locker (and try to take lock)
	//rwlock_qnode_t *prev = lock_xchg_64(&lock->tail, qnode);
	rwlock_qnode_t *prev = __sync_lock_test_and_set(&lock->tail, qnode);

	// if no previous locker
	if (!prev) {
		//lock_xadd_64(&lock->num_read, 1);
		__sync_fetch_and_add(&lock->num_read, 1);
		qnode->state = STATE_SET_SPIN(qnode->state, 0);
	}
	// there is a previous locker
	else {
		if ((prev->type == RWLOCK_WRITE) ||
				//(lock_cmpxchg_64(&prev->state, STATE_WAIT_NIL, STATE_WAIT_READ) 
					//== STATE_WAIT_NIL)) {
				__sync_bool_compare_and_swap(&prev->state, STATE_WAIT_NIL, STATE_WAIT_READ)) {
			// if predecessor is a writer || is waiting reader, wait to be released
			prev->next = qnode;
			while (STATE_GET_SPIN(qnode->state));
		}
		else {
			// otherwise, got read lock, increment count
			//lock_xadd_64(&lock->num_read, 1);
			__sync_fetch_and_add(&lock->num_read, 1);
			prev->next = qnode;
			qnode->state = STATE_SET_SPIN(qnode->state, 0);
		}
	}

	// check for next thread, if reader, allow to proceed
	if (STATE_GET_NEXT(qnode->state) == RWLOCK_READ) {
		while (!qnode->next);
		//lock_xadd_64(&lock->num_read, 1);
		__sync_fetch_and_add(&lock->num_read, 1);
		qnode->next->state = STATE_SET_SPIN(qnode->state, 0);
	}
}

void mcs_runlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	// check for next thread
	if (qnode->next || //(lock_cmpxchg_64(&lock->tail, qnode, NULL) == qnode)) {
			__sync_bool_compare_and_swap(&lock->tail, qnode, NULL)) {
		// wait for next thread to identify itself
		while (!qnode->next);
		// if next is writer, set it as the 
		if (STATE_GET_NEXT(qnode->state) == RWLOCK_WRITE)
			lock->next_wr = qnode->next;
	}

	// if last reader, wake any writer if it exists
	rwlock_qnode_t *writer;
	//if ((lock_xadd_64(&lock->num_read, -1) == 1) && 
	if ((__sync_fetch_and_add(&lock->num_read, -1) == 1) && 
			(writer = lock->next_wr) && !lock->num_read && 
			//(lock_cmpxchg_64(&lock->next_wr, writer, NULL) == writer))
			__sync_bool_compare_and_swap(&lock->next_wr, writer, NULL))
		writer->state = STATE_SET_SPIN(writer->state, 0);
}

void mcs_wlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	*qnode = INIT_WLOCK_QNODE;

	// get previous thread (and try to set as owner)
	//rwlock_qnode_t *prev = lock_xchg_64(&lock->tail, qnode);
	rwlock_qnode_t *prev = __sync_lock_test_and_set(&lock->tail, qnode);

	// check prev predecessor
	if (prev) 
		// identify self to predecessor and allow it to continue
		prev->state = STATE_NOWAIT_WRITE;
	else {
		// otherwise, this is the next writer, wait for all readers to leave
		lock->next_wr = qnode;
		if (!lock->num_read && //(lock_xchg_64(&lock->next_wr, NULL) == qnode))
				(__sync_lock_test_and_set(&lock->next_wr, NULL) == qnode))
			qnode->state = STATE_SET_SPIN(qnode->state, 0);
	}

	// wait for predecessor or last reader to wake up this thread
	while (STATE_GET_SPIN(qnode->state));
}

void mcs_wunlock(mcs_rwlock_t *lock, rwlock_qnode_t *qnode) {
	// check (and wait) for successor
	if (qnode->next || //!(lock_cmpxchg_64(&lock->tail, qnode, NULL) == qnode)) {
			!__sync_bool_compare_and_swap(&lock->tail, qnode, NULL)) {
		while (qnode->next);

		// increment read count if next is a reader
		if (qnode->next->type == RWLOCK_READ)
			//lock_xadd_64(&lock->num_read, 1);
			__sync_fetch_and_add(&lock->num_read, 1);
	
		// acknowledge next thread
		qnode->next->state = STATE_SET_SPIN(qnode->next->state, 0);
	}
}
