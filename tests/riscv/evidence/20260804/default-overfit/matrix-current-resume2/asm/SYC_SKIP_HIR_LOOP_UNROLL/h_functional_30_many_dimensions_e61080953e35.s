
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t6, -2097408
	add	sp, sp, t6
	li	t6, 2097360
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 2097368
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 2097376
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 2097384
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 2097392
	add	t6, sp, t6
	sd	s4, 0(t6)
	addi	t5, sp, 208
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	li	t5, 2097152
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	memset
	li	t0, 0
	addi	t5, sp, 208
	sw	t0, 0(t5)
	li	t1, 0
	sd	t1, 56(sp)
.main.bb1:
	li	t2, 2
	ld	t5, 56(sp)
	bge	t5, t2, .main.bb3
.main.bb2:
	ld	t5, 56(sp)
	slliw	t5, t5, 20
	sd	t5, 24(sp)
	li	t3, 0
	sd	t3, 72(sp)
.main.bb4:
	li	a2, 2
	ld	t5, 72(sp)
	bge	t5, a2, .main.bb6
.main.bb5:
	ld	t5, 72(sp)
	slliw	a2, t5, 19
	ld	t5, 24(sp)
	addw	t5, t5, a2
	sd	t5, 32(sp)
	li	a3, 0
	sd	a3, 96(sp)
.main.bb7:
	li	a4, 2
	ld	t5, 96(sp)
	bge	t5, a4, .main.bb9
.main.bb8:
	ld	t5, 96(sp)
	slliw	a4, t5, 18
	ld	t5, 32(sp)
	addw	t5, t5, a4
	sd	t5, 40(sp)
	li	a5, 0
	sd	a5, 112(sp)
.main.bb10:
	li	a6, 2
	ld	t5, 112(sp)
	bge	t5, a6, .main.bb12
.main.bb11:
	ld	t5, 112(sp)
	slliw	a6, t5, 17
	ld	t5, 40(sp)
	addw	t5, t5, a6
	sd	t5, 48(sp)
	li	a7, 0
	sd	a7, 136(sp)
.main.bb13:
	li	t4, 2
	ld	t5, 136(sp)
	bge	t5, t4, .main.bb15
.main.bb14:
	ld	t5, 136(sp)
	slliw	t4, t5, 16
	ld	t5, 48(sp)
	addw	t5, t5, t4
	sd	t5, 64(sp)
	li	a0, 0
	sd	a0, 152(sp)
.main.bb16:
	li	a1, 2
	ld	t5, 152(sp)
	bge	t5, a1, .main.bb18
.main.bb17:
	ld	t5, 152(sp)
	slliw	a1, t5, 15
	ld	t5, 64(sp)
	addw	t5, t5, a1
	sd	t5, 80(sp)
	li	a1, 0
	sd	a1, 168(sp)
.main.bb19:
	li	t2, 2
	ld	t5, 168(sp)
	bge	t5, t2, .main.bb21
.main.bb20:
	ld	t5, 168(sp)
	slliw	t2, t5, 14
	ld	t5, 80(sp)
	addw	t5, t5, t2
	sd	t5, 88(sp)
	li	a2, 0
	sd	a2, 184(sp)
.main.bb22:
	li	a4, 2
	ld	t5, 184(sp)
	bge	t5, a4, .main.bb24
.main.bb23:
	ld	t5, 184(sp)
	slliw	a4, t5, 13
	ld	t5, 88(sp)
	addw	t5, t5, a4
	sd	t5, 104(sp)
	li	a6, 0
	sd	a6, 192(sp)
.main.bb25:
	li	t1, 2
	ld	t5, 192(sp)
	bge	t5, t1, .main.bb27
.main.bb26:
	ld	t5, 192(sp)
	slliw	t1, t5, 12
	ld	t5, 104(sp)
	addw	t5, t5, t1
	sd	t5, 120(sp)
	li	t4, 0
.main.bb28:
	li	t3, 2
	bge	t4, t3, .main.bb30
.main.bb29:
	slliw	t3, t4, 11
	ld	t5, 120(sp)
	addw	t5, t5, t3
	sd	t5, 128(sp)
	li	s2, 0
.main.bb31:
	li	t2, 2
	bge	s2, t2, .main.bb33
.main.bb32:
	slliw	t2, s2, 10
	ld	t5, 128(sp)
	addw	t5, t5, t2
	sd	t5, 144(sp)
	li	a3, 0
.main.bb34:
	li	a4, 2
	bge	a3, a4, .main.bb36
.main.bb35:
	slliw	a4, a3, 9
	ld	t5, 144(sp)
	addw	t5, t5, a4
	sd	t5, 160(sp)
	li	a5, 0
.main.bb37:
	li	t1, 2
	bge	a5, t1, .main.bb39
.main.bb38:
	slliw	t1, a5, 8
	ld	t5, 160(sp)
	addw	t5, t5, t1
	sd	t5, 176(sp)
	li	t3, 0
.main.bb40:
	li	a7, 2
	bge	t3, a7, .main.bb42
.main.bb41:
	slliw	a7, t3, 7
	ld	t5, 176(sp)
	addw	t5, t5, a7
	sd	t5, 200(sp)
	li	t2, 0
.main.bb43:
	li	a0, 2
	bge	t2, a0, .main.bb45
.main.bb44:
	slliw	a0, t2, 6
	ld	t5, 200(sp)
	addw	s3, t5, a0
	li	a0, 0
.main.bb46:
	li	a4, 2
	bge	a0, a4, .main.bb48
.main.bb47:
	slliw	a4, a0, 5
	li	a1, 0
	addw	a4, s3, a4
.main.bb49:
	li	t1, 2
	bge	a1, t1, .main.bb51
.main.bb50:
	slliw	t1, a1, 4
	li	a2, 0
	addw	t1, a4, t1
.main.bb52:
	li	a6, 2
	bge	a2, a6, .main.bb54
.main.bb53:
	slliw	a6, a2, 3
	addi	t5, sp, 208
	addw	a6, t1, a6
	li	s1, 0
	add	a6, t5, a6
	li	s4, 2
.main.bb55:
	bge	s1, s4, .main.bb57
.main.bb56:
	sw	t0, 0(a6)
	addiw	s1, s1, 1
	addiw	t0, t0, 1
	addi	a6, a6, 4
	blt	s1, s4, .main.bb56	# rotated-loop-branch
.main.bb57:
	addiw	a2, a2, 1
	j	.main.bb52
.main.bb54:
	addiw	a1, a1, 1
	j	.main.bb49
.main.bb51:
	addiw	a0, a0, 1
	j	.main.bb46
.main.bb48:
	addiw	t2, t2, 1
	j	.main.bb43
.main.bb45:
	addiw	t3, t3, 1
	j	.main.bb40
.main.bb42:
	addiw	a5, a5, 1
	j	.main.bb37
.main.bb39:
	addiw	a3, a3, 1
	j	.main.bb34
.main.bb36:
	addiw	s2, s2, 1
	j	.main.bb31
.main.bb33:
	addiw	t4, t4, 1
	j	.main.bb28
.main.bb30:
	ld	t5, 192(sp)
	addiw	t1, t5, 1
	sd	t1, 192(sp)
	j	.main.bb25
.main.bb27:
	ld	t5, 184(sp)
	addiw	t1, t5, 1
	sd	t1, 184(sp)
	j	.main.bb22
.main.bb24:
	ld	t5, 168(sp)
	addiw	t2, t5, 1
	sd	t2, 168(sp)
	j	.main.bb19
.main.bb21:
	ld	t5, 152(sp)
	addiw	t2, t5, 1
	sd	t2, 152(sp)
	j	.main.bb16
.main.bb18:
	ld	t5, 136(sp)
	addiw	a7, t5, 1
	sd	a7, 136(sp)
	j	.main.bb13
.main.bb15:
	ld	t5, 112(sp)
	addiw	a5, t5, 1
	sd	a5, 112(sp)
	j	.main.bb10
.main.bb12:
	ld	t5, 96(sp)
	addiw	a3, t5, 1
	sd	a3, 96(sp)
	j	.main.bb7
.main.bb9:
	ld	t5, 72(sp)
	addiw	t3, t5, 1
	sd	t3, 72(sp)
	j	.main.bb4
.main.bb6:
	ld	t5, 56(sp)
	addiw	t1, t5, 1
	sd	t1, 56(sp)
	j	.main.bb1
.main.bb3:
	addi	t5, sp, 208
	lw	t0, 0(t5)
	addi	t5, sp, 208
	lw	t1, 8(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 4(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 40(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 24(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 228(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 56(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 964(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 224(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 804(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 1996(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	addi	t1, t5, 1177
	addi	t1, t1, 2047
	lw	t1, 0(t1)
	addw	t0, t0, t1
	li	t1, 26400
	addi	t5, sp, 208
	add	t1, t5, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 508(t5)
	addw	t0, t0, t1
	addi	t5, sp, 208
	lw	t1, 124(t5)
	addw	t0, t0, t1
	li	t1, 246672
	addi	t5, sp, 208
	add	t1, t5, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	li	t1, 15956
	addi	t5, sp, 208
	add	t1, t5, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	li	t1, 828364
	addi	t5, sp, 208
	add	t1, t5, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	li	t1, 281600
	addi	t5, sp, 208
	add	t1, t5, t1
	lw	t1, 0(t1)
	addw	a0, t0, t1
	call	putint
	li	a0, 0
	li	t6, 2097392
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 2097384
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 2097376
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 2097368
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 2097360
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 2097408
	add	sp, sp, t6
	ret
	.size main, .-main
