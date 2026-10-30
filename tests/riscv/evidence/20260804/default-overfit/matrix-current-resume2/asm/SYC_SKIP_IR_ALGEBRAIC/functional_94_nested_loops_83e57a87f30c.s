
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
	addw	s5, s3, t3
	addw	a3, a3, a4
	addw	a3, s4, a3
	addw	a4, s5, s1
	la	a5, arr1
	addw	a4, a4, s2
	add	s6, a5, a3
	addiw	s7, s5, 1
	sw	a4, 0(s6)
	addw	a3, s7, s1
	addiw	a4, s7, 1
	addw	s8, a3, s2
	addw	a4, a4, s1
	sw	s8, 4(s6)
	addw	a4, a4, s2
	sw	s8, 8(s6)
	sw	a4, 12(s6)
	addiw	s7, s5, 2
	addw	a4, s7, s1
	addw	a4, a4, s2
	sw	a4, 16(s6)
	addiw	a4, s7, 1
	addw	a4, a4, s1
	addiw	s7, s5, 3
	addw	a4, a4, s2
	sw	a4, 20(s6)
	addw	a4, s7, s1
	addw	a4, a4, s2
	sw	a4, 24(s6)
	addiw	a4, s7, 1
	addw	a4, a4, s1
	addiw	s7, s5, 4
	addw	a4, a4, s2
	addiw	s5, s5, 5
	sw	a4, 28(s6)
	addw	a4, s7, s1
	addiw	t3, t3, 1
	addw	a4, a4, s2
	sw	a4, 32(s6)
	addiw	a4, s7, 1
	addw	a4, a4, s1
	addw	a4, a4, s2
	sw	a4, 36(s6)
	addw	a4, s5, s1
	addw	a4, a4, s2
	sw	a4, 40(s6)
	addiw	a4, s5, 1
	addw	a4, a4, s1
	addw	a4, a4, s2
	sw	a4, 44(s6)
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
	addw	s6, s5, a2
	addw	a4, a4, a5
	addw	a4, t3, a4
	addw	a5, s6, s1
	la	a6, arr1
	addw	a5, a5, s2
	add	s7, a6, a4
	addiw	s8, s6, 1
	sw	a5, 0(s7)
	addw	a4, s8, s1
	addiw	a5, s8, 1
	addw	s9, a4, s2
	addw	a5, a5, s1
	sw	s9, 4(s7)
	addw	a5, a5, s2
	sw	s9, 8(s7)
	sw	a5, 12(s7)
	addiw	s8, s6, 2
	addw	a5, s8, s1
	addw	a5, a5, s2
	sw	a5, 16(s7)
	addiw	a5, s8, 1
	addw	a5, a5, s1
	addiw	s8, s6, 3
	addw	a5, a5, s2
	sw	a5, 20(s7)
	addw	a5, s8, s1
	addw	a5, a5, s2
	sw	a5, 24(s7)
	addiw	a5, s8, 1
	addw	a5, a5, s1
	addiw	s8, s6, 4
	addw	a5, a5, s2
	addiw	s6, s6, 5
	sw	a5, 28(s7)
	addw	a5, s8, s1
	addiw	a2, a2, 1
	addw	a5, a5, s2
	sw	a5, 32(s7)
	addiw	a5, s8, 1
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 36(s7)
	addw	a5, s6, s1
	addw	a5, a5, s2
	sw	a5, 40(s7)
	addiw	a5, s6, 1
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s7)
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
	addw	s7, s6, a2
	addw	a4, a4, a5
	addw	a4, t3, a4
	addw	a5, s7, s1
	la	a6, arr1
	addw	a5, a5, s2
	add	s8, a6, a4
	addiw	s9, s7, 1
	sw	a5, 0(s8)
	addw	a4, s9, s1
	addiw	a5, s9, 1
	addw	s10, a4, s2
	addw	a5, a5, s1
	sw	s10, 4(s8)
	addw	a5, a5, s2
	sw	s10, 8(s8)
	sw	a5, 12(s8)
	addiw	s9, s7, 2
	addw	a5, s9, s1
	addw	a5, a5, s2
	sw	a5, 16(s8)
	addiw	a5, s9, 1
	addw	a5, a5, s1
	addiw	s9, s7, 3
	addw	a5, a5, s2
	sw	a5, 20(s8)
	addw	a5, s9, s1
	addw	a5, a5, s2
	sw	a5, 24(s8)
	addiw	a5, s9, 1
	addw	a5, a5, s1
	addiw	s9, s7, 4
	addw	a5, a5, s2
	addiw	s7, s7, 5
	sw	a5, 28(s8)
	addw	a5, s9, s1
	addiw	a2, a2, 1
	addw	a5, a5, s2
	sw	a5, 32(s8)
	addiw	a5, s9, 1
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 36(s8)
	addw	a5, s7, s1
	addw	a5, a5, s2
	sw	a5, 40(s8)
	addiw	a5, s7, 1
	addw	a5, a5, s1
	addw	a5, a5, s2
	sw	a5, 44(s8)
	blt	a2, a3, .loop1.bb14	# rotated-loop-branch
.loop1.bb15:
	addiw	t3, s3, 3
	addiw	a2, s4, 720
	li	a3, 0
	li	a4, 5
.loop1.bb16:
	bge	a3, a4, .loop1.bb18
.loop1.bb17:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	s3, t3, a3
	addw	a5, a5, a6
	addw	a5, a2, a5
	addw	a6, s3, s1
	la	a7, arr1
	addw	a6, a6, s2
	add	s7, a7, a5
	addiw	s8, s3, 1
	sw	a6, 0(s7)
	addw	a5, s8, s1
	addiw	a6, s8, 1
	addw	s9, a5, s2
	addw	a6, a6, s1
	sw	s9, 4(s7)
	addw	a6, a6, s2
	sw	s9, 8(s7)
	sw	a6, 12(s7)
	addiw	s8, s3, 2
	addw	a6, s8, s1
	addw	a6, a6, s2
	sw	a6, 16(s7)
	addiw	a6, s8, 1
	addw	a6, a6, s1
	addiw	s8, s3, 3
	addw	a6, a6, s2
	sw	a6, 20(s7)
	addw	a6, s8, s1
	addw	a6, a6, s2
	sw	a6, 24(s7)
	addiw	a6, s8, 1
	addw	a6, a6, s1
	addiw	s8, s3, 4
	addw	a6, a6, s2
	addiw	s3, s3, 5
	sw	a6, 28(s7)
	addw	a6, s8, s1
	addiw	a3, a3, 1
	addw	a6, a6, s2
	sw	a6, 32(s7)
	addiw	a6, s8, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 36(s7)
	addw	a6, s3, s1
	addw	a6, a6, s2
	sw	a6, 40(s7)
	addiw	a6, s3, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 44(s7)
	blt	a3, a4, .loop1.bb17	# rotated-loop-branch
.loop1.bb18:
	addiw	s3, s4, 960
	li	t3, 0
	li	a2, 5
.loop1.bb19:
	bge	t3, a2, .loop1.bb21
.loop1.bb20:
	slliw	a3, t3, 5
	slliw	a4, t3, 4
	addw	s7, s5, t3
	addw	a3, a3, a4
	addw	a3, s3, a3
	addw	a4, s7, s1
	la	a5, arr1
	addw	a4, a4, s2
	add	s8, a5, a3
	addiw	s9, s7, 1
	sw	a4, 0(s8)
	addw	a3, s9, s1
	addiw	a4, s9, 1
	addw	s10, a3, s2
	addw	a4, a4, s1
	sw	s10, 4(s8)
	addw	a4, a4, s2
	sw	s10, 8(s8)
	sw	a4, 12(s8)
	addiw	s9, s7, 2
	addw	a4, s9, s1
	addw	a4, a4, s2
	sw	a4, 16(s8)
	addiw	a4, s9, 1
	addw	a4, a4, s1
	addiw	s9, s7, 3
	addw	a4, a4, s2
	sw	a4, 20(s8)
	addw	a4, s9, s1
	addw	a4, a4, s2
	sw	a4, 24(s8)
	addiw	a4, s9, 1
	addw	a4, a4, s1
	addiw	s9, s7, 4
	addw	a4, a4, s2
	addiw	s7, s7, 5
	sw	a4, 28(s8)
	addw	a4, s9, s1
	addiw	t3, t3, 1
	addw	a4, a4, s2
	sw	a4, 32(s8)
	addiw	a4, s9, 1
	addw	a4, a4, s1
	addw	a4, a4, s2
	sw	a4, 36(s8)
	addw	a4, s7, s1
	addw	a4, a4, s2
	sw	a4, 40(s8)
	addiw	a4, s7, 1
	addw	a4, a4, s1
	addw	a4, a4, s2
	sw	a4, 44(s8)
	blt	t3, a2, .loop1.bb20	# rotated-loop-branch
.loop1.bb21:
	addiw	t3, s5, 1
	addiw	a2, s3, 240
	li	a3, 0
	li	a4, 5
.loop1.bb22:
	bge	a3, a4, .loop1.bb24
.loop1.bb23:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	s7, t3, a3
	addw	a5, a5, a6
	addw	a5, a2, a5
	addw	a6, s7, s1
	la	a7, arr1
	addw	a6, a6, s2
	add	s8, a7, a5
	addiw	s9, s7, 1
	sw	a6, 0(s8)
	addw	a5, s9, s1
	addiw	a6, s9, 1
	addw	s10, a5, s2
	addw	a6, a6, s1
	sw	s10, 4(s8)
	addw	a6, a6, s2
	sw	s10, 8(s8)
	sw	a6, 12(s8)
	addiw	s9, s7, 2
	addw	a6, s9, s1
	addw	a6, a6, s2
	sw	a6, 16(s8)
	addiw	a6, s9, 1
	addw	a6, a6, s1
	addiw	s9, s7, 3
	addw	a6, a6, s2
	sw	a6, 20(s8)
	addw	a6, s9, s1
	addw	a6, a6, s2
	sw	a6, 24(s8)
	addiw	a6, s9, 1
	addw	a6, a6, s1
	addiw	s9, s7, 4
	addw	a6, a6, s2
	addiw	s7, s7, 5
	sw	a6, 28(s8)
	addw	a6, s9, s1
	addiw	a3, a3, 1
	addw	a6, a6, s2
	sw	a6, 32(s8)
	addiw	a6, s9, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 36(s8)
	addw	a6, s7, s1
	addw	a6, a6, s2
	sw	a6, 40(s8)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 44(s8)
	blt	a3, a4, .loop1.bb23	# rotated-loop-branch
.loop1.bb24:
	addiw	t3, s5, 2
	addiw	a2, s3, 480
	li	a3, 0
	li	a4, 5
.loop1.bb25:
	bge	a3, a4, .loop1.bb27
.loop1.bb26:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	s7, t3, a3
	addw	a5, a5, a6
	addw	a5, a2, a5
	addw	a6, s7, s1
	la	a7, arr1
	addw	a6, a6, s2
	add	s8, a7, a5
	addiw	s9, s7, 1
	sw	a6, 0(s8)
	addw	a5, s9, s1
	addiw	a6, s9, 1
	addw	s10, a5, s2
	addw	a6, a6, s1
	sw	s10, 4(s8)
	addw	a6, a6, s2
	sw	s10, 8(s8)
	sw	a6, 12(s8)
	addiw	s9, s7, 2
	addw	a6, s9, s1
	addw	a6, a6, s2
	sw	a6, 16(s8)
	addiw	a6, s9, 1
	addw	a6, a6, s1
	addiw	s9, s7, 3
	addw	a6, a6, s2
	sw	a6, 20(s8)
	addw	a6, s9, s1
	addw	a6, a6, s2
	sw	a6, 24(s8)
	addiw	a6, s9, 1
	addw	a6, a6, s1
	addiw	s9, s7, 4
	addw	a6, a6, s2
	addiw	s7, s7, 5
	sw	a6, 28(s8)
	addw	a6, s9, s1
	addiw	a3, a3, 1
	addw	a6, a6, s2
	sw	a6, 32(s8)
	addiw	a6, s9, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 36(s8)
	addw	a6, s7, s1
	addw	a6, a6, s2
	sw	a6, 40(s8)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 44(s8)
	blt	a3, a4, .loop1.bb26	# rotated-loop-branch
.loop1.bb27:
	addiw	t3, s5, 3
	addiw	a2, s3, 720
	li	a3, 0
	li	a4, 5
.loop1.bb28:
	bge	a3, a4, .loop1.bb30
.loop1.bb29:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	s3, t3, a3
	addw	a5, a5, a6
	addw	a5, a2, a5
	addw	a6, s3, s1
	la	a7, arr1
	addw	a6, a6, s2
	add	s5, a7, a5
	addiw	s7, s3, 1
	sw	a6, 0(s5)
	addw	a5, s7, s1
	addiw	a6, s7, 1
	addw	s8, a5, s2
	addw	a6, a6, s1
	sw	s8, 4(s5)
	addw	a6, a6, s2
	sw	s8, 8(s5)
	sw	a6, 12(s5)
	addiw	s7, s3, 2
	addw	a6, s7, s1
	addw	a6, a6, s2
	sw	a6, 16(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addiw	s7, s3, 3
	addw	a6, a6, s2
	sw	a6, 20(s5)
	addw	a6, s7, s1
	addw	a6, a6, s2
	sw	a6, 24(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addiw	s7, s3, 4
	addw	a6, a6, s2
	addiw	s3, s3, 5
	sw	a6, 28(s5)
	addw	a6, s7, s1
	addiw	a3, a3, 1
	addw	a6, a6, s2
	sw	a6, 32(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 36(s5)
	addw	a6, s3, s1
	addw	a6, a6, s2
	sw	a6, 40(s5)
	addiw	a6, s3, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 44(s5)
	blt	a3, a4, .loop1.bb29	# rotated-loop-branch
.loop1.bb30:
	addiw	s3, s4, 1920
	li	t3, 0
	li	a2, 5
.loop1.bb31:
	bge	t3, a2, .loop1.bb33
.loop1.bb32:
	slliw	a3, t3, 5
	slliw	a4, t3, 4
	addw	s4, s6, t3
	addw	a3, a3, a4
	addw	a3, s3, a3
	addw	a4, s4, s1
	la	a5, arr1
	addw	a4, a4, s2
	add	s5, a5, a3
	addiw	s7, s4, 1
	sw	a4, 0(s5)
	addw	a3, s7, s1
	addiw	a4, s7, 1
	addw	s8, a3, s2
	addw	a4, a4, s1
	sw	s8, 4(s5)
	addw	a4, a4, s2
	sw	s8, 8(s5)
	sw	a4, 12(s5)
	addiw	s7, s4, 2
	addw	a4, s7, s1
	addw	a4, a4, s2
	sw	a4, 16(s5)
	addiw	a4, s7, 1
	addw	a4, a4, s1
	addiw	s7, s4, 3
	addw	a4, a4, s2
	sw	a4, 20(s5)
	addw	a4, s7, s1
	addw	a4, a4, s2
	sw	a4, 24(s5)
	addiw	a4, s7, 1
	addw	a4, a4, s1
	addiw	s7, s4, 4
	addw	a4, a4, s2
	addiw	s4, s4, 5
	sw	a4, 28(s5)
	addw	a4, s7, s1
	addiw	t3, t3, 1
	addw	a4, a4, s2
	sw	a4, 32(s5)
	addiw	a4, s7, 1
	addw	a4, a4, s1
	addw	a4, a4, s2
	sw	a4, 36(s5)
	addw	a4, s4, s1
	addw	a4, a4, s2
	sw	a4, 40(s5)
	addiw	a4, s4, 1
	addw	a4, a4, s1
	addw	a4, a4, s2
	sw	a4, 44(s5)
	blt	t3, a2, .loop1.bb32	# rotated-loop-branch
.loop1.bb33:
	addiw	t3, s6, 1
	addiw	a2, s3, 240
	li	a3, 0
	li	a4, 5
.loop1.bb34:
	bge	a3, a4, .loop1.bb36
.loop1.bb35:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	s4, t3, a3
	addw	a5, a5, a6
	addw	a5, a2, a5
	addw	a6, s4, s1
	la	a7, arr1
	addw	a6, a6, s2
	add	s5, a7, a5
	addiw	s7, s4, 1
	sw	a6, 0(s5)
	addw	a5, s7, s1
	addiw	a6, s7, 1
	addw	s8, a5, s2
	addw	a6, a6, s1
	sw	s8, 4(s5)
	addw	a6, a6, s2
	sw	s8, 8(s5)
	sw	a6, 12(s5)
	addiw	s7, s4, 2
	addw	a6, s7, s1
	addw	a6, a6, s2
	sw	a6, 16(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addiw	s7, s4, 3
	addw	a6, a6, s2
	sw	a6, 20(s5)
	addw	a6, s7, s1
	addw	a6, a6, s2
	sw	a6, 24(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addiw	s7, s4, 4
	addw	a6, a6, s2
	addiw	s4, s4, 5
	sw	a6, 28(s5)
	addw	a6, s7, s1
	addiw	a3, a3, 1
	addw	a6, a6, s2
	sw	a6, 32(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 36(s5)
	addw	a6, s4, s1
	addw	a6, a6, s2
	sw	a6, 40(s5)
	addiw	a6, s4, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 44(s5)
	blt	a3, a4, .loop1.bb35	# rotated-loop-branch
.loop1.bb36:
	addiw	t3, s6, 2
	addiw	a2, s3, 480
	li	a3, 0
	li	a4, 5
.loop1.bb37:
	bge	a3, a4, .loop1.bb39
.loop1.bb38:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	s4, t3, a3
	addw	a5, a5, a6
	addw	a5, a2, a5
	addw	a6, s4, s1
	la	a7, arr1
	addw	a6, a6, s2
	add	s5, a7, a5
	addiw	s7, s4, 1
	sw	a6, 0(s5)
	addw	a5, s7, s1
	addiw	a6, s7, 1
	addw	s8, a5, s2
	addw	a6, a6, s1
	sw	s8, 4(s5)
	addw	a6, a6, s2
	sw	s8, 8(s5)
	sw	a6, 12(s5)
	addiw	s7, s4, 2
	addw	a6, s7, s1
	addw	a6, a6, s2
	sw	a6, 16(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addiw	s7, s4, 3
	addw	a6, a6, s2
	sw	a6, 20(s5)
	addw	a6, s7, s1
	addw	a6, a6, s2
	sw	a6, 24(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addiw	s7, s4, 4
	addw	a6, a6, s2
	addiw	s4, s4, 5
	sw	a6, 28(s5)
	addw	a6, s7, s1
	addiw	a3, a3, 1
	addw	a6, a6, s2
	sw	a6, 32(s5)
	addiw	a6, s7, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 36(s5)
	addw	a6, s4, s1
	addw	a6, a6, s2
	sw	a6, 40(s5)
	addiw	a6, s4, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 44(s5)
	blt	a3, a4, .loop1.bb38	# rotated-loop-branch
.loop1.bb39:
	addiw	t3, s6, 3
	addiw	a2, s3, 720
	li	a3, 0
	li	a4, 5
.loop1.bb40:
	bge	a3, a4, .loop1.bb42
.loop1.bb41:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	s3, t3, a3
	addw	a5, a5, a6
	addw	a5, a2, a5
	addw	a6, s3, s1
	la	a7, arr1
	addw	a6, a6, s2
	add	s4, a7, a5
	addiw	s5, s3, 1
	sw	a6, 0(s4)
	addw	a5, s5, s1
	addiw	a6, s5, 1
	addw	s6, a5, s2
	addw	a6, a6, s1
	sw	s6, 4(s4)
	addw	a6, a6, s2
	sw	s6, 8(s4)
	sw	a6, 12(s4)
	addiw	s5, s3, 2
	addw	a6, s5, s1
	addw	a6, a6, s2
	sw	a6, 16(s4)
	addiw	a6, s5, 1
	addw	a6, a6, s1
	addiw	s5, s3, 3
	addw	a6, a6, s2
	sw	a6, 20(s4)
	addw	a6, s5, s1
	addw	a6, a6, s2
	sw	a6, 24(s4)
	addiw	a6, s5, 1
	addw	a6, a6, s1
	addiw	s5, s3, 4
	addw	a6, a6, s2
	addiw	s3, s3, 5
	sw	a6, 28(s4)
	addw	a6, s5, s1
	addiw	a3, a3, 1
	addw	a6, a6, s2
	sw	a6, 32(s4)
	addiw	a6, s5, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 36(s4)
	addw	a6, s3, s1
	addw	a6, a6, s2
	sw	a6, 40(s4)
	addiw	a6, s3, 1
	addw	a6, a6, s1
	addw	a6, a6, s2
	sw	a6, 44(s4)
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
	addi	sp, sp, -96
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
	sd	s11, 80(sp)
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
	addiw	s8, s2, 1
	addiw	s9, s2, 2
	addiw	s10, s2, 3
	addiw	s11, s2, 4
	addiw	a2, s2, 5
	addiw	a3, s2, 6
	li	a4, 0
.loop2.bb7:
	li	a5, 3
	bge	a4, a5, .loop2.bb9
.loop2.bb8:
	slliw	a5, a4, 11
	slliw	a6, a4, 8
	subw	a5, a5, a6
	li	a7, 8
	addw	a5, t3, a5
	li	a6, 0
.loop2.bb10:
	bge	a6, a7, .loop2.bb12
.loop2.bb11:
	slliw	t4, a6, 5
	slliw	a0, a6, 2
	subw	t4, t4, a0
	addiw	a6, a6, 1
	addw	t4, a5, t4
	la	a0, arr2
	add	t4, a0, t4
	addi	a0, t4, 4
	sw	s1, 0(t4)
	sw	s2, 4(t4)
	sw	s3, 8(t4)
	sw	s4, 12(t4)
	sw	s5, 16(t4)
	sw	s6, 20(t4)
	sw	s7, 24(t4)
	blt	a6, a7, .loop2.bb11	# rotated-loop-branch
.loop2.bb12:
	addiw	a6, a5, 224
	li	a7, 0
	li	t4, 8
.loop2.bb13:
	bge	a7, t4, .loop2.bb15
.loop2.bb14:
	slliw	a0, a7, 5
	slliw	a1, a7, 2
	subw	a0, a0, a1
	addiw	a7, a7, 1
	addw	a0, a6, a0
	la	a1, arr2
	add	a0, a1, a0
	sw	s1, 0(a0)
	sw	s2, 4(a0)
	sw	s3, 8(a0)
	sw	s4, 12(a0)
	sw	s5, 16(a0)
	sw	s6, 20(a0)
	sw	s7, 24(a0)
	blt	a7, t4, .loop2.bb14	# rotated-loop-branch
.loop2.bb15:
	addiw	a6, a5, 448
	li	a7, 0
	li	t4, 8
.loop2.bb16:
	bge	a7, t4, .loop2.bb18
.loop2.bb17:
	slliw	a0, a7, 5
	slliw	a1, a7, 2
	subw	a0, a0, a1
	addiw	a7, a7, 1
	addw	a0, a6, a0
	la	a1, arr2
	add	a0, a1, a0
	sw	s1, 0(a0)
	sw	s2, 4(a0)
	sw	s3, 8(a0)
	sw	s4, 12(a0)
	sw	s5, 16(a0)
	sw	s6, 20(a0)
	sw	s7, 24(a0)
	blt	a7, t4, .loop2.bb17	# rotated-loop-branch
.loop2.bb18:
	addiw	a6, a5, 672
	li	a7, 0
	li	t4, 8
.loop2.bb19:
	bge	a7, t4, .loop2.bb21
.loop2.bb20:
	slliw	a0, a7, 5
	slliw	a1, a7, 2
	subw	a0, a0, a1
	addiw	a7, a7, 1
	addw	a0, a6, a0
	la	a1, arr2
	add	a0, a1, a0
	sw	s1, 0(a0)
	sw	s2, 4(a0)
	sw	s3, 8(a0)
	sw	s4, 12(a0)
	sw	s5, 16(a0)
	sw	s6, 20(a0)
	sw	s7, 24(a0)
	blt	a7, t4, .loop2.bb20	# rotated-loop-branch
.loop2.bb21:
	addiw	a5, a5, 896
	li	a6, 0
	li	a7, 8
.loop2.bb22:
	bge	a6, a7, .loop2.bb24
.loop2.bb23:
	slliw	t4, a6, 5
	slliw	a0, a6, 2
	subw	t4, t4, a0
	addiw	a6, a6, 1
	addw	t4, a5, t4
	la	a0, arr2
	add	t4, a0, t4
	addi	a0, t4, 4
	sw	s2, 0(t4)
	sw	s8, 4(t4)
	sw	s9, 8(t4)
	sw	s10, 12(t4)
	sw	s11, 16(t4)
	sw	a2, 20(t4)
	sw	a3, 24(t4)
	blt	a6, a7, .loop2.bb23	# rotated-loop-branch
.loop2.bb24:
	addiw	a6, a5, 224
	li	a7, 0
	li	t4, 8
.loop2.bb25:
	bge	a7, t4, .loop2.bb27
.loop2.bb26:
	slliw	a0, a7, 5
	slliw	a1, a7, 2
	subw	a0, a0, a1
	addiw	a7, a7, 1
	addw	a0, a6, a0
	la	a1, arr2
	add	a0, a1, a0
	sw	s2, 0(a0)
	sw	s8, 4(a0)
	sw	s9, 8(a0)
	sw	s10, 12(a0)
	sw	s11, 16(a0)
	sw	a2, 20(a0)
	sw	a3, 24(a0)
	blt	a7, t4, .loop2.bb26	# rotated-loop-branch
.loop2.bb27:
	addiw	a6, a5, 448
	li	a7, 0
	li	t4, 8
.loop2.bb28:
	bge	a7, t4, .loop2.bb30
.loop2.bb29:
	slliw	a0, a7, 5
	slliw	a1, a7, 2
	subw	a0, a0, a1
	addiw	a7, a7, 1
	addw	a0, a6, a0
	la	a1, arr2
	add	a0, a1, a0
	sw	s2, 0(a0)
	sw	s8, 4(a0)
	sw	s9, 8(a0)
	sw	s10, 12(a0)
	sw	s11, 16(a0)
	sw	a2, 20(a0)
	sw	a3, 24(a0)
	blt	a7, t4, .loop2.bb29	# rotated-loop-branch
.loop2.bb30:
	addiw	a5, a5, 672
	li	a6, 0
	li	a7, 8
.loop2.bb31:
	bge	a6, a7, .loop2.bb33
.loop2.bb32:
	slliw	t4, a6, 5
	slliw	a0, a6, 2
	subw	t4, t4, a0
	addiw	a6, a6, 1
	addw	t4, a5, t4
	la	a0, arr2
	add	t4, a0, t4
	addi	a0, t4, 4
	sw	s2, 0(t4)
	sw	s8, 4(t4)
	sw	s9, 8(t4)
	sw	s10, 12(t4)
	sw	s11, 16(t4)
	sw	a2, 20(t4)
	sw	a3, 24(t4)
	blt	a6, a7, .loop2.bb32	# rotated-loop-branch
.loop2.bb33:
	addiw	a4, a4, 1
	j	.loop2.bb7
.loop2.bb9:
	addiw	t2, t2, 1
	j	.loop2.bb4
.loop2.bb6:
	addiw	t0, t0, 1
	j	.loop2.bb1
.loop2.bb3:
	ld	s11, 80(sp)
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
	addi	sp, sp, 96
	ret
	.size loop2, .-loop2

  .globl loop3
  .type  loop3, @function
loop3:
.loop3.bb0:
	addi	sp, sp, -160
	sd	s1, 136(sp)
	sd	s2, 144(sp)
	mv	t5, a0
	sd	t5, 16(sp)
	mv	t5, a1
	sd	t5, 40(sp)
	mv	t5, a2
	sd	t5, 64(sp)
	mv	t5, a3
	sd	t5, 88(sp)
	mv	t5, a4
	sd	t5, 128(sp)
	mv	a3, a5
	mv	a4, a6
	li	a5, 0
	li	a6, 0
	sd	a6, 112(sp)
.loop3.bb1:
	li	a7, 10
	ld	t5, 112(sp)
	bge	t5, a7, .loop3.bb36
.loop3.bb2:
	li	a7, 5760
	ld	t5, 112(sp)
	mulw	t5, t5, a7
	sd	t5, 0(sp)
	li	t4, 10752
	ld	t5, 112(sp)
	mulw	t5, t5, t4
	sd	t5, 8(sp)
	li	a0, 0
.loop3.bb4:
	li	a1, 100
	bge	a0, a1, .loop3.bb37
.loop3.bb5:
	li	a1, 2880
	mulw	a1, a0, a1
	ld	t5, 0(sp)
	addw	t5, t5, a1
	sd	t5, 24(sp)
	li	a7, 5376
	mulw	a7, a0, a7
	ld	t5, 8(sp)
	addw	t5, t5, a7
	sd	t5, 32(sp)
	li	t4, 0
.loop3.bb7:
	li	t0, 1000
	bge	t4, t0, .loop3.bb38
.loop3.bb8:
	slliw	t0, t4, 10
	ld	t5, 24(sp)
	addw	t0, t5, t0
	slliw	a1, t4, 6
	subw	t5, t0, a1
	sd	t5, 48(sp)
	slliw	a1, t4, 11
	ld	t5, 32(sp)
	addw	a1, t5, a1
	slliw	a7, t4, 8
	subw	t5, a1, a7
	sd	t5, 56(sp)
	li	a1, 0
.loop3.bb10:
	li	t1, 10000
	bge	a1, t1, .loop3.bb39
.loop3.bb11:
	slliw	t1, a1, 8
	ld	t5, 48(sp)
	addw	t1, t5, t1
	slliw	t0, a1, 4
	subw	t5, t1, t0
	sd	t5, 72(sp)
	slliw	t1, a1, 10
	ld	t5, 56(sp)
	addw	t1, t5, t1
	slliw	a7, a1, 7
	subw	t5, t1, a7
	sd	t5, 80(sp)
	li	a7, 0
.loop3.bb13:
	li	t2, 100000
	bge	a7, t2, .loop3.bb40
.loop3.bb14:
	slliw	s1, a7, 5
	ld	t5, 72(sp)
	addw	t2, t5, s1
	slliw	t0, a7, 4
	addw	t5, t2, t0
	sd	t5, 96(sp)
	slliw	t2, a7, 8
	ld	t5, 80(sp)
	addw	t2, t5, t2
	subw	t5, t2, s1
	sd	t5, 104(sp)
	li	t1, 0
.loop3.bb16:
	li	t3, 1000000
	bge	t1, t3, .loop3.bb41
.loop3.bb17:
	slliw	t3, t1, 3
	ld	t5, 96(sp)
	addw	t3, t5, t3
	la	t0, arr1
	add	t0, t0, t3
	slliw	t3, t1, 5
	ld	t5, 104(sp)
	addw	t3, t5, t3
	slliw	t2, t1, 2
	subw	t2, t3, t2
	la	t3, arr2
	add	t2, t3, t2
	li	t3, 0
	li	t5, 10000000
	sd	t5, 120(sp)
.loop3.bb19:
	ld	t5, 120(sp)
	bge	t3, t5, .loop3.bb42
.loop3.bb20:
	li	a6, -1603384363
	li	a2, 817
	mul	a6, a5, a6	# const-div.fullmul
	addiw	s2, t3, 1
	srai	a6, a6, 32	# const-div.mulhi
	addw	a6, a6, a5	# const-div.fixup
	sraiw	a6, a6, 9	# const-div.shift
	srliw	s1, a6, 31	# const-div.sign
	addw	a6, a6, s1	# const-div.round
	mulw	a2, a6, a2	# const-rem.mul
	subw	a2, a5, a2	# const-rem.sub
	lw	a5, 0(t0)
	addw	a2, a2, a5
	lw	a5, 0(t2)
	addw	s1, a2, a5
	bge	s2, a4, .loop3.bb21
.loop3.bb23:
	addi	t0, t0, 4
	addi	t2, t2, 4
	mv	a5, s1	# phi-copy
	mv	t3, s2	# phi-copy
	j	.loop3.bb19
.loop3.bb42:
	mv	s1, a5	# phi-copy
.loop3.bb21:
	addiw	s2, t1, 1
	bge	s2, a3, .loop3.bb18
.loop3.bb25:
	mv	t1, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb16
.loop3.bb22:
	j	.loop3.bb21
.loop3.bb24:
.loop3.bb18:
	addiw	s2, a7, 1
	ld	t5, 128(sp)
	bge	s2, t5, .loop3.bb15
.loop3.bb27:
	mv	a7, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb13
.loop3.bb41:
	mv	s1, a5	# phi-copy
	j	.loop3.bb18
.loop3.bb26:
.loop3.bb15:
	addiw	s2, a1, 1
	ld	t5, 88(sp)
	bge	s2, t5, .loop3.bb12
.loop3.bb29:
	mv	a1, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb10
.loop3.bb40:
	mv	s1, a5	# phi-copy
	j	.loop3.bb15
.loop3.bb28:
.loop3.bb12:
	addiw	s2, t4, 1
	ld	t5, 64(sp)
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
	addiw	s2, a0, 1
	ld	t5, 40(sp)
	bge	s2, t5, .loop3.bb6
.loop3.bb33:
	mv	a0, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb4
.loop3.bb38:
	mv	s1, a5	# phi-copy
	j	.loop3.bb9
.loop3.bb32:
.loop3.bb6:
	ld	t5, 112(sp)
	addiw	s2, t5, 1
	ld	t5, 16(sp)
	bge	s2, t5, .loop3.bb3
.loop3.bb35:
	sd	s2, 112(sp)
	mv	a5, s1	# phi-copy
	j	.loop3.bb1
.loop3.bb37:
	mv	s1, a5	# phi-copy
	j	.loop3.bb6
.loop3.bb34:
.loop3.bb3:
	mv	a0, s1
	ld	s2, 144(sp)
	ld	s1, 136(sp)
	addi	sp, sp, 160
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
