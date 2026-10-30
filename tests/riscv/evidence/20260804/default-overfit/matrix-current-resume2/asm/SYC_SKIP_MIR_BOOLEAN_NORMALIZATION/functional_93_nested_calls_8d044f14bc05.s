
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -144
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	sd	s2, 56(sp)
	sd	s3, 64(sp)
	sd	s4, 72(sp)
	sd	s5, 80(sp)
	sd	s6, 88(sp)
	sd	s7, 96(sp)
	sd	s8, 104(sp)
	sd	s9, 112(sp)
	sd	s10, 120(sp)
	sd	s11, 128(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	call	getint
	mv	s3, a0
	call	getint
	mv	s4, a0
	li	s6, 0
	li	s5, 10
.main.bb1:
	bge	s6, s5, .main.bb3
.main.bb2:
	call	getint
	mv	t0, a0
	slliw	t1, s6, 2
	addi	t2, sp, 0
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s6, s6, 1
	blt	s6, s5, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
	subw	t0, s1, t0
	sltiu	t0, t0, 1
.main.bb4:
	subw	t1, zero, s2
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	and	t0, t0, t1
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
.main.bb5:
	mv	s5, s3	# phi-copy
.main.bb18:
	beq	s5, zero, .main.bb20
.main.bb19:
	remw	t0, t0, s5
	li	s5, 0
	bne	s5, zero, .main.bb19	# rotated-loop-branch
.main.bb20:
	mv	t1, s4	# phi-copy
.main.bb36:
	beq	t1, zero, .main.bb37
.main.bb38:
	addw	t0, t0, t1
	li	t1, 0
	bne	t1, zero, .main.bb38	# rotated-loop-branch
.main.bb37:
	addi	t1, sp, 0
	lw	s5, 0(t1)
	lw	s6, 4(t1)
	addiw	t0, t0, 1
	lw	s7, 8(t1)
	lw	s8, 12(t1)
	subw	t0, zero, t0
	subw	t2, zero, s6
	li	t1, 0
	subw	t1, s8, t1
	sltiu	t1, t1, 1
.main.bb6:
	li	t3, 0
	li	a2, 0
	subw	t3, s7, t3
	subw	t1, t1, a2
	sltu	t3, zero, t3
	sltu	t1, zero, t1
	and	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.main.bb7:
	addi	t3, sp, 0
	lw	s9, 16(t3)
	lw	s10, 20(t3)
	li	t3, 0
	subw	t3, s10, t3
	sltiu	t3, t3, 1
.main.bb8:
	mv	s11, s9	# phi-copy
.main.bb21:
	beq	t3, zero, .main.bb23
.main.bb22:
	remw	s11, s11, t3
	li	t3, 0
	bne	t3, zero, .main.bb22	# rotated-loop-branch
.main.bb23:
	beq	t2, zero, .main.bb9
.main.bb10:
	mv	s11, t1	# phi-copy
.main.bb9:
	addi	t1, sp, 0
	lw	t1, 24(t1)
	mv	t2, t1	# phi-copy
.main.bb24:
	beq	t2, zero, .main.bb25
.main.bb26:
	addw	s11, s11, t2
	li	t2, 0
	bne	t2, zero, .main.bb26	# rotated-loop-branch
.main.bb25:
	addiw	t2, s11, 1
	addi	t3, sp, 0
	lw	s11, 28(t3)
	mv	t3, s11	# phi-copy
.main.bb39:
	beq	t3, zero, .main.bb41
.main.bb40:
	remw	t2, t2, t3
	li	t3, 0
	bne	t3, zero, .main.bb40	# rotated-loop-branch
.main.bb41:
	addi	t3, sp, 0
	li	a3, 0
	lw	a2, 32(t3)
	lw	t3, 36(t3)
	subw	a3, t3, a3
	sltiu	a3, a3, 1
.main.bb12:
	mv	a4, a3	# phi-copy
	mv	a3, a2	# phi-copy
.main.bb27:
	beq	a4, zero, .main.bb28
.main.bb29:
	addw	a3, a3, a4
	li	a4, 0
	bne	a4, zero, .main.bb29	# rotated-loop-branch
.main.bb28:
	addiw	a3, a3, 1
	mv	a4, s1	# phi-copy
.main.bb42:
	beq	a4, zero, .main.bb43
.main.bb44:
	subw	a3, a3, a4
	li	a4, 0
	bne	a4, zero, .main.bb44	# rotated-loop-branch
.main.bb43:
	mulw	t2, t2, a3
	beq	t0, zero, .main.bb13
.main.bb14:
	mv	t2, s5	# phi-copy
.main.bb13:
	li	t0, 0
	subw	t0, s3, t0
	sltiu	t0, t0, 1
	j	.main.bb30
.main.bb16:
.main.bb30:
	beq	s4, zero, .main.bb31
.main.bb32:
	addw	t0, t0, s4
	li	s4, 0
	bne	s4, zero, .main.bb32	# rotated-loop-branch
.main.bb31:
	addiw	t0, t0, 1
.main.bb45:
	beq	t0, zero, .main.bb51
.main.bb46:
	remw	s2, s2, t0
	li	t0, 0
	bne	t0, zero, .main.bb46	# rotated-loop-branch
	j	.main.bb51	# rotated-loop-exit
.main.bb47:
.main.bb51:
	beq	s2, zero, .main.bb52
.main.bb53:
	addw	t2, t2, s2
	li	s2, 0
	bne	s2, zero, .main.bb53	# rotated-loop-branch
.main.bb52:
	addiw	t0, t2, 1
.main.bb57:
	beq	s6, zero, .main.bb58
.main.bb59:
	subw	s5, s5, s6
	li	s6, 0
	bne	s6, zero, .main.bb59	# rotated-loop-branch
.main.bb58:
	mulw	t0, t0, s5
.main.bb63:
	beq	s7, zero, .main.bb65
.main.bb64:
	remw	t0, t0, s7
	li	s7, 0
	bne	s7, zero, .main.bb64	# rotated-loop-branch
.main.bb65:
	subw	t2, zero, s10
.main.bb66:
	beq	t2, zero, .main.bb67
.main.bb68:
	addw	s9, s9, t2
	li	t2, 0
	bne	t2, zero, .main.bb68	# rotated-loop-branch
.main.bb67:
	addiw	t2, s9, 1
	subw	t1, zero, t1
.main.bb69:
	beq	t1, zero, .main.bb71
.main.bb70:
	remw	t2, t2, t1
	li	t1, 0
	bne	t1, zero, .main.bb70	# rotated-loop-branch
.main.bb71:
	li	t1, 0
	subw	t1, a2, t1
	sltiu	t1, t1, 1
	j	.main.bb33
.main.bb17:
.main.bb33:
	beq	t1, zero, .main.bb34
.main.bb35:
	subw	s11, s11, t1
	li	t1, 0
	bne	t1, zero, .main.bb35	# rotated-loop-branch
.main.bb34:
	mulw	t1, t2, s11
	subw	t2, zero, t3
.main.bb48:
	beq	t2, zero, .main.bb54
.main.bb49:
	remw	t1, t1, t2
	li	t2, 0
	bne	t2, zero, .main.bb49	# rotated-loop-branch
	j	.main.bb54	# rotated-loop-exit
.main.bb50:
.main.bb54:
	beq	s1, zero, .main.bb55
.main.bb56:
	addw	t1, t1, s1
	li	s1, 0
	bne	s1, zero, .main.bb56	# rotated-loop-branch
.main.bb55:
	addiw	t1, t1, 1
.main.bb60:
	beq	t1, zero, .main.bb61
.main.bb62:
	subw	s8, s8, t1
	li	t1, 0
	bne	t1, zero, .main.bb62	# rotated-loop-branch
.main.bb61:
	mulw	a0, t0, s8
	ld	s11, 128(sp)
	ld	s10, 120(sp)
	ld	s9, 112(sp)
	ld	s8, 104(sp)
	ld	s7, 96(sp)
	ld	s6, 88(sp)
	ld	s5, 80(sp)
	ld	s4, 72(sp)
	ld	s3, 64(sp)
	ld	s2, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 144
	ret
.main.bb11:
	j	.main.bb9
.main.bb15:
	j	.main.bb13
	.size main, .-main
