
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

  .globl getNumPos
  .type  getNumPos, @function
getNumPos:
.getNumPos.bb0:
	mv	t0, a0
	mv	t1, a1
	blt	t1, zero, .getNumPos.bb1
.getNumPos.bb2:
	li	t2, 7
	blt	t2, t1, .getNumPos.bb3
.getNumPos.bb4:
	slliw	t1, t1, 2
	bge	t0, zero, .getNumPos.bb5
.getNumPos.bb6:
	li	t2, 1
	sraiw	t3, t0, 31
	sllw	t2, t2, t1
	addiw	t2, t2, -1
	and	t2, t3, t2
	addw	t0, t0, t2
	sraw	t0, t0, t1
	sraiw	t1, t0, 31
	andi	t1, t1, 15
	addw	t0, t0, t1
	andi	t0, t0, 15
	subw	t0, t0, t1
.getNumPos.bb7:
	mv	a0, t0
	ret
.getNumPos.bb3:
	li	t0, 0
	j	.getNumPos.bb7
.getNumPos.bb5:
	sraw	t0, t0, t1
	andi	t0, t0, 15
	j	.getNumPos.bb7
.getNumPos.bb1:
	sraiw	t1, t0, 31
	andi	t1, t1, 15
	addw	t0, t0, t1
	andi	t0, t0, 15
	subw	t0, t0, t1
	j	.getNumPos.bb7
	.size getNumPos, .-getNumPos

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
.radixSort.bb16:
	mv	a0, s7	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	call	getNumPos
	mv	s8, a0
	beq	s8, s4, .radixSort.bb18
.radixSort.bb17:
	slliw	t0, s8, 2
	addi	t1, sp, 32
	add	s8, t1, t0
	lw	s9, 0(s8)
	slliw	t0, s9, 2
	add	t0, s2, t0
	lw	t1, 0(t0)
	sw	s7, 0(t0)
	addiw	t0, s9, 1
	sw	t0, 0(s8)
	mv	s7, t1	# phi-copy
	j	.radixSort.bb16
.radixSort.bb18:
	lw	s8, 0(s6)
	slliw	t0, s8, 2
	add	t0, s2, t0
	sw	s7, 0(t0)
	addiw	t0, s8, 1
	sw	t0, 0(s6)
.radixSort.bb13:
	lw	s7, 0(s6)
	bge	s7, s5, .radixSort.bb15
.radixSort.bb14:
	slliw	t0, s7, 2
	add	t0, s2, t0
	lw	s7, 0(t0)
	j	.radixSort.bb16
.radixSort.bb15:
	addiw	s4, s4, 1
.radixSort.bb10:
	li	t0, 16
	bge	s4, t0, .radixSort.bb12
.radixSort.bb11:
	slliw	s5, s4, 2
	addi	t0, sp, 32
	add	s6, t0, s5
	addi	t0, sp, 96
	add	t0, t0, s5
	lw	s5, 0(t0)
	j	.radixSort.bb13
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
	beq	t0, zero, .radixSort.bb24
.radixSort.bb25:
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
.radixSort.bb24:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s7	# call-arg-copy
	mv	a3, s6	# call-arg-copy
	call	radixSort
	j	.radixSort.bb25
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
.radixSort.bb5:
	slliw	t0, s5, 2
	add	t0, s2, t0
	lw	a0, 0(t0)
	mv	a1, s1	# call-arg-copy
	call	getNumPos
	mv	t0, a0
	slliw	t0, t0, 2
	addi	t1, sp, 160
	add	s6, t1, t0
	lw	t0, 0(s6)
	addiw	t0, t0, 1
	sw	t0, 0(s6)
	addiw	s5, s5, 1
	blt	s5, s4, .radixSort.bb5	# rotated-loop-branch
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
	slliw	s6, t0, 2
	add	s4, t3, t2
	addiw	t0, t0, 1
	lw	s5, 0(s4)
	addi	t2, sp, 32
	add	t2, t2, s6
	sw	s5, 0(t2)
	addi	t2, sp, 160
	add	t2, t2, s6
	lw	t2, 0(t2)
	addw	t2, s5, t2
	sw	t2, 4(s4)
	blt	t0, t1, .radixSort.bb8	# rotated-loop-branch
.radixSort.bb9:
	li	s4, 0
	j	.radixSort.bb10
.radixSort.bb3:
	mv	s5, s3	# phi-copy
.radixSort.bb4:
	bge	s5, s4, .radixSort.bb6
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
