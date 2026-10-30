
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
  .size  weight, 200
weight:
  .zero 200

  .bss
  .globl value
  .align 2
  .type  value, @object
  .size  value, 200
value:
  .zero 200

  .bss
  .globl __memo.knapsack_naive.epoch
  .align 2
  .type  __memo.knapsack_naive.epoch, @object
  .size  __memo.knapsack_naive.epoch, 4
__memo.knapsack_naive.epoch:
  .zero 4

  .bss
  .globl __memo.knapsack_naive.depth
  .align 2
  .type  __memo.knapsack_naive.depth, @object
  .size  __memo.knapsack_naive.depth, 4
__memo.knapsack_naive.depth:
  .zero 4

  .bss
  .globl __memo.knapsack_naive.stamp
  .align 2
  .type  __memo.knapsack_naive.stamp, @object
  .size  __memo.knapsack_naive.stamp, 32768
__memo.knapsack_naive.stamp:
  .zero 32768

  .bss
  .globl __memo.knapsack_naive.value
  .align 2
  .type  __memo.knapsack_naive.value, @object
  .size  __memo.knapsack_naive.value, 32768
__memo.knapsack_naive.value:
  .zero 32768

  .text

  .globl knapsack_naive
  .type  knapsack_naive, @function
knapsack_naive:
.knapsack_naive.bb0:
	mv	t0, a0
	mv	t1, a1
	li	t2, 0
	slt	t2, t0, t2
	xori	t2, t2, 1
	li	t3, 32
	slt	t3, t0, t3
	and	t2, t2, t3
	andi	t2, t2, 1
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
	la	a2, __memo.knapsack_naive.stamp
	add	a2, a2, t3
	lw	a3, 0(a2)
	li	a4, 1
	subw	a3, a3, a4
	sltiu	a3, a3, 1
	and	a3, a3, t2
	la	a4, __memo.knapsack_naive.value
	add	t3, a4, t3
	bnez	a3, .knapsack_naive.bb9
.knapsack_naive.bb8:
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
	bne	t1, zero, .knapsack_naive.bb2
.knapsack_naive.bb3:
	addiw	s5, t0, -1
	slliw	s6, s5, 2
	la	t0, weight
	add	t0, t0, s6
	lw	s7, 0(t0)
	blt	s4, s7, .knapsack_naive.bb4
.knapsack_naive.bb5:
	mv	a0, s5	# call-arg-copy
	mv	a1, s4	# call-arg-copy
	call	knapsack_naive
	mv	s8, a0
	la	t0, value
	add	t0, t0, s6
	lw	s6, 0(t0)
	subw	a1, s4, s7
	mv	a0, s5	# call-arg-copy
	call	knapsack_naive
	mv	t0, a0
	addw	t0, s6, t0
	blt	s8, t0, .knapsack_naive.bb6
.knapsack_naive.bb7:
	j	.knapsack_naive.bb10
.knapsack_naive.bb1:
.knapsack_naive.bb10:
	bnez	s3, .knapsack_naive.bb11
.knapsack_naive.bb12:
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
.knapsack_naive.bb2:
	li	s8, 0
	j	.knapsack_naive.bb10
.knapsack_naive.bb4:
	mv	a0, s5	# call-arg-copy
	mv	a1, s4	# call-arg-copy
	call	knapsack_naive
	mv	t0, a0
	mv	s8, t0	# phi-copy
	j	.knapsack_naive.bb10
.knapsack_naive.bb6:
	mv	s8, t0	# phi-copy
	j	.knapsack_naive.bb10
.knapsack_naive.bb11:
	sw	s8, 0(s1)
	li	t0, 1
	sw	t0, 0(s2)
	j	.knapsack_naive.bb12
.knapsack_naive.bb9:
	lw	a0, 0(t3)
	ret
	.size knapsack_naive, .-knapsack_naive

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
	li	a0, 32
	call	_sysy_starttime
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, s2
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	knapsack_naive
	mv	s1, a0
	li	t5, 34
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
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
