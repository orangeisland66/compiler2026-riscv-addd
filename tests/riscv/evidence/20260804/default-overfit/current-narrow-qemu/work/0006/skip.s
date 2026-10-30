
  .section .rodata
  .globl base
  .align 2
  .type  base, @object
  .size  base, 4
base:
  .word 16

  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 120000040
a:
  .zero 120000040

  .bss
  .globl ans
  .align 2
  .type  ans, @object
  .size  ans, 4
ans:
  .zero 4

  .text

  .globl radixSort
  .type  radixSort, @function
radixSort:
.radixSort.bb0:
	addi	sp, sp, -304
	sd	ra, 224(sp)
	sd	s1, 232(sp)
	sd	s2, 240(sp)
	sd	s3, 248(sp)
	sd	s4, 256(sp)
	sd	s5, 264(sp)
	sd	s6, 272(sp)
	sd	s7, 280(sp)
	sd	s8, 288(sp)
	sd	s9, 296(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	addi	t5, sp, 32
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	addi	t5, sp, 96
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	addi	t5, sp, 160
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	li	t0, -1
	subw	t0, s1, t0
	sltiu	t0, t0, 1
	addiw	t1, s3, 1
	slt	t1, t1, s4
	xori	t1, t1, 1
	or	t0, t0, t1
	bne	t0, zero, .radixSort.bb1
	j	.radixSort.bb3
.radixSort.bb2:
.radixSort.bb1:
	ld	s9, 296(sp)
	ld	s8, 288(sp)
	ld	s7, 280(sp)
	ld	s6, 272(sp)
	ld	s5, 264(sp)
	ld	s4, 256(sp)
	ld	s3, 248(sp)
	ld	s2, 240(sp)
	ld	s1, 232(sp)
	ld	ra, 224(sp)
	addi	sp, sp, 304
	ret
.radixSort.bb39:
	beq	a3, t1, .radixSort.bb18
.radixSort.bb17:
	slliw	a3, a3, 2
	addi	a4, sp, 32
	add	s4, a4, a3
	lw	s7, 0(s4)
	slliw	a3, s7, 2
	add	a3, s2, a3
	lw	a4, 0(a3)
	sw	a2, 0(a3)
	addiw	a2, s7, 1
	sw	a2, 0(s4)
	mv	a2, a4	# phi-copy
.radixSort.bb16:
	blt	s1, zero, .radixSort.bb33
.radixSort.bb34:
	li	a3, 7
	blt	a3, s1, .radixSort.bb35
.radixSort.bb36:
	bge	a2, zero, .radixSort.bb37
.radixSort.bb38:
	sraiw	a3, a2, 31
	and	a3, a3, s6
	addw	a3, a2, a3
	sraw	s4, a3, s5
	sraiw	a3, s4, 31
	andi	s7, a3, 15
	addw	a3, s4, s7
	andi	a3, a3, 15
	subw	a3, a3, s7
	j	.radixSort.bb39
.radixSort.bb33:
	sraiw	a3, a2, 31
	andi	s4, a3, 15
	addw	a3, a2, s4
	andi	a3, a3, 15
	subw	a3, a3, s4
	j	.radixSort.bb39
.radixSort.bb35:
	li	a3, 0
	j	.radixSort.bb39
.radixSort.bb37:
	sraw	a3, a2, s5
	andi	a3, a3, 15
	j	.radixSort.bb39
.radixSort.bb18:
	lw	s4, 0(t0)
	slliw	a3, s4, 2
	add	a3, s2, a3
	sw	a2, 0(a3)
	addiw	a2, s4, 1
	sw	a2, 0(t0)
.radixSort.bb13:
	lw	s4, 0(t0)
	bge	s4, t3, .radixSort.bb15
.radixSort.bb14:
	slliw	a2, s4, 2
	add	a2, s2, a2
	lw	a2, 0(a2)
	j	.radixSort.bb16
.radixSort.bb15:
	addiw	t1, t1, 1
	addi	t0, t0, 4
	addi	t2, t2, 4
.radixSort.bb10:
	li	t3, 16
	bge	t1, t3, .radixSort.bb12
.radixSort.bb11:
	lw	t3, 0(t2)
	j	.radixSort.bb13
.radixSort.bb7:
	li	t3, 16
	bge	t1, t3, .radixSort.bb9
.radixSort.bb24:
	addiw	t3, t1, 3
	li	a2, 16
	blt	t3, a2, .radixSort.bb25
.radixSort.bb8:
	addiw	t3, t1, -1
	addi	a2, sp, 96
	slliw	t3, t3, 2
	slliw	s8, t1, 2
	add	s4, a2, t3
	addiw	t1, t1, 1
	lw	s7, 0(s4)
	addi	t3, sp, 32
	add	t3, t3, s8
	sw	s7, 0(t3)
	addi	t3, sp, 160
	add	t3, t3, s8
	lw	t3, 0(t3)
	addw	t3, s7, t3
	sw	t3, 4(s4)
	j	.radixSort.bb7
.radixSort.bb25:
	addiw	t3, t1, -1
	addi	a2, sp, 96
	slliw	t3, t3, 2
	slliw	s8, t1, 2
	add	s4, a2, t3
	addiw	t1, t1, 4
	lw	s7, 0(s4)
	addi	t3, sp, 32
	add	s9, t3, s8
	addi	t3, sp, 160
	sw	s7, 0(s9)
	add	s8, t3, s8
	lw	t3, 0(s8)
	addw	s7, s7, t3
	sw	s7, 4(s4)
	sw	s7, 4(s9)
	lw	a4, 4(s8)
	addw	s7, s7, a4
	sw	s7, 8(s4)
	sw	s7, 8(s9)
	lw	a4, 8(s8)
	addw	s7, s7, a4
	sw	s7, 12(s4)
	sw	s7, 12(s9)
	lw	a2, 12(s8)
	addw	a2, s7, a2
	sw	a2, 16(s4)
	j	.radixSort.bb7
.radixSort.bb9:
	li	t1, 0
	j	.radixSort.bb10
.radixSort.bb32:
	slliw	t1, t1, 2
	addi	t2, sp, 160
	add	s8, t2, t1
	addi	t0, t0, 4
	lw	t1, 0(s8)
	addiw	t1, t1, 1
	sw	t1, 0(s8)
	addiw	s7, s7, 1
	bge	s7, s4, .radixSort.bb6	# rotated-loop-branch
.radixSort.bb5:
	lw	s8, 0(t0)
	blt	s1, zero, .radixSort.bb26
.radixSort.bb27:
	li	t1, 7
	blt	t1, s1, .radixSort.bb28
.radixSort.bb29:
	bge	s8, zero, .radixSort.bb30
.radixSort.bb31:
	sraiw	t1, s8, 31
	and	t1, t1, s6
	addw	t1, s8, t1
	sraw	s8, t1, s5
	sraiw	t1, s8, 31
	andi	s9, t1, 15
	addw	t1, s8, s9
	andi	t1, t1, 15
	subw	t1, t1, s9
	j	.radixSort.bb32
.radixSort.bb26:
	sraiw	t1, s8, 31
	andi	s9, t1, 15
	addw	t1, s8, s9
	andi	t1, t1, 15
	subw	t1, t1, s9
	j	.radixSort.bb32
.radixSort.bb28:
	li	t1, 0
	j	.radixSort.bb32
.radixSort.bb30:
	sraw	t1, s8, s5
	andi	t1, t1, 15
	j	.radixSort.bb32
.radixSort.bb6:
	addi	t1, sp, 32
	addi	t3, sp, 96
	sw	s3, 0(t1)
	addi	t0, sp, 32
	addi	t2, sp, 96
	addi	t1, sp, 160
	lw	t1, 0(t1)
	addw	t1, s3, t1
	sw	t1, 0(t3)
	li	t1, 1
	j	.radixSort.bb7
.radixSort.bb20:
	blt	zero, s5, .radixSort.bb22
.radixSort.bb23:
	slliw	s6, s5, 2
	addi	t0, sp, 32
	add	t0, t0, s6
	lw	s7, 0(t0)
	addi	t0, sp, 96
	add	t0, t0, s6
	lw	s6, 0(t0)
	addiw	t0, s7, 1
	slt	t0, t0, s6
	xori	t0, t0, 1
	or	t0, s3, t0
	beq	t0, zero, .radixSort.bb40
.radixSort.bb41:
	addiw	s5, s5, 1
	blt	s5, s4, .radixSort.bb20	# rotated-loop-branch
	j	.radixSort.bb1	# rotated-loop-exit
.radixSort.bb21:
	j	.radixSort.bb1
.radixSort.bb22:
	addiw	t0, s5, -1
	addi	t1, sp, 96
	slliw	t0, t0, 2
	slliw	s8, s5, 2
	add	s6, t1, t0
	lw	s7, 0(s6)
	addi	t0, sp, 32
	add	t0, t0, s8
	sw	s7, 0(t0)
	addi	t0, sp, 160
	add	t0, t0, s8
	lw	t0, 0(t0)
	addw	t0, s7, t0
	sw	t0, 4(s6)
	j	.radixSort.bb23
.radixSort.bb40:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s7	# call-arg-copy
	mv	a3, s6	# call-arg-copy
	call	radixSort
	j	.radixSort.bb41
.radixSort.bb12:
	addi	t0, sp, 32
	addi	t1, sp, 96
	sw	s3, 0(t0)
	addiw	s1, s1, -1
	addi	t0, sp, 160
	li	s4, 16
	lw	t0, 0(t0)
	addw	t0, s3, t0
	sw	t0, 0(t1)
	li	t0, -1
	subw	t0, s1, t0
	sltiu	s3, t0, 1
	li	s5, 0
.radixSort.bb19:
	bge	s5, s4, .radixSort.bb1
	j	.radixSort.bb20
.radixSort.bb3:
	slliw	s5, s1, 2
	li	t1, 1
	slliw	t0, s3, 2
	sllw	t1, t1, s5
	add	t0, s2, t0
	addiw	s6, t1, -1
	mv	s7, s3	# phi-copy
.radixSort.bb4:
	bge	s7, s4, .radixSort.bb6
	j	.radixSort.bb5
	.size radixSort, .-radixSort

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -96
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	sd	s6, 80(sp)
	la	t5, a
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s1, a0
	li	t5, 90
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 1
	blt	t0, s1, .main.bb8
.main.bb9:
	li	t0, 0
.main.bb1:
	bge	t0, s1, .main.bb3
.main.bb6:
	addiw	s2, t0, 3
	blt	s2, s1, .main.bb7
.main.bb2:
	slliw	t2, t0, 2
	la	t3, a
	add	t2, t3, t2
	la	s2, ans
	lw	t2, 0(t2)
	addiw	t3, t0, 2
	lw	t1, 0(s2)
	remw	t2, t2, t3
	mulw	t2, t0, t2
	addw	t1, t1, t2
	addiw	t0, t0, 1
	addiw	t1, t1, 3
	sw	t1, 0(s2)
	j	.main.bb1
.main.bb7:
	slliw	t2, t0, 2
	la	t3, a
	add	s4, t3, t2
	addiw	s5, t0, 2
	lw	t2, 0(s4)
	addiw	s6, t0, 1
	lw	t3, 4(s4)
	addiw	a2, s6, 2
	la	s3, ans
	lw	t1, 0(s3)
	remw	t2, t2, s5
	remw	t3, t3, a2
	mulw	t2, t0, t2
	mulw	t3, s6, t3
	addiw	a2, s5, 2
	addw	t1, t1, t2
	addiw	t0, t0, 4
	addiw	t1, t1, 3
	addw	t1, t1, t3
	lw	t3, 8(s4)
	lw	t2, 12(s4)
	addiw	t1, t1, 3
	remw	t3, t3, a2
	mulw	t3, s5, t3
	addw	t1, t1, t3
	addiw	t3, s2, 2
	addiw	t1, t1, 3
	remw	t2, t2, t3
	mulw	t2, s2, t2
	addw	t1, t1, t2
	addiw	t1, t1, 3
	sw	t1, 0(s3)
	j	.main.bb1
.main.bb3:
	la	t0, ans
	lw	t0, 0(t0)
	blt	t0, zero, .main.bb4
.main.bb5:
	li	a0, 102
	call	_sysy_stoptime
	la	t0, ans
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s6, 80(sp)
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 96
	ret
.main.bb4:
	subw	t0, zero, t0
	la	t1, ans
	sw	t0, 0(t1)
	j	.main.bb5
.main.bb8:
	li	t5, 9
	sd	t5, 0(sp)
	la	t5, a
	sd	t5, 8(sp)
	li	t5, 0
	sd	t5, 16(sp)
	mv	t5, s1
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	radixSort
	j	.main.bb9
	.size main, .-main
