
  .bss
  .globl A
  .align 2
  .type  A, @object
  .size  A, 4194304
A:
  .zero 4194304

  .bss
  .globl B
  .align 2
  .type  B, @object
  .size  B, 4194304
B:
  .zero 4194304

  .bss
  .globl C
  .align 2
  .type  C, @object
  .size  C, 4194304
C:
  .zero 4194304

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t6, -4192
	add	sp, sp, t6
	li	t6, 4104
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 4112
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 4120
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 4128
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 4136
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 4144
	add	t6, sp, t6
	sd	s5, 0(t6)
	li	t6, 4152
	add	t6, sp, t6
	sd	s6, 0(t6)
	li	t6, 4160
	add	t6, sp, t6
	sd	s7, 0(t6)
	li	t6, 4168
	add	t6, sp, t6
	sd	s8, 0(t6)
	li	t6, 4176
	add	t6, sp, t6
	sd	s9, 0(t6)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	srliw	t0, s1, 31	# const-div.pow2.bias-fold
	addw	t0, s1, t0
	sraiw	s3, t0, 1
	li	s4, 0
.main.bb1:
	bge	s4, s1, .main.bb3
.main.bb2:
	blt	s4, s3, .main.bb4
.main.bb5:
	addiw	s4, s4, 1
	blt	s4, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	s4, 0
.main.bb6:
	bge	s4, s1, .main.bb8
.main.bb7:
	bge	s4, s3, .main.bb9
.main.bb10:
	addiw	s4, s4, 1
	blt	s4, s1, .main.bb7	# rotated-loop-branch
.main.bb8:
	li	t5, 25
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	bge	zero, s3, .main.bb82
.main.bb81:
	mv	t0, s3
	j	.main.bb11
.main.bb83:
.main.bb11:
	bge	t0, s1, .main.bb13
.main.bb12:
	slliw	t1, t0, 12
	la	t2, A
	add	t1, t2, t1
	li	t2, 0
.main.bb14:
	bge	t2, s1, .main.bb16
.main.bb15:
	li	t3, -1
	addiw	t2, t2, 1
	sw	t3, 0(t1)
	addi	t1, t1, 4
	blt	t2, s1, .main.bb15	# rotated-loop-branch
.main.bb16:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb12	# rotated-loop-branch
.main.bb13:
	bge	s3, s1, .main.bb85
.main.bb84:
	mv	t0, s3
.main.bb86:
	li	t1, 0
.main.bb17:
	bge	t1, t0, .main.bb19
.main.bb18:
	slliw	t2, t1, 12
	la	t3, B
	add	t2, t3, t2
	li	t3, 0
.main.bb20:
	bge	t3, s1, .main.bb22
.main.bb21:
	li	a2, -1
	addiw	t3, t3, 1
	sw	a2, 0(t2)
	addi	t2, t2, 4
	blt	t3, s1, .main.bb21	# rotated-loop-branch
.main.bb22:
	addiw	t1, t1, 1
	blt	t1, t0, .main.bb18	# rotated-loop-branch
.main.bb19:
	li	t0, 0
.main.bb23:
	bge	t0, s1, .main.bb25
.main.bb24:
	slliw	t1, t0, 12
	li	t2, 0
.main.bb75:
	addiw	s4, t2, 8
	bge	t2, s1, .main.bb28
.main.bb76:
	bge	s4, s1, .main.bb77
.main.bb78:
.main.bb79:
	slliw	t3, t2, 2
	la	a2, A
	addw	s5, t1, t3
	la	a3, B
	la	t3, C
	add	a2, a2, s5
	add	t3, t3, s5
	add	a3, a3, s5
.main.bb26:
	bge	t2, s4, .main.bb75
.main.bb27:
	lw	a5, 0(a3)
	addiw	t2, t2, 1
	lw	a4, 0(a2)
	slliw	a6, a5, 1
	addi	a3, a3, 4
	slliw	a4, a4, 1
	addw	a5, a6, a5
	addw	a4, a4, a5
	addi	a2, a2, 4
	sw	a4, 0(t3)
	addi	t3, t3, 4
	blt	t2, s4, .main.bb27	# rotated-loop-branch
	j	.main.bb75	# rotated-loop-exit
.main.bb80:
	j	.main.bb75
.main.bb77:
	mv	s4, s1	# phi-copy
	j	.main.bb79
.main.bb43:
	lw	a4, 0(a2)
	addiw	a3, a3, 1
	mulw	a4, a4, a4
	addi	a2, a2, 4
	addw	t2, t2, a4
	blt	a3, s1, .main.bb43	# rotated-loop-branch
.main.bb44:
	addiw	t3, t3, 1
.main.bb39:
	bge	t3, s1, .main.bb41
.main.bb40:
	slliw	a2, t3, 12
	la	a3, A
	add	a2, a3, a2
	li	a3, 0
.main.bb42:
	bge	a3, s1, .main.bb44
	j	.main.bb43
.main.bb51:
	lw	a5, 0(a2)
	addiw	a3, a3, 1
	lw	a4, 0(s6)
	mulw	a5, t3, a5
	addi	a2, a2, 4
	addw	a4, a4, a5
	sw	a4, 0(s6)
	addi	s6, s6, 4
	blt	a3, s1, .main.bb51	# rotated-loop-branch
.main.bb52:
	addiw	t2, t2, 1
.main.bb48:
	bge	t2, t1, .main.bb53
.main.bb49:
	slliw	t3, t2, 2
	la	a2, C
	addw	t3, s5, t3
	la	a3, A
	add	t3, a2, t3
	lw	t3, 0(t3)
	slliw	a2, t2, 12
	add	a2, a3, a2
	li	a3, 0
	mv	s6, s4	# phi-copy
.main.bb50:
	bge	a3, s1, .main.bb52
	j	.main.bb51
.main.bb60:
	lw	a3, 0(t2)
	addiw	t3, t3, 1
	lw	a2, 0(s9)
	mulw	a3, t1, a3
	addi	t2, t2, 4
	addw	a2, a2, a3
	sw	a2, 0(s9)
	addi	s9, s9, 4
	blt	t3, s1, .main.bb60	# rotated-loop-branch
.main.bb61:
	addiw	s8, s8, 1
.main.bb57:
	bge	s8, s3, .main.bb62
.main.bb58:
	slliw	t1, s8, 2
	la	t2, C
	addw	t1, s5, t1
	la	t3, A
	add	t1, t2, t1
	lw	t1, 0(t1)
	slliw	t2, s8, 12
	add	t2, t3, t2
	li	t3, 0
	mv	s9, s4	# phi-copy
.main.bb59:
	bge	t3, s1, .main.bb61
	j	.main.bb60
.main.bb28:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb24	# rotated-loop-branch
.main.bb25:
	li	t0, 0
.main.bb29:
	bge	t0, s1, .main.bb31
.main.bb30:
	slliw	t1, t0, 12
	la	t2, C
	add	t1, t2, t1
	li	t2, 0
.main.bb32:
	bge	t2, s1, .main.bb34
.main.bb33:
	lw	t3, 0(t1)
	addiw	t2, t2, 1
	li	a2, 1431655766
	mulw	t3, t3, t3
	addiw	t3, t3, 7
	mul	t3, t3, a2	# const-div.fullmul
	srai	t3, t3, 32	# const-div.mulhi
	srliw	a2, t3, 31	# const-div.sign
	addw	t3, t3, a2	# const-div.round
	sw	t3, 0(t1)
	addi	t1, t1, 4
	blt	t2, s1, .main.bb33	# rotated-loop-branch
.main.bb34:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb30	# rotated-loop-branch
.main.bb31:
	li	t0, 0
	li	t1, 0
	slt	t0, s1, t0
	addi	s4, sp, 8
	subw	t0, zero, t0	# sel.negmask
	xori	t2, t0, -1	# sel.invmask
	and	t0, t1, t0	# sel.true
	and	t1, s1, t2	# sel.false
	or	t0, t0, t1
	slt	t2, s3, t0
	li	t1, 0
	xori	t2, t2, 1
	slt	t1, s3, t1
	subw	t2, zero, t2	# sel.negmask
	subw	t1, zero, t1	# sel.negmask
	xori	t3, t2, -1	# sel.invmask
	and	t0, t0, t2	# sel.true
	and	t2, s3, t3	# sel.false
	or	t0, t0, t2
	xori	t3, t1, -1	# sel.invmask
	li	t2, 0
	and	t0, t0, t3	# sel.false
	and	t1, t2, t1	# sel.true
	or	s3, t1, t0
	la	t0, C
	li	t1, 0
.main.bb45:
	bge	t1, s1, .main.bb35
.main.bb72:
	li	t2, 0
	mv	s5, s4	# phi-copy
.main.bb46:
	bge	t2, s1, .main.bb70
.main.bb47:
	li	t3, 0
	addiw	t2, t2, 1
	sw	t3, 0(s5)
	addi	s5, s5, 4
	blt	t2, s1, .main.bb47	# rotated-loop-branch
.main.bb70:
	slliw	s5, t1, 12
	li	t2, 0
	j	.main.bb48
.main.bb41:
	addiw	t1, t1, 1
	bge	t1, t0, .main.bb38	# rotated-loop-branch
.main.bb37:
	li	t3, 0
	j	.main.bb39
.main.bb53:
	lw	t2, 0(t0)
	la	t3, A
	add	s6, t3, s5
	li	t3, 0
	mv	s7, s6	# phi-copy
	mv	s8, s4	# phi-copy
.main.bb54:
	bge	t3, s1, .main.bb56
.main.bb55:
	lw	a3, 0(s7)
	addiw	t3, t3, 1
	lw	a2, 0(s8)
	mulw	a3, t2, a3
	addw	a2, a2, a3
	sw	a2, 0(s8)
	addi	s7, s7, 4
	addi	s8, s8, 4
	blt	t3, s1, .main.bb55	# rotated-loop-branch
.main.bb56:
	addiw	s7, t1, 1
	bge	t1, s3, .main.bb87
.main.bb71:
	mv	s8, s7	# phi-copy
	j	.main.bb57
.main.bb88:
.main.bb62:
	slliw	t1, s8, 2
	la	t2, C
	addw	t1, s5, t1
	add	t1, t2, t1
	li	t2, 0
.main.bb63:
	bge	s8, s1, .main.bb73
.main.bb64:
	lw	t3, 0(t1)
	addw	t2, t2, t3
	addi	t1, t1, 4
	addiw	s8, s8, 1
	blt	s8, s1, .main.bb64	# rotated-loop-branch
.main.bb73:
	li	t1, 0
	mv	s5, s4	# phi-copy
.main.bb65:
	bge	t1, s1, .main.bb74
.main.bb66:
	lw	t3, 0(s5)
	addiw	t1, t1, 1
	subw	t3, t3, t2
	sw	t3, 0(s5)
	addi	s5, s5, 4
	blt	t1, s1, .main.bb66	# rotated-loop-branch
.main.bb74:
	li	t1, 0
	mv	s5, s4	# phi-copy
.main.bb67:
	bge	t1, s1, .main.bb69
.main.bb68:
	lw	t2, 0(s5)
	addiw	t1, t1, 1
	addi	t3, s6, 4
	sw	t2, 0(s6)
	addi	s5, s5, 4
	mv	s6, t3	# phi-copy
	blt	t1, s1, .main.bb68	# rotated-loop-branch
.main.bb69:
	li	t1, 4100
	add	t0, t0, t1
	mv	t1, s7	# phi-copy
	blt	t1, s1, .main.bb72	# rotated-loop-branch
.main.bb35:
	li	t0, 0
	li	t1, 0
	slt	t0, t0, s2
	li	t2, 0
.main.bb36:
	blt	t1, t0, .main.bb37
.main.bb38:
	mulw	s1, t2, s2
	li	a0, 105
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
	li	t6, 4176
	add	t6, sp, t6
	ld	s9, 0(t6)
	li	t6, 4168
	add	t6, sp, t6
	ld	s8, 0(t6)
	li	t6, 4160
	add	t6, sp, t6
	ld	s7, 0(t6)
	li	t6, 4152
	add	t6, sp, t6
	ld	s6, 0(t6)
	li	t6, 4144
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 4136
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 4128
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 4120
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 4112
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 4104
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 4192
	add	sp, sp, t6
	ret
.main.bb4:
	slliw	t0, s4, 12
	la	t1, A
	add	t5, t1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	j	.main.bb5
.main.bb9:
	slliw	t0, s4, 12
	la	t1, B
	add	t5, t1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	j	.main.bb10
.main.bb87:
	mv	s8, s7	# phi-copy
	j	.main.bb62
.main.bb82:
	li	t0, 0
	j	.main.bb11
.main.bb85:
	mv	t0, s1
	j	.main.bb86
	.size main, .-main
