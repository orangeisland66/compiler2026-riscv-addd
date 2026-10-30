
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -160
	addi	t5, sp, 0
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sw	zero, 56(t5)	# memset-zero
	addi	t5, sp, 64
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	sd	zero, 64(t5)	# memset-zero
	sd	zero, 72(t5)	# memset-zero
	sd	zero, 80(t5)	# memset-zero
	sd	zero, 88(t5)	# memset-zero
	li	t0, 1
	addi	t1, sp, 64
	sw	t0, 0(t1)
	li	t0, 2
	sw	t0, 4(t1)
	li	t0, 3
	sw	t0, 8(t1)
	li	t0, 4
	sw	t0, 12(t1)
	li	t0, 5
	sw	t0, 48(t1)
	li	a0, 4
	addi	sp, sp, 160
	ret
	.size main, .-main
