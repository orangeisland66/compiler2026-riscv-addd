
  .bss
  .globl values
  .align 2
  .type  values, @object
  .size  values, 256
values:
  .zero 256

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	call	getint
	mv	s1, a0
	mulw	s2, s1, s1
	li	t0, 0
.main.bb1:
	bge	t0, s2, .main.bb3
.main.bb22:
	addiw	s3, t0, 3
	blt	s3, s2, .main.bb23
.main.bb2:
	addiw	s3, t0, 1
	la	t1, values
	slliw	t0, t0, 2
	add	t0, t1, t0
	sw	s3, 0(t0)
	mv	t0, s3	# phi-copy
	j	.main.bb1
.main.bb10:
	bge	t3, s5, .main.bb16
.main.bb26:
	addiw	s6, t3, 3
	blt	s6, s5, .main.bb27
.main.bb11:
	addw	a2, s3, t3
	slliw	a2, a2, 2
	addiw	t3, t3, 1
	add	s6, s4, a2
	lw	a2, 0(s6)
	subw	a2, a2, t2
	sw	a2, 0(s6)
	j	.main.bb10
.main.bb27:
	addw	a2, s3, t3
	slliw	a2, a2, 2
	add	s7, s4, a2
	lw	a2, 0(s7)
	subw	a2, a2, t2
	sw	a2, 0(s7)
	addiw	a2, t3, 1
	addw	a2, s3, a2
	slliw	a2, a2, 2
	add	s7, s4, a2
	lw	a2, 0(s7)
	subw	a2, a2, t2
	sw	a2, 0(s7)
	addiw	a2, t3, 2
	addw	a2, s3, a2
	addiw	t3, t3, 4
	slliw	a2, a2, 2
	add	s7, s4, a2
	lw	a2, 0(s7)
	subw	a2, a2, t2
	sw	a2, 0(s7)
	addw	a2, s3, s6
	slliw	a2, a2, 2
	add	s6, s4, a2
	lw	a2, 0(s6)
	subw	a2, a2, t2
	sw	a2, 0(s6)
	j	.main.bb10
.main.bb21:
.main.bb16:
	addiw	s5, t3, 8
	bge	t3, s1, .main.bb12
.main.bb17:
	bge	s5, s1, .main.bb18
.main.bb19:
	j	.main.bb10
.main.bb20:
	j	.main.bb10
.main.bb18:
	mv	s5, s1	# phi-copy
	j	.main.bb10
.main.bb7:
	bge	t3, s1, .main.bb9
.main.bb24:
	addiw	s4, t3, 3
	blt	s4, s1, .main.bb25
.main.bb8:
	addw	a2, s3, t3
	la	a3, values
	slliw	a2, a2, 2
	addiw	t3, t3, 1
	add	a2, a3, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	addw	t0, t0, a2
	j	.main.bb7
.main.bb25:
	addw	a2, s3, t3
	la	s5, values
	slliw	a2, a2, 2
	add	a2, s5, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	addw	t0, t0, a2
	addiw	a2, t3, 1
	addw	a2, s3, a2
	slliw	a2, a2, 2
	add	a2, s5, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	addw	t0, t0, a2
	addiw	a2, t3, 2
	addw	a2, s3, a2
	addiw	t3, t3, 4
	slliw	a2, a2, 2
	add	a2, s5, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	addw	t0, t0, a2
	addw	a2, s3, s4
	slliw	a2, a2, 2
	add	a2, s5, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	addw	t0, t0, a2
	j	.main.bb7
.main.bb9:
	li	t3, 0
	la	s4, values	# loop.addr.hoisted
	j	.main.bb16
.main.bb12:
	addiw	t1, t1, 1
	bge	t1, s1, .main.bb6	# rotated-loop-branch
.main.bb5:
	mulw	s3, t1, s1
	li	t2, 0
	li	t3, 0
	j	.main.bb7
.main.bb23:
	slliw	t2, t0, 2
	la	t3, values
	addiw	t1, t0, 1
	add	s4, t3, t2
	sw	t1, 0(s4)
	addiw	t1, t0, 2
	sw	s3, 8(s4)
	sw	t1, 4(s4)
	addiw	t0, t0, 4
	sw	t0, 12(s4)
	j	.main.bb1
.main.bb13:
	bge	t2, s2, .main.bb15
.main.bb28:
	addiw	t3, t2, 3
	blt	t3, s2, .main.bb29
.main.bb14:
	slliw	t3, t2, 2
	la	a2, values
	add	t3, a2, t3
	addiw	t2, t2, 1
	lw	t3, 0(t3)
	addw	t1, t1, t3
	j	.main.bb13
.main.bb29:
	slliw	t3, t2, 2
	la	a2, values
	add	s1, a2, t3
	addiw	t2, t2, 4
	lw	t3, 0(s1)
	lw	a2, 4(s1)
	addw	t1, t1, t3
	addw	t1, t1, a2
	lw	a2, 8(s1)
	lw	t3, 12(s1)
	addw	t1, t1, a2
	addw	t1, t1, t3
	j	.main.bb13
.main.bb3:
	li	t0, 0
	li	t1, 0
.main.bb4:
	blt	t1, s1, .main.bb5
.main.bb6:
	li	t1, 0
	li	t2, 0
	j	.main.bb13
.main.bb15:
	addw	a0, t1, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
