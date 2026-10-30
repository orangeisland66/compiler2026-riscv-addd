
  .bss
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .zero 4

  .bss
  .globl W
  .align 2
  .type  W, @object
  .size  W, 4
W:
  .zero 4

  .bss
  .globl weight
  .align 2
  .type  weight, @object
  .size  weight, 32
weight:
  .zero 32

  .bss
  .globl value
  .align 2
  .type  value, @object
  .size  value, 32
value:
  .zero 32

  .bss
  .globl __memo.knapsack_mixed.epoch
  .align 2
  .type  __memo.knapsack_mixed.epoch, @object
  .size  __memo.knapsack_mixed.epoch, 4
__memo.knapsack_mixed.epoch:
  .zero 4

  .bss
  .globl __memo.knapsack_mixed.depth
  .align 2
  .type  __memo.knapsack_mixed.depth, @object
  .size  __memo.knapsack_mixed.depth, 4
__memo.knapsack_mixed.depth:
  .zero 4

  .bss
  .globl __memo.knapsack_mixed.stamp
  .align 2
  .type  __memo.knapsack_mixed.stamp, @object
  .size  __memo.knapsack_mixed.stamp, 32768
__memo.knapsack_mixed.stamp:
  .zero 32768

  .bss
  .globl __memo.knapsack_mixed.value
  .align 2
  .type  __memo.knapsack_mixed.value, @object
  .size  __memo.knapsack_mixed.value, 32768
__memo.knapsack_mixed.value:
  .zero 32768

  .text

  .globl knapsack_mixed
  .type  knapsack_mixed, @function
knapsack_mixed:
.knapsack_mixed.bb0:
	mv	t0, a0
	mv	t1, a1
	li	t2, 0
	slt	t2, t0, t2
	xori	t2, t2, 1
	li	t3, 32
	slt	t3, t0, t3
	and	t2, t2, t3
	li	t3, 0
	slt	t3, t1, t3
	xori	t3, t3, 1
	li	a2, 256
	slt	a2, t1, a2
	and	t3, t3, a2
	and	t2, t2, t3
	slliw	t3, t0, 8
	addw	t3, t3, t1
	mulw	t3, t3, t2
	slliw	t3, t3, 2
	la	a2, __memo.knapsack_mixed.stamp
	add	a2, a2, t3
	lw	a3, 0(a2)
	li	a4, 1
	subw	a3, a3, a4
	sltiu	a3, a3, 1
	and	a3, a3, t2
	la	a4, __memo.knapsack_mixed.value
	add	t3, a4, t3
	bnez	a3, .knapsack_mixed.bb9
.knapsack_mixed.bb8:
	addi	sp, sp, -96
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	sd	s8, 80(sp)
	mv	s3, t2	# early-return.slow-copy
	mv	s4, t1	# early-return.slow-copy
	li	t1, 0
	li	t2, 0
	subw	t1, t0, t1
	subw	t2, s4, t2
	sltiu	t1, t1, 1
	sltiu	t2, t2, 1
	mv	s1, t3	# early-return.slow-copy
	mv	s2, a2	# early-return.slow-copy
	or	t1, t1, t2
	bne	t1, zero, .knapsack_mixed.bb2
.knapsack_mixed.bb3:
	addiw	s5, t0, -1
	slliw	s6, s5, 2
	la	t0, weight
	add	t0, t0, s6
	lw	s7, 0(t0)
	blt	s4, s7, .knapsack_mixed.bb4
.knapsack_mixed.bb5:
	mv	a0, s5	# call-arg-copy
	mv	a1, s4	# call-arg-copy
	call	knapsack_mixed
	mv	s8, a0
	la	t0, value
	add	t0, t0, s6
	lw	s6, 0(t0)
	subw	a1, s4, s7
	mv	a0, s5	# call-arg-copy
	call	knapsack_mixed
	mv	t0, a0
	addw	t0, s6, t0
	blt	s8, t0, .knapsack_mixed.bb6
.knapsack_mixed.bb7:
	j	.knapsack_mixed.bb10
.knapsack_mixed.bb1:
.knapsack_mixed.bb10:
	bnez	s3, .knapsack_mixed.bb11
.knapsack_mixed.bb12:
	mv	a0, s8
	ld	s8, 80(sp)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 96
	ret
.knapsack_mixed.bb2:
	li	s8, 0
	j	.knapsack_mixed.bb10
.knapsack_mixed.bb4:
	mv	a0, s5	# call-arg-copy
	mv	a1, s4	# call-arg-copy
	call	knapsack_mixed
	mv	t0, a0
	mv	s8, t0	# phi-copy
	j	.knapsack_mixed.bb10
.knapsack_mixed.bb6:
	mv	s8, t0	# phi-copy
	j	.knapsack_mixed.bb10
.knapsack_mixed.bb11:
	sw	s8, 0(s1)
	li	t0, 1
	sw	t0, 0(s2)
	j	.knapsack_mixed.bb12
.knapsack_mixed.bb9:
	lw	a0, 0(t3)
	ret
	.size knapsack_mixed, .-knapsack_mixed

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	la	t5, weight
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	la	t5, value
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	li	a0, 23
	call	_sysy_starttime
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, s2
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	knapsack_mixed
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 25
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_stoptime
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
