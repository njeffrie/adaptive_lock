/*
 *	Implementation of test and test and set lock with exponential backoff
 */

typedef struct ttslock {
	unsigned int locked;
} ttslock_t;

#define INIT_TTSLOCK (ttslock_t){0}

inline void ttslock_init(ttslock_t *lock){
	lock->locked = 0;
}

inline void wait_cycles(int cycles){
	int count;
	for (int i=0; i<cycles; i++){
		count++;
	}
}

/* basic test and test and set lock with exponential backoff */
inline void tts_lock(ttslock_t *lock){
	int backoff = 1;
	do {
		while (lock->locked){
			wait_cycles(backoff);
			backoff *= 2;
		}
	} while (!__sync_bool_compare_and_swap(&lock->locked, 0, 1));
	//while (!_lock_xchg_64(&lock->locked, 1));
}

inline void tts_unlock(ttslock_t *lock){
	lock->locked = 0;
}
