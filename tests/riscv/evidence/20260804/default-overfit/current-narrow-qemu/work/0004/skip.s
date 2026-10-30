
  .bss
  .globl matrix
  .align 2
  .type  matrix, @object
  .size  matrix, 80000000
matrix:
  .zero 80000000

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
	addi	sp, sp, -64
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	call	getint
	mv	s1, a0
	la	t5, a
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s2, a0
	li	t5, 28
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 0
.main.bb20:
	addiw	s3, t0, 8
	bge	t0, s1, .main.bb3
.main.bb21:
	bge	s3, s1, .main.bb22
.main.bb23:
.main.bb24:
	slliw	t1, t0, 2
	la	t2, matrix
	add	t1, t2, t1
.main.bb1:
	bge	t0, s3, .main.bb20
.main.bb2:
	sraiw	t2, t0, 31
	sw	t0, 0(t1)
	andi	s4, t2, 3
	addw	t2, t0, s4
	andi	t2, t2, 3
	subw	t2, t2, s4
	beq	t2, zero, .main.bb4
.main.bb5:
	addiw	t0, t0, 1
	addi	t1, t1, 4
	j	.main.bb1
.main.bb4:
	li	t2, 4
	sw	t2, 0(t1)
	j	.main.bb5
.main.bb17:
	slt	a6, t3, a4
	slt	a5, a4, s3
	xori	a6, a6, 1
	and	a5, a5, a6
	beqz	a5, .main.bb18
.main.bb19:
	lw	a5, 0(a2)
	addiw	a4, a4, 1
	sw	a5, 0(a3)
	addi	a2, a2, 4
	add	a3, a3, t2
	j	.main.bb17
.main.bb18:
	addiw	t3, t3, 1
.main.bb14:
	bge	t3, s4, .main.bb16
.main.bb15:
	mulw	a2, t3, s3
	la	s5, matrix
	slliw	a3, t3, 2
	li	a4, 0
	add	a3, s5, a3
	slliw	a2, a2, 2
	add	a2, s5, a2
	j	.main.bb17
.main.bb16:
	addiw	t1, t1, 1
	addi	t0, t0, 4
	bge	t1, s2, .main.bb8	# rotated-loop-branch
.main.bb7:
	lw	s3, 0(t0)
	li	t3, 0
	divw	s4, s1, s3
	slliw	t2, s4, 2
	j	.main.bb14
.main.bb25:
	j	.main.bb20
.main.bb9:
	bge	t1, s2, .main.bb11
.main.bb26:
	mulw	s3, t1, t1
	addiw	s1, t1, 3
	blt	s1, s2, .main.bb27
.main.bb10:
	slliw	t2, t1, 2
	la	t3, matrix
	add	t2, t3, t2
	addiw	t1, t1, 1
	lw	t2, 0(t2)
	mulw	t2, s3, t2
	addw	t0, t0, t2
	j	.main.bb9
.main.bb27:
	slliw	t2, t1, 2
	la	t3, matrix
	add	s4, t3, t2
	lw	t2, 0(s4)
	lw	a2, 4(s4)
	lw	t3, 12(s4)
	mulw	t2, s3, t2
	addw	t0, t0, t2
	addiw	t2, t1, 1
	mulw	t2, t2, t2
	mulw	t2, t2, a2
	lw	a2, 8(s4)
	addw	t0, t0, t2
	addiw	t2, t1, 2
	mulw	t2, t2, t2
	addiw	t1, t1, 4
	mulw	t2, t2, a2
	addw	t0, t0, t2
	mulw	t2, s1, s1
	mulw	t2, t2, t3
	addw	t0, t0, t2
	j	.main.bb9
.main.bb22:
	mv	s3, s1	# phi-copy
	j	.main.bb24
.main.bb11:
	blt	t0, zero, .main.bb12
.main.bb28:
	mv	s1, t0	# phi-copy
.main.bb13:
	li	a0, 49
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
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
.main.bb8:
	li	t0, 0
	li	t1, 0
	j	.main.bb9
.main.bb3:
	la	t0, a
	li	t1, 0
.main.bb6:
	bge	t1, s2, .main.bb8
	j	.main.bb7
.main.bb12:
	subw	s1, zero, t0
	j	.main.bb13
	.size main, .-main
