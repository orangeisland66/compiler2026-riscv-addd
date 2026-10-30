
  .bss
  .globl x
  .align 2
  .type  x, @object
  .size  x, 62500000
x:
  .zero 62500000

  .bss
  .globl y
  .align 2
  .type  y, @object
  .size  y, 62500000
y:
  .zero 62500000

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -112
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	sd	s8, 80(sp)
	sd	s9, 88(sp)
	sd	s10, 96(sp)
	sd	s11, 104(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	li	t5, 13
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 0
.main.bb1:
	bge	t0, s1, .main.bb3
.main.bb2:
	li	t1, 250000
	li	t2, 0
	mulw	t1, t0, t1
.main.bb4:
	bge	t2, s1, .main.bb6
.main.bb5:
	li	t3, 1000
	li	a2, 0
	mulw	t3, t2, t3
	addw	t3, t1, t3
.main.bb23:
	addiw	s3, a2, 8
	bge	a2, s1, .main.bb9
.main.bb24:
	bge	s3, s1, .main.bb25
.main.bb26:
.main.bb27:
	slliw	a3, a2, 2
	la	a4, x
	addw	a3, t3, a3
	add	a3, a4, a3
.main.bb7:
	bge	a2, s3, .main.bb23
.main.bb8:
	li	a4, 1
	addiw	a2, a2, 1
	sw	a4, 0(a3)
	addi	a3, a3, 4
	blt	a2, s3, .main.bb8	# rotated-loop-branch
	j	.main.bb23	# rotated-loop-exit
.main.bb28:
	j	.main.bb23
.main.bb25:
	mv	s3, s1	# phi-copy
	j	.main.bb27
.main.bb9:
	addiw	t2, t2, 1
	j	.main.bb4
.main.bb17:
	lw	a6, 0(s11)
	addiw	a5, a5, 1
	lw	a7, 0(a3)
	addi	t4, s11, 4
	addw	a6, a6, a7
	addi	a3, a3, 4
	lw	a7, 0(s10)
	addw	a6, a6, a7
	lw	a7, 0(a2)
	addw	a6, a6, a7
	addi	a2, a2, 4
	lw	a7, 0(t3)
	addw	a6, a6, a7
	addi	t3, t3, 4
	lw	a7, 0(a4)
	addw	a6, a6, a7
	addi	a4, a4, 4
	lw	a7, 0(t2)
	addw	a6, a6, a7
	addi	t2, t2, 4
	divw	a6, a6, s2
	addi	s10, s10, 4
	sw	a6, 0(s9)
	addi	s9, s9, 4
	mv	s11, t4	# phi-copy
	blt	a5, s3, .main.bb17	# rotated-loop-branch
.main.bb18:
	addiw	t1, t1, 1
.main.bb13:
	bge	t1, s3, .main.bb15
.main.bb14:
	li	t2, 1000
	mulw	s7, t1, t2
	li	t2, 3
	bne	s2, t2, .main.bb22
.main.bb21:
	addw	t2, s4, s7
	la	s8, x
	addiw	t2, t2, 4
	addw	a3, s6, s7
	add	s9, s8, t2
	addiw	a3, a3, 4
	addi	s10, s9, -1000
	addw	t2, s5, s7
	addiw	t2, t2, 4
	addi	t3, s10, 996
	add	s11, s8, t2
	addi	a2, t3, 1004
	addi	t2, s11, -1004
	add	a3, s8, a3
	addi	a4, a2, -996
	li	a5, 1
.main.bb19:
	bge	a5, s3, .main.bb18
.main.bb20:
	lw	a6, 0(s11)
	addiw	a5, a5, 1
	lw	a7, 0(a3)
	addi	t4, s11, 4
	addw	a6, a6, a7
	addi	a3, a3, 4
	lw	a7, 0(s10)
	addw	a6, a6, a7
	lw	a7, 0(a2)
	addw	a6, a6, a7
	addi	a2, a2, 4
	lw	a7, 0(t3)
	addw	a6, a6, a7
	addi	t3, t3, 4
	lw	a7, 0(a4)
	addw	a6, a6, a7
	addi	a4, a4, 4
	lw	a7, 0(t2)
	addw	a6, a6, a7
	addi	t2, t2, 4
	li	a7, 1431655766
	mul	a6, a6, a7	# const-div.fullmul
	srai	a6, a6, 32	# const-div.mulhi
	srliw	a7, a6, 31	# const-div.sign
	addw	a6, a6, a7	# const-div.round
	sw	a6, 0(s9)
	addi	s10, s10, 4
	addi	s9, s9, 4
	mv	s11, t4	# phi-copy
	blt	a5, s3, .main.bb20	# rotated-loop-branch
	j	.main.bb18	# rotated-loop-exit
.main.bb22:
	addw	t2, s4, s7
	la	s8, x
	addiw	t2, t2, 4
	addw	a3, s6, s7
	add	s9, s8, t2
	addiw	a3, a3, 4
	addi	s10, s9, -1000
	addw	t2, s5, s7
	addiw	t2, t2, 4
	addi	t3, s10, 996
	add	s11, s8, t2
	addi	a2, t3, 1004
	addi	t2, s11, -1004
	add	a3, s8, a3
	addi	a4, a2, -996
	li	a5, 1
.main.bb16:
	bge	a5, s3, .main.bb18
	j	.main.bb17
.main.bb6:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	addiw	s3, s1, -1
	li	s5, 1
	li	s4, 1
.main.bb10:
	bge	s5, s3, .main.bb12
.main.bb11:
	li	t1, 250000
	addiw	t0, s5, 1
	mulw	s4, s5, t1
	li	t1, -250000
	addw	s5, s4, t1
	li	t1, 250000
	addw	s6, s4, t1
	li	t1, 1
	j	.main.bb13
.main.bb15:
	mv	s4, t1	# phi-copy
	mv	s5, t0	# phi-copy
	blt	s5, s3, .main.bb11	# rotated-loop-branch
.main.bb12:
	li	a0, 53
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	la	t5, x
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	srliw	t0, s1, 31	# const-div.pow2.bias-fold
	addw	t0, s1, t0
	sraiw	t0, t0, 1
	li	t1, 250000
	mulw	t1, t0, t1
	li	t2, 1000
	mulw	t0, t0, t2
	addw	t0, t1, t0
	la	s2, x
	add	t0, s2, t0
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	addiw	t0, s5, -1
	li	t1, 250000
	mulw	t0, t0, t1
	addiw	t1, s4, -1
	li	t2, 1000
	mulw	t1, t1, t2
	addw	t0, t0, t1
	add	t0, s2, t0
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	li	a0, 0
	ld	s11, 104(sp)
	ld	s10, 96(sp)
	ld	s9, 88(sp)
	ld	s8, 80(sp)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 112
	ret
	.size main, .-main
