
%macro gensys 2
	global sys_%2:function
sys_%2:
	push	r10
	mov	r10, rcx
	mov	rax, %1
	syscall
	pop	r10
	ret
%endmacro

; RDI, RSI, RDX, RCX, R8, R9

extern	errno

	section .data

	section .text

	gensys   0, read
	gensys   1, write
	gensys   2, open
	gensys   3, close
	gensys   9, mmap
	gensys  10, mprotect
	gensys  11, munmap
	gensys  13, rt_sigaction
	gensys  14, rt_sigprocmask
	gensys  15, rt_sigreturn
	gensys  22, pipe
	gensys  32, dup
	gensys  33, dup2
	gensys  34, pause
	gensys  35, nanosleep
	gensys  37, alarm
	gensys  57, fork
	gensys  60, exit
	gensys  79, getcwd
	gensys  80, chdir
	gensys  82, rename
	gensys  83, mkdir
	gensys  84, rmdir
	gensys  85, creat
	gensys  86, link
	gensys  88, unlink
	gensys  89, readlink
	gensys  90, chmod
	gensys  92, chown
	gensys  95, umask
	gensys  96, gettimeofday
	gensys 102, getuid
	gensys 104, getgid
	gensys 105, setuid
	gensys 106, setgid
	gensys 107, geteuid
	gensys 108, getegid
	gensys 127, rt_sigpending

	global open:function
open:
	call	sys_open
	cmp	rax, 0
	jge	open_success	; no error :)
open_error:
	neg	rax
%ifdef NASM
	mov	rdi, [rel errno wrt ..gotpc]
%else
	mov	rdi, [rel errno wrt ..gotpcrel]
%endif
	mov	[rdi], rax	; errno = -rax
	mov	rax, -1
	jmp	open_quit
open_success:
%ifdef NASM
	mov	rdi, [rel errno wrt ..gotpc]
%else
	mov	rdi, [rel errno wrt ..gotpcrel]
%endif
	mov	QWORD [rdi], 0	; errno = 0
open_quit:
	ret

	global sleep:function
sleep:
	sub	rsp, 32		; allocate timespec * 2
	mov	[rsp], rdi		; req.tv_sec
	mov	QWORD [rsp+8], 0	; req.tv_nsec
	mov	rdi, rsp	; rdi = req @ rsp
	lea	rsi, [rsp+16]	; rsi = rem @ rsp+16
	call	sys_nanosleep
	cmp	rax, 0
	jge	sleep_quit	; no error :)
sleep_error:
	neg	rax
	cmp	rax, 4		; rax == EINTR?
	jne	sleep_failed
sleep_interrupted:
	lea	rsi, [rsp+16]
	mov	rax, [rsi]	; return rem.tv_sec
	jmp	sleep_quit
sleep_failed:
	mov	rax, 0		; return 0 on error
sleep_quit:
	add	rsp, 32
	ret

	global __myrt:function
__myrt:
	mov rax, 15
	syscall
	ret

extern sigsetjmp
	global setjmp:function
setjmp:
	push rdi ; store setjmp() parameters
%ifdef NASM
	call [rel sigsetjmp wrt ..gotpc]
%else
	call [rel sigsetjmp wrt ..gotpcrel]
%endif

	pop rdi ; get setjmp() parameters
	mov [rdi + 8 * 0], rbx
	mov [rdi + 8 * 1], rsp
	mov [rdi + 8 * 2], rbp
	mov [rdi + 8 * 3], r12
	mov [rdi + 8 * 4], r13
	mov [rdi + 8 * 5], r14
	mov [rdi + 8 * 6], r15
	pop QWORD[rdi + 8 * 7] ; get return address and store to variable
	push QWORD[rdi + 8 * 7] ; push back return address
	ret

extern siglongjmp
	global longjmp: function
longjmp:
	pop rax ; original return address, we have to discard it
	push rdi
	push rsi ; ??
	%ifdef NASM
		call [rel siglongjmp wrt ..gotpc]
	%else
		call [rel siglongjmp wrt ..gotpcrel]
	%endif
	pop rsi
	pop rdi
	test rsi, rsi ; rsi is 0 or not
	jne normal
	mov rsi, 1 ; set the return value to 1
normal:
	mov rax, rsi
	mov rbx, QWORD [rdi + 8 * 0]
	mov rsp, QWORD [rdi + 8 * 1]
	mov rbp, QWORD [rdi + 8 * 2]
	mov r12, QWORD [rdi + 8 * 3]
	mov r13, QWORD [rdi + 8 * 4]
	mov r14, QWORD [rdi + 8 * 5]
	mov r15, QWORD [rdi + 8 * 6]
	push QWORD [rdi + 8 * 7] ; return to setjmp
	ret