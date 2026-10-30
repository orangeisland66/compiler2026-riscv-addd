
  .bss
  .globl left
  .align 2
  .type  left, @object
  .size  left, 64
left:
  .zero 64

  .bss
  .globl right
  .align 2
  .type  right, @object
  .size  right, 64
right:
  .zero 64

  .bss
  .globl output
  .align 2
  .type  output, @object
  .size  output, 64
output:
  .zero 64

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
	li	t1, 4
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s1, t0, 4
	li	t1, 0
.main.bb4:
	addiw	t2, t1, 1
	li	t3, 4
	bge	t2, t3, .main.bb6
.main.bb5:
	slliw	s2, t1, 2
	li	t2, 0
	li	t3, 0
	li	a2, 0
	li	a3, 4
	la	s3, left	# loop.addr.hoisted
	la	s4, right	# loop.addr.hoisted
.main.bb16:
	addiw	s5, t2, 2
	bge	t2, a3, .main.bb9
.main.bb17:
	li	a4, 4
	bge	s5, a4, .main.bb18
.main.bb19:
	mv	a4, s5	# phi-copy
	j	.main.bb7
.main.bb20:
.main.bb7:
	bge	t2, a4, .main.bb16
.main.bb28:
	addiw	a5, t2, 3
	blt	a5, a4, .main.bb29
.main.bb8:
	slliw	a5, t2, 2
	addw	a5, s1, a5
	add	a5, s3, a5
	lw	s5, 0(a5)
	slliw	a5, t2, 4
	addw	a5, a5, s2
	addiw	t2, t2, 1
	add	s6, s4, a5
	lw	a5, 0(s6)
	mulw	a5, s5, a5
	addw	a2, a2, a5
	lw	a5, 4(s6)
	mulw	a5, s5, a5
	addw	t3, t3, a5
	j	.main.bb7
.main.bb29:
	slliw	a5, t2, 2
	addw	a5, s1, a5
	add	s5, s3, a5
	lw	s6, 0(s5)
	slliw	a5, t2, 4
	addw	a5, a5, s2
	addiw	t2, t2, 4
	add	s7, s4, a5
	lw	a5, 0(s7)
	lw	a6, 4(s7)
	lw	a7, 16(s7)
	mulw	a5, s6, a5
	mulw	a6, s6, a6
	addw	a2, a2, a5
	lw	s6, 4(s5)
	addw	t3, t3, a6
	mulw	a7, s6, a7
	lw	a6, 48(s7)
	addw	a2, a2, a7
	lw	a7, 20(s7)
	lw	a5, 52(s7)
	mulw	a7, s6, a7
	lw	s6, 8(s5)
	addw	t3, t3, a7
	lw	a7, 32(s7)
	lw	s5, 12(s5)
	mulw	a7, s6, a7
	mulw	a6, s5, a6
	mulw	a5, s5, a5
	addw	a2, a2, a7
	lw	a7, 36(s7)
	addw	a2, a2, a6
	mulw	a7, s6, a7
	addw	t3, t3, a7
	addw	t3, t3, a5
	j	.main.bb7
.main.bb21:
	j	.main.bb16
.main.bb13:
	bge	t3, a4, .main.bb22
.main.bb30:
	addiw	a5, t3, 3
	blt	a5, a4, .main.bb31
.main.bb14:
	slliw	a5, t3, 2
	slliw	a6, t3, 4
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
	slliw	a6, t3, 4
	addw	a5, s1, a5
	addw	a6, a6, s2
	add	s5, s3, a5
	add	s6, s4, a6
	lw	a5, 0(s5)
	lw	a6, 0(s6)
	lw	t4, 16(s6)
	addiw	t3, t3, 4
	mulw	a5, a5, a6
	lw	a6, 4(s5)
	addw	a2, a2, a5
	mulw	a6, a6, t4
	lw	a5, 12(s5)
	lw	t4, 32(s6)
	addw	a2, a2, a6
	lw	a6, 8(s5)
	mulw	a6, a6, t4
	addw	a2, a2, a6
	lw	a6, 48(s6)
	mulw	a5, a5, a6
	addw	a2, a2, a5
	j	.main.bb13
.main.bb27:
.main.bb22:
	addiw	s5, t3, 2
	bge	t3, a3, .main.bb15
.main.bb23:
	li	a4, 4
	bge	s5, a4, .main.bb24
.main.bb25:
	mv	a4, s5	# phi-copy
	j	.main.bb13
.main.bb26:
	j	.main.bb13
.main.bb18:
	li	a4, 4
	j	.main.bb7
.main.bb24:
	li	a4, 4
	j	.main.bb13
.main.bb9:
	addw	t2, s1, s2
	la	a3, output
	add	s2, a3, t2
	addiw	t1, t1, 2
	sw	a2, 0(s2)
	sw	t3, 4(s2)
	j	.main.bb4
.main.bb15:
	sw	a2, 0(t2)
	addiw	t1, t1, 1
	addi	t2, t2, 4
.main.bb10:
	li	t3, 4
	bge	t1, t3, .main.bb12
.main.bb11:
	slliw	s2, t1, 2
	li	t3, 0
	li	a2, 0
	li	a3, 4
	la	s3, left	# loop.addr.hoisted
	la	s4, right	# loop.addr.hoisted
	j	.main.bb22
.main.bb6:
	slliw	t2, t1, 2
	la	t3, output
	addw	t2, s1, t2
	add	t2, t3, t2
	j	.main.bb10
.main.bb12:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	la	t0, output
	lw	a0, 20(t0)
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
