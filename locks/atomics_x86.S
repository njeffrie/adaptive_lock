/* x86_64 atomic 64-bit compare-and-swap wrapper */
.global _lock_cmpxchg_64
_lock_cmpxchg_64:
	mov %rsi, %rax # move old value to check into %rax
	lock cmpxchg %rdx, (%rdi) # (%rdi) = (%rdi) == %rax ? %rdx : (%rdi)
	ret

/* x86_64 atomic 64-bit swap wrapper */
.global _lock_xchg_64
_lock_xchg_64:
	mov %rsi, %rax # move value to exchange into %rax
	lock xchg %rax, (%rdi) # (%rdi) = %rax, %rax = (%rdi)
	ret

/* x86_64 atomic 64-bit fetch-and-add wrapper */
.global _lock_xadd_64
_lock_xadd_64:
	mov %rsi, %rax # move value to add into %rax
	lock xadd %rax, (%rdi) # %rax = (%rdi), (%rdi) = (%rdi) + %rax
	ret
