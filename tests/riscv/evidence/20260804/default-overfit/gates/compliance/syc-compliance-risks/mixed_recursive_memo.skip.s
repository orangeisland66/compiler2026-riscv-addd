
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

  .text

  .globl knapsack_mixed
  .type  knapsack_mixed, @function
knapsack_mixed:
.knapsack_mixed.bb0:
	addi	sp, sp, -64
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	mv	t0, a0
	mv	s1, a1
	li	t1, 0
	subw	t1, t0, t1
	sltiu	t1, t1, 1
	li	t2, 0
	subw	t2, s1, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	bne	t1, zero, .knapsack_mixed.bb2
.knapsack_mixed.bb3:
	addiw	s2, t0, -1
	slliw	s3, s2, 2
	la	t0, weight
	add	t0, t0, s3
	lw	s4, 0(t0)
	blt	s1, s4, .knapsack_mixed.bb4
.knapsack_mixed.bb5:
	mv	a0, s2	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	call	knapsack_mixed
	mv	s5, a0
	la	t0, value
	add	t0, t0, s3
	lw	s3, 0(t0)
	subw	a1, s1, s4
	mv	a0, s2	# call-arg-copy
	call	knapsack_mixed
	mv	t0, a0
	addw	t0, s3, t0
	blt	s5, t0, .knapsack_mixed.bb6
.knapsack_mixed.bb7:
.knapsack_mixed.bb1:
	mv	a0, s5
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 64
	ret
.knapsack_mixed.bb4:
	mv	a0, s2	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	call	knapsack_mixed
	mv	t0, a0
	mv	s5, t0	# phi-copy
	j	.knapsack_mixed.bb1
.knapsack_mixed.bb6:
	mv	s5, t0	# phi-copy
	j	.knapsack_mixed.bb1
.knapsack_mixed.bb2:
	li	s5, 0
	j	.knapsack_mixed.bb1
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
	la	t0, N
	sw	s1, 0(t0)
	call	getint
	mv	s2, a0
	la	t0, W
	sw	s2, 0(t0)
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
