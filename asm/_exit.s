	.text
	.globl	_exit
  movq $60, %rax # _exit syscall number
  #  movq $0, %rdi ; Store exit number to rdi, but exit number is already stored because of the exit()'s argument
  syscall
  hlt
