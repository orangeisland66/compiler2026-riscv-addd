
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t0, 10
	la	t1, a
	sw	t0, 0(t1)
	li	a0, 9
	ret
	.size main, .-main
