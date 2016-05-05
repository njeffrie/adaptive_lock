/*
 *	Implementation of test and test and set lock with exponential backoff
 */

#include <assert.h>

typedef struct ttslock {
	volatile bool locked;
} ttslock_t;

#define INIT_TTSLOCK (ttslock_t){0}
#define LOCKED 1
#define UNLOCKED 0

inline void ttslock_init(ttslock_t *lock){
	lock->locked = UNLOCKED;
}

inline void wait_cycles(int cycles){
	volatile int i;
	for (i=0; i<cycles; i++);
}

/* basic test and test and set lock with exponential backoff */
inline void tts_lock(ttslock_t *lock){
	/*int backoff = 1;
	do {
		while (lock->locked){
			wait_cycles(backoff);
			backoff *= 2;
		}
	} */while (__sync_lock_test_and_set(&lock->locked, LOCKED));
	//while (!lock_cmpxchg_64(&lock->locked, UNLOCKED, LOCKED));
}

inline void tts_unlock(ttslock_t *lock){
	//assert(lock->locked);
	__sync_synchronize();
	lock->locked = UNLOCKED;
}
