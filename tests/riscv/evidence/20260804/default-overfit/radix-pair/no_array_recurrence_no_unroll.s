
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
.radixSort.bb37:
	beq	t3, t0, .radixSort.bb18
.radixSort.bb17:
	slliw	t3, t3, 2
	addi	a2, sp, 32
	add	s4, a2, t3
	lw	s8, 0(s4)
	slliw	t3, s8, 2
	add	t3, s2, t3
	lw	a2, 0(t3)
	sw	t2, 0(t3)
	addiw	t2, s8, 1
	sw	t2, 0(s4)
	mv	t2, a2	# phi-copy
.radixSort.bb16:
	blt	s1, zero, .radixSort.bb31
.radixSort.bb32:
	li	t3, 7
	blt	t3, s1, .radixSort.bb33
.radixSort.bb34:
	bge	t2, zero, .radixSort.bb35
.radixSort.bb36:
	sraiw	t3, t2, 31
	and	t3, t3, s6
	addw	t3, t2, t3
	sraw	s4, t3, s5
	sraiw	t3, s4, 31
	andi	s8, t3, 15
	addw	t3, s4, s8
	andi	t3, t3, 15
	subw	t3, t3, s8
	j	.radixSort.bb37
.radixSort.bb31:
	sraiw	t3, t2, 31
	andi	s4, t3, 15
	addw	t3, t2, s4
	andi	t3, t3, 15
	subw	t3, t3, s4
	j	.radixSort.bb37
.radixSort.bb33:
	li	t3, 0
	j	.radixSort.bb37
.radixSort.bb35:
	sraw	t3, t2, s5
	andi	t3, t3, 15
	j	.radixSort.bb37
.radixSort.bb18:
	lw	s4, 0(s7)
	slliw	t3, s4, 2
	add	t3, s2, t3
	sw	t2, 0(t3)
	addiw	t2, s4, 1
	sw	t2, 0(s7)
.radixSort.bb13:
	lw	s4, 0(s7)
	bge	s4, t1, .radixSort.bb15
.radixSort.bb14:
	slliw	t2, s4, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	j	.radixSort.bb16
.radixSort.bb15:
	addiw	t0, t0, 1
.radixSort.bb10:
	li	t1, 16
	bge	t0, t1, .radixSort.bb12
.radixSort.bb11:
	slliw	s4, t0, 2
	addi	t1, sp, 32
	add	s7, t1, s4
	addi	t1, sp, 96
	add	t1, t1, s4
	lw	t1, 0(t1)
	j	.radixSort.bb13
.radixSort.bb30:
	slliw	t0, t0, 2
	addi	t1, sp, 160
	add	s8, t1, t0
	lw	t0, 0(s8)
	addiw	t0, t0, 1
	sw	t0, 0(s8)
	addiw	s7, s7, 1
	bge	s7, s4, .radixSort.bb6	# rotated-loop-branch
.radixSort.bb5:
	slliw	t0, s7, 2
	add	t0, s2, t0
	lw	s8, 0(t0)
	blt	s1, zero, .radixSort.bb24
.radixSort.bb25:
	li	t0, 7
	blt	t0, s1, .radixSort.bb26
.radixSort.bb27:
	bge	s8, zero, .radixSort.bb28
.radixSort.bb29:
	sraiw	t0, s8, 31
	and	t0, t0, s6
	addw	t0, s8, t0
	sraw	s8, t0, s5
	sraiw	t0, s8, 31
	andi	s9, t0, 15
	addw	t0, s8, s9
	andi	t0, t0, 15
	subw	t0, t0, s9
	j	.radixSort.bb30
.radixSort.bb24:
	sraiw	t0, s8, 31
	andi	s9, t0, 15
	addw	t0, s8, s9
	andi	t0, t0, 15
	subw	t0, t0, s9
	j	.radixSort.bb30
.radixSort.bb26:
	li	t0, 0
	j	.radixSort.bb30
.radixSort.bb28:
	sraw	t0, s8, s5
	andi	t0, t0, 15
	j	.radixSort.bb30
.radixSort.bb6:
	addi	t0, sp, 32
	addi	t1, sp, 96
	sw	s3, 0(t0)
	addi	t0, sp, 160
	lw	t0, 0(t0)
	addw	t0, s3, t0
	sw	t0, 0(t1)
	li	t0, 1
	li	t1, 16
.radixSort.bb7:
	bge	t0, t1, .radixSort.bb9
.radixSort.bb8:
	addiw	t2, t0, -1
	addi	t3, sp, 96
	slliw	t2, t2, 2
	slliw	s8, t0, 2
	add	s4, t3, t2
	addiw	t0, t0, 1
	lw	s7, 0(s4)
	addi	t2, sp, 32
	add	t2, t2, s8
	sw	s7, 0(t2)
	addi	t2, sp, 160
	add	t2, t2, s8
	lw	t2, 0(t2)
	addw	t2, s7, t2
	sw	t2, 4(s4)
	blt	t0, t1, .radixSort.bb8	# rotated-loop-branch
.radixSort.bb9:
	li	t0, 0
	j	.radixSort.bb10
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
	beq	t0, zero, .radixSort.bb38
.radixSort.bb39:
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
.radixSort.bb38:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s7	# call-arg-copy
	mv	a3, s6	# call-arg-copy
	call	radixSort
	j	.radixSort.bb39
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
	li	t0, 1
	sllw	t0, t0, s5
	addiw	s6, t0, -1
	mv	s7, s3	# phi-copy
.radixSort.bb4:
	bge	s7, s4, .radixSort.bb6
	j	.radixSort.bb5
	.size radixSort, .-radixSort

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
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
	blt	t0, s1, .main.bb6
.main.bb7:
	li	t0, 0
	la	t1, a	# loop.addr.init
	slliw	t2, t0, 2	# loop.addr.init
	add	t1, t1, t2	# loop.addr.init
.main.bb1:
	bge	t0, s1, .main.bb3
.main.bb2:
	la	s2, ans
	lw	t3, 0(t1)
	addiw	a2, t0, 2
	lw	t2, 0(s2)
	remw	t3, t3, a2
	addi	t1, t1, 4	# loop.addr.bump
	mulw	t3, t0, t3
	addw	t2, t2, t3
	addiw	t0, t0, 1
	sw	t2, 0(s2)
	blt	t0, s1, .main.bb2	# rotated-loop-branch
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
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 64
	ret
.main.bb4:
	subw	t0, zero, t0
	la	t1, ans
	sw	t0, 0(t1)
	j	.main.bb5
.main.bb6:
	li	t5, 8
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
	j	.main.bb7
	.size main, .-main
