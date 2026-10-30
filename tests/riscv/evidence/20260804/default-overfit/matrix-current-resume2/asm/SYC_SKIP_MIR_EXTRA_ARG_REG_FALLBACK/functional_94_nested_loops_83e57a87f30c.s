
  .bss
  .globl arr1
  .align 2
  .type  arr1, @object
  .size  arr1, 57600
arr1:
  .zero 57600

  .bss
  .globl arr2
  .align 2
  .type  arr2, @object
  .size  arr2, 107520
arr2:
  .zero 107520

  .text

  .globl loop1
  .type  loop1, @function
loop1:
.loop1.bb0:
	addi	sp, sp, -80
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	sd	s7, 48(sp)
	sd	s8, 56(sp)
	sd	s9, 64(sp)
	sd	s10, 72(sp)
	mv	s1, a0
	mv	s2, a1
	li	t0, 0
.loop1.bb1:
	slt	t1, t0, s1
	slt	t2, t0, s2
	and	t1, t1, t2
	beq	t1, zero, .loop1.bb3
.loop1.bb2:
	li	t1, 5760
	li	t2, 0
	mulw	t1, t0, t1
.loop1.bb4:
	li	t3, 2
	bge	t2, t3, .loop1.bb6
.loop1.bb5:
	li	t3, 2880
	addw	s3, t0, t2
	mulw	t3, t2, t3
	li	a2, 5
	addw	s4, t1, t3
	li	t3, 0
.loop1.bb7:
	bge	t3, a2, .loop1.bb9
.loop1.bb8:
	slliw	a3, t3, 5
	slliw	a4, t3, 4
	addw	a3, a3, a4
	addw	s5, s3, t3
	addw	a3, s4, a3
	la	a5, arr1
	addw	a4, s5, s1
	add	s6, a5, a3
	addw	a4, a4, s2
	addiw	a3, s5, 1
	sw	a4, 0(s6)
	addw	a3, a3, s1
	addw	s7, a3, s2
	addiw	a4, s5, 2
	sw	s7, 4(s6)
	addw	a4, a4, s1
	sw	s7, 8(s6)
	addw	s7, a4, s2
	addiw	a4, s5, 3
	sw	s7, 12(s6)
	sw	s7, 16(s6)
	addw	a4, a4, s1
	addw	s7, a4, s2
	addiw	a4, s5, 4
	sw	s7, 20(s6)
	sw	s7, 24(s6)
	addw	a4, a4, s1
	addw	s7, a4, s2
	sw	s7, 28(s6)
	addiw	a4, s5, 5
	sw	s7, 32(s6)
	addw	a4, a4, s1
	addw	s7, a4, s2
	sw	s7, 36(s6)
	addiw	a4, s5, 6
	sw	s7, 40(s6)
	addw	a4, a4, s1
	addw	a4, a4, s2
	sw	a4, 44(s6)
	addiw	t3, t3, 1
	blt	t3, a2, .loop1.bb8	# rotated-loop-branch
.loop1.bb9:
	addiw	s5, s3, 1
	addiw	t3, s4, 240
	li	a2, 0
	li	a3, 5
.loop1.bb10:
	bge	a2, a3, .loop1.bb12
.loop1.bb11:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s6, s5, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s6, s1
	add	s7, a6, a4
	addw	a5, a5, s2
	addiw	a4, s6, 1
	sw	a5, 0(s7)
	addw	a4, a4, s1
	addw	s8, a4, s2
	addiw	a5, s6, 2
	sw	s8, 4(s7)
	addw	a5, a5, s1
	sw	s8, 8(s7)
	addw	s8, a5, s2
	addiw	a5, s6, 3
	sw	s8, 12(s7)
	sw	s8, 16(s7)
	addw	a5, a5, s1
	addw	s8, a5, s2
	addiw	a5, s6, 4
	sw	s8, 20(s7)
	sw	s8, 24(s7)
	addw	a5, a5, s1
	addw	s8, a5, s2
	sw	s8, 28(s7)
	addiw	a5, s6, 5
	sw	s8, 32(s7)
	addw	a5, a5, s1
	addw	s8, a5, s2
	sw	s8, 36(s7)
	addiw	a5, s6, 6
	sw	s8, 40(s7)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s7)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb11	# rotated-loop-branch
.loop1.bb12:
	addiw	s6, s3, 2
	addiw	t3, s4, 480
	li	a2, 0
	li	a3, 5
.loop1.bb13:
	bge	a2, a3, .loop1.bb15
.loop1.bb14:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s7, s6, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s7, s1
	add	s8, a6, a4
	addw	a5, a5, s2
	addiw	a4, s7, 1
	sw	a5, 0(s8)
	addw	a4, a4, s1
	addw	s9, a4, s2
	addiw	a5, s7, 2
	sw	s9, 4(s8)
	addw	a5, a5, s1
	sw	s9, 8(s8)
	addw	s9, a5, s2
	addiw	a5, s7, 3
	sw	s9, 12(s8)
	sw	s9, 16(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	addiw	a5, s7, 4
	sw	s9, 20(s8)
	sw	s9, 24(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	sw	s9, 28(s8)
	addiw	a5, s7, 5
	sw	s9, 32(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	sw	s9, 36(s8)
	addiw	a5, s7, 6
	sw	s9, 40(s8)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s8)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb14	# rotated-loop-branch
.loop1.bb15:
	addiw	s7, s3, 3
	addiw	t3, s4, 720
	li	a2, 0
	li	a3, 5
.loop1.bb16:
	bge	a2, a3, .loop1.bb18
.loop1.bb17:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s8, s7, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s8, s1
	add	s9, a6, a4
	addw	a5, a5, s2
	addiw	a4, s8, 1
	sw	a5, 0(s9)
	addw	a4, a4, s1
	addw	s10, a4, s2
	addiw	a5, s8, 2
	sw	s10, 4(s9)
	addw	a5, a5, s1
	sw	s10, 8(s9)
	addw	s10, a5, s2
	addiw	a5, s8, 3
	sw	s10, 12(s9)
	sw	s10, 16(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	addiw	a5, s8, 4
	sw	s10, 20(s9)
	sw	s10, 24(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	sw	s10, 28(s9)
	addiw	a5, s8, 5
	sw	s10, 32(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	sw	s10, 36(s9)
	addiw	a5, s8, 6
	sw	s10, 40(s9)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s9)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb17	# rotated-loop-branch
.loop1.bb18:
	addiw	t3, s4, 960
	li	a2, 0
	li	a3, 5
.loop1.bb19:
	bge	a2, a3, .loop1.bb21
.loop1.bb20:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s8, s5, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s8, s1
	add	s9, a6, a4
	addw	a5, a5, s2
	addiw	a4, s8, 1
	sw	a5, 0(s9)
	addw	a4, a4, s1
	addw	s10, a4, s2
	addiw	a5, s8, 2
	sw	s10, 4(s9)
	addw	a5, a5, s1
	sw	s10, 8(s9)
	addw	s10, a5, s2
	addiw	a5, s8, 3
	sw	s10, 12(s9)
	sw	s10, 16(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	addiw	a5, s8, 4
	sw	s10, 20(s9)
	sw	s10, 24(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	sw	s10, 28(s9)
	addiw	a5, s8, 5
	sw	s10, 32(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	sw	s10, 36(s9)
	addiw	a5, s8, 6
	sw	s10, 40(s9)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s9)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb20	# rotated-loop-branch
.loop1.bb21:
	addiw	t3, s4, 1200
	li	a2, 0
	li	a3, 5
.loop1.bb22:
	bge	a2, a3, .loop1.bb24
.loop1.bb23:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s5, s6, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s5, s1
	add	s8, a6, a4
	addw	a5, a5, s2
	addiw	a4, s5, 1
	sw	a5, 0(s8)
	addw	a4, a4, s1
	addw	s9, a4, s2
	addiw	a5, s5, 2
	sw	s9, 4(s8)
	addw	a5, a5, s1
	sw	s9, 8(s8)
	addw	s9, a5, s2
	addiw	a5, s5, 3
	sw	s9, 12(s8)
	sw	s9, 16(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	addiw	a5, s5, 4
	sw	s9, 20(s8)
	sw	s9, 24(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	sw	s9, 28(s8)
	addiw	a5, s5, 5
	sw	s9, 32(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	sw	s9, 36(s8)
	addiw	a5, s5, 6
	sw	s9, 40(s8)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s8)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb23	# rotated-loop-branch
.loop1.bb24:
	addiw	t3, s4, 1440
	li	a2, 0
	li	a3, 5
.loop1.bb25:
	bge	a2, a3, .loop1.bb27
.loop1.bb26:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s5, s7, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s5, s1
	add	s8, a6, a4
	addw	a5, a5, s2
	addiw	a4, s5, 1
	sw	a5, 0(s8)
	addw	a4, a4, s1
	addw	s9, a4, s2
	addiw	a5, s5, 2
	sw	s9, 4(s8)
	addw	a5, a5, s1
	sw	s9, 8(s8)
	addw	s9, a5, s2
	addiw	a5, s5, 3
	sw	s9, 12(s8)
	sw	s9, 16(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	addiw	a5, s5, 4
	sw	s9, 20(s8)
	sw	s9, 24(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	sw	s9, 28(s8)
	addiw	a5, s5, 5
	sw	s9, 32(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	sw	s9, 36(s8)
	addiw	a5, s5, 6
	sw	s9, 40(s8)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s8)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb26	# rotated-loop-branch
.loop1.bb27:
	addiw	s5, s3, 4
	addiw	t3, s4, 1680
	li	a2, 0
	li	a3, 5
.loop1.bb28:
	bge	a2, a3, .loop1.bb30
.loop1.bb29:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s8, s5, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s8, s1
	add	s9, a6, a4
	addw	a5, a5, s2
	addiw	a4, s8, 1
	sw	a5, 0(s9)
	addw	a4, a4, s1
	addw	s10, a4, s2
	addiw	a5, s8, 2
	sw	s10, 4(s9)
	addw	a5, a5, s1
	sw	s10, 8(s9)
	addw	s10, a5, s2
	addiw	a5, s8, 3
	sw	s10, 12(s9)
	sw	s10, 16(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	addiw	a5, s8, 4
	sw	s10, 20(s9)
	sw	s10, 24(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	sw	s10, 28(s9)
	addiw	a5, s8, 5
	sw	s10, 32(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	sw	s10, 36(s9)
	addiw	a5, s8, 6
	sw	s10, 40(s9)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s9)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb29	# rotated-loop-branch
.loop1.bb30:
	addiw	t3, s4, 1920
	li	a2, 0
	li	a3, 5
.loop1.bb31:
	bge	a2, a3, .loop1.bb33
.loop1.bb32:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s8, s6, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s8, s1
	add	s9, a6, a4
	addw	a5, a5, s2
	addiw	a4, s8, 1
	sw	a5, 0(s9)
	addw	a4, a4, s1
	addw	s10, a4, s2
	addiw	a5, s8, 2
	sw	s10, 4(s9)
	addw	a5, a5, s1
	sw	s10, 8(s9)
	addw	s10, a5, s2
	addiw	a5, s8, 3
	sw	s10, 12(s9)
	sw	s10, 16(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	addiw	a5, s8, 4
	sw	s10, 20(s9)
	sw	s10, 24(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	sw	s10, 28(s9)
	addiw	a5, s8, 5
	sw	s10, 32(s9)
	addw	a5, a5, s1
	addw	s10, a5, s2
	sw	s10, 36(s9)
	addiw	a5, s8, 6
	sw	s10, 40(s9)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s9)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb32	# rotated-loop-branch
.loop1.bb33:
	li	t3, 2160
	li	a2, 0
	addw	t3, s4, t3
	li	a3, 5
.loop1.bb34:
	bge	a2, a3, .loop1.bb36
.loop1.bb35:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s6, s7, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s6, s1
	add	s8, a6, a4
	addw	a5, a5, s2
	addiw	a4, s6, 1
	sw	a5, 0(s8)
	addw	a4, a4, s1
	addw	s9, a4, s2
	addiw	a5, s6, 2
	sw	s9, 4(s8)
	addw	a5, a5, s1
	sw	s9, 8(s8)
	addw	s9, a5, s2
	addiw	a5, s6, 3
	sw	s9, 12(s8)
	sw	s9, 16(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	addiw	a5, s6, 4
	sw	s9, 20(s8)
	sw	s9, 24(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	sw	s9, 28(s8)
	addiw	a5, s6, 5
	sw	s9, 32(s8)
	addw	a5, a5, s1
	addw	s9, a5, s2
	sw	s9, 36(s8)
	addiw	a5, s6, 6
	sw	s9, 40(s8)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s8)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb35	# rotated-loop-branch
.loop1.bb36:
	li	t3, 2400
	li	a2, 0
	addw	t3, s4, t3
	li	a3, 5
.loop1.bb37:
	bge	a2, a3, .loop1.bb39
.loop1.bb38:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s6, s5, a2
	addw	a4, t3, a4
	la	a6, arr1
	addw	a5, s6, s1
	add	s7, a6, a4
	addw	a5, a5, s2
	addiw	a4, s6, 1
	sw	a5, 0(s7)
	addw	a4, a4, s1
	addw	s8, a4, s2
	addiw	a5, s6, 2
	sw	s8, 4(s7)
	addw	a5, a5, s1
	sw	s8, 8(s7)
	addw	s8, a5, s2
	addiw	a5, s6, 3
	sw	s8, 12(s7)
	sw	s8, 16(s7)
	addw	a5, a5, s1
	addw	s8, a5, s2
	addiw	a5, s6, 4
	sw	s8, 20(s7)
	sw	s8, 24(s7)
	addw	a5, a5, s1
	addw	s8, a5, s2
	sw	s8, 28(s7)
	addiw	a5, s6, 5
	sw	s8, 32(s7)
	addw	a5, a5, s1
	addw	s8, a5, s2
	sw	s8, 36(s7)
	addiw	a5, s6, 6
	sw	s8, 40(s7)
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s7)
	addiw	a2, a2, 1
	blt	a2, a3, .loop1.bb38	# rotated-loop-branch
.loop1.bb39:
	li	a2, 2640
	addiw	t3, s3, 5
	addw	a2, s4, a2
	li	a3, 0
	li	a4, 5
.loop1.bb40:
	bge	a3, a4, .loop1.bb42
.loop1.bb41:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	a5, a5, a6
	addw	s3, t3, a3
	addw	a5, a2, a5
	la	a7, arr1
	addw	a6, s3, s1
	add	s4, a7, a5
	addw	a6, a6, s2
	addiw	a5, s3, 1
	sw	a6, 0(s4)
	addw	a5, a5, s1
	addw	s5, a5, s2
	addiw	a6, s3, 2
	sw	s5, 4(s4)
	addw	a6, a6, s1
	sw	s5, 8(s4)
	addw	s5, a6, s2
	addiw	a6, s3, 3
	sw	s5, 12(s4)
	sw	s5, 16(s4)
	addw	a6, a6, s1
	addw	s5, a6, s2
	addiw	a6, s3, 4
	sw	s5, 20(s4)
	sw	s5, 24(s4)
	addw	a6, a6, s1
	addw	s5, a6, s2
	sw	s5, 28(s4)
	addiw	a6, s3, 5
	sw	s5, 32(s4)
	addw	a6, a6, s1
	addw	s5, a6, s2
	sw	s5, 36(s4)
	addiw	a6, s3, 6
	sw	s5, 40(s4)
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 44(s4)
	addiw	a3, a3, 1
	blt	a3, a4, .loop1.bb41	# rotated-loop-branch
.loop1.bb42:
	addiw	t2, t2, 1
	j	.loop1.bb4
.loop1.bb6:
	addiw	t0, t0, 1
	j	.loop1.bb1
.loop1.bb3:
	ld	s10, 72(sp)
	ld	s9, 64(sp)
	ld	s8, 56(sp)
	ld	s7, 48(sp)
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 80
	ret
	.size loop1, .-loop1

  .globl loop2
  .type  loop2, @function
loop2:
.loop2.bb0:
	addi	sp, sp, -80
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	sd	s7, 48(sp)
	sd	s8, 56(sp)
	sd	s9, 64(sp)
	sd	s10, 72(sp)
	li	t0, 0
.loop2.bb1:
	li	t1, 10
	bge	t0, t1, .loop2.bb3
.loop2.bb2:
	li	t1, 10752
	li	t2, 0
	mulw	t1, t0, t1
.loop2.bb4:
	li	t3, 2
	bge	t2, t3, .loop2.bb6
.loop2.bb5:
	li	t3, 5376
	addw	s1, t0, t2
	mulw	t3, t2, t3
	addiw	s2, s1, 1
	addiw	s3, s1, 2
	addiw	s4, s1, 3
	addiw	s5, s1, 4
	addiw	s6, s1, 5
	addw	t3, t1, t3
	addiw	s7, s1, 6
	addiw	s8, s1, 7
	li	a2, 0
.loop2.bb7:
	li	a3, 3
	bge	a2, a3, .loop2.bb9
.loop2.bb8:
	slliw	a3, a2, 11
	slliw	a4, a2, 8
	subw	a3, a3, a4
	addw	s9, t3, a3
	li	a4, 8
	li	a3, 0
.loop2.bb10:
	bge	a3, a4, .loop2.bb12
.loop2.bb11:
	slliw	a5, a3, 5
	slliw	a6, a3, 2
	subw	a5, a5, a6
	addiw	a3, a3, 1
	addw	a5, s9, a5
	la	a6, arr2
	add	s10, a6, a5
	sw	s1, 0(s10)
	sw	s2, 4(s10)
	sw	s3, 8(s10)
	sw	s4, 12(s10)
	sw	s5, 16(s10)
	sw	s6, 20(s10)
	sw	s7, 24(s10)
	blt	a3, a4, .loop2.bb11	# rotated-loop-branch
.loop2.bb12:
	addiw	a3, s9, 224
	li	a4, 0
	li	a5, 8
.loop2.bb13:
	bge	a4, a5, .loop2.bb15
.loop2.bb14:
	slliw	a6, a4, 5
	slliw	a7, a4, 2
	subw	a6, a6, a7
	addiw	a4, a4, 1
	addw	a6, a3, a6
	la	a7, arr2
	add	s10, a7, a6
	sw	s1, 0(s10)
	sw	s2, 4(s10)
	sw	s3, 8(s10)
	sw	s4, 12(s10)
	sw	s5, 16(s10)
	sw	s6, 20(s10)
	sw	s7, 24(s10)
	blt	a4, a5, .loop2.bb14	# rotated-loop-branch
.loop2.bb15:
	addiw	a3, s9, 448
	li	a4, 0
	li	a5, 8
.loop2.bb16:
	bge	a4, a5, .loop2.bb18
.loop2.bb17:
	slliw	a6, a4, 5
	slliw	a7, a4, 2
	subw	a6, a6, a7
	addiw	a4, a4, 1
	addw	a6, a3, a6
	la	a7, arr2
	add	s10, a7, a6
	sw	s1, 0(s10)
	sw	s2, 4(s10)
	sw	s3, 8(s10)
	sw	s4, 12(s10)
	sw	s5, 16(s10)
	sw	s6, 20(s10)
	sw	s7, 24(s10)
	blt	a4, a5, .loop2.bb17	# rotated-loop-branch
.loop2.bb18:
	addiw	a3, s9, 672
	li	a4, 0
	li	a5, 8
.loop2.bb19:
	bge	a4, a5, .loop2.bb21
.loop2.bb20:
	slliw	a6, a4, 5
	slliw	a7, a4, 2
	subw	a6, a6, a7
	addiw	a4, a4, 1
	addw	a6, a3, a6
	la	a7, arr2
	add	s10, a7, a6
	sw	s1, 0(s10)
	sw	s2, 4(s10)
	sw	s3, 8(s10)
	sw	s4, 12(s10)
	sw	s5, 16(s10)
	sw	s6, 20(s10)
	sw	s7, 24(s10)
	blt	a4, a5, .loop2.bb20	# rotated-loop-branch
.loop2.bb21:
	addiw	a3, s9, 896
	li	a4, 0
	li	a5, 8
.loop2.bb22:
	bge	a4, a5, .loop2.bb24
.loop2.bb23:
	slliw	a6, a4, 5
	slliw	a7, a4, 2
	subw	a6, a6, a7
	addiw	a4, a4, 1
	addw	a6, a3, a6
	la	a7, arr2
	add	s10, a7, a6
	sw	s2, 0(s10)
	sw	s3, 4(s10)
	sw	s4, 8(s10)
	sw	s5, 12(s10)
	sw	s6, 16(s10)
	sw	s7, 20(s10)
	sw	s8, 24(s10)
	blt	a4, a5, .loop2.bb23	# rotated-loop-branch
.loop2.bb24:
	addiw	a3, s9, 1120
	li	a4, 0
	li	a5, 8
.loop2.bb25:
	bge	a4, a5, .loop2.bb27
.loop2.bb26:
	slliw	a6, a4, 5
	slliw	a7, a4, 2
	subw	a6, a6, a7
	addiw	a4, a4, 1
	addw	a6, a3, a6
	la	a7, arr2
	add	s10, a7, a6
	sw	s2, 0(s10)
	sw	s3, 4(s10)
	sw	s4, 8(s10)
	sw	s5, 12(s10)
	sw	s6, 16(s10)
	sw	s7, 20(s10)
	sw	s8, 24(s10)
	blt	a4, a5, .loop2.bb26	# rotated-loop-branch
.loop2.bb27:
	addiw	a3, s9, 1344
	li	a4, 0
	li	a5, 8
.loop2.bb28:
	bge	a4, a5, .loop2.bb30
.loop2.bb29:
	slliw	a6, a4, 5
	slliw	a7, a4, 2
	subw	a6, a6, a7
	addiw	a4, a4, 1
	addw	a6, a3, a6
	la	a7, arr2
	add	s10, a7, a6
	sw	s2, 0(s10)
	sw	s3, 4(s10)
	sw	s4, 8(s10)
	sw	s5, 12(s10)
	sw	s6, 16(s10)
	sw	s7, 20(s10)
	sw	s8, 24(s10)
	blt	a4, a5, .loop2.bb29	# rotated-loop-branch
.loop2.bb30:
	addiw	a3, s9, 1568
	li	a4, 0
	li	a5, 8
.loop2.bb31:
	bge	a4, a5, .loop2.bb33
.loop2.bb32:
	slliw	a6, a4, 5
	slliw	a7, a4, 2
	subw	a6, a6, a7
	addiw	a4, a4, 1
	addw	a6, a3, a6
	la	a7, arr2
	add	s9, a7, a6
	sw	s2, 0(s9)
	sw	s3, 4(s9)
	sw	s4, 8(s9)
	sw	s5, 12(s9)
	sw	s6, 16(s9)
	sw	s7, 20(s9)
	sw	s8, 24(s9)
	blt	a4, a5, .loop2.bb32	# rotated-loop-branch
.loop2.bb33:
	addiw	a2, a2, 1
	j	.loop2.bb7
.loop2.bb9:
	addiw	t2, t2, 1
	j	.loop2.bb4
.loop2.bb6:
	addiw	t0, t0, 1
	j	.loop2.bb1
.loop2.bb3:
	ld	s10, 72(sp)
	ld	s9, 64(sp)
	ld	s8, 56(sp)
	ld	s7, 48(sp)
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 80
	ret
	.size loop2, .-loop2

  .globl loop3
  .type  loop3, @function
loop3:
.loop3.bb0:
	addi	sp, sp, -176
	sd	s1, 152(sp)
	sd	s2, 160(sp)
	mv	t5, a0
	sd	t5, 0(sp)
	mv	t5, a1
	sd	t5, 8(sp)
	mv	t5, a2
	sd	t5, 32(sp)
	mv	t5, a3
	sd	t5, 56(sp)
	mv	t5, a4
	sd	t5, 80(sp)
	mv	t5, a5
	sd	t5, 144(sp)
	mv	a4, a6
	li	a5, 0
	li	a6, 0
	sd	a6, 104(sp)
.loop3.bb1:
	li	a7, 10
	ld	t5, 104(sp)
	bge	t5, a7, .loop3.bb36
.loop3.bb2:
	li	a7, 5760
	ld	t5, 104(sp)
	mulw	t5, t5, a7
	sd	t5, 16(sp)
	li	t4, 10752
	ld	t5, 104(sp)
	mulw	t5, t5, t4
	sd	t5, 24(sp)
	li	t0, 0
	sd	t0, 136(sp)
.loop3.bb4:
	li	t1, 100
	ld	t5, 136(sp)
	bge	t5, t1, .loop3.bb37
.loop3.bb5:
	li	t1, 2880
	ld	t5, 136(sp)
	mulw	t1, t5, t1
	ld	t5, 16(sp)
	addw	t5, t5, t1
	sd	t5, 40(sp)
	li	a7, 5376
	ld	t5, 136(sp)
	mulw	a7, t5, a7
	ld	t5, 24(sp)
	addw	t5, t5, a7
	sd	t5, 48(sp)
	li	t4, 0
.loop3.bb7:
	li	t2, 1000
	bge	t4, t2, .loop3.bb38
.loop3.bb8:
	slliw	t2, t4, 10
	ld	t5, 40(sp)
	addw	t2, t5, t2
	slliw	t1, t4, 6
	subw	t5, t2, t1
	sd	t5, 64(sp)
	slliw	t2, t4, 11
	ld	t5, 48(sp)
	addw	t2, t5, t2
	slliw	a7, t4, 8
	subw	t5, t2, a7
	sd	t5, 72(sp)
	li	a7, 0
.loop3.bb10:
	li	t3, 10000
	bge	a7, t3, .loop3.bb39
.loop3.bb11:
	slliw	t3, a7, 8
	ld	t5, 64(sp)
	addw	t3, t5, t3
	slliw	t1, a7, 4
	subw	t5, t3, t1
	sd	t5, 88(sp)
	slliw	t3, a7, 10
	ld	t5, 72(sp)
	addw	t3, t5, t3
	slliw	t2, a7, 7
	subw	t5, t3, t2
	sd	t5, 96(sp)
	li	t3, 0
.loop3.bb13:
	li	a2, 100000
	bge	t3, a2, .loop3.bb40
.loop3.bb14:
	slliw	s1, t3, 5
	ld	t5, 88(sp)
	addw	a2, t5, s1
	slliw	t1, t3, 4
	addw	t5, a2, t1
	sd	t5, 112(sp)
	slliw	a2, t3, 8
	ld	t5, 96(sp)
	addw	a2, t5, a2
	subw	t5, a2, s1
	sd	t5, 120(sp)
	li	t2, 0
.loop3.bb16:
	li	a6, 1000000
	bge	t2, a6, .loop3.bb41
.loop3.bb17:
	slliw	a6, t2, 3
	ld	t5, 112(sp)
	addw	a6, t5, a6
	la	t1, arr1
	add	t1, t1, a6
	slliw	a6, t2, 5
	ld	t5, 120(sp)
	addw	a6, t5, a6
	slliw	a2, t2, 2
	subw	a2, a6, a2
	la	a6, arr2
	add	a2, a6, a2
	li	a6, 0
	li	t5, 10000000
	sd	t5, 128(sp)
.loop3.bb19:
	ld	t5, 128(sp)
	bge	a6, t5, .loop3.bb42
.loop3.bb20:
	li	s1, -1603384363
	addiw	s2, a6, 1
	mul	t0, a5, s1	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	addw	t0, t0, a5	# const-div.fixup
	sraiw	t0, t0, 9	# const-div.shift
	srliw	a3, t0, 31	# const-div.sign
	addw	t0, t0, a3	# const-div.round
	li	a3, 817
	mulw	t0, t0, a3	# const-rem.mul
	lw	a3, 0(t1)
	subw	t0, a5, t0	# const-rem.sub
	addw	t0, t0, a3
	lw	a3, 0(a2)
	addw	s1, t0, a3
	bge	s2, a4, .loop3.bb21
.loop3.bb23:
	addi	t1, t1, 4
	addi	a2, a2, 4
	mv	a5, s1	# phi-copy
	mv	a6, s2	# phi-copy
	j	.loop3.bb19
.loop3.bb42:
	mv	s1, a5	# phi-copy
.loop3.bb21:
	addiw	s2, t2, 1
	ld	t5, 144(sp)
	bge	s2, t5, .loop3.bb18
.loop3.bb25:
	mv	t2, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb16
.loop3.bb22:
	j	.loop3.bb21
.loop3.bb24:
.loop3.bb18:
	addiw	s2, t3, 1
	ld	t5, 80(sp)
	bge	s2, t5, .loop3.bb15
.loop3.bb27:
	mv	t3, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb13
.loop3.bb41:
	mv	s1, a5	# phi-copy
	j	.loop3.bb18
.loop3.bb26:
.loop3.bb15:
	addiw	s2, a7, 1
	ld	t5, 56(sp)
	bge	s2, t5, .loop3.bb12
.loop3.bb29:
	mv	a7, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb10
.loop3.bb40:
	mv	s1, a5	# phi-copy
	j	.loop3.bb15
.loop3.bb28:
.loop3.bb12:
	addiw	s2, t4, 1
	ld	t5, 32(sp)
	bge	s2, t5, .loop3.bb9
.loop3.bb31:
	mv	t4, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb7
.loop3.bb39:
	mv	s1, a5	# phi-copy
	j	.loop3.bb12
.loop3.bb30:
.loop3.bb9:
	ld	t5, 136(sp)
	addiw	s2, t5, 1
	ld	t5, 8(sp)
	bge	s2, t5, .loop3.bb6
.loop3.bb33:
	sd	s2, 136(sp)
	mv	a5, s1	# phi-copy
	j	.loop3.bb4
.loop3.bb38:
	mv	s1, a5	# phi-copy
	j	.loop3.bb9
.loop3.bb32:
.loop3.bb6:
	ld	t5, 104(sp)
	addiw	s2, t5, 1
	ld	t5, 0(sp)
	bge	s2, t5, .loop3.bb3
.loop3.bb35:
	sd	s2, 104(sp)
	mv	a5, s1	# phi-copy
	j	.loop3.bb1
.loop3.bb37:
	mv	s1, a5	# phi-copy
	j	.loop3.bb6
.loop3.bb34:
.loop3.bb3:
	mv	a0, s1
	ld	s2, 160(sp)
	ld	s1, 152(sp)
	addi	sp, sp, 176
	ret
.loop3.bb36:
	mv	s1, a5	# phi-copy
	j	.loop3.bb3
	.size loop3, .-loop3

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -144
	sd	ra, 56(sp)
	sd	s1, 64(sp)
	sd	s2, 72(sp)
	sd	s3, 80(sp)
	sd	s4, 88(sp)
	sd	s5, 96(sp)
	sd	s6, 104(sp)
	sd	s7, 112(sp)
	sd	s8, 120(sp)
	sd	s9, 128(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	call	getint
	mv	s3, a0
	call	getint
	mv	s4, a0
	call	getint
	mv	s5, a0
	call	getint
	mv	s6, a0
	call	getint
	mv	s7, a0
	call	getint
	mv	s8, a0
	call	getint
	mv	s9, a0
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, s2
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	loop1
	call	loop2
	mv	t5, s3
	sd	t5, 0(sp)
	mv	t5, s4
	sd	t5, 8(sp)
	mv	t5, s5
	sd	t5, 16(sp)
	mv	t5, s6
	sd	t5, 24(sp)
	mv	t5, s7
	sd	t5, 32(sp)
	mv	t5, s8
	sd	t5, 40(sp)
	mv	t5, s9
	sd	t5, 48(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	ld	a5, 40(sp)
	ld	a6, 48(sp)
	call	loop3
	mv	t0, a0
	mv	a0, t0
	ld	s9, 128(sp)
	ld	s8, 120(sp)
	ld	s7, 112(sp)
	ld	s6, 104(sp)
	ld	s5, 96(sp)
	ld	s4, 88(sp)
	ld	s3, 80(sp)
	ld	s2, 72(sp)
	ld	s1, 64(sp)
	ld	ra, 56(sp)
	addi	sp, sp, 144
	ret
	.size main, .-main
