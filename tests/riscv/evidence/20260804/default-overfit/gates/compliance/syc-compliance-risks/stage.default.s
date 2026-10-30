
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
.main.bb25:
	addiw	t1, t0, 3
	blt	t1, s2, .main.bb26
.main.bb2:
	slliw	t1, t0, 2
	la	t2, values
	add	t1, t2, t1
	addiw	t0, t0, 1
	li	t2, 5
	sw	t2, 0(t1)
	j	.main.bb1
.main.bb13:
	bge	t2, s5, .main.bb19
.main.bb31:
	addiw	s6, t2, 3
	blt	s6, s5, .main.bb32
.main.bb14:
	addw	t3, s3, t2
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	s6, s4, t3
	lw	t3, 0(s6)
	subw	t3, t3, t1
	sw	t3, 0(s6)
	j	.main.bb13
.main.bb32:
	addw	t3, s3, t2
	slliw	t3, t3, 2
	add	s7, s4, t3
	lw	t3, 0(s7)
	subw	t3, t3, t1
	sw	t3, 0(s7)
	addiw	t3, t2, 1
	addw	t3, s3, t3
	slliw	t3, t3, 2
	add	s7, s4, t3
	lw	t3, 0(s7)
	subw	t3, t3, t1
	sw	t3, 0(s7)
	addiw	t3, t2, 2
	addw	t3, s3, t3
	addiw	t2, t2, 4
	slliw	t3, t3, 2
	add	s7, s4, t3
	lw	t3, 0(s7)
	subw	t3, t3, t1
	sw	t3, 0(s7)
	addw	t3, s3, s6
	slliw	t3, t3, 2
	add	s6, s4, t3
	lw	t3, 0(s6)
	subw	t3, t3, t1
	sw	t3, 0(s6)
	j	.main.bb13
.main.bb24:
.main.bb19:
	addiw	s5, t2, 8
	bge	t2, s1, .main.bb15
.main.bb20:
	bge	s5, s1, .main.bb21
.main.bb22:
	j	.main.bb13
.main.bb23:
	j	.main.bb13
.main.bb21:
	mv	s5, s1	# phi-copy
	j	.main.bb13
.main.bb10:
	bge	t2, s1, .main.bb12
.main.bb29:
	addiw	s4, t2, 3
	blt	s4, s1, .main.bb30
.main.bb11:
	addw	t3, s3, t2
	la	a2, values
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, a2, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	j	.main.bb10
.main.bb30:
	addw	t3, s3, t2
	la	s5, values
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addiw	t3, t2, 1
	addw	t3, s3, t3
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addiw	t3, t2, 2
	addw	t3, s3, t3
	addiw	t2, t2, 4
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addw	t3, s3, s4
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	j	.main.bb10
.main.bb12:
	li	t2, 0
	la	s4, values	# loop.addr.hoisted
	j	.main.bb19
.main.bb15:
	addiw	t0, t0, 1
	bge	t0, s1, .main.bb9	# rotated-loop-branch
.main.bb8:
	mulw	s3, t0, s1
	li	t1, 0
	li	t2, 0
	j	.main.bb10
.main.bb26:
	slliw	t1, t0, 2
	la	t2, values
	add	s3, t2, t1
	addiw	t0, t0, 4
	li	t1, 5
	li	t2, 5
	sw	t1, 0(s3)
	sw	t2, 4(s3)
	sw	t2, 8(s3)
	sw	t1, 12(s3)
	j	.main.bb1
.main.bb4:
	bge	t0, s1, .main.bb6
.main.bb27:
	addiw	s3, t0, 3
	blt	s3, s1, .main.bb28
.main.bb5:
	addiw	s3, t0, 1
	la	t1, values
	slliw	t0, t0, 2
	add	t0, t1, t0
	sw	s3, 0(t0)
	mv	t0, s3	# phi-copy
	j	.main.bb4
.main.bb28:
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
	j	.main.bb4
.main.bb16:
	bge	t1, s2, .main.bb18
.main.bb33:
	addiw	t2, t1, 3
	blt	t2, s2, .main.bb34
.main.bb17:
	slliw	t2, t1, 2
	la	t3, values
	add	t2, t3, t2
	addiw	t1, t1, 1
	lw	t2, 0(t2)
	addw	t0, t0, t2
	j	.main.bb16
.main.bb34:
	slliw	t2, t1, 2
	la	t3, values
	add	s1, t3, t2
	addiw	t1, t1, 4
	lw	t2, 0(s1)
	lw	t3, 4(s1)
	addw	t0, t0, t2
	addw	t0, t0, t3
	lw	t3, 8(s1)
	lw	t2, 12(s1)
	addw	t0, t0, t3
	addw	t0, t0, t2
	j	.main.bb16
.main.bb3:
	li	t0, 0
	j	.main.bb4
.main.bb6:
	li	t0, 0
.main.bb7:
	blt	t0, s1, .main.bb8
.main.bb9:
	li	t0, 0
	li	t1, 0
	j	.main.bb16
.main.bb18:
	mv	a0, t0
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
