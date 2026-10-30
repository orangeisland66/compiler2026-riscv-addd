
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
	li	a0, 4000
	ret
	.size main, .-main
