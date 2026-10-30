
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
	li	t6, -4208
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
	li	t6, 4184
	add	t6, sp, t6
	sd	s10, 0(t6)
	li	t6, 4192
	add	t6, sp, t6
	sd	s11, 0(t6)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	srliw	t0, s1, 31	# const-div.pow2.bias-fold
	addw	t0, s1, t0
	sraiw	s3, t0, 1
	addi	s4, sp, 8
	li	s5, 0
.main.bb1:
	bge	s5, s1, .main.bb3
.main.bb2:
	blt	s5, s3, .main.bb4
.main.bb5:
	addiw	s5, s5, 1
	blt	s5, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	s5, 0
.main.bb6:
	bge	s5, s1, .main.bb8
.main.bb7:
	bge	s5, s3, .main.bb9
.main.bb10:
	addiw	s5, s5, 1
	blt	s5, s1, .main.bb7	# rotated-loop-branch
.main.bb8:
	li	t5, 25
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	bge	zero, s3, .main.bb98
.main.bb97:
	mv	t0, s3
.main.bb99:
	la	s5, A	# loop.addr.hoisted
.main.bb11:
	bge	t0, s1, .main.bb13
.main.bb12:
	slliw	s6, t0, 12
	li	t1, 0
.main.bb14:
	bge	t1, s1, .main.bb16
.main.bb81:
	addiw	t2, t1, 3
	blt	t2, s1, .main.bb82
.main.bb15:
	slliw	t2, t1, 2
	li	t3, -1
	addw	t2, s6, t2
	addiw	t1, t1, 1
	add	t2, s5, t2
	sw	t3, 0(t2)
	j	.main.bb14
.main.bb26:
	bge	t1, s9, .main.bb75
.main.bb85:
	addiw	t2, t1, 3
	blt	t2, s9, .main.bb86
.main.bb27:
	slliw	t2, t1, 2
	addw	s10, s5, t2
	addiw	t1, t1, 1
	add	t3, s7, s10
	add	t2, s6, s10
	lw	t3, 0(t3)
	lw	t2, 0(t2)
	slliw	a2, t3, 1
	slliw	t2, t2, 1
	addw	t3, a2, t3
	addw	t2, t2, t3
	add	t3, s8, s10
	sw	t2, 0(t3)
	j	.main.bb26
.main.bb86:
	slliw	t2, t1, 2
	addw	s10, s5, t2
	addiw	t1, t1, 4
	add	t3, s7, s10
	add	s11, s6, s10
	lw	a2, 0(t3)
	add	s10, s8, s10
	lw	t2, 0(s11)
	slliw	a3, a2, 1
	slliw	t2, t2, 1
	addw	a2, a3, a2
	addw	t2, t2, a2
	sw	t2, 0(s10)
	lw	a4, 4(t3)
	lw	a2, 4(s11)
	slliw	a5, a4, 1
	slliw	a2, a2, 1
	addw	a4, a5, a4
	addw	a2, a2, a4
	sw	a2, 4(s10)
	lw	a5, 8(t3)
	lw	a2, 8(s11)
	slliw	a6, a5, 1
	slliw	a2, a2, 1
	addw	a5, a6, a5
	addw	a2, a2, a5
	sw	a2, 8(s10)
	lw	t3, 12(t3)
	lw	t2, 12(s11)
	slliw	a2, t3, 1
	slliw	t2, t2, 1
	addw	t3, a2, t3
	addw	t2, t2, t3
	sw	t2, 12(s10)
	j	.main.bb26
.main.bb80:
.main.bb75:
	addiw	s9, t1, 8
	bge	t1, s1, .main.bb28
.main.bb76:
	bge	s9, s1, .main.bb77
.main.bb78:
	j	.main.bb26
.main.bb79:
	j	.main.bb26
.main.bb42:
	bge	a2, s1, .main.bb44
.main.bb87:
	addiw	a3, a2, 3
	blt	a3, s1, .main.bb88
.main.bb43:
	slliw	a3, a2, 2
	addw	a3, s4, a3
	addiw	a2, a2, 1
	add	a3, s3, a3
	lw	a3, 0(a3)
	mulw	a3, a3, a3
	addw	t2, t2, a3
	j	.main.bb42
.main.bb88:
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
	j	.main.bb42
.main.bb44:
	addiw	t3, t3, 1
.main.bb39:
	bge	t3, s1, .main.bb41
.main.bb40:
	slliw	s4, t3, 12
	li	a2, 0
	j	.main.bb42
.main.bb50:
	bge	a2, s1, .main.bb52
.main.bb91:
	addiw	a3, a2, 15
	blt	a3, s1, .main.bb92
.main.bb51:
	slliw	s9, a2, 2
	addw	a4, s8, s9
	add	s10, s4, s9
	lw	a3, 0(s10)
	add	a4, s6, a4
	lw	a4, 0(a4)
	addiw	a2, a2, 1
	mulw	a4, s7, a4
	addw	a3, a3, a4
	sw	a3, 0(s10)
	j	.main.bb50
.main.bb92:
	slliw	s9, a2, 2
	add	s10, s4, s9
	addw	a4, s8, s9
	lw	a3, 0(s10)
	add	s9, s6, a4
	lw	a4, 0(s9)
	addiw	a2, a2, 16
	mulw	a4, s7, a4
	addw	a3, a3, a4
	lw	a4, 4(s10)
	sw	a3, 0(s10)
	lw	a6, 4(s9)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 4(s10)
	lw	a6, 8(s9)
	lw	a4, 8(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	lw	a3, 60(s10)
	sw	a4, 8(s10)
	lw	a6, 12(s9)
	lw	a4, 12(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 12(s10)
	lw	a6, 16(s9)
	lw	a4, 16(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 16(s10)
	lw	a6, 20(s9)
	lw	a4, 20(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 20(s10)
	lw	a6, 24(s9)
	lw	a4, 24(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 24(s10)
	lw	a6, 28(s9)
	lw	a4, 28(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 28(s10)
	lw	a6, 32(s9)
	lw	a4, 32(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 32(s10)
	lw	a6, 36(s9)
	lw	a4, 36(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 36(s10)
	lw	a6, 40(s9)
	lw	a4, 40(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 40(s10)
	lw	a6, 44(s9)
	lw	a4, 44(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 44(s10)
	lw	a6, 48(s9)
	lw	a4, 48(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 48(s10)
	lw	a6, 52(s9)
	lw	a4, 52(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 52(s10)
	lw	a6, 56(s9)
	lw	a4, 56(s10)
	mulw	a6, s7, a6
	addw	a4, a4, a6
	sw	a4, 56(s10)
	lw	a4, 60(s9)
	mulw	a4, s7, a4
	addw	a3, a3, a4
	sw	a3, 60(s10)
	j	.main.bb50
.main.bb52:
	addiw	t3, t3, 1
.main.bb48:
	bge	t3, t2, .main.bb53
.main.bb49:
	slliw	a2, t3, 2
	la	a3, C
	addw	a2, s5, a2
	slliw	s8, t3, 12
	add	a2, a3, a2
	lw	s7, 0(a2)
	li	a2, 0
	j	.main.bb50
.main.bb59:
	bge	t2, s1, .main.bb61
.main.bb95:
	addiw	t3, t2, 15
	blt	t3, s1, .main.bb96
.main.bb60:
	slliw	s11, t2, 2
	addw	a3, s10, s11
	add	t3, s4, s11
	lw	a2, 0(t3)
	add	a3, s7, a3
	lw	a3, 0(a3)
	addiw	t2, t2, 1
	mulw	a3, s9, a3
	addw	a2, a2, a3
	sw	a2, 0(t3)
	j	.main.bb59
.main.bb96:
	slliw	s11, t2, 2
	add	t3, s4, s11
	addw	a3, s10, s11
	lw	a2, 0(t3)
	add	s11, s7, a3
	lw	a3, 0(s11)
	addiw	t2, t2, 16
	mulw	a3, s9, a3
	addw	a2, a2, a3
	lw	a3, 4(t3)
	sw	a2, 0(t3)
	lw	a5, 4(s11)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 4(t3)
	lw	a5, 8(s11)
	lw	a3, 8(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	lw	a2, 60(t3)
	sw	a3, 8(t3)
	lw	a5, 12(s11)
	lw	a3, 12(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 12(t3)
	lw	a5, 16(s11)
	lw	a3, 16(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 16(t3)
	lw	a5, 20(s11)
	lw	a3, 20(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 20(t3)
	lw	a5, 24(s11)
	lw	a3, 24(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 24(t3)
	lw	a5, 28(s11)
	lw	a3, 28(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 28(t3)
	lw	a5, 32(s11)
	lw	a3, 32(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 32(t3)
	lw	a5, 36(s11)
	lw	a3, 36(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 36(t3)
	lw	a5, 40(s11)
	lw	a3, 40(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 40(t3)
	lw	a5, 44(s11)
	lw	a3, 44(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 44(t3)
	lw	a5, 48(s11)
	lw	a3, 48(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 48(t3)
	lw	a5, 52(s11)
	lw	a3, 52(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 52(t3)
	lw	a5, 56(s11)
	lw	a3, 56(t3)
	mulw	a5, s9, a5
	addw	a3, a3, a5
	sw	a3, 56(t3)
	lw	a3, 60(s11)
	mulw	a3, s9, a3
	addw	a2, a2, a3
	sw	a2, 60(t3)
	j	.main.bb59
.main.bb61:
	addiw	s8, s8, 1
.main.bb57:
	bge	s8, s3, .main.bb62
.main.bb58:
	slliw	t2, s8, 2
	la	t3, C
	addw	t2, s5, t2
	slliw	s10, s8, 12
	add	t2, t3, t2
	lw	s9, 0(t2)
	li	t2, 0
	j	.main.bb59
.main.bb82:
	slliw	t2, t1, 2
	li	t3, -1
	addw	t2, s6, t2
	addiw	t1, t1, 4
	add	s7, s5, t2
	li	t2, -1
	sw	t3, 4(s7)
	sw	t2, 0(s7)
	sw	t3, 8(s7)
	sw	t2, 12(s7)
	j	.main.bb14
.main.bb77:
	mv	s9, s1	# phi-copy
	j	.main.bb26
.main.bb16:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb12	# rotated-loop-branch
.main.bb13:
	bge	s3, s1, .main.bb101
.main.bb100:
	mv	t0, s3
.main.bb102:
	li	t1, 0
	la	s5, B	# loop.addr.hoisted
.main.bb17:
	bge	t1, t0, .main.bb19
.main.bb18:
	slliw	s6, t1, 12
	li	t2, 0
.main.bb20:
	bge	t2, s1, .main.bb22
.main.bb83:
	addiw	t3, t2, 3
	blt	t3, s1, .main.bb84
.main.bb21:
	slliw	t3, t2, 2
	li	a2, -1
	addw	t3, s6, t3
	addiw	t2, t2, 1
	add	t3, s5, t3
	sw	a2, 0(t3)
	j	.main.bb20
.main.bb84:
	slliw	t3, t2, 2
	li	a2, -1
	addw	t3, s6, t3
	addiw	t2, t2, 4
	add	s7, s5, t3
	li	t3, -1
	sw	a2, 4(s7)
	sw	t3, 0(s7)
	sw	a2, 8(s7)
	sw	t3, 12(s7)
	j	.main.bb20
.main.bb22:
	addiw	t1, t1, 1
	blt	t1, t0, .main.bb18	# rotated-loop-branch
.main.bb19:
	li	t0, 0
.main.bb23:
	bge	t0, s1, .main.bb25
.main.bb24:
	slliw	s5, t0, 12
	li	t1, 0
	la	s6, A	# loop.addr.hoisted
	la	s7, B	# loop.addr.hoisted
	la	s8, C	# loop.addr.hoisted
	j	.main.bb75
.main.bb46:
	bge	t3, s1, .main.bb70
.main.bb89:
	addiw	a2, t3, 3
	blt	a2, s1, .main.bb90
.main.bb47:
	slliw	a2, t3, 2
	li	a3, 0
	add	a2, s4, a2
	addiw	t3, t3, 1
	sw	a3, 0(a2)
	j	.main.bb46
.main.bb90:
	slliw	a2, t3, 2
	li	a3, 0
	add	s5, s4, a2
	addiw	t3, t3, 4
	li	a2, 0
	sw	a3, 4(s5)
	sw	a2, 0(s5)
	sw	a3, 8(s5)
	sw	a2, 12(s5)
	j	.main.bb46
.main.bb54:
	bge	t3, s1, .main.bb56
.main.bb93:
	addiw	a2, t3, 15
	blt	a2, s1, .main.bb94
.main.bb55:
	slliw	s7, t3, 2
	la	a4, A
	addw	a3, s5, s7
	add	s8, s4, s7
	lw	a2, 0(s8)
	add	a3, a4, a3
	lw	a3, 0(a3)
	addiw	t3, t3, 1
	mulw	a3, s6, a3
	addw	a2, a2, a3
	sw	a2, 0(s8)
	j	.main.bb54
.main.bb94:
	slliw	s7, t3, 2
	la	a4, A
	add	s8, s4, s7
	addw	a3, s5, s7
	lw	a2, 0(s8)
	add	s7, a4, a3
	lw	a3, 0(s7)
	addiw	t3, t3, 16
	mulw	a3, s6, a3
	addw	a2, a2, a3
	lw	a3, 4(s8)
	sw	a2, 0(s8)
	lw	a5, 4(s7)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 4(s8)
	lw	a5, 8(s7)
	lw	a3, 8(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	lw	a2, 60(s8)
	sw	a3, 8(s8)
	lw	a5, 12(s7)
	lw	a3, 12(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 12(s8)
	lw	a5, 16(s7)
	lw	a3, 16(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 16(s8)
	lw	a5, 20(s7)
	lw	a3, 20(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 20(s8)
	lw	a5, 24(s7)
	lw	a3, 24(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 24(s8)
	lw	a5, 28(s7)
	lw	a3, 28(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 28(s8)
	lw	a5, 32(s7)
	lw	a3, 32(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 32(s8)
	lw	a5, 36(s7)
	lw	a3, 36(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 36(s8)
	lw	a5, 40(s7)
	lw	a3, 40(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 40(s8)
	lw	a5, 44(s7)
	lw	a3, 44(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 44(s8)
	lw	a5, 48(s7)
	lw	a3, 48(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 48(s8)
	lw	a5, 52(s7)
	lw	a3, 52(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 52(s8)
	lw	a5, 56(s7)
	lw	a3, 56(s8)
	mulw	a5, s6, a5
	addw	a3, a3, a5
	sw	a3, 56(s8)
	lw	a3, 60(s7)
	mulw	a3, s6, a3
	addw	a2, a2, a3
	sw	a2, 60(s8)
	j	.main.bb54
.main.bb56:
	addiw	s6, t2, 1
	bge	t2, s3, .main.bb109
.main.bb71:
	la	s7, A	# loop.addr.hoisted
	mv	s8, s6	# phi-copy
	j	.main.bb57
.main.bb63:
	bge	s8, s1, .main.bb73
.main.bb103:
	addiw	t3, s8, 3
	blt	t3, s1, .main.bb104
.main.bb64:
	slliw	t3, s8, 2
	la	a2, C
	addw	t3, s5, t3
	add	t3, a2, t3
	lw	t3, 0(t3)
	addw	t2, t2, t3
	addiw	s8, s8, 1
	j	.main.bb63
.main.bb104:
	slliw	t3, s8, 2
	la	a2, C
	addw	t3, s5, t3
	add	s7, a2, t3
	lw	t3, 0(s7)
	lw	a2, 4(s7)
	addw	t2, t2, t3
	addw	t2, t2, a2
	lw	a2, 8(s7)
	lw	t3, 12(s7)
	addw	t2, t2, a2
	addw	t2, t2, t3
	addiw	s8, s8, 4
	j	.main.bb63
.main.bb65:
	bge	t3, s1, .main.bb74
.main.bb105:
	addiw	a2, t3, 3
	blt	a2, s1, .main.bb106
.main.bb66:
	lw	a2, 0(s7)
	addiw	t3, t3, 1
	subw	a2, a2, t2
	sw	a2, 0(s7)
	addi	s7, s7, 4
	j	.main.bb65
.main.bb106:
	lw	a3, 4(s7)
	addiw	t3, t3, 4
	lw	a2, 0(s7)
	subw	a3, a3, t2
	sw	a3, 4(s7)
	subw	a2, a2, t2
	lw	a3, 8(s7)
	sw	a2, 0(s7)
	addi	a2, s7, 4
	subw	a3, a3, t2
	sw	a3, 8(s7)
	lw	a3, 12(s7)
	addi	a2, a2, 12
	subw	a3, a3, t2
	sw	a3, 12(s7)
	mv	s7, a2	# phi-copy
	j	.main.bb65
.main.bb67:
	bge	t2, s1, .main.bb69
.main.bb107:
	addiw	t3, t2, 3
	blt	t3, s1, .main.bb108
.main.bb68:
	slliw	s7, t2, 2
	la	a3, A
	add	t3, s4, s7
	addw	a2, s5, s7
	lw	t3, 0(t3)
	add	a2, a3, a2
	addiw	t2, t2, 1
	sw	t3, 0(a2)
	j	.main.bb67
.main.bb108:
	slliw	s7, t2, 2
	la	a3, A
	add	s8, s4, s7
	addw	a2, s5, s7
	lw	t3, 0(s8)
	add	s7, a3, a2
	addiw	t2, t2, 4
	sw	t3, 0(s7)
	lw	a2, 4(s8)
	sw	a2, 4(s7)
	lw	a2, 8(s8)
	sw	a2, 8(s7)
	lw	t3, 12(s8)
	sw	t3, 12(s7)
	j	.main.bb67
.main.bb69:
	li	t2, 4100
	add	t0, t0, t2
	mv	t2, s6	# phi-copy
	bge	t2, s1, .main.bb35	# rotated-loop-branch
.main.bb72:
	li	t3, 0
	j	.main.bb46
.main.bb70:
	slliw	s5, t2, 12
	li	t3, 0
	la	s6, A	# loop.addr.hoisted
	j	.main.bb48
.main.bb73:
	li	t3, 0
	mv	s7, t1	# phi-copy
	j	.main.bb65
.main.bb74:
	li	t2, 0
	j	.main.bb67
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
	sltu	t0, zero, t0	# sel.mask
	subw	t0, zero, t0	# sel.negmask
	xori	t2, t0, -1	# sel.invmask
	and	t0, t1, t0	# sel.true
	and	t1, s1, t2	# sel.false
	or	t0, t0, t1
	slt	t2, s3, t0
	li	t1, 0
	xori	t2, t2, 1
	slt	t1, s3, t1
	sltu	t2, zero, t2	# sel.mask
	sltu	t1, zero, t1	# sel.mask
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
	addi	t1, sp, 8
.main.bb45:
	blt	t2, s1, .main.bb72
.main.bb35:
	li	t0, 0
	li	t1, 0
	slt	t0, t0, s2
	li	t2, 0
.main.bb36:
	bge	t1, t0, .main.bb38
.main.bb37:
	li	t3, 0
	la	s3, A	# loop.addr.hoisted
	j	.main.bb39
.main.bb41:
	addiw	t1, t1, 1
	blt	t1, t0, .main.bb37	# rotated-loop-branch
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
	li	t6, 4192
	add	t6, sp, t6
	ld	s11, 0(t6)
	li	t6, 4184
	add	t6, sp, t6
	ld	s10, 0(t6)
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
	li	t6, 4208
	add	sp, sp, t6
	ret
.main.bb53:
	lw	s6, 0(t0)
	li	t3, 0
	j	.main.bb54
.main.bb110:
.main.bb62:
	li	t2, 0
	j	.main.bb63
.main.bb4:
	slliw	t0, s5, 12
	la	t1, A
	add	t5, t1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	j	.main.bb5
.main.bb9:
	slliw	t0, s5, 12
	la	t1, B
	add	t5, t1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	j	.main.bb10
.main.bb109:
	mv	s8, s6	# phi-copy
	j	.main.bb62
.main.bb98:
	li	t0, 0
	j	.main.bb99
.main.bb101:
	mv	t0, s1
	j	.main.bb102
	.size main, .-main
