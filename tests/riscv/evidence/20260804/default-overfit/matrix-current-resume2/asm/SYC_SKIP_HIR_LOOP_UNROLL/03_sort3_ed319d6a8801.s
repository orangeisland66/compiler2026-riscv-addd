
  .section .rodata
  .globl base
  .align 2
  .type  base, @object
  .size  base, 4
base:
  .word 16

  .bss
  .globl __syc_radix_scratch
  .align 2
  .type  __syc_radix_scratch, @object
  .size  __syc_radix_scratch, 120000040
__syc_radix_scratch:
  .zero 120000040

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
	li	t6, -2144
	add	sp, sp, t6
	li	t6, 2048
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 2056
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 2064
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 2072
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 2080
	add	t6, sp, t6
	sd	s5, 0(t6)
	li	t6, 2088
	add	t6, sp, t6
	sd	s6, 0(t6)
	li	t6, 2096
	add	t6, sp, t6
	sd	s7, 0(t6)
	li	t6, 2104
	add	t6, sp, t6
	sd	s8, 0(t6)
	li	t6, 2112
	add	t6, sp, t6
	sd	s9, 0(t6)
	li	t6, 2120
	add	t6, sp, t6
	sd	s10, 0(t6)
	li	t6, 2128
	add	t6, sp, t6
	sd	s11, 0(t6)
	mv	s1, a0
	mv	t0, a1
	mv	s2, a2
	mv	s3, a3
	li	t1, 0
	slt	t1, s1, t1
	addiw	t2, s2, 1
	slt	t2, t2, s3
	xori	t2, t2, 1
	or	t1, t1, t2
	addi	t2, sp, 0
	addi	s4, sp, 1024
	bne	t1, zero, .radixSort.bb1
	j	.radixSort.bb3
.radixSort.bb2:
.radixSort.bb1:
	li	t6, 2128
	add	t6, sp, t6
	ld	s11, 0(t6)
	li	t6, 2120
	add	t6, sp, t6
	ld	s10, 0(t6)
	li	t6, 2112
	add	t6, sp, t6
	ld	s9, 0(t6)
	li	t6, 2104
	add	t6, sp, t6
	ld	s8, 0(t6)
	li	t6, 2096
	add	t6, sp, t6
	ld	s7, 0(t6)
	li	t6, 2088
	add	t6, sp, t6
	ld	s6, 0(t6)
	li	t6, 2080
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 2072
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 2064
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 2056
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 2048
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 2144
	add	sp, sp, t6
	ret
.radixSort.bb19:
	li	a5, 0
	addiw	a3, a3, 1
	sw	a5, 0(s5)
	addi	s5, s5, 4
	blt	a3, a4, .radixSort.bb19	# rotated-loop-branch
.radixSort.bb20:
	addiw	s5, a2, 1
	slliw	s7, a2, 3
	subw	a2, t0, s7
	li	a4, 1
	slliw	a3, s5, 3
	sllw	a2, a4, a2
	slt	a3, t0, a3
	addiw	a2, a2, -1
	li	a4, 0
	xori	a2, a2, 255
	subw	a3, a4, a3
	and	a2, a2, a3
	xori	s8, a2, 255
.radixSort.bb21:
	mv	s9, s2	# phi-copy
	mv	s10, s6	# phi-copy
.radixSort.bb22:
	bge	s9, s3, .radixSort.bb24
.radixSort.bb23:
	lw	a2, 0(s10)
	addi	a3, s10, 4
	sraw	a2, a2, s7
	and	a2, a2, s8
	slliw	a2, a2, 2
	add	s11, t2, a2
	lw	a2, 0(s11)
	addiw	a2, a2, 1
	sw	a2, 0(s11)
	addiw	s9, s9, 1
	mv	s10, a3	# phi-copy
	blt	s9, s3, .radixSort.bb23	# rotated-loop-branch
.radixSort.bb24:
	sw	s2, 0(s4)
	li	a2, 1
	li	a3, 256
	mv	s9, s1	# phi-copy
.radixSort.bb25:
	bge	a2, a3, .radixSort.bb27
.radixSort.bb26:
	lw	a5, 0(s9)
	addiw	a4, a2, -1
	slliw	a4, a4, 2
	addiw	a2, a2, 1
	add	s10, s4, a4
	lw	a4, 0(s10)
	addw	a4, a4, a5
	sw	a4, 4(s10)
	addi	s9, s9, 4
	blt	a2, a3, .radixSort.bb26	# rotated-loop-branch
.radixSort.bb27:
	mv	s9, s2	# phi-copy
	mv	s10, s6	# phi-copy
.radixSort.bb28:
	bge	s9, s3, .radixSort.bb30
.radixSort.bb29:
	lw	s11, 0(s10)
	la	a5, __syc_radix_scratch
	sraw	a2, s11, s7
	and	a2, a2, s8
	slliw	a2, a2, 2
	add	a2, s4, a2
	lw	a3, 0(a2)
	slliw	a4, a3, 2
	add	a4, a5, a4
	addiw	a3, a3, 1
	sw	s11, 0(a4)
	sw	a3, 0(a2)
	addiw	s9, s9, 1
	addi	s10, s10, 4
	blt	s9, s3, .radixSort.bb29	# rotated-loop-branch
.radixSort.bb30:
	mv	s7, s2	# phi-copy
	mv	s8, t3	# phi-copy
	mv	s9, s6	# phi-copy
.radixSort.bb31:
	bge	s7, s3, .radixSort.bb33
.radixSort.bb32:
	lw	a2, 0(s8)
	addi	s8, s8, 4
	addi	a4, s9, 4
	sw	a2, 0(s9)
	addiw	s7, s7, 1
	mv	s9, a4	# phi-copy
	blt	s7, s3, .radixSort.bb32	# rotated-loop-branch
.radixSort.bb33:
	mv	a2, s5	# phi-copy
	bge	a2, t1, .radixSort.bb1	# rotated-loop-branch
.radixSort.bb16:
	li	a3, 0
	li	a4, 256
	mv	s5, s1	# phi-copy
.radixSort.bb18:
	bge	a3, a4, .radixSort.bb20
	j	.radixSort.bb19
.radixSort.bb17:
	j	.radixSort.bb1
.radixSort.bb5:
	lw	s9, 0(s8)
	blt	t0, s9, .radixSort.bb8
	j	.radixSort.bb34
.radixSort.bb7:
.radixSort.bb8:
	addiw	s7, s7, 1
	addi	s8, s8, 4
	mv	t0, s9	# phi-copy
	blt	s7, s3, .radixSort.bb5	# rotated-loop-branch
.radixSort.bb6:
	li	t1, 1
	li	t3, 256
.radixSort.bb9:
	blt	t0, t3, .radixSort.bb11
.radixSort.bb10:
	sraiw	a2, t0, 31
	addiw	t1, t1, 1
	andi	a2, a2, 255
	addw	t0, t0, a2
	sraiw	t0, t0, 8
	bge	t0, t3, .radixSort.bb10	# rotated-loop-branch
.radixSort.bb11:
	addiw	t0, s1, 1
	li	t3, 32
	slliw	t0, t0, 2
	blt	t3, t0, .radixSort.bb12
.radixSort.bb35:
.radixSort.bb13:
	addiw	a2, t0, 7
	slliw	t3, t1, 3
	sraiw	a3, a2, 31
	slt	t3, t0, t3
	andi	a3, a3, 7
	addw	a2, a2, a3
	sraiw	a2, a2, 3
	li	a3, 0
	subw	t3, a3, t3
	xor	a2, t1, a2
	and	t3, a2, t3
	xor	t1, t1, t3
.radixSort.bb14:
	addi	s1, sp, 0
	la	t3, __syc_radix_scratch
	li	a2, 0
	add	t3, t3, s5
.radixSort.bb15:
	bge	a2, t1, .radixSort.bb1
	j	.radixSort.bb16
.radixSort.bb34:
	mv	s9, t0	# phi-copy
	j	.radixSort.bb8
.radixSort.bb12:
	li	t0, 32
	j	.radixSort.bb13
.radixSort.bb3:
	slliw	s5, s2, 2
	add	s6, t0, s5
	li	t0, 0
	mv	s7, s2	# phi-copy
	mv	s8, s6	# phi-copy
.radixSort.bb4:
	bge	s7, s3, .radixSort.bb6
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
	la	t0, ans
	lw	s1, 0(t0)
	la	t5, a
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s2, a0
	li	t5, 90
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 1
	blt	t0, s2, .main.bb6
.main.bb7:
	la	t0, a
	li	t1, 0
.main.bb1:
	bge	t1, s2, .main.bb3
.main.bb2:
	lw	t2, 0(t0)
	addiw	t3, t1, 2
	remw	t2, t2, t3
	addi	t0, t0, 4
	mulw	t2, t1, t2
	addw	t2, s1, t2
	addiw	t1, t1, 1
	addiw	s1, t2, 3
	blt	t1, s2, .main.bb2	# rotated-loop-branch
.main.bb3:
	blt	s1, zero, .main.bb4
.main.bb8:
.main.bb5:
	li	a0, 102
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	la	t0, ans
	sw	s1, 0(t0)
	li	a0, 0
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 64
	ret
.main.bb4:
	subw	s1, zero, s1
	j	.main.bb5
.main.bb6:
	li	t5, 9
	sd	t5, 0(sp)
	la	t5, a
	sd	t5, 8(sp)
	li	t5, 0
	sd	t5, 16(sp)
	mv	t5, s2
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	radixSort
	j	.main.bb7
	.size main, .-main
