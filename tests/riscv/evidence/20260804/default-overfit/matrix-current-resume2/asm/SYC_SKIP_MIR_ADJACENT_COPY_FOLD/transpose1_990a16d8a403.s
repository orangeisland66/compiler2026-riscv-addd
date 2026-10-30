
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
	addi	sp, sp, -112
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	sd	s9, 80(sp)
	sd	s10, 88(sp)
	sd	s11, 96(sp)
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
	addiw	s3, s2, -1
	slliw	t0, s3, 2
	la	t1, a
	add	t0, t1, t0
	li	t1, 0
	li	t2, 0
.main.bb1:
	bge	t1, s2, .main.bb3
.main.bb2:
	mv	s4, t1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t0	# phi-copy
.main.bb4:
	blt	s5, zero, .main.bb6
.main.bb5:
	lw	s7, 0(s6)
	divw	s8, s1, s7
	bge	zero, s8, .main.bb8
.main.bb7:
	li	t3, 1
	mv	s9, s8	# phi-copy
	mv	s10, s7	# phi-copy
.main.bb9:
	beq	t3, zero, .main.bb8
.main.bb10:
	remw	s11, s4, s8
	divw	a2, s4, s8
	slt	a4, s11, a2
	subw	a5, s11, s9
	slt	a3, a2, s7
	xori	a4, a4, 1
	sltiu	a5, a5, 1
	slt	a6, a2, s10
	and	a3, a3, a4
	and	a5, a5, a6
	slt	a4, s11, s9
	or	a4, a4, a5
	and	a3, a3, a4
	beq	a3, zero, .main.bb13
.main.bb12:
	mulw	a3, s11, s7
	addw	a3, a3, a2
	mv	s4, a3	# phi-copy
	mv	s9, s11	# phi-copy
	mv	s10, a2	# phi-copy
	j	.main.bb9
.main.bb14:
	j	.main.bb9
.main.bb13:
	li	t3, 0
	j	.main.bb9
.main.bb11:
.main.bb8:
	addiw	s5, s5, -1
	addi	a2, s6, -4
	mv	s6, a2	# phi-copy
	j	.main.bb4
.main.bb22:
	j	.main.bb8
.main.bb6:
	bge	s4, s1, .main.bb16
.main.bb15:
	sraiw	t3, s4, 31
	andi	s5, t3, 3
	addw	t3, s4, s5
	andi	t3, t3, 3
	subw	t3, t3, s5
	bne	t3, zero, .main.bb23
.main.bb18:
	li	t3, 4
	j	.main.bb17
.main.bb19:
.main.bb17:
	mulw	a2, t1, t1
	mulw	t3, a2, t3
	addiw	t1, t1, 1
	addw	t2, t2, t3
	blt	t1, s2, .main.bb2	# rotated-loop-branch
.main.bb3:
	bge	t2, zero, .main.bb24
.main.bb20:
	subw	t0, zero, t2
	mv	s1, t0	# phi-copy
.main.bb21:
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
	ld	s11, 96(sp)
	ld	s10, 88(sp)
	ld	s9, 80(sp)
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 112
	ret
.main.bb16:
	slliw	t3, s4, 2
	la	a2, matrix
	add	t3, a2, t3
	lw	t3, 0(t3)
	j	.main.bb17
.main.bb23:
	mv	t3, s4	# phi-copy
	j	.main.bb17
.main.bb24:
	mv	s1, t2	# phi-copy
	j	.main.bb21
	.size main, .-main
