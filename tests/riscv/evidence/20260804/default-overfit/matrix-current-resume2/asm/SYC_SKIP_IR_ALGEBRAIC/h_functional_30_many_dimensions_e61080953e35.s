
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t6, -2097360
	add	sp, sp, t6
	li	t6, 2097304
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 2097312
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 2097320
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 2097328
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 2097336
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 2097344
	add	t6, sp, t6
	sd	s5, 0(t6)
	addi	t5, sp, 152
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
	addi	s1, sp, 152
	sw	t0, 0(s1)
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
	sd	t3, 80(sp)
.main.bb4:
	li	a2, 2
	ld	t5, 80(sp)
	bge	t5, a2, .main.bb6
.main.bb5:
	ld	t5, 80(sp)
	slliw	a2, t5, 19
	ld	t5, 24(sp)
	addw	t5, t5, a2
	sd	t5, 32(sp)
	li	a3, 0
	sd	a3, 104(sp)
.main.bb7:
	li	a4, 2
	ld	t5, 104(sp)
	bge	t5, a4, .main.bb9
.main.bb8:
	ld	t5, 104(sp)
	slliw	a4, t5, 18
	ld	t5, 32(sp)
	addw	t5, t5, a4
	sd	t5, 40(sp)
	li	a5, 0
	sd	a5, 128(sp)
.main.bb10:
	li	a6, 2
	ld	t5, 128(sp)
	bge	t5, a6, .main.bb12
.main.bb11:
	ld	t5, 128(sp)
	slliw	a6, t5, 17
	ld	t5, 40(sp)
	addw	t5, t5, a6
	sd	t5, 48(sp)
	li	a7, 0
	sd	a7, 144(sp)
.main.bb13:
	li	t4, 2
	ld	t5, 144(sp)
	bge	t5, t4, .main.bb15
.main.bb14:
	ld	t5, 144(sp)
	slliw	t4, t5, 16
	ld	t5, 48(sp)
	addw	t5, t5, t4
	sd	t5, 64(sp)
	li	a0, 0
.main.bb16:
	li	a1, 2
	bge	a0, a1, .main.bb18
.main.bb17:
	slliw	a1, a0, 15
	ld	t5, 64(sp)
	addw	t5, t5, a1
	sd	t5, 72(sp)
	li	a1, 0
.main.bb19:
	li	t2, 2
	bge	a1, t2, .main.bb21
.main.bb20:
	slliw	t2, a1, 14
	ld	t5, 72(sp)
	addw	t5, t5, t2
	sd	t5, 88(sp)
	li	a2, 0
.main.bb22:
	li	a4, 2
	bge	a2, a4, .main.bb24
.main.bb23:
	slliw	a4, a2, 13
	ld	t5, 88(sp)
	addw	t5, t5, a4
	sd	t5, 96(sp)
	li	a6, 0
.main.bb25:
	li	t1, 2
	bge	a6, t1, .main.bb27
.main.bb26:
	slliw	t1, a6, 12
	ld	t5, 96(sp)
	addw	t5, t5, t1
	sd	t5, 112(sp)
	li	t4, 0
.main.bb28:
	li	s2, 2
	bge	t4, s2, .main.bb30
.main.bb29:
	slliw	t3, t4, 11
	ld	t5, 112(sp)
	addw	t5, t5, t3
	sd	t5, 120(sp)
	li	t2, 0
.main.bb31:
	li	a4, 2
	bge	t2, a4, .main.bb33
.main.bb32:
	slliw	a4, t2, 10
	ld	t5, 120(sp)
	addw	t5, t5, a4
	sd	t5, 136(sp)
	li	a3, 0
.main.bb34:
	li	t1, 2
	bge	a3, t1, .main.bb36
.main.bb35:
	slliw	t1, a3, 9
	ld	t5, 136(sp)
	addw	t1, t5, t1
	li	t3, 0
.main.bb37:
	li	a5, 2
	bge	t3, a5, .main.bb39
.main.bb38:
	slliw	a5, t3, 8
	li	s3, 2
	addw	s2, t1, a5
	li	a5, 0
.main.bb40:
	bge	a5, s3, .main.bb42
.main.bb41:
	slliw	a4, a5, 5
	addiw	s5, t0, 1
	addw	a4, s2, a4
	add	s4, s1, a4
	sw	s5, 4(s4)
	addiw	a4, a5, 1
	addiw	s5, s5, 1
	sw	t0, 0(s4)
	sw	s5, 8(s4)
	addiw	s5, s5, 1
	sw	s5, 12(s4)
	addiw	s5, s5, 1
	sw	s5, 16(s4)
	addiw	s5, s5, 1
	sw	s5, 20(s4)
	addiw	s5, s5, 1
	sw	s5, 24(s4)
	addiw	s5, s5, 1
	sw	s5, 28(s4)
	addiw	t0, s5, 1
	mv	a5, a4	# phi-copy
	blt	a5, s3, .main.bb41	# rotated-loop-branch
.main.bb42:
	addiw	a5, s2, 64
	li	a4, 0
	li	s3, 2
.main.bb43:
	bge	a4, s3, .main.bb45
.main.bb44:
	slliw	a7, a4, 5
	addiw	s5, t0, 1
	addw	a7, a5, a7
	addiw	a4, a4, 1
	add	s4, s1, a7
	sw	s5, 4(s4)
	addiw	s5, s5, 1
	sw	t0, 0(s4)
	sw	s5, 8(s4)
	addiw	s5, s5, 1
	sw	s5, 12(s4)
	addiw	s5, s5, 1
	sw	s5, 16(s4)
	addiw	s5, s5, 1
	sw	s5, 20(s4)
	addiw	s5, s5, 1
	sw	s5, 24(s4)
	addiw	s5, s5, 1
	sw	s5, 28(s4)
	addiw	t0, s5, 1
	blt	a4, s3, .main.bb44	# rotated-loop-branch
.main.bb45:
	addiw	s2, s2, 128
	li	a4, 0
	li	a5, 2
.main.bb46:
	bge	a4, a5, .main.bb48
.main.bb47:
	slliw	a7, a4, 5
	addiw	s4, t0, 1
	addw	a7, s2, a7
	addiw	a4, a4, 1
	add	s3, s1, a7
	sw	s4, 4(s3)
	addiw	s4, s4, 1
	sw	t0, 0(s3)
	sw	s4, 8(s3)
	addiw	s4, s4, 1
	sw	s4, 12(s3)
	addiw	s4, s4, 1
	sw	s4, 16(s3)
	addiw	s4, s4, 1
	sw	s4, 20(s3)
	addiw	s4, s4, 1
	sw	s4, 24(s3)
	addiw	s4, s4, 1
	sw	s4, 28(s3)
	addiw	t0, s4, 1
	blt	a4, a5, .main.bb47	# rotated-loop-branch
.main.bb48:
	addiw	a4, s2, 64
	li	a5, 0
	li	s2, 2
.main.bb49:
	bge	a5, s2, .main.bb51
.main.bb50:
	slliw	a7, a5, 5
	addiw	s4, t0, 1
	addw	a7, a4, a7
	addiw	a5, a5, 1
	add	s3, s1, a7
	sw	s4, 4(s3)
	addiw	s4, s4, 1
	sw	t0, 0(s3)
	sw	s4, 8(s3)
	addiw	s4, s4, 1
	sw	s4, 12(s3)
	addiw	s4, s4, 1
	sw	s4, 16(s3)
	addiw	s4, s4, 1
	sw	s4, 20(s3)
	addiw	s4, s4, 1
	sw	s4, 24(s3)
	addiw	s4, s4, 1
	sw	s4, 28(s3)
	addiw	t0, s4, 1
	blt	a5, s2, .main.bb50	# rotated-loop-branch
.main.bb51:
	addiw	t3, t3, 1
	j	.main.bb37
.main.bb39:
	addiw	a3, a3, 1
	j	.main.bb34
.main.bb36:
	addiw	t2, t2, 1
	j	.main.bb31
.main.bb33:
	addiw	t4, t4, 1
	j	.main.bb28
.main.bb30:
	addiw	a6, a6, 1
	j	.main.bb25
.main.bb27:
	addiw	a2, a2, 1
	j	.main.bb22
.main.bb24:
	addiw	a1, a1, 1
	j	.main.bb19
.main.bb21:
	addiw	a0, a0, 1
	j	.main.bb16
.main.bb18:
	ld	t5, 144(sp)
	addiw	a7, t5, 1
	sd	a7, 144(sp)
	j	.main.bb13
.main.bb15:
	ld	t5, 128(sp)
	addiw	a5, t5, 1
	sd	a5, 128(sp)
	j	.main.bb10
.main.bb12:
	ld	t5, 104(sp)
	addiw	a3, t5, 1
	sd	a3, 104(sp)
	j	.main.bb7
.main.bb9:
	ld	t5, 80(sp)
	addiw	t3, t5, 1
	sd	t3, 80(sp)
	j	.main.bb4
.main.bb6:
	ld	t5, 56(sp)
	addiw	t1, t5, 1
	sd	t1, 56(sp)
	j	.main.bb1
.main.bb3:
	lw	t0, 0(s1)
	lw	t1, 8(s1)
	addw	t0, t0, t1
	lw	t1, 4(s1)
	addw	t0, t0, t1
	lw	t1, 40(s1)
	addw	t0, t0, t1
	lw	t1, 24(s1)
	addw	t0, t0, t1
	lw	t1, 228(s1)
	addw	t0, t0, t1
	lw	t1, 56(s1)
	addw	t0, t0, t1
	lw	t1, 964(s1)
	addw	t0, t0, t1
	lw	t1, 224(s1)
	addw	t0, t0, t1
	lw	t1, 804(s1)
	addw	t0, t0, t1
	lw	t1, 1996(s1)
	addw	t0, t0, t1
	addi	t1, s1, 1177
	addi	t1, t1, 2047
	lw	t1, 0(t1)
	addw	t0, t0, t1
	li	t1, 26400
	add	t1, s1, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	lw	t1, 508(s1)
	addw	t0, t0, t1
	lw	t1, 124(s1)
	addw	t0, t0, t1
	li	t1, 246672
	add	t1, s1, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	li	t1, 15956
	add	t1, s1, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	li	t1, 828364
	add	t1, s1, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	li	t1, 281600
	add	t1, s1, t1
	lw	t1, 0(t1)
	addw	a0, t0, t1
	call	putint
	li	a0, 0
	li	t6, 2097344
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 2097336
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 2097328
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 2097320
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 2097312
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 2097304
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 2097360
	add	sp, sp, t6
	ret
	.size main, .-main
