
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
	addi	sp, sp, -64
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	li	t0, 0
.main.bb1:
	li	t1, 4
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s1, t0, 4
	li	t1, 0
	li	t2, 0
	li	t3, 4
	la	s2, left	# loop.addr.hoisted
	la	s3, right	# loop.addr.hoisted
.main.bb7:
	addiw	s4, t1, 2
	bge	t1, t3, .main.bb6
.main.bb8:
	li	a2, 4
	bge	s4, a2, .main.bb9
.main.bb10:
	mv	a2, s4	# phi-copy
	j	.main.bb4
.main.bb11:
.main.bb4:
	bge	t1, a2, .main.bb7
.main.bb13:
	addiw	a3, t1, 3
	blt	a3, a2, .main.bb14
.main.bb5:
	slliw	a3, t1, 2
	slliw	a4, t1, 4
	addw	a3, s1, a3
	add	a4, s3, a4
	lw	a4, 0(a4)
	add	a3, s2, a3
	lw	a3, 0(a3)
	addiw	t1, t1, 1
	mulw	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb4
.main.bb14:
	slliw	a3, t1, 2
	slliw	a4, t1, 4
	addw	a3, s1, a3
	add	s5, s3, a4
	lw	a4, 0(s5)
	add	s4, s2, a3
	lw	a3, 0(s4)
	lw	a6, 16(s5)
	addiw	t1, t1, 4
	mulw	a3, a3, a4
	lw	a4, 4(s4)
	addw	t2, t2, a3
	mulw	a4, a4, a6
	lw	a3, 12(s4)
	lw	a6, 32(s5)
	addw	t2, t2, a4
	lw	a4, 8(s4)
	mulw	a4, a4, a6
	addw	t2, t2, a4
	lw	a4, 48(s5)
	mulw	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb4
.main.bb12:
	j	.main.bb7
.main.bb15:
	bge	t1, a2, .main.bb18
.main.bb24:
	addiw	a3, t1, 3
	blt	a3, a2, .main.bb25
.main.bb16:
	slliw	a3, t1, 2
	slliw	a4, t1, 4
	addw	a3, s1, a3
	addiw	a4, a4, 4
	add	a3, s2, a3
	add	a4, s3, a4
	lw	a3, 0(a3)
	addiw	t1, t1, 1
	lw	a4, 0(a4)
	mulw	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb15
.main.bb25:
	slliw	a3, t1, 2
	slliw	a4, t1, 4
	addw	a3, s1, a3
	addiw	a4, a4, 4
	add	s4, s2, a3
	add	s5, s3, a4
	lw	a3, 0(s4)
	lw	a4, 0(s5)
	lw	a6, 16(s5)
	addiw	t1, t1, 4
	mulw	a3, a3, a4
	lw	a4, 4(s4)
	addw	t2, t2, a3
	mulw	a4, a4, a6
	lw	a3, 12(s4)
	lw	a6, 32(s5)
	addw	t2, t2, a4
	lw	a4, 8(s4)
	mulw	a4, a4, a6
	addw	t2, t2, a4
	lw	a4, 48(s5)
	mulw	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb15
.main.bb23:
.main.bb18:
	addiw	s4, t1, 2
	bge	t1, t3, .main.bb17
.main.bb19:
	li	a2, 4
	bge	s4, a2, .main.bb20
.main.bb21:
	mv	a2, s4	# phi-copy
	j	.main.bb15
.main.bb22:
	j	.main.bb15
.main.bb26:
	bge	t1, a2, .main.bb29
.main.bb35:
	addiw	a3, t1, 3
	blt	a3, a2, .main.bb36
.main.bb27:
	slliw	a3, t1, 2
	slliw	a4, t1, 4
	addw	a3, s1, a3
	addiw	a4, a4, 8
	add	a3, s2, a3
	add	a4, s3, a4
	lw	a3, 0(a3)
	addiw	t1, t1, 1
	lw	a4, 0(a4)
	mulw	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb26
.main.bb36:
	slliw	a3, t1, 2
	slliw	a4, t1, 4
	addw	a3, s1, a3
	addiw	a4, a4, 8
	add	s4, s2, a3
	add	s5, s3, a4
	lw	a3, 0(s4)
	lw	a4, 0(s5)
	lw	a6, 16(s5)
	addiw	t1, t1, 4
	mulw	a3, a3, a4
	lw	a4, 4(s4)
	addw	t2, t2, a3
	mulw	a4, a4, a6
	lw	a3, 12(s4)
	lw	a6, 32(s5)
	addw	t2, t2, a4
	lw	a4, 8(s4)
	mulw	a4, a4, a6
	addw	t2, t2, a4
	lw	a4, 48(s5)
	mulw	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb26
.main.bb34:
.main.bb29:
	addiw	s4, t1, 2
	bge	t1, t3, .main.bb28
.main.bb30:
	li	a2, 4
	bge	s4, a2, .main.bb31
.main.bb32:
	mv	a2, s4	# phi-copy
	j	.main.bb26
.main.bb33:
	j	.main.bb26
.main.bb37:
	bge	t1, a2, .main.bb40
.main.bb46:
	addiw	a3, t1, 3
	blt	a3, a2, .main.bb47
.main.bb38:
	slliw	a3, t1, 2
	slliw	a4, t1, 4
	addw	a3, s1, a3
	addiw	a4, a4, 12
	add	a3, s2, a3
	add	a4, s3, a4
	lw	a3, 0(a3)
	addiw	t1, t1, 1
	lw	a4, 0(a4)
	mulw	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb37
.main.bb47:
	slliw	a3, t1, 2
	slliw	a4, t1, 4
	addw	a3, s1, a3
	addiw	a4, a4, 12
	add	s4, s2, a3
	add	s5, s3, a4
	lw	a3, 0(s4)
	lw	a4, 0(s5)
	lw	a6, 16(s5)
	addiw	t1, t1, 4
	mulw	a3, a3, a4
	lw	a4, 4(s4)
	addw	t2, t2, a3
	mulw	a4, a4, a6
	lw	a3, 12(s4)
	lw	a6, 32(s5)
	addw	t2, t2, a4
	lw	a4, 8(s4)
	mulw	a4, a4, a6
	addw	t2, t2, a4
	lw	a4, 48(s5)
	mulw	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb37
.main.bb45:
.main.bb40:
	addiw	s4, t1, 2
	bge	t1, t3, .main.bb39
.main.bb41:
	li	a2, 4
	bge	s4, a2, .main.bb42
.main.bb43:
	mv	a2, s4	# phi-copy
	j	.main.bb37
.main.bb44:
	j	.main.bb37
.main.bb9:
	li	a2, 4
	j	.main.bb4
.main.bb20:
	li	a2, 4
	j	.main.bb15
.main.bb31:
	li	a2, 4
	j	.main.bb26
.main.bb42:
	li	a2, 4
	j	.main.bb37
.main.bb6:
	la	t1, output
	li	t3, 4
	add	t1, t1, s1
	la	s2, left	# loop.addr.hoisted
	sw	t2, 0(t1)
	la	s3, right	# loop.addr.hoisted
	li	t1, 0
	li	t2, 0
	j	.main.bb18
.main.bb17:
	addiw	t1, s1, 4
	la	t3, output
	add	t1, t3, t1
	la	s2, left	# loop.addr.hoisted
	sw	t2, 0(t1)
	li	t3, 4
	li	t1, 0
	li	t2, 0
	la	s3, right	# loop.addr.hoisted
	j	.main.bb29
.main.bb28:
	addiw	t1, s1, 8
	la	t3, output
	add	t1, t3, t1
	la	s2, left	# loop.addr.hoisted
	sw	t2, 0(t1)
	li	t3, 4
	li	t1, 0
	li	t2, 0
	la	s3, right	# loop.addr.hoisted
	j	.main.bb40
.main.bb39:
	addiw	t1, s1, 12
	la	t3, output
	add	t1, t3, t1
	addiw	t0, t0, 1
	sw	t2, 0(t1)
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
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main
