
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 400
a:
  .zero 400

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	a0, 0
	ret
	.size main, .-main
