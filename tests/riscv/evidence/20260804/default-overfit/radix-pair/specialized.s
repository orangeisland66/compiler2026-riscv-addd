
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
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	li	t0, 0
	slt	t0, s1, t0
	addiw	t1, s3, 1
	slt	t1, t1, s4
	xori	t1, t1, 1
	or	t0, t0, t1
	addi	s5, sp, 0
	addi	s6, sp, 1024
	bne	t0, zero, .radixSort.bb1
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
.radixSort.bb18:
	li	a2, 256
	bge	t3, a2, .radixSort.bb20
.radixSort.bb34:
	addiw	a2, t3, 3
	li	a3, 256
	blt	a2, a3, .radixSort.bb35
.radixSort.bb19:
	slliw	a2, t3, 2
	li	a3, 0
	add	a2, s5, a2
	addiw	t3, t3, 1
	sw	a3, 0(a2)
	j	.radixSort.bb18
.radixSort.bb35:
	slliw	a2, t3, 2
	li	a3, 0
	add	s7, s5, a2
	addiw	t3, t3, 4
	li	a2, 0
	sw	a3, 4(s7)
	sw	a2, 0(s7)
	sw	a3, 8(s7)
	sw	a2, 12(s7)
	j	.radixSort.bb18
.radixSort.bb20:
	addiw	s7, t2, 1
	slliw	s8, t2, 3
	subw	t2, t1, s8
	li	a2, 1
	slliw	t3, s7, 3
	sllw	t2, a2, t2
	slt	t3, t1, t3
	addiw	t2, t2, -1
	li	a2, 0
	xori	t2, t2, 255
	subw	t3, a2, t3
	and	t2, t2, t3
	xori	s9, t2, 255
.radixSort.bb21:
	mv	s10, s3	# phi-copy
.radixSort.bb22:
	bge	s10, s4, .radixSort.bb24
.radixSort.bb36:
	addiw	t2, s10, 15
	blt	t2, s4, .radixSort.bb37
.radixSort.bb23:
	slliw	t2, s10, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	sraw	t2, t2, s8
	and	t2, t2, s9
	slliw	t2, t2, 2
	add	s11, s5, t2
	lw	t2, 0(s11)
	addiw	t2, t2, 1
	sw	t2, 0(s11)
	addiw	s10, s10, 1
	j	.radixSort.bb22
.radixSort.bb37:
	slliw	t2, s10, 2
	add	s11, s2, t2
	lw	t2, 0(s11)
	sraw	t2, t2, s8
	and	t2, t2, s9
	slliw	t2, t2, 2
	add	t2, s5, t2
	lw	t3, 0(t2)
	addiw	t3, t3, 1
	sw	t3, 0(t2)
	lw	t3, 4(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 8(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 12(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 16(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 20(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 24(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 28(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 32(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 36(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 40(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 44(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 48(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 52(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t3, 56(s11)
	sraw	t3, t3, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s5, t3
	lw	a2, 0(t3)
	addiw	a2, a2, 1
	sw	a2, 0(t3)
	lw	t2, 60(s11)
	sraw	t2, t2, s8
	and	t2, t2, s9
	slliw	t2, t2, 2
	add	s11, s5, t2
	lw	t2, 0(s11)
	addiw	t2, t2, 1
	sw	t2, 0(s11)
	addiw	s10, s10, 16
	j	.radixSort.bb22
.radixSort.bb24:
	sw	s3, 0(s6)
	li	t2, 1
.radixSort.bb25:
	li	t3, 256
	bge	t2, t3, .radixSort.bb27
.radixSort.bb38:
	addiw	t3, t2, 3
	li	a2, 256
	blt	t3, a2, .radixSort.bb39
.radixSort.bb26:
	addiw	t3, t2, -1
	slliw	s10, t3, 2
	addiw	t2, t2, 1
	add	s11, s6, s10
	add	a2, s5, s10
	lw	t3, 0(s11)
	lw	a2, 0(a2)
	addw	t3, t3, a2
	sw	t3, 4(s11)
	j	.radixSort.bb25
.radixSort.bb39:
	addiw	t3, t2, -1
	slliw	s10, t3, 2
	addiw	t2, t2, 4
	add	s11, s6, s10
	lw	t3, 0(s11)
	add	s10, s5, s10
	lw	a2, 0(s10)
	addw	t3, t3, a2
	sw	t3, 4(s11)
	lw	a4, 4(s10)
	addw	t3, t3, a4
	sw	t3, 8(s11)
	lw	a4, 8(s10)
	addw	t3, t3, a4
	sw	t3, 12(s11)
	lw	a3, 12(s10)
	addw	t3, t3, a3
	sw	t3, 16(s11)
	j	.radixSort.bb25
.radixSort.bb27:
	mv	s10, s3	# phi-copy
.radixSort.bb28:
	bge	s10, s4, .radixSort.bb30
.radixSort.bb40:
	addiw	t2, s10, 3
	blt	t2, s4, .radixSort.bb41
.radixSort.bb29:
	slliw	t2, s10, 2
	add	t2, s2, t2
	lw	s11, 0(t2)
	sraw	t2, s11, s8
	and	t2, t2, s9
	slliw	t2, t2, 2
	add	t2, s6, t2
	lw	t3, 0(t2)
	slliw	a2, t3, 2
	add	a2, s1, a2
	addiw	t3, t3, 1
	sw	s11, 0(a2)
	sw	t3, 0(t2)
	addiw	s10, s10, 1
	j	.radixSort.bb28
.radixSort.bb41:
	slliw	t2, s10, 2
	add	s11, s2, t2
	lw	t2, 0(s11)
	sraw	t3, t2, s8
	and	t3, t3, s9
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	a2, 0(t3)
	slliw	a3, a2, 2
	add	a3, s1, a3
	sw	t2, 0(a3)
	addiw	t2, a2, 1
	sw	t2, 0(t3)
	lw	t3, 4(s11)
	sraw	a2, t3, s8
	and	a2, a2, s9
	slliw	a2, a2, 2
	add	a2, s6, a2
	lw	a3, 0(a2)
	slliw	a4, a3, 2
	add	a4, s1, a4
	sw	t3, 0(a4)
	addiw	t3, a3, 1
	sw	t3, 0(a2)
	lw	t3, 8(s11)
	sraw	a2, t3, s8
	and	a2, a2, s9
	slliw	a2, a2, 2
	add	a2, s6, a2
	lw	a3, 0(a2)
	slliw	a4, a3, 2
	add	a4, s1, a4
	sw	t3, 0(a4)
	addiw	t3, a3, 1
	sw	t3, 0(a2)
	lw	s11, 12(s11)
	sraw	t2, s11, s8
	and	t2, t2, s9
	slliw	t2, t2, 2
	add	t2, s6, t2
	lw	t3, 0(t2)
	slliw	a2, t3, 2
	add	a2, s1, a2
	addiw	t3, t3, 1
	sw	s11, 0(a2)
	sw	t3, 0(t2)
	addiw	s10, s10, 4
	j	.radixSort.bb28
.radixSort.bb30:
	mv	s8, s3	# phi-copy
.radixSort.bb31:
	bge	s8, s4, .radixSort.bb33
.radixSort.bb42:
	addiw	t2, s8, 3
	blt	t2, s4, .radixSort.bb43
.radixSort.bb32:
	slliw	s9, s8, 2
	add	t2, s1, s9
	add	t3, s2, s9
	lw	t2, 0(t2)
	sw	t2, 0(t3)
	addiw	s8, s8, 1
	j	.radixSort.bb31
.radixSort.bb43:
	slliw	s9, s8, 2
	add	s10, s1, s9
	lw	t2, 0(s10)
	add	s9, s2, s9
	sw	t2, 0(s9)
	lw	t3, 4(s10)
	sw	t3, 4(s9)
	lw	t3, 8(s10)
	sw	t3, 8(s9)
	lw	t2, 12(s10)
	sw	t2, 12(s9)
	addiw	s8, s8, 4
	j	.radixSort.bb31
.radixSort.bb33:
	mv	t2, s7	# phi-copy
	bge	t2, t0, .radixSort.bb1	# rotated-loop-branch
.radixSort.bb16:
	li	t3, 0
	j	.radixSort.bb18
.radixSort.bb5:
	lw	s8, 0(t0)
	blt	t1, s8, .radixSort.bb8
	j	.radixSort.bb44
.radixSort.bb7:
.radixSort.bb8:
	addiw	s7, s7, 1
	addi	t0, t0, 4
	mv	t1, s8	# phi-copy
	blt	s7, s4, .radixSort.bb5	# rotated-loop-branch
.radixSort.bb6:
	li	t0, 1
	li	t2, 256
.radixSort.bb9:
	blt	t1, t2, .radixSort.bb11
.radixSort.bb10:
	sraiw	t3, t1, 31
	addiw	t0, t0, 1
	andi	t3, t3, 255
	addw	t1, t1, t3
	sraiw	t1, t1, 8
	bge	t1, t2, .radixSort.bb10	# rotated-loop-branch
.radixSort.bb11:
	addiw	t1, s1, 1
	li	t2, 32
	slliw	t1, t1, 2
	blt	t2, t1, .radixSort.bb12
.radixSort.bb45:
.radixSort.bb13:
	addiw	t3, t1, 7
	slliw	t2, t0, 3
	sraiw	a2, t3, 31
	slt	t2, t1, t2
	andi	a2, a2, 7
	addw	t3, t3, a2
	sraiw	t3, t3, 3
	li	a2, 0
	subw	t2, a2, t2
	xor	t3, t0, t3
	and	t2, t3, t2
	xor	t0, t0, t2
.radixSort.bb14:
	la	s1, __syc_radix_scratch	# loop.addr.hoisted
	li	t2, 0
.radixSort.bb15:
	blt	t2, t0, .radixSort.bb16
.radixSort.bb17:
	j	.radixSort.bb1
.radixSort.bb44:
	mv	s8, t1	# phi-copy
	j	.radixSort.bb8
.radixSort.bb12:
	li	t1, 32
	j	.radixSort.bb13
.radixSort.bb3:
	slliw	t0, s3, 2
	li	t1, 0
	add	t0, s2, t0
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
	addw	s1, s1, t2
	addiw	t1, t1, 1
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
	li	t5, 8
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
