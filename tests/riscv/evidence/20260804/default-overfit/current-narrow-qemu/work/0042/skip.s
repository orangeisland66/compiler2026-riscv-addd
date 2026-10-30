
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
	addi	sp, sp, -96
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
	bge	zero, s3, .main.bb76
.main.bb75:
	mv	t0, s3
.main.bb77:
	la	s4, A	# loop.addr.hoisted
.main.bb11:
	bge	t0, s1, .main.bb13
.main.bb12:
	slliw	s5, t0, 12
	li	t1, 0
.main.bb14:
	bge	t1, s1, .main.bb16
.main.bb65:
	addiw	t2, t1, 3
	blt	t2, s1, .main.bb66
.main.bb15:
	slliw	t2, t1, 2
	li	t3, -1
	addw	t2, s5, t2
	addiw	t1, t1, 1
	add	t2, s4, t2
	sw	t3, 0(t2)
	j	.main.bb14
.main.bb41:
	bge	t3, s7, .main.bb53
.main.bb71:
	addiw	a3, t3, 3
	blt	a3, s7, .main.bb72
.main.bb42:
	slliw	a3, t3, 2
	slliw	a4, t3, 12
	addw	a3, s3, a3
	addw	a4, a4, s4
	add	a3, s6, a3
	add	a4, s5, a4
	lw	a3, 0(a3)
	addiw	t3, t3, 1
	lw	a4, 0(a4)
	mulw	a3, a3, a4
	addw	a2, a2, a3
	j	.main.bb41
.main.bb72:
	slliw	a3, t3, 2
	slliw	a4, t3, 12
	addw	a3, s3, a3
	addw	a4, a4, s4
	add	s8, s6, a3
	add	s9, s5, a4
	lw	a3, 0(s8)
	addiw	t3, t3, 4
	lw	a4, 0(s9)
	li	a6, 4096
	add	a6, s9, a6
	lw	a6, 0(a6)
	mulw	a3, a3, a4
	lw	a4, 4(s8)
	addw	a2, a2, a3
	mulw	a4, a4, a6
	lw	a3, 12(s8)
	addw	a2, a2, a4
	lw	a4, 8(s8)
	li	a6, 8192
	add	a6, s9, a6
	lw	a6, 0(a6)
	mulw	a4, a4, a6
	addw	a2, a2, a4
	li	a4, 12288
	add	a4, s9, a4
	lw	a4, 0(a4)
	mulw	a3, a3, a4
	addw	a2, a2, a3
	j	.main.bb41
.main.bb58:
.main.bb53:
	addiw	s7, t3, 32
	bge	t3, s1, .main.bb43
.main.bb54:
	bge	s7, s1, .main.bb55
.main.bb56:
	j	.main.bb41
.main.bb57:
	j	.main.bb41
.main.bb55:
	mv	s7, s1	# phi-copy
	j	.main.bb41
.main.bb26:
	bge	t1, s7, .main.bb59
.main.bb69:
	addiw	t2, t1, 3
	blt	t2, s7, .main.bb70
.main.bb27:
	slliw	t2, t1, 2
	addw	s8, s3, t2
	addiw	t1, t1, 1
	add	t3, s5, s8
	add	t2, s4, s8
	lw	t3, 0(t3)
	lw	t2, 0(t2)
	slliw	a2, t3, 1
	slliw	t2, t2, 1
	addw	t3, a2, t3
	addw	t2, t2, t3
	add	t3, s6, s8
	sw	t2, 0(t3)
	j	.main.bb26
.main.bb70:
	slliw	t2, t1, 2
	addw	s8, s3, t2
	addiw	t1, t1, 4
	add	s10, s5, s8
	add	s9, s4, s8
	lw	t3, 0(s10)
	add	s8, s6, s8
	lw	t2, 0(s9)
	slliw	a2, t3, 1
	slliw	t2, t2, 1
	addw	t3, a2, t3
	addw	t2, t2, t3
	sw	t2, 0(s8)
	lw	a3, 4(s10)
	lw	t3, 4(s9)
	slliw	a4, a3, 1
	slliw	t3, t3, 1
	addw	a3, a4, a3
	addw	t3, t3, a3
	sw	t3, 4(s8)
	lw	a4, 8(s10)
	lw	t3, 8(s9)
	slliw	a5, a4, 1
	slliw	t3, t3, 1
	addw	a4, a5, a4
	addw	t3, t3, a4
	sw	t3, 8(s8)
	lw	t2, 12(s9)
	lw	t3, 12(s10)
	slliw	t2, t2, 1
	slliw	a2, t3, 1
	addw	t3, a2, t3
	addw	t2, t2, t3
	sw	t2, 12(s8)
	j	.main.bb26
.main.bb64:
.main.bb59:
	addiw	s7, t1, 8
	bge	t1, s1, .main.bb28
.main.bb60:
	bge	s7, s1, .main.bb61
.main.bb62:
	j	.main.bb26
.main.bb63:
	j	.main.bb26
.main.bb50:
	bge	a2, s1, .main.bb52
.main.bb73:
	addiw	a3, a2, 3
	blt	a3, s1, .main.bb74
.main.bb51:
	slliw	a3, a2, 2
	addw	a3, s4, a3
	addiw	a2, a2, 1
	add	a3, s3, a3
	lw	a3, 0(a3)
	mulw	a3, a3, a3
	addw	t2, t2, a3
	j	.main.bb50
.main.bb74:
	slliw	a3, a2, 2
	addw	a3, s4, a3
	addiw	a2, a2, 4
	add	s5, s3, a3
	lw	a3, 0(s5)
	lw	a4, 4(s5)
	mulw	a3, a3, a3
	mulw	a4, a4, a4
	addw	t2, t2, a3
	addw	t2, t2, a4
	lw	a4, 8(s5)
	lw	a3, 12(s5)
	mulw	a4, a4, a4
	mulw	a3, a3, a3
	addw	t2, t2, a4
	addw	t2, t2, a3
	j	.main.bb50
.main.bb52:
	addiw	t3, t3, 1
.main.bb47:
	bge	t3, s1, .main.bb49
.main.bb48:
	slliw	s4, t3, 12
	li	a2, 0
	j	.main.bb50
.main.bb43:
	sw	a2, 0(t1)
	addiw	t2, t2, 1
	addi	t1, t1, 4
.main.bb38:
	bge	t2, s1, .main.bb40
.main.bb39:
	slliw	s4, t2, 2
	li	t3, 0
	li	a2, 0
	la	s5, A	# loop.addr.hoisted
	la	s6, C	# loop.addr.hoisted
	j	.main.bb53
.main.bb66:
	slliw	t2, t1, 2
	li	t3, -1
	addw	t2, s5, t2
	addiw	t1, t1, 4
	add	s6, s4, t2
	li	t2, -1
	sw	t3, 4(s6)
	sw	t2, 0(s6)
	sw	t3, 8(s6)
	sw	t2, 12(s6)
	j	.main.bb14
.main.bb61:
	mv	s7, s1	# phi-copy
	j	.main.bb26
.main.bb16:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb12	# rotated-loop-branch
.main.bb13:
	bge	s3, s1, .main.bb79
.main.bb78:
	mv	t0, s3	# phi-copy
.main.bb80:
	li	t1, 0
	la	s3, B	# loop.addr.hoisted
.main.bb17:
	bge	t1, t0, .main.bb19
.main.bb18:
	slliw	s4, t1, 12
	li	t2, 0
.main.bb20:
	bge	t2, s1, .main.bb22
.main.bb67:
	addiw	t3, t2, 3
	blt	t3, s1, .main.bb68
.main.bb21:
	slliw	t3, t2, 2
	li	a2, -1
	addw	t3, s4, t3
	addiw	t2, t2, 1
	add	t3, s3, t3
	sw	a2, 0(t3)
	j	.main.bb20
.main.bb68:
	slliw	t3, t2, 2
	li	a2, -1
	addw	t3, s4, t3
	addiw	t2, t2, 4
	add	s5, s3, t3
	li	t3, -1
	sw	a2, 4(s5)
	sw	t3, 0(s5)
	sw	a2, 8(s5)
	sw	t3, 12(s5)
	j	.main.bb20
.main.bb22:
	addiw	t1, t1, 1
	blt	t1, t0, .main.bb18	# rotated-loop-branch
.main.bb19:
	li	t0, 0
.main.bb23:
	bge	t0, s1, .main.bb25
.main.bb24:
	slliw	s3, t0, 12
	li	t1, 0
	la	s4, A	# loop.addr.hoisted
	la	s5, B	# loop.addr.hoisted
	la	s6, C	# loop.addr.hoisted
	j	.main.bb59
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
.main.bb35:
	bge	t0, s1, .main.bb37
.main.bb36:
	slliw	s3, t0, 12
	la	t1, A
	add	t1, t1, s3
	li	t2, 0
	j	.main.bb38
.main.bb40:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb36	# rotated-loop-branch
.main.bb37:
	li	t0, 0
	li	t1, 0
	slt	t0, t0, s2
	li	t2, 0
.main.bb44:
	bge	t1, t0, .main.bb46
.main.bb45:
	li	t3, 0
	la	s3, A	# loop.addr.hoisted
	j	.main.bb47
.main.bb49:
	addiw	t1, t1, 1
	blt	t1, t0, .main.bb45	# rotated-loop-branch
.main.bb46:
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
	addi	sp, sp, 96
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
.main.bb76:
	li	t0, 0
	j	.main.bb77
.main.bb79:
	mv	t0, s1
	j	.main.bb80
	.size main, .-main
