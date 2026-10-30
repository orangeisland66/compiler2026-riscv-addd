
  .section .rodata
  .globl DIGITS
  .align 2
  .type  DIGITS, @object
  .size  DIGITS, 4
DIGITS:
  .word 8

  .bss
  .globl __syc_radix_scratch
  .align 2
  .type  __syc_radix_scratch, @object
  .size  __syc_radix_scratch, 256
__syc_radix_scratch:
  .zero 256

  .bss
  .globl first_values_extra
  .align 2
  .type  first_values_extra, @object
  .size  first_values_extra, 256
first_values_extra:
  .zero 256

  .bss
  .globl second_values_extra
  .align 2
  .type  second_values_extra, @object
  .size  second_values_extra, 256
second_values_extra:
  .zero 256

  .text

  .globl sort_by_digits_extra
  .type  sort_by_digits_extra, @function
sort_by_digits_extra:
.sort_by_digits_extra.bb0:
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
	bne	t0, zero, .sort_by_digits_extra.bb1
	j	.sort_by_digits_extra.bb3
.sort_by_digits_extra.bb2:
.sort_by_digits_extra.bb1:
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
.sort_by_digits_extra.bb18:
	li	a2, 256
	bge	t3, a2, .sort_by_digits_extra.bb20
.sort_by_digits_extra.bb34:
	addiw	a2, t3, 3
	li	a3, 256
	blt	a2, a3, .sort_by_digits_extra.bb35
.sort_by_digits_extra.bb19:
	slliw	a2, t3, 2
	li	a3, 0
	add	a2, s5, a2
	addiw	t3, t3, 1
	sw	a3, 0(a2)
	j	.sort_by_digits_extra.bb18
.sort_by_digits_extra.bb35:
	slliw	a2, t3, 2
	li	a3, 0
	add	s7, s5, a2
	addiw	t3, t3, 4
	li	a2, 0
	sw	a3, 4(s7)
	sw	a2, 0(s7)
	sw	a3, 8(s7)
	sw	a2, 12(s7)
	j	.sort_by_digits_extra.bb18
.sort_by_digits_extra.bb20:
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
.sort_by_digits_extra.bb21:
	mv	s10, s3	# phi-copy
.sort_by_digits_extra.bb22:
	bge	s10, s4, .sort_by_digits_extra.bb24
.sort_by_digits_extra.bb36:
	addiw	t2, s10, 15
	blt	t2, s4, .sort_by_digits_extra.bb37
.sort_by_digits_extra.bb23:
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
	j	.sort_by_digits_extra.bb22
.sort_by_digits_extra.bb37:
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
	j	.sort_by_digits_extra.bb22
.sort_by_digits_extra.bb24:
	sw	s3, 0(s6)
	li	t2, 1
.sort_by_digits_extra.bb25:
	li	t3, 256
	bge	t2, t3, .sort_by_digits_extra.bb27
.sort_by_digits_extra.bb38:
	addiw	t3, t2, 3
	li	a2, 256
	blt	t3, a2, .sort_by_digits_extra.bb39
.sort_by_digits_extra.bb26:
	addiw	t3, t2, -1
	slliw	s10, t3, 2
	addiw	t2, t2, 1
	add	s11, s6, s10
	add	a2, s5, s10
	lw	t3, 0(s11)
	lw	a2, 0(a2)
	addw	t3, t3, a2
	sw	t3, 4(s11)
	j	.sort_by_digits_extra.bb25
.sort_by_digits_extra.bb39:
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
	j	.sort_by_digits_extra.bb25
.sort_by_digits_extra.bb27:
	mv	s10, s3	# phi-copy
.sort_by_digits_extra.bb28:
	bge	s10, s4, .sort_by_digits_extra.bb30
.sort_by_digits_extra.bb40:
	addiw	t2, s10, 3
	blt	t2, s4, .sort_by_digits_extra.bb41
.sort_by_digits_extra.bb29:
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
	j	.sort_by_digits_extra.bb28
.sort_by_digits_extra.bb41:
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
	j	.sort_by_digits_extra.bb28
.sort_by_digits_extra.bb30:
	mv	s8, s3	# phi-copy
.sort_by_digits_extra.bb31:
	bge	s8, s4, .sort_by_digits_extra.bb33
.sort_by_digits_extra.bb42:
	addiw	t2, s8, 3
	blt	t2, s4, .sort_by_digits_extra.bb43
.sort_by_digits_extra.bb32:
	slliw	s9, s8, 2
	add	t2, s1, s9
	add	t3, s2, s9
	lw	t2, 0(t2)
	sw	t2, 0(t3)
	addiw	s8, s8, 1
	j	.sort_by_digits_extra.bb31
.sort_by_digits_extra.bb43:
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
	j	.sort_by_digits_extra.bb31
.sort_by_digits_extra.bb33:
	mv	t2, s7	# phi-copy
	bge	t2, t0, .sort_by_digits_extra.bb1	# rotated-loop-branch
.sort_by_digits_extra.bb16:
	li	t3, 0
	j	.sort_by_digits_extra.bb18
.sort_by_digits_extra.bb5:
	lw	s8, 0(t0)
	blt	t1, s8, .sort_by_digits_extra.bb8
	j	.sort_by_digits_extra.bb44
.sort_by_digits_extra.bb7:
.sort_by_digits_extra.bb8:
	addiw	s7, s7, 1
	addi	t0, t0, 4
	mv	t1, s8	# phi-copy
	blt	s7, s4, .sort_by_digits_extra.bb5	# rotated-loop-branch
.sort_by_digits_extra.bb6:
	li	t0, 1
	li	t2, 256
.sort_by_digits_extra.bb9:
	blt	t1, t2, .sort_by_digits_extra.bb11
.sort_by_digits_extra.bb10:
	sraiw	t3, t1, 31
	addiw	t0, t0, 1
	andi	t3, t3, 255
	addw	t1, t1, t3
	sraiw	t1, t1, 8
	bge	t1, t2, .sort_by_digits_extra.bb10	# rotated-loop-branch
.sort_by_digits_extra.bb11:
	addiw	t1, s1, 1
	slliw	t2, t1, 1
	addw	t1, t2, t1
	li	t2, 32
	blt	t2, t1, .sort_by_digits_extra.bb12
.sort_by_digits_extra.bb45:
.sort_by_digits_extra.bb13:
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
.sort_by_digits_extra.bb14:
	la	s1, __syc_radix_scratch	# loop.addr.hoisted
	li	t2, 0
.sort_by_digits_extra.bb15:
	blt	t2, t0, .sort_by_digits_extra.bb16
.sort_by_digits_extra.bb17:
	j	.sort_by_digits_extra.bb1
.sort_by_digits_extra.bb44:
	mv	s8, t1	# phi-copy
	j	.sort_by_digits_extra.bb8
.sort_by_digits_extra.bb12:
	li	t1, 32
	j	.sort_by_digits_extra.bb13
.sort_by_digits_extra.bb3:
	slliw	t0, s3, 2
	li	t1, 0
	add	t0, s2, t0
	mv	s7, s3	# phi-copy
.sort_by_digits_extra.bb4:
	bge	s7, s4, .sort_by_digits_extra.bb6
	j	.sort_by_digits_extra.bb5
	.size sort_by_digits_extra, .-sort_by_digits_extra

  .globl run_radix_extra
  .type  run_radix_extra, @function
run_radix_extra:
.run_radix_extra.bb0:
	addi	sp, sp, -112
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	sd	s2, 56(sp)
	sd	s3, 64(sp)
	sd	s4, 72(sp)
	sd	s5, 80(sp)
	sd	s6, 88(sp)
	sd	s7, 96(sp)
	sd	s8, 104(sp)
	mv	s1, a0
	addiw	t0, s1, 1
	li	t1, 1
	blt	t1, s1, .run_radix_extra.bb12
.run_radix_extra.bb13:
	addiw	a4, s1, 2
	mv	a3, s1	# call-arg-copy
	li	a0, 3
	la	a1, second_values_extra
	li	a2, 0
	call	sort_by_digits_extra
	li	t0, 0
	li	t1, 0
	la	s2, first_values_extra	# loop.addr.hoisted
	la	s3, second_values_extra	# loop.addr.hoisted
.run_radix_extra.bb4:
	addiw	s4, t0, 8
	bge	t0, s1, .run_radix_extra.bb3
.run_radix_extra.bb5:
	bge	s4, s1, .run_radix_extra.bb6
.run_radix_extra.bb7:
	j	.run_radix_extra.bb1
.run_radix_extra.bb8:
.run_radix_extra.bb1:
	bge	t0, s4, .run_radix_extra.bb4
.run_radix_extra.bb10:
	addiw	s5, t0, 3
	blt	s5, s4, .run_radix_extra.bb11
.run_radix_extra.bb2:
	slliw	s6, t0, 2
	addiw	s7, t0, 1
	add	t2, s2, s6
	lw	t2, 0(t2)
	mulw	t0, t2, s7
	addw	t0, t1, t0
	add	t1, s3, s6
	lw	t1, 0(t1)
	mulw	t1, t1, s5
	subw	t1, t0, t1
	mv	t0, s7	# phi-copy
	j	.run_radix_extra.bb1
.run_radix_extra.bb11:
	slliw	s6, t0, 2
	addiw	t3, t0, 1
	add	s7, s2, s6
	addiw	a2, t0, 2
	lw	t2, 0(s7)
	add	s6, s3, s6
	addiw	s8, t0, 4
	addiw	a3, t0, 5
	addiw	t0, t0, 6
	mulw	t2, t2, t3
	lw	t3, 4(s7)
	addw	t1, t1, t2
	lw	t2, 0(s6)
	mulw	t3, t3, a2
	mulw	t2, t2, s5
	lw	a2, 4(s6)
	subw	t1, t1, t2
	addw	t1, t1, t3
	mulw	a2, a2, s8
	lw	t2, 12(s7)
	subw	t1, t1, a2
	lw	a2, 8(s7)
	mulw	t2, t2, s8
	mulw	a2, a2, s5
	addw	t1, t1, a2
	lw	a2, 8(s6)
	mulw	a2, a2, a3
	subw	t1, t1, a2
	addw	t1, t1, t2
	lw	t2, 12(s6)
	mulw	t0, t2, t0
	subw	t1, t1, t0
	mv	t0, s8	# phi-copy
	j	.run_radix_extra.bb1
.run_radix_extra.bb9:
	j	.run_radix_extra.bb4
.run_radix_extra.bb6:
	mv	s4, s1	# phi-copy
	j	.run_radix_extra.bb1
.run_radix_extra.bb3:
	mv	a0, t1
	ld	s8, 104(sp)
	ld	s7, 96(sp)
	ld	s6, 88(sp)
	ld	s5, 80(sp)
	ld	s4, 72(sp)
	ld	s3, 64(sp)
	ld	s2, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 112
	ret
.run_radix_extra.bb12:
	mv	a3, s1	# call-arg-copy
	mv	a4, t0	# call-arg-copy
	li	a0, 3
	la	a1, first_values_extra
	li	a2, 0
	call	sort_by_digits_extra
	j	.run_radix_extra.bb13
	.size run_radix_extra, .-run_radix_extra

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
	la	s1, first_values_extra	# loop.addr.hoisted
	la	s2, second_values_extra	# loop.addr.hoisted
.main.bb4:
	addiw	s3, t0, 8
	bge	t0, t1, .main.bb3
.main.bb5:
	li	t2, 64
	bge	s3, t2, .main.bb6
.main.bb7:
	mv	t2, s3	# phi-copy
	j	.main.bb1
.main.bb8:
.main.bb1:
	bge	t0, t2, .main.bb4
.main.bb10:
	li	t3, 197
	addiw	s3, t0, 3
	mulw	s4, t0, t3
	li	t3, 313
	mulw	s5, t0, t3
	blt	s3, t2, .main.bb11
.main.bb2:
	addiw	s3, s4, 53
	li	a2, 4095
	sraiw	t3, s3, 31
	and	s4, t3, a2
	addw	t3, s3, s4
	and	t3, t3, a2
	slliw	s3, t0, 2
	subw	t3, t3, s4
	add	a2, s1, s3
	sw	t3, 0(a2)
	addiw	s4, s5, 91
	sraiw	t3, s4, 31
	li	a2, 4095
	and	s5, t3, a2
	addiw	t0, t0, 1
	addw	t3, s4, s5
	and	t3, t3, a2
	subw	t3, t3, s5
	add	a2, s2, s3
	sw	t3, 0(a2)
	j	.main.bb1
.main.bb11:
	addiw	s4, s4, 53
	li	a2, 4095
	sraiw	t3, s4, 31
	addiw	s5, s5, 91
	and	s6, t3, a2
	li	a3, 4095
	addw	t3, s4, s6
	and	t3, t3, a2
	slliw	s4, t0, 2
	subw	t3, t3, s6
	add	s6, s1, s4
	li	a4, 4095
	sw	t3, 0(s6)
	add	s4, s2, s4
	sraiw	t3, s5, 31
	and	s7, t3, a2
	addw	t3, s5, s7
	and	t3, t3, a2
	addiw	s5, t0, 1
	subw	t3, t3, s7
	sw	t3, 0(s4)
	li	t3, 197
	mulw	t3, s5, t3
	addiw	s7, t3, 53
	sraiw	t3, s7, 31
	and	s8, t3, a2
	addw	t3, s7, s8
	and	t3, t3, a2
	subw	t3, t3, s8
	sw	t3, 4(s6)
	li	t3, 313
	mulw	t3, s5, t3
	addiw	s5, t3, 91
	sraiw	t3, s5, 31
	and	s7, t3, a3
	addw	t3, s5, s7
	and	t3, t3, a3
	addiw	s5, t0, 2
	subw	t3, t3, s7
	sw	t3, 4(s4)
	li	t3, 197
	addiw	t0, t0, 4
	mulw	t3, s5, t3
	addiw	s7, t3, 53
	sraiw	t3, s7, 31
	and	s8, t3, a4
	addw	t3, s7, s8
	and	t3, t3, a4
	subw	t3, t3, s8
	sw	t3, 8(s6)
	li	t3, 313
	mulw	t3, s5, t3
	addiw	s5, t3, 91
	sraiw	t3, s5, 31
	and	s7, t3, a4
	addw	t3, s5, s7
	and	t3, t3, a4
	subw	t3, t3, s7
	sw	t3, 8(s4)
	li	t3, 197
	mulw	t3, s3, t3
	addiw	s5, t3, 53
	sraiw	t3, s5, 31
	and	s7, t3, a4
	addw	t3, s5, s7
	and	t3, t3, a4
	subw	t3, t3, s7
	sw	t3, 12(s6)
	li	t3, 313
	mulw	t3, s3, t3
	addiw	s3, t3, 91
	sraiw	t3, s3, 31
	and	s5, t3, a2
	addw	t3, s3, s5
	and	t3, t3, a2
	subw	t3, t3, s5
	sw	t3, 12(s4)
	j	.main.bb1
.main.bb9:
	j	.main.bb4
.main.bb6:
	li	t2, 64
	j	.main.bb1
.main.bb3:
	li	t5, 64
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	run_radix_extra
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	run_radix_extra
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	run_radix_extra
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
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
