
  .bss
  .globl left
  .align 2
  .type  left, @object
  .size  left, 4096
left:
  .zero 4096

  .bss
  .globl right
  .align 2
  .type  right, @object
  .size  right, 4096
right:
  .zero 4096

  .bss
  .globl result
  .align 2
  .type  result, @object
  .size  result, 4096
result:
  .zero 4096

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
	li	t0, 0
.main.bb1:
	li	t1, 32
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s1, t0, 7
	li	t1, 0
	li	t2, 32
	la	s2, left	# loop.addr.hoisted
	la	s3, right	# loop.addr.hoisted
.main.bb22:
	addiw	s4, t1, 8
	bge	t1, t2, .main.bb6
.main.bb23:
	li	t3, 32
	bge	s4, t3, .main.bb24
.main.bb25:
	mv	t3, s4	# phi-copy
	j	.main.bb4
.main.bb26:
.main.bb4:
	bge	t1, t3, .main.bb22
.main.bb28:
	addiw	s4, t1, 3
	blt	s4, t3, .main.bb29
.main.bb5:
	slliw	a3, t1, 2
	addw	a2, t0, t1
	addw	s4, s1, a3
	addiw	a2, a2, 1
	add	a3, s2, s4
	sw	a2, 0(a3)
	subw	a2, t0, t1
	add	a3, s3, s4
	sw	a2, 0(a3)
	addiw	t1, t1, 1
	j	.main.bb4
.main.bb29:
	slliw	a3, t1, 2
	addw	a2, t0, t1
	addw	s5, s1, a3
	addiw	a2, a2, 1
	add	s6, s2, s5
	addiw	s7, t1, 1
	sw	a2, 0(s6)
	add	s5, s3, s5
	subw	a2, t0, t1
	sw	a2, 0(s5)
	addw	a2, t0, s7
	addiw	a2, a2, 1
	sw	a2, 4(s6)
	subw	a2, t0, s7
	sw	a2, 4(s5)
	addiw	s7, t1, 2
	addw	a2, t0, s7
	addiw	t1, t1, 4
	addiw	a2, a2, 1
	sw	a2, 8(s6)
	subw	a2, t0, s7
	sw	a2, 8(s5)
	addw	a2, t0, s4
	addiw	a2, a2, 1
	sw	a2, 12(s6)
	subw	a2, t0, s4
	sw	a2, 12(s5)
	j	.main.bb4
.main.bb13:
	bge	t3, a4, .main.bb16
.main.bb30:
	addiw	a5, t3, 3
	blt	a5, a4, .main.bb31
.main.bb14:
	slliw	a5, t3, 2
	slliw	a6, t3, 7
	addw	a5, s1, a5
	addw	a6, a6, s2
	add	a5, s3, a5
	add	a6, s4, a6
	lw	a5, 0(a5)
	addiw	t3, t3, 1
	lw	a6, 0(a6)
	mulw	a5, a5, a6
	addw	a2, a2, a5
	j	.main.bb13
.main.bb31:
	slliw	a5, t3, 2
	slliw	a6, t3, 7
	addw	a5, s1, a5
	addw	a6, a6, s2
	add	s5, s3, a5
	add	s6, s4, a6
	lw	a5, 0(s5)
	lw	a6, 0(s6)
	lw	t4, 128(s6)
	addiw	t3, t3, 4
	mulw	a5, a5, a6
	lw	a6, 4(s5)
	addw	a2, a2, a5
	mulw	a6, a6, t4
	lw	a5, 12(s5)
	lw	t4, 256(s6)
	addw	a2, a2, a6
	lw	a6, 8(s5)
	mulw	a6, a6, t4
	addw	a2, a2, a6
	lw	a6, 384(s6)
	mulw	a5, a5, a6
	addw	a2, a2, a5
	j	.main.bb13
.main.bb21:
.main.bb16:
	addiw	s5, t3, 16
	bge	t3, a3, .main.bb15
.main.bb17:
	li	a4, 32
	bge	s5, a4, .main.bb18
.main.bb19:
	mv	a4, s5	# phi-copy
	j	.main.bb13
.main.bb20:
	j	.main.bb13
.main.bb18:
	li	a4, 32
	j	.main.bb13
.main.bb27:
	j	.main.bb22
.main.bb15:
	sw	a2, 0(t1)
	addiw	t2, t2, 1
	addi	t1, t1, 4
.main.bb10:
	li	t3, 32
	bge	t2, t3, .main.bb12
.main.bb11:
	slliw	s2, t2, 2
	li	t3, 0
	li	a2, 0
	li	a3, 32
	la	s3, left	# loop.addr.hoisted
	la	s4, right	# loop.addr.hoisted
	j	.main.bb16
.main.bb24:
	li	t3, 32
	j	.main.bb4
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb12:
	addiw	t0, t0, 1
.main.bb7:
	li	t1, 32
	bge	t0, t1, .main.bb9
.main.bb8:
	slliw	s1, t0, 7
	la	t1, result
	add	t1, t1, s1
	li	t2, 0
	j	.main.bb10
.main.bb3:
	li	t0, 0
	j	.main.bb7
.main.bb9:
	la	t0, result
	lw	a0, 404(t0)
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
