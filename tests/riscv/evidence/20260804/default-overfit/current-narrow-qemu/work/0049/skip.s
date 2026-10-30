
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 400000
a:
  .zero 400000

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t0, 1
	la	t1, a
	li	t2, 399996
	add	t1, t1, t2
	sw	t0, 0(t1)
	li	a0, 4000
	ret
	.size main, .-main
