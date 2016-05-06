/*
 *	Implementation of test and test and set lock with exponential backoff
 */

#include <assert.h>
#include <mic_xeon_phi.h>

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
	uint64_t back_off = 1;
	do {
		while (lock->locked) {
			uint64_t i;
			busy_wait(back_off, i);
			back_off *= 2;
		};
	} while (__sync_lock_test_and_set(&lock->locked, LOCKED));
}

inline void tts_unlock(ttslock_t *lock){
	__sync_synchronize();
	lock->locked = UNLOCKED;
}
