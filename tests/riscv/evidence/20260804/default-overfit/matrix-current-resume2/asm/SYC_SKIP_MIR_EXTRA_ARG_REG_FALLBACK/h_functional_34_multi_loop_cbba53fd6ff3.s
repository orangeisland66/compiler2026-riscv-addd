
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	s1, 48(sp)
	sd	s2, 56(sp)
	li	t0, 0
	li	t1, 0
	sd	t0, 0(sp)
.main.bb1:
	li	t2, 1
	ld	t5, 0(sp)
	bge	t5, t2, .main.bb3
.main.bb2:
	li	t2, 0
	sd	t2, 8(sp)
.main.bb4:
	li	t3, 1
	ld	t5, 8(sp)
	bge	t5, t3, .main.bb6
.main.bb5:
	li	t3, 0
	sd	t3, 16(sp)
.main.bb7:
	li	a2, 2
	ld	t5, 16(sp)
	bge	t5, a2, .main.bb9
.main.bb8:
	li	a2, 0
	sd	a2, 24(sp)
.main.bb10:
	li	a3, 3
	ld	t5, 24(sp)
	bge	t5, a3, .main.bb12
.main.bb11:
	li	a3, 0
	sd	a3, 32(sp)
.main.bb13:
	li	a4, 5
	ld	t5, 32(sp)
	bge	t5, a4, .main.bb15
.main.bb14:
	li	a4, 0
	sd	a4, 40(sp)
.main.bb16:
	li	a5, 4
	ld	t5, 40(sp)
	bge	t5, a5, .main.bb18
.main.bb17:
	li	a5, 0
.main.bb19:
	li	a6, 6
	bge	a5, a6, .main.bb21
.main.bb20:
	li	a6, 0
.main.bb22:
	li	a7, 5
	bge	a6, a7, .main.bb24
.main.bb23:
	li	a7, 0
.main.bb25:
	li	t4, 5
	bge	a7, t4, .main.bb27
.main.bb26:
	li	t4, 0
.main.bb28:
	li	t0, 3
	bge	t4, t0, .main.bb30
.main.bb29:
	li	t0, 0
.main.bb31:
	li	t2, 2
	bge	t0, t2, .main.bb33
.main.bb32:
	li	t2, 0
.main.bb34:
	li	t3, 5
	bge	t2, t3, .main.bb36
.main.bb35:
	li	t3, 0
.main.bb37:
	li	a2, 5
	bge	t3, a2, .main.bb39
.main.bb38:
	li	a2, 0
.main.bb40:
	li	a3, 3
	bge	a2, a3, .main.bb42
.main.bb41:
	li	a3, 0
	li	s1, 6
.main.bb43:
	bge	a3, s1, .main.bb45
.main.bb44:
	addiw	s2, t1, 3
	addiw	a3, a3, 3
	li	t1, -2093742815
	mul	t1, s2, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s2	# const-div.fixup
	sraiw	t1, t1, 9	# const-div.shift
	srliw	a4, t1, 31	# const-div.sign
	addw	t1, t1, a4	# const-div.round
	li	a4, 999
	mulw	t1, t1, a4	# const-rem.mul
	subw	t1, s2, t1	# const-rem.sub
	blt	a3, s1, .main.bb44	# rotated-loop-branch
.main.bb45:
	addiw	a2, a2, 1
	j	.main.bb40
.main.bb42:
	addiw	t3, t3, 2
	j	.main.bb37
.main.bb39:
	addiw	t2, t2, 2
	j	.main.bb34
.main.bb36:
	addiw	t0, t0, 2
	j	.main.bb31
.main.bb33:
	addiw	t4, t4, 1
	j	.main.bb28
.main.bb30:
	addiw	a7, a7, 1
	j	.main.bb25
.main.bb27:
	addiw	a6, a6, 1
	j	.main.bb22
.main.bb24:
	addiw	a5, a5, 1
	j	.main.bb19
.main.bb21:
	ld	t5, 40(sp)
	addiw	a4, t5, 1
	sd	a4, 40(sp)
	j	.main.bb16
.main.bb18:
	ld	t5, 32(sp)
	addiw	a3, t5, 1
	sd	a3, 32(sp)
	j	.main.bb13
.main.bb15:
	ld	t5, 24(sp)
	addiw	a2, t5, 1
	sd	a2, 24(sp)
	j	.main.bb10
.main.bb12:
	ld	t5, 16(sp)
	addiw	t3, t5, 1
	sd	t3, 16(sp)
	j	.main.bb7
.main.bb9:
	ld	t5, 8(sp)
	addiw	t2, t5, 1
	sd	t2, 8(sp)
	j	.main.bb4
.main.bb6:
	ld	t5, 0(sp)
	addiw	t0, t5, 1
	sd	t0, 0(sp)
	j	.main.bb1
.main.bb3:
	mv	a0, t1
	ld	s2, 56(sp)
	ld	s1, 48(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main
