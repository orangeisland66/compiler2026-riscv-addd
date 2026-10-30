
  .section .rodata
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .word 1024

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
	call	getint
	mv	s1, a0
	li	s2, 0
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	slliw	s3, s2, 12
	li	s4, 0
.main.bb4:
	bge	s4, s1, .main.bb6
.main.bb5:
	call	getint
	mv	t0, a0
	slliw	t1, s4, 2
	addw	t1, s3, t1
	la	t2, A
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s4, s4, 1
	blt	s4, s1, .main.bb5	# rotated-loop-branch
.main.bb6:
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	s2, 0
.main.bb7:
	bge	s2, s1, .main.bb9
.main.bb8:
	slliw	s3, s2, 12
	li	s4, 0
.main.bb10:
	bge	s4, s1, .main.bb12
.main.bb11:
	call	getint
	mv	t0, a0
	slliw	t1, s4, 2
	addw	t1, s3, t1
	la	t2, B
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s4, s4, 1
	blt	s4, s1, .main.bb11	# rotated-loop-branch
.main.bb12:
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	li	t5, 65
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 0
.main.bb13:
	li	t1, 5
	bge	t0, t1, .main.bb15
.main.bb14:
	li	t1, 0
	la	s2, C	# loop.addr.hoisted
.main.bb22:
	bge	t1, s1, .main.bb24
.main.bb23:
	slliw	s3, t1, 12
	li	t2, 0
.main.bb25:
	bge	t2, s1, .main.bb27
.main.bb70:
	addiw	t3, t2, 3
	blt	t3, s1, .main.bb71
.main.bb26:
	slliw	t3, t2, 2
	li	a2, 0
	addw	t3, s3, t3
	addiw	t2, t2, 1
	add	t3, s2, t3
	sw	a2, 0(t3)
	j	.main.bb25
.main.bb36:
	bge	a2, s7, .main.bb56
.main.bb72:
	addiw	a3, a2, 15
	blt	a3, s7, .main.bb73
.main.bb37:
	slliw	s8, a2, 2
	addw	a3, s3, s8
	addw	a4, s2, s8
	add	s9, s6, a3
	add	a4, s5, a4
	lw	a3, 0(s9)
	addiw	a2, a2, 1
	lw	a4, 0(a4)
	mulw	a3, a3, s4
	addw	a3, a3, a4
	sw	a3, 0(s9)
	j	.main.bb36
.main.bb73:
	slliw	s8, a2, 2
	addw	a3, s3, s8
	addw	a4, s2, s8
	add	s9, s6, a3
	add	s8, s5, a4
	lw	a3, 0(s9)
	lw	a4, 0(s8)
	addiw	a2, a2, 16
	mulw	a3, a3, s4
	addw	a3, a3, a4
	lw	a4, 4(s9)
	sw	a3, 0(s9)
	lw	a6, 4(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 4(s9)
	lw	a4, 8(s9)
	lw	a6, 8(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 8(s9)
	lw	a4, 12(s9)
	lw	a6, 12(s8)
	lw	a3, 60(s9)
	mulw	a4, a4, s4
	mulw	a3, a3, s4
	addw	a4, a4, a6
	sw	a4, 12(s9)
	lw	a4, 16(s9)
	lw	a6, 16(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 16(s9)
	lw	a4, 20(s9)
	lw	a6, 20(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 20(s9)
	lw	a4, 24(s9)
	lw	a6, 24(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 24(s9)
	lw	a4, 28(s9)
	lw	a6, 28(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 28(s9)
	lw	a4, 32(s9)
	lw	a6, 32(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 32(s9)
	lw	a4, 36(s9)
	lw	a6, 36(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 36(s9)
	lw	a4, 40(s9)
	lw	a6, 40(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 40(s9)
	lw	a4, 44(s9)
	lw	a6, 44(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 44(s9)
	lw	a4, 48(s9)
	lw	a6, 48(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 48(s9)
	lw	a4, 52(s9)
	lw	a6, 52(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 52(s9)
	lw	a4, 56(s9)
	lw	a6, 56(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 56(s9)
	lw	a4, 60(s8)
	addw	a3, a3, a4
	sw	a3, 60(s9)
	j	.main.bb36
.main.bb61:
.main.bb56:
	addiw	s7, a2, 16
	bge	a2, s1, .main.bb38
.main.bb57:
	bge	s7, s1, .main.bb58
.main.bb59:
	j	.main.bb36
.main.bb60:
	j	.main.bb36
.main.bb58:
	mv	s7, s1	# phi-copy
	j	.main.bb36
.main.bb53:
	bge	a2, s7, .main.bb62
.main.bb76:
	addiw	a3, a2, 15
	blt	a3, s7, .main.bb77
.main.bb54:
	slliw	s8, a2, 2
	addw	a3, s3, s8
	addw	a4, s2, s8
	add	s9, s5, a3
	add	a4, s6, a4
	lw	a3, 0(s9)
	addiw	a2, a2, 1
	lw	a4, 0(a4)
	mulw	a3, a3, s4
	addw	a3, a3, a4
	sw	a3, 0(s9)
	j	.main.bb53
.main.bb77:
	slliw	s8, a2, 2
	addw	a3, s3, s8
	addw	a4, s2, s8
	add	s9, s5, a3
	add	s8, s6, a4
	lw	a3, 0(s9)
	lw	a4, 0(s8)
	addiw	a2, a2, 16
	mulw	a3, a3, s4
	addw	a3, a3, a4
	lw	a4, 4(s9)
	sw	a3, 0(s9)
	lw	a6, 4(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 4(s9)
	lw	a4, 8(s9)
	lw	a6, 8(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 8(s9)
	lw	a4, 12(s9)
	lw	a6, 12(s8)
	lw	a3, 60(s9)
	mulw	a4, a4, s4
	mulw	a3, a3, s4
	addw	a4, a4, a6
	sw	a4, 12(s9)
	lw	a4, 16(s9)
	lw	a6, 16(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 16(s9)
	lw	a4, 20(s9)
	lw	a6, 20(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 20(s9)
	lw	a4, 24(s9)
	lw	a6, 24(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 24(s9)
	lw	a4, 28(s9)
	lw	a6, 28(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 28(s9)
	lw	a4, 32(s9)
	lw	a6, 32(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 32(s9)
	lw	a4, 36(s9)
	lw	a6, 36(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 36(s9)
	lw	a4, 40(s9)
	lw	a6, 40(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 40(s9)
	lw	a4, 44(s9)
	lw	a6, 44(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 44(s9)
	lw	a4, 48(s9)
	lw	a6, 48(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 48(s9)
	lw	a4, 52(s9)
	lw	a6, 52(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 52(s9)
	lw	a4, 56(s9)
	lw	a6, 56(s8)
	mulw	a4, a4, s4
	addw	a4, a4, a6
	sw	a4, 56(s9)
	lw	a4, 60(s8)
	addw	a3, a3, a4
	sw	a3, 60(s9)
	j	.main.bb53
.main.bb67:
.main.bb62:
	addiw	s7, a2, 16
	bge	a2, s1, .main.bb55
.main.bb63:
	bge	s7, s1, .main.bb64
.main.bb65:
	j	.main.bb53
.main.bb66:
	j	.main.bb53
.main.bb64:
	mv	s7, s1	# phi-copy
	j	.main.bb53
.main.bb38:
	li	a2, 4096
	addiw	t3, t3, 1
	add	t2, t2, a2
.main.bb31:
	bge	t3, s1, .main.bb33
.main.bb32:
	lw	s4, 0(t2)
	slliw	s3, t3, 12
	li	a2, 1
	bne	s4, a2, .main.bb35
.main.bb34:
	li	a2, 4096
	addiw	t3, t3, 1
	add	t2, t2, a2
	j	.main.bb31
.main.bb55:
	li	a2, 4096
	addiw	t3, t3, 1
	add	t2, t2, a2
.main.bb48:
	bge	t3, s1, .main.bb50
.main.bb49:
	lw	s4, 0(t2)
	slliw	s3, t3, 12
	li	a2, 1
	bne	s4, a2, .main.bb52
.main.bb51:
	li	a2, 4096
	addiw	t3, t3, 1
	add	t2, t2, a2
	j	.main.bb48
.main.bb35:
	li	a2, 0
	la	s5, B	# loop.addr.hoisted
	la	s6, C	# loop.addr.hoisted
	j	.main.bb56
.main.bb52:
	li	a2, 0
	la	s5, B	# loop.addr.hoisted
	la	s6, C	# loop.addr.hoisted
	j	.main.bb62
.main.bb71:
	slliw	t3, t2, 2
	li	a2, 0
	addw	t3, s3, t3
	addiw	t2, t2, 4
	add	s4, s2, t3
	li	t3, 0
	sw	a2, 4(s4)
	sw	t3, 0(s4)
	sw	a2, 8(s4)
	sw	t3, 12(s4)
	j	.main.bb25
.main.bb27:
	addiw	t1, t1, 1
	j	.main.bb22
.main.bb33:
	addiw	t1, t1, 1
.main.bb28:
	bge	t1, s1, .main.bb30
.main.bb29:
	slliw	t2, t1, 2
	la	t3, A
	add	t2, t3, t2
	slliw	s2, t1, 12
	li	t3, 0
	j	.main.bb31
.main.bb50:
	addiw	t1, t1, 1
.main.bb45:
	bge	t1, s1, .main.bb47
.main.bb46:
	slliw	t2, t1, 2
	la	t3, A
	add	t2, t3, t2
	slliw	s2, t1, 12
	li	t3, 0
	j	.main.bb48
.main.bb42:
	bge	t2, s1, .main.bb44
.main.bb74:
	addiw	t3, t2, 3
	blt	t3, s1, .main.bb75
.main.bb43:
	slliw	t3, t2, 2
	li	a2, 0
	addw	t3, s3, t3
	addiw	t2, t2, 1
	add	t3, s2, t3
	sw	a2, 0(t3)
	j	.main.bb42
.main.bb75:
	slliw	t3, t2, 2
	li	a2, 0
	addw	t3, s3, t3
	addiw	t2, t2, 4
	add	s4, s2, t3
	li	t3, 0
	sw	a2, 4(s4)
	sw	t3, 0(s4)
	sw	a2, 8(s4)
	sw	t3, 12(s4)
	j	.main.bb42
.main.bb44:
	addiw	t1, t1, 1
.main.bb39:
	bge	t1, s1, .main.bb41
.main.bb40:
	slliw	s3, t1, 12
	li	t2, 0
	j	.main.bb42
.main.bb19:
	bge	t0, s1, .main.bb21
.main.bb68:
	addiw	t2, t0, 3
	blt	t2, s1, .main.bb69
.main.bb20:
	slliw	t2, t0, 2
	addw	t2, s4, t2
	addiw	t0, t0, 1
	add	t2, s2, t2
	lw	t2, 0(t2)
	addw	s3, s3, t2
	j	.main.bb19
.main.bb69:
	slliw	t2, t0, 2
	addw	t2, s4, t2
	addiw	t0, t0, 4
	add	s5, s2, t2
	lw	t2, 0(s5)
	lw	a2, 4(s5)
	lw	t3, 12(s5)
	addw	t2, s3, t2
	addw	t2, t2, a2
	lw	a2, 8(s5)
	addw	t2, t2, a2
	addw	s3, t2, t3
	j	.main.bb19
.main.bb21:
	addiw	t1, t1, 1
	bge	t1, s1, .main.bb18	# rotated-loop-branch
.main.bb17:
	slliw	s4, t1, 12
	li	t0, 0
	j	.main.bb19
.main.bb24:
	li	t1, 0
	j	.main.bb28
.main.bb30:
	li	t1, 0
	la	s2, B	# loop.addr.hoisted
	j	.main.bb39
.main.bb41:
	li	t1, 0
	j	.main.bb45
.main.bb47:
	addiw	t0, t0, 1
	j	.main.bb13
.main.bb15:
	li	s3, 0
	li	t1, 0
	la	s2, B	# loop.addr.hoisted
.main.bb16:
	blt	t1, s1, .main.bb17
.main.bb18:
	li	a0, 84
	call	_sysy_stoptime
	mv	t5, s3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
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
	.size main, .-main
