
  .data
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .word 3

  .data
  .globl b
  .align 2
  .type  b, @object
  .size  b, 4
b:
  .word 5

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	a0, 10
	ret
	.size main, .-main
