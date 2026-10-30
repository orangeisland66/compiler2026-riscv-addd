
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	addi	t5, sp, 0
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	li	a0, 21
	addi	sp, sp, 32
	ret
	.size main, .-main
