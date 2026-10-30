
  .section .rodata
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .word 5

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	a0, 25
	ret
	.size main, .-main
