
  .section .rodata
  .globl a
  .align 2
  .type  a, @object
  .size  a, 20
a:
  .word 0
  .word 1
  .word 2
  .word 3
  .word 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	a0, 4
	ret
	.size main, .-main
