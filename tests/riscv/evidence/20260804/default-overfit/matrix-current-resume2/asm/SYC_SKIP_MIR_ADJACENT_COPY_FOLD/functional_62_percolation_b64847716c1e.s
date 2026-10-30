
  .bss
  .globl array
  .align 2
  .type  array, @object
  .size  array, 440
array:
  .zero 440

  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .text

  .globl findfa
  .type  findfa, @function
findfa:
.findfa.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	mv	t0, a0
	slliw	t1, t0, 2
	la	t2, array
	add	s1, t2, t1
	lw	t1, 0(s1)
	beq	t1, t0, .findfa.bb1
	j	.findfa.bb3
.findfa.bb2:
.findfa.bb1:
	mv	a0, t0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.findfa.bb3:
	mv	a0, t1	# call-arg-copy
	call	findfa
	mv	t0, a0
	sw	t0, 0(s1)
	j	.findfa.bb1
	.size findfa, .-findfa

  .globl mmerge
  .type  mmerge, @function
mmerge:
.mmerge.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	mv	t0, a0
	mv	s1, a1
	mv	a0, t0	# call-arg-copy
	call	findfa
	mv	s2, a0
	mv	a0, s1	# call-arg-copy
	call	findfa
	mv	t0, a0
	bne	s2, t0, .mmerge.bb1
.mmerge.bb2:
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.mmerge.bb1:
	slliw	t1, s2, 2
	la	t2, array
	add	t1, t2, t1
	sw	t0, 0(t1)
	j	.mmerge.bb2
	.size mmerge, .-mmerge

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
	la	t0, array
	addi	s1, t0, 4
	li	t0, 1
	mv	s2, t0	# phi-copy
.main.bb1:
	beq	s2, zero, .main.bb3
.main.bb2:
	li	t0, 4
	la	t1, n
	sw	t0, 0(t1)
	li	t0, 1
	li	t1, 17
	mv	s3, s1	# phi-copy
.main.bb27:
	blt	t1, t0, .main.bb29
.main.bb28:
	li	t2, -1
	addiw	t0, t0, 1
	sw	t2, 0(s3)
	addi	t2, s3, 4
	mv	s3, t2	# phi-copy
	bge	t1, t0, .main.bb28	# rotated-loop-branch
.main.bb29:
	la	t0, n
	la	t1, array
	lw	t0, 0(t0)
	li	s5, 10
	mulw	t0, t0, t0
	addiw	s3, t0, 1
	slliw	t0, s3, 2
	add	s4, t1, t0
	li	s7, 0
	li	t1, 0
	mv	s6, t1	# phi-copy
.main.bb4:
	bge	s6, s5, .main.bb6
.main.bb5:
	call	getint
	mv	s8, a0
	call	getint
	mv	s9, a0
	bne	s7, zero, .main.bb30
.main.bb7:
	la	t0, n
	addiw	t1, s8, -1
	lw	t0, 0(t0)
	mulw	t0, t0, t1
	addw	s10, t0, s9
	la	t1, array
	slliw	t0, s10, 2
	add	t0, t1, t0
	sw	s10, 0(t0)
	li	t0, 1
	beq	s8, t0, .main.bb9
.main.bb10:
	la	t0, n
	lw	t0, 0(t0)
	beq	s8, t0, .main.bb11
.main.bb12:
	addiw	s11, s10, 1
	la	t2, array
	slliw	t1, s11, 2
	la	t0, n
	lw	t0, 0(t0)
	add	t1, t2, t1
	lw	t1, 0(t1)
	li	t2, -1
	slt	t0, s9, t0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	and	t0, t0, t1
	bne	t0, zero, .main.bb13
.main.bb14:
	li	t0, 1
	la	t2, array
	slt	t0, t0, s9
	addiw	s9, s10, -1
	slliw	t1, s9, 2
	add	t1, t2, t1
	lw	t1, 0(t1)
	li	t2, -1
	subw	t1, t1, t2
	sltu	t1, zero, t1
	and	t0, t0, t1
	bne	t0, zero, .main.bb15
.main.bb16:
	la	t0, n
	la	t2, array
	lw	t0, 0(t0)
	slt	t1, s8, t0
	addw	s9, s10, t0
	slliw	t0, s9, 2
	add	t0, t2, t0
	lw	t0, 0(t0)
	li	t2, -1
	subw	t0, t0, t2
	sltu	t0, zero, t0
	and	t0, t1, t0
	bne	t0, zero, .main.bb17
.main.bb18:
	la	t1, n
	li	t0, 1
	lw	t1, 0(t1)
	slt	t0, t0, s8
	la	t2, array
	subw	s8, s10, t1
	slliw	t1, s8, 2
	add	t1, t2, t1
	lw	t1, 0(t1)
	li	t2, -1
	subw	t1, t1, t2
	sltu	t1, zero, t1
	and	t0, t0, t1
	bne	t0, zero, .main.bb19
.main.bb20:
	la	t0, array
	li	t1, -1
	lw	t0, 0(t0)
	li	t2, -1
	subw	t0, t0, t1
	lw	t1, 0(s4)
	sltu	t0, zero, t0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	and	s8, t0, t1
	beqz	s8, .main.bb31
.main.bb22:
	li	t5, 0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	findfa
	mv	s8, a0
	mv	t5, s3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	findfa
	mv	t0, a0
	subw	t0, s8, t0
	sltiu	t0, t0, 1
.main.bb21:
	beq	t0, zero, .main.bb32
.main.bb23:
	addiw	t5, s6, 1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 1
	j	.main.bb8
.main.bb24:
.main.bb8:
	addiw	t1, s6, 1
	mv	s6, t1	# phi-copy
	mv	s7, t0	# phi-copy
	j	.main.bb4
.main.bb11:
	sw	s3, 0(s4)
	mv	t5, s10
	sd	t5, 0(sp)
	mv	t5, s3
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	mmerge
	j	.main.bb12
.main.bb13:
	mv	t5, s10
	sd	t5, 0(sp)
	mv	t5, s11
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	mmerge
	j	.main.bb14
.main.bb15:
	mv	t5, s10
	sd	t5, 0(sp)
	mv	t5, s9
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	mmerge
	j	.main.bb16
.main.bb17:
	mv	t5, s10
	sd	t5, 0(sp)
	mv	t5, s9
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	mmerge
	j	.main.bb18
.main.bb19:
	mv	t5, s10
	sd	t5, 0(sp)
	mv	t5, s8
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	mmerge
	j	.main.bb20
.main.bb31:
	mv	t0, s8	# phi-copy
	j	.main.bb21
.main.bb32:
	mv	t0, s7	# phi-copy
	j	.main.bb8
.main.bb9:
	li	t0, 0
	la	t1, array
	sw	t0, 0(t1)
	mv	t5, s10
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	mmerge
	j	.main.bb10
.main.bb30:
	mv	t0, s7	# phi-copy
	j	.main.bb8
.main.bb6:
	beq	s7, zero, .main.bb25
.main.bb26:
	addiw	t0, s2, -1
	mv	s2, t0	# phi-copy
	bne	s2, zero, .main.bb2	# rotated-loop-branch
.main.bb3:
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
.main.bb25:
	li	t5, -1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb26
	.size main, .-main
