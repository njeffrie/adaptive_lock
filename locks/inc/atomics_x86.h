#ifndef ATOMICS_X86_H
#define ATOMICS_X86_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**	
 *	@brief atomic compare-and-swap wrapper for x86_64 
 *
 *	@param data Pointer to memory location
 *	@param old_val The value to check against
 *	@param new_val The value to replace with if check succeeds
 *	@return The original value at the memory location
 */
uint64_t _lock_cmpxchg_64(uint64_t *data, uint64_t old_val, uint64_t new_val);
#define lock_cmpxchg_64(data, old_val, new_val) \
	(typeof(*(data)))_lock_cmpxchg_64((uint64_t *)data, (uint64_t)old_val, (uint64_t)new_val)

/**	
 *	@brief atomic swap wrapper for x86_64 
 *
 *	@param data Pointer to memory location
 *	@param new_val The value to replace with if check succeeds
 *	@return The original value at the memory location
 */
uint64_t _lock_xchg_64(uint64_t *data, uint64_t new_val);
#define lock_xchg_64(data, new_val) \
	(typeof(*(data)))_lock_xchg_64((uint64_t *)data, (uint64_t)new_val)

/**	
 *	@brief atomic fetch-and-add wrapper for x86_64 
 *
 *	@param data Pointer to memory location
 *	@param addend The value to add to memory location
 *	@return The original value at the memory location
 */
uint64_t _lock_xadd_64(uint64_t *data, uint64_t addend);
#define lock_xadd_64(data, addend) \
	(typeof(*(data)))_lock_xadd_64((uint64_t *)data, (uint64_t)addend)

#ifdef __cplusplus
}
#endif

#endif /* ATOMICS_X86_H */
