
  .bss
  .globl input_plane
  .align 2
  .type  input_plane, @object
  .size  input_plane, 16384
input_plane:
  .zero 16384

  .bss
  .globl transposed_plane
  .align 2
  .type  transposed_plane, @object
  .size  transposed_plane, 16384
transposed_plane:
  .zero 16384

  .bss
  .globl output_plane
  .align 2
  .type  output_plane, @object
  .size  output_plane, 16384
output_plane:
  .zero 16384

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
	sd	s8, 72(sp)
	li	t0, 0
	li	t1, 64
	la	s1, input_plane	# loop.addr.hoisted
.main.bb1:
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s2, t0, 8
	li	t2, 0
.main.bb4:
	li	t3, 64
	bge	t2, t3, .main.bb6
.main.bb34:
	addiw	s3, t2, 3
	li	t3, 64
	blt	s3, t3, .main.bb35
.main.bb5:
	slliw	a2, t2, 2
	addw	t3, t0, t2
	addw	a2, s2, a2
	addiw	t3, t3, 1
	add	a2, s1, a2
	addiw	t2, t2, 1
	sw	t3, 0(a2)
	j	.main.bb4
.main.bb19:
	bge	t3, a4, .main.bb28
.main.bb38:
	addiw	a5, t3, 3
	blt	a5, a4, .main.bb39
.main.bb20:
	slliw	s5, t3, 2
	addw	a5, s1, s5
	addiw	t3, t3, 1
	add	a5, s4, a5
	lw	s6, 0(a5)
	addw	a5, s5, s3
	add	s5, s4, a5
	lw	a5, 0(s5)
	mulw	a5, s6, a5
	addw	a2, a2, a5
	lw	a5, 256(s5)
	mulw	a5, s6, a5
	addw	t1, t1, a5
	j	.main.bb19
.main.bb39:
	slliw	s5, t3, 2
	addw	a5, s1, s5
	addiw	t3, t3, 4
	add	s6, s4, a5
	lw	s7, 0(s6)
	addw	a5, s5, s3
	add	s5, s4, a5
	lw	a5, 0(s5)
	lw	a6, 256(s5)
	lw	a7, 4(s5)
	mulw	a5, s7, a5
	mulw	a6, s7, a6
	addw	a2, a2, a5
	lw	s7, 4(s6)
	addw	t1, t1, a6
	mulw	a7, s7, a7
	lw	a6, 12(s5)
	addw	a2, a2, a7
	lw	a7, 260(s5)
	lw	a5, 268(s5)
	mulw	a7, s7, a7
	lw	s7, 8(s6)
	addw	t1, t1, a7
	lw	a7, 8(s5)
	lw	s6, 12(s6)
	mulw	a7, s7, a7
	mulw	a6, s6, a6
	mulw	a5, s6, a5
	addw	a2, a2, a7
	lw	a7, 264(s5)
	addw	a2, a2, a6
	mulw	a7, s7, a7
	addw	t1, t1, a7
	addw	t1, t1, a5
	j	.main.bb19
.main.bb33:
.main.bb28:
	addiw	s5, t3, 16
	bge	t3, a3, .main.bb21
.main.bb29:
	li	a4, 64
	bge	s5, a4, .main.bb30
.main.bb31:
	mv	a4, s5	# phi-copy
	j	.main.bb19
.main.bb32:
	j	.main.bb19
.main.bb42:
	bge	t3, a4, .main.bb45
.main.bb51:
	addiw	a5, t3, 3
	blt	a5, a4, .main.bb52
.main.bb43:
	slliw	s6, t3, 2
	addw	a5, s1, s6
	addiw	t3, t3, 1
	add	a5, s5, a5
	lw	s7, 0(a5)
	addw	a5, s6, s4
	add	s6, s5, a5
	lw	a5, 0(s6)
	mulw	a5, s7, a5
	addw	a2, a2, a5
	lw	a5, 256(s6)
	mulw	a5, s7, a5
	addw	t1, t1, a5
	j	.main.bb42
.main.bb52:
	slliw	s6, t3, 2
	addw	a5, s1, s6
	addiw	t3, t3, 4
	add	s7, s5, a5
	lw	s8, 0(s7)
	addw	a5, s6, s4
	add	s6, s5, a5
	lw	a5, 0(s6)
	lw	a6, 256(s6)
	lw	a7, 4(s6)
	mulw	a5, s8, a5
	mulw	a6, s8, a6
	addw	a2, a2, a5
	lw	s8, 4(s7)
	addw	t1, t1, a6
	mulw	a7, s8, a7
	lw	a6, 12(s6)
	addw	a2, a2, a7
	lw	a7, 260(s6)
	lw	a5, 268(s6)
	mulw	a7, s8, a7
	lw	s8, 8(s7)
	addw	t1, t1, a7
	lw	a7, 8(s6)
	lw	s7, 12(s7)
	mulw	a7, s8, a7
	mulw	a6, s7, a6
	mulw	a5, s7, a5
	addw	a2, a2, a7
	lw	a7, 264(s6)
	addw	a2, a2, a6
	mulw	a7, s8, a7
	addw	t1, t1, a7
	addw	t1, t1, a5
	j	.main.bb42
.main.bb50:
.main.bb45:
	addiw	s6, t3, 16
	bge	t3, a3, .main.bb44
.main.bb46:
	li	a4, 64
	bge	s6, a4, .main.bb47
.main.bb48:
	mv	a4, s6	# phi-copy
	j	.main.bb42
.main.bb49:
	j	.main.bb42
.main.bb53:
	bge	t3, a4, .main.bb56
.main.bb62:
	addiw	a5, t3, 3
	blt	a5, a4, .main.bb63
.main.bb54:
	slliw	s5, t3, 2
	addw	a5, s1, s5
	addiw	t3, t3, 1
	add	a5, s4, a5
	lw	s6, 0(a5)
	addw	a5, s5, s2
	add	s5, s4, a5
	lw	a5, 0(s5)
	mulw	a5, s6, a5
	addw	a2, a2, a5
	lw	a5, 256(s5)
	mulw	a5, s6, a5
	addw	t2, t2, a5
	j	.main.bb53
.main.bb63:
	slliw	s5, t3, 2
	addw	a5, s1, s5
	addiw	t3, t3, 4
	add	s6, s4, a5
	lw	s7, 0(s6)
	addw	a5, s5, s2
	add	s5, s4, a5
	lw	a5, 0(s5)
	lw	a6, 256(s5)
	lw	a7, 4(s5)
	mulw	a5, s7, a5
	mulw	a6, s7, a6
	addw	a2, a2, a5
	lw	s7, 4(s6)
	addw	t2, t2, a6
	mulw	a7, s7, a7
	lw	a6, 12(s5)
	addw	a2, a2, a7
	lw	a7, 260(s5)
	lw	a5, 268(s5)
	mulw	a7, s7, a7
	lw	s7, 8(s6)
	addw	t2, t2, a7
	lw	a7, 8(s5)
	lw	s6, 12(s6)
	mulw	a7, s7, a7
	mulw	a6, s6, a6
	mulw	a5, s6, a5
	addw	a2, a2, a7
	lw	a7, 264(s5)
	addw	a2, a2, a6
	mulw	a7, s7, a7
	addw	t2, t2, a7
	addw	t2, t2, a5
	j	.main.bb53
.main.bb61:
.main.bb56:
	addiw	s5, t3, 16
	bge	t3, a3, .main.bb55
.main.bb57:
	li	a4, 64
	bge	s5, a4, .main.bb58
.main.bb59:
	mv	a4, s5	# phi-copy
	j	.main.bb53
.main.bb60:
	j	.main.bb53
.main.bb30:
	li	a4, 64
	j	.main.bb19
.main.bb47:
	li	a4, 64
	j	.main.bb42
.main.bb58:
	li	a4, 64
	j	.main.bb53
.main.bb10:
	bge	t1, t3, .main.bb22
.main.bb36:
	addiw	a2, t1, 3
	blt	a2, t3, .main.bb37
.main.bb11:
	addiw	t1, t1, 1
	j	.main.bb10
.main.bb37:
	addiw	t1, t1, 4
	j	.main.bb10
.main.bb27:
.main.bb22:
	addiw	s1, t1, 32
	bge	t1, t2, .main.bb12
.main.bb23:
	li	t3, 64
	bge	s1, t3, .main.bb24
.main.bb25:
	mv	t3, s1	# phi-copy
	j	.main.bb10
.main.bb26:
	j	.main.bb10
.main.bb21:
	addw	t2, s1, t2
	la	t3, output_plane
	add	s3, t3, t2
	addiw	t2, s3, 4
	sw	t1, 0(t2)
	sw	a2, 0(s3)
	mv	t1, s2	# phi-copy
.main.bb16:
	li	t2, 64
	bge	t1, t2, .main.bb18
.main.bb40:
	addiw	s2, t1, 2
	addiw	s3, t1, 4
	slt	t2, t1, s2
	slt	t3, s2, s3
	li	a2, 64
	and	t2, t2, t3
	slt	a2, s2, a2
	and	t2, t2, a2
	bnez	t2, .main.bb41
.main.bb17:
	slliw	t2, t1, 2
	slliw	s3, t1, 8
	li	t1, 0
	li	t3, 0
	li	a2, 0
	li	a3, 64
	la	s4, input_plane	# loop.addr.hoisted
	j	.main.bb28
.main.bb44:
	addw	t2, s1, t2
	la	t3, output_plane
	add	s4, t3, t2
	li	a3, 64
	addiw	t2, s4, 4
	li	t3, 0
	sw	t1, 0(t2)
	sw	a2, 0(s4)
	slliw	t1, s2, 2
	slliw	s2, s2, 8
	li	t2, 0
	li	a2, 0
	la	s4, input_plane	# loop.addr.hoisted
	j	.main.bb56
.main.bb55:
	addw	t1, s1, t1
	la	t3, output_plane
	add	s2, t3, t1
	addiw	t1, s2, 4
	sw	t2, 0(t1)
	sw	a2, 0(s2)
	mv	t1, s3	# phi-copy
	j	.main.bb16
.main.bb35:
	slliw	a2, t2, 2
	addw	t3, t0, t2
	addw	a2, s2, a2
	addiw	t3, t3, 1
	add	s4, s1, a2
	sw	t3, 0(s4)
	addiw	t3, t2, 1
	addw	t3, t0, t3
	addiw	t3, t3, 1
	sw	t3, 4(s4)
	addiw	t3, t2, 2
	addw	t3, t0, t3
	addiw	t2, t2, 4
	addiw	t3, t3, 1
	sw	t3, 8(s4)
	addw	t3, t0, s3
	addiw	t3, t3, 1
	sw	t3, 12(s4)
	j	.main.bb4
.main.bb41:
	slliw	t2, t1, 2
	slliw	s4, t1, 8
	li	t1, 0
	li	t3, 0
	li	a2, 0
	li	a3, 64
	la	s5, input_plane	# loop.addr.hoisted
	j	.main.bb45
.main.bb24:
	li	t3, 64
	j	.main.bb10
.main.bb6:
	addiw	t0, t0, 1
	blt	t0, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
.main.bb7:
	li	t1, 64
	bge	t0, t1, .main.bb9
.main.bb8:
	li	t1, 0
	li	t2, 64
	j	.main.bb22
.main.bb18:
	addiw	t0, t0, 1
.main.bb13:
	li	t1, 64
	bge	t0, t1, .main.bb15
.main.bb14:
	slliw	s1, t0, 8
	li	t1, 0
	j	.main.bb16
.main.bb12:
	addiw	t0, t0, 1
	j	.main.bb7
.main.bb9:
	li	t0, 0
	j	.main.bb13
.main.bb15:
	la	t0, output_plane
	lw	a0, 788(t0)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s8, 72(sp)
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
