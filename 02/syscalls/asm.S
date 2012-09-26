.global _start
_start:
	mov $1, %rdi
	lea text, %rsi

	mov $etext, %rdx
	sub $text, %rdx

	mov $1, %rax
	syscall # syscall 1, 1, &text, etext-text

	mov $5, %rdi
	mov $60, %rax
	syscall

.data
text:
	.ascii "ahoj\n"
etext:
