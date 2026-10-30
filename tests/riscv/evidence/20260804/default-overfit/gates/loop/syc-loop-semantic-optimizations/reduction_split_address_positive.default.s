
  .bss
  .globl left_matrix
  .align 2
  .type  left_matrix, @object
  .size  left_matrix, 4096
left_matrix:
  .zero 4096

  .bss
  .globl right_matrix
  .align 2
  .type  right_matrix, @object
  .size  right_matrix, 4096
right_matrix:
  .zero 4096

  .bss
  .globl output_matrix
  .align 2
  .type  output_matrix, @object
  .size  output_matrix, 4096
output_matrix:
  .zero 4096

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
	li	t0, 0
.main.bb1:
	li	t1, 32
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s1, t0, 7
	li	t1, 0
	li	t2, 32
	la	s2, left_matrix	# loop.addr.hoisted
	la	s3, right_matrix	# loop.addr.hoisted
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
	bge	t2, a3, .main.bb16
.main.bb30:
	addiw	a4, t2, 3
	blt	a4, a3, .main.bb31
.main.bb14:
	slliw	a4, t2, 2
	addw	a4, s1, a4
	add	a4, s4, a4
	lw	s6, 0(a4)
	slliw	a4, t2, 7
	addw	a4, a4, s3
	addiw	t2, t2, 1
	add	s7, s5, a4
	lw	a4, 0(s7)
	mulw	a4, s6, a4
	addw	t3, t3, a4
	lw	a4, 4(s7)
	mulw	a4, s6, a4
	addw	t1, t1, a4
	j	.main.bb13
.main.bb31:
	slliw	a4, t2, 2
	addw	a4, s1, a4
	add	s6, s4, a4
	lw	s7, 0(s6)
	slliw	a4, t2, 7
	addw	a4, a4, s3
	addiw	t2, t2, 4
	add	s8, s5, a4
	lw	a4, 0(s8)
	lw	a5, 4(s8)
	lw	a6, 128(s8)
	mulw	a4, s7, a4
	mulw	a5, s7, a5
	addw	t3, t3, a4
	lw	s7, 4(s6)
	addw	t1, t1, a5
	mulw	a6, s7, a6
	lw	a5, 384(s8)
	addw	t3, t3, a6
	lw	a6, 132(s8)
	lw	a4, 388(s8)
	mulw	a6, s7, a6
	lw	s7, 8(s6)
	addw	t1, t1, a6
	lw	a6, 256(s8)
	lw	s6, 12(s6)
	mulw	a6, s7, a6
	mulw	a5, s6, a5
	mulw	a4, s6, a4
	addw	t3, t3, a6
	lw	a6, 260(s8)
	addw	t3, t3, a5
	mulw	a6, s7, a6
	addw	t1, t1, a6
	addw	t1, t1, a4
	j	.main.bb13
.main.bb21:
.main.bb16:
	addiw	s6, t2, 16
	bge	t2, a2, .main.bb15
.main.bb17:
	li	a3, 32
	bge	s6, a3, .main.bb18
.main.bb19:
	mv	a3, s6	# phi-copy
	j	.main.bb13
.main.bb20:
	j	.main.bb13
.main.bb34:
	bge	t2, a3, .main.bb37
.main.bb43:
	addiw	a4, t2, 3
	blt	a4, a3, .main.bb44
.main.bb35:
	slliw	a4, t2, 2
	addw	a4, s1, a4
	add	a4, s5, a4
	lw	s7, 0(a4)
	slliw	a4, t2, 7
	addw	a4, a4, s4
	addiw	t2, t2, 1
	add	s8, s6, a4
	lw	a4, 0(s8)
	mulw	a4, s7, a4
	addw	t3, t3, a4
	lw	a4, 4(s8)
	mulw	a4, s7, a4
	addw	t1, t1, a4
	j	.main.bb34
.main.bb44:
	slliw	a4, t2, 2
	addw	a4, s1, a4
	add	s7, s5, a4
	lw	s8, 0(s7)
	slliw	a4, t2, 7
	addw	a4, a4, s4
	addiw	t2, t2, 4
	add	s9, s6, a4
	lw	a4, 0(s9)
	lw	a5, 4(s9)
	lw	a6, 128(s9)
	mulw	a4, s8, a4
	mulw	a5, s8, a5
	addw	t3, t3, a4
	lw	s8, 4(s7)
	addw	t1, t1, a5
	mulw	a6, s8, a6
	lw	a5, 384(s9)
	addw	t3, t3, a6
	lw	a6, 132(s9)
	lw	a4, 388(s9)
	mulw	a6, s8, a6
	lw	s8, 8(s7)
	addw	t1, t1, a6
	lw	a6, 256(s9)
	lw	s7, 12(s7)
	mulw	a6, s8, a6
	mulw	a5, s7, a5
	mulw	a4, s7, a4
	addw	t3, t3, a6
	lw	a6, 260(s9)
	addw	t3, t3, a5
	mulw	a6, s8, a6
	addw	t1, t1, a6
	addw	t1, t1, a4
	j	.main.bb34
.main.bb42:
.main.bb37:
	addiw	s7, t2, 16
	bge	t2, a2, .main.bb36
.main.bb38:
	li	a3, 32
	bge	s7, a3, .main.bb39
.main.bb40:
	mv	a3, s7	# phi-copy
	j	.main.bb34
.main.bb41:
	j	.main.bb34
.main.bb45:
	bge	t2, a3, .main.bb48
.main.bb54:
	addiw	a4, t2, 3
	blt	a4, a3, .main.bb55
.main.bb46:
	slliw	a4, t2, 2
	addw	a4, s1, a4
	add	a4, s4, a4
	lw	s6, 0(a4)
	slliw	a4, t2, 7
	addw	a4, a4, s2
	addiw	t2, t2, 1
	add	s7, s5, a4
	lw	a4, 0(s7)
	mulw	a4, s6, a4
	addw	t3, t3, a4
	lw	a4, 4(s7)
	mulw	a4, s6, a4
	addw	t1, t1, a4
	j	.main.bb45
.main.bb55:
	slliw	a4, t2, 2
	addw	a4, s1, a4
	add	s6, s4, a4
	lw	s7, 0(s6)
	slliw	a4, t2, 7
	addw	a4, a4, s2
	addiw	t2, t2, 4
	add	s8, s5, a4
	lw	a4, 0(s8)
	lw	a5, 4(s8)
	lw	a6, 128(s8)
	mulw	a4, s7, a4
	mulw	a5, s7, a5
	addw	t3, t3, a4
	lw	s7, 4(s6)
	addw	t1, t1, a5
	mulw	a6, s7, a6
	lw	a5, 384(s8)
	addw	t3, t3, a6
	lw	a6, 132(s8)
	lw	a4, 388(s8)
	mulw	a6, s7, a6
	lw	s7, 8(s6)
	addw	t1, t1, a6
	lw	a6, 256(s8)
	lw	s6, 12(s6)
	mulw	a6, s7, a6
	mulw	a5, s6, a5
	mulw	a4, s6, a4
	addw	t3, t3, a6
	lw	a6, 260(s8)
	addw	t3, t3, a5
	mulw	a6, s7, a6
	addw	t1, t1, a6
	addw	t1, t1, a4
	j	.main.bb45
.main.bb53:
.main.bb48:
	addiw	s6, t2, 16
	bge	t2, a2, .main.bb47
.main.bb49:
	li	a3, 32
	bge	s6, a3, .main.bb50
.main.bb51:
	mv	a3, s6	# phi-copy
	j	.main.bb45
.main.bb52:
	j	.main.bb45
.main.bb18:
	li	a3, 32
	j	.main.bb13
.main.bb39:
	li	a3, 32
	j	.main.bb34
.main.bb50:
	li	a3, 32
	j	.main.bb45
.main.bb27:
	j	.main.bb22
.main.bb15:
	addw	t2, s1, s3
	la	a2, output_matrix
	add	s3, a2, t2
	addiw	t2, s3, 4
	sw	t1, 0(t2)
	sw	t3, 0(s3)
	mv	t1, s2	# phi-copy
.main.bb10:
	li	t2, 32
	bge	t1, t2, .main.bb12
.main.bb32:
	addiw	s2, t1, 2
	addiw	s3, t1, 4
	slt	t2, t1, s2
	slt	t3, s2, s3
	li	a2, 32
	and	t2, t2, t3
	slt	a2, s2, a2
	and	t2, t2, a2
	bnez	t2, .main.bb33
.main.bb11:
	slliw	s3, t1, 2
	li	t2, 0
	li	t1, 0
	li	t3, 0
	li	a2, 32
	la	s4, left_matrix	# loop.addr.hoisted
	la	s5, right_matrix	# loop.addr.hoisted
	j	.main.bb16
.main.bb36:
	addw	t2, s1, s4
	la	a2, output_matrix
	add	s4, a2, t2
	slliw	s2, s2, 2
	addiw	t2, s4, 4
	li	a2, 32
	sw	t1, 0(t2)
	la	s5, right_matrix	# loop.addr.hoisted
	sw	t3, 0(s4)
	li	t1, 0
	li	t2, 0
	li	t3, 0
	la	s4, left_matrix	# loop.addr.hoisted
	j	.main.bb48
.main.bb47:
	addw	t2, s1, s2
	la	a2, output_matrix
	add	s2, a2, t2
	addiw	t2, s2, 4
	sw	t1, 0(t2)
	sw	t3, 0(s2)
	mv	t1, s3	# phi-copy
	j	.main.bb10
.main.bb33:
	slliw	s4, t1, 2
	li	t2, 0
	li	t1, 0
	li	t3, 0
	li	a2, 32
	la	s5, left_matrix	# loop.addr.hoisted
	la	s6, right_matrix	# loop.addr.hoisted
	j	.main.bb37
.main.bb24:
	li	t3, 32
	j	.main.bb4
.main.bb12:
	addiw	t0, t0, 1
.main.bb7:
	li	t1, 32
	bge	t0, t1, .main.bb9
.main.bb8:
	slliw	s1, t0, 7
	li	t1, 0
	j	.main.bb10
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	li	t0, 0
	j	.main.bb7
.main.bb9:
	la	t0, output_matrix
	lw	a0, 404(t0)
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
