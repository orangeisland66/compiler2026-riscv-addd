
  .section .rodata
  .globl SIDE
  .align 2
  .type  SIDE, @object
  .size  SIDE, 4
SIDE:
  .word 6

  .text

  .globl advance_rows_extra
  .type  advance_rows_extra, @function
advance_rows_extra:
.advance_rows_extra.bb0:
	addi	sp, sp, -64
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	sd	s7, 48(sp)
	sd	s8, 56(sp)
	mv	s1, a0
	mv	t0, a1
	mv	s2, a2
	mv	s3, a3
	li	t1, 0
.advance_rows_extra.bb1:
	bge	t1, s1, .advance_rows_extra.bb3
.advance_rows_extra.bb2:
	slliw	t2, t1, 4
	slliw	t3, t1, 3
	addw	s4, t2, t3
	li	t2, 0
.advance_rows_extra.bb4:
	bge	t2, s1, .advance_rows_extra.bb6
.advance_rows_extra.bb24:
	addiw	t3, t2, 3
	blt	t3, s1, .advance_rows_extra.bb25
.advance_rows_extra.bb5:
	slliw	t3, t2, 2
	li	a2, 0
	addw	t3, s4, t3
	addiw	t2, t2, 1
	add	t3, s3, t3
	sw	a2, 0(t3)
	j	.advance_rows_extra.bb4
.advance_rows_extra.bb15:
	bge	a2, s6, .advance_rows_extra.bb18
.advance_rows_extra.bb26:
	addiw	a3, a2, 15
	blt	a3, s6, .advance_rows_extra.bb27
.advance_rows_extra.bb16:
	lw	a4, 0(t2)
	slliw	s7, a2, 2
	addw	a3, s5, s7
	addiw	a2, a2, 1
	add	s8, s3, a3
	lw	a3, 0(s8)
	mulw	a3, a3, a4
	addw	a4, s4, s7
	add	a4, s2, a4
	lw	a4, 0(a4)
	addw	a3, a3, a4
	sw	a3, 0(s8)
	j	.advance_rows_extra.bb15
.advance_rows_extra.bb27:
	lw	a4, 0(t2)
	slliw	s7, a2, 2
	addw	a3, s5, s7
	addiw	a2, a2, 16
	add	s8, s3, a3
	lw	a3, 0(s8)
	mulw	a3, a3, a4
	addw	a4, s4, s7
	add	s7, s2, a4
	lw	a4, 0(s7)
	addw	a3, a3, a4
	lw	a4, 4(s8)
	sw	a3, 0(s8)
	lw	a5, 0(t2)
	lw	a6, 4(s7)
	mulw	a4, a4, a5
	addw	a4, a4, a6
	sw	a4, 4(s8)
	lw	a4, 8(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 8(s7)
	lw	a3, 60(s8)
	addw	a4, a4, a6
	sw	a4, 8(s8)
	lw	a4, 12(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 12(s7)
	addw	a4, a4, a6
	sw	a4, 12(s8)
	lw	a4, 16(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 16(s7)
	addw	a4, a4, a6
	sw	a4, 16(s8)
	lw	a4, 20(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 20(s7)
	addw	a4, a4, a6
	sw	a4, 20(s8)
	lw	a4, 24(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 24(s7)
	addw	a4, a4, a6
	sw	a4, 24(s8)
	lw	a4, 28(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 28(s7)
	addw	a4, a4, a6
	sw	a4, 28(s8)
	lw	a4, 32(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 32(s7)
	addw	a4, a4, a6
	sw	a4, 32(s8)
	lw	a4, 36(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 36(s7)
	addw	a4, a4, a6
	sw	a4, 36(s8)
	lw	a4, 40(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 40(s7)
	addw	a4, a4, a6
	sw	a4, 40(s8)
	lw	a4, 44(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 44(s7)
	addw	a4, a4, a6
	sw	a4, 44(s8)
	lw	a4, 48(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 48(s7)
	addw	a4, a4, a6
	sw	a4, 48(s8)
	lw	a4, 52(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 52(s7)
	addw	a4, a4, a6
	sw	a4, 52(s8)
	lw	a4, 56(s8)
	lw	a6, 0(t2)
	mulw	a4, a4, a6
	lw	a6, 56(s7)
	addw	a4, a4, a6
	sw	a4, 56(s8)
	lw	a4, 0(t2)
	mulw	a3, a3, a4
	lw	a4, 60(s7)
	addw	a3, a3, a4
	sw	a3, 60(s8)
	j	.advance_rows_extra.bb15
.advance_rows_extra.bb23:
.advance_rows_extra.bb18:
	addiw	s6, a2, 16
	bge	a2, s1, .advance_rows_extra.bb17
.advance_rows_extra.bb19:
	bge	s6, s1, .advance_rows_extra.bb20
.advance_rows_extra.bb21:
	j	.advance_rows_extra.bb15
.advance_rows_extra.bb22:
	j	.advance_rows_extra.bb15
.advance_rows_extra.bb20:
	mv	s6, s1	# phi-copy
	j	.advance_rows_extra.bb15
.advance_rows_extra.bb17:
	addiw	t3, t3, 1
	addi	t2, t2, 24
.advance_rows_extra.bb10:
	bge	t3, s1, .advance_rows_extra.bb12
.advance_rows_extra.bb11:
	slliw	a2, t3, 4
	slliw	a3, t3, 3
	addw	s5, a2, a3
	lw	a2, 0(t2)
	li	a3, 1
	bne	a2, a3, .advance_rows_extra.bb14
.advance_rows_extra.bb13:
	addiw	t3, t3, 1
	addi	t2, t2, 24
	j	.advance_rows_extra.bb10
.advance_rows_extra.bb14:
	li	a2, 0
	j	.advance_rows_extra.bb18
.advance_rows_extra.bb25:
	slliw	t3, t2, 2
	li	a2, 0
	addw	t3, s4, t3
	addiw	t2, t2, 4
	add	s5, s3, t3
	li	t3, 0
	sw	a2, 4(s5)
	sw	t3, 0(s5)
	sw	a2, 8(s5)
	sw	t3, 12(s5)
	j	.advance_rows_extra.bb4
.advance_rows_extra.bb6:
	addiw	t1, t1, 1
	blt	t1, s1, .advance_rows_extra.bb2	# rotated-loop-branch
.advance_rows_extra.bb3:
	li	t1, 0
.advance_rows_extra.bb7:
	bge	t1, s1, .advance_rows_extra.bb9
.advance_rows_extra.bb8:
	slliw	t2, t1, 4
	slliw	t3, t1, 3
	addw	s4, t2, t3
	slliw	t2, t1, 2
	li	t3, 0
	add	t2, t0, t2
	j	.advance_rows_extra.bb10
.advance_rows_extra.bb12:
	addiw	t1, t1, 1
	blt	t1, s1, .advance_rows_extra.bb8	# rotated-loop-branch
.advance_rows_extra.bb9:
	ld	s8, 56(sp)
	ld	s7, 48(sp)
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 64
	ret
	.size advance_rows_extra, .-advance_rows_extra

  .globl run_extra_summary
  .type  run_extra_summary, @function
run_extra_summary:
.run_extra_summary.bb0:
	addi	sp, sp, -576
	sd	ra, 472(sp)
	sd	s1, 480(sp)
	sd	s2, 488(sp)
	sd	s3, 496(sp)
	sd	s4, 504(sp)
	sd	s5, 512(sp)
	sd	s6, 520(sp)
	sd	s7, 528(sp)
	sd	s8, 536(sp)
	sd	s9, 544(sp)
	sd	s10, 552(sp)
	sd	s11, 560(sp)
	mv	s1, a0
	mv	s2, a1
	li	t0, 0
.run_extra_summary.bb1:
	bge	t0, s1, .run_extra_summary.bb3
.run_extra_summary.bb2:
	slliw	t1, t0, 1
	slliw	t2, t0, 3
	addw	s3, t1, t0
	slliw	t1, t0, 4
	addw	s4, t1, t2
	li	t1, 11
	mulw	s5, t0, t1
	li	t1, 0
.run_extra_summary.bb4:
	bge	t1, s1, .run_extra_summary.bb6
.run_extra_summary.bb16:
	addiw	s6, t1, 3
	blt	s6, s1, .run_extra_summary.bb17
.run_extra_summary.bb5:
	slliw	s6, t1, 2
	addi	t3, sp, 40
	addw	t2, s6, t1
	addw	s7, s3, t2
	addw	s6, s4, s6
	sraiw	t2, s7, 31
	add	t3, t3, s6
	andi	s8, t2, 3
	addw	t2, s7, s8
	andi	t2, t2, 3
	subw	t2, t2, s8
	addiw	t2, t2, 1
	sw	t2, 0(t3)
	slliw	t2, t1, 3
	addi	t3, sp, 184
	subw	t2, t2, t1
	add	t3, t3, s6
	subw	t2, s5, t2
	addiw	t1, t1, 1
	addiw	t2, t2, 3
	sw	t2, 0(t3)
	addi	t2, sp, 328
	li	t3, 0
	add	t2, t2, s6
	sw	t3, 0(t2)
	j	.run_extra_summary.bb4
.run_extra_summary.bb17:
	slliw	s7, t1, 2
	addi	t3, sp, 40
	addw	t2, s7, t1
	addiw	s10, t1, 1
	addw	s8, s3, t2
	addw	s7, s4, s7
	sraiw	t2, s8, 31
	li	a3, 0
	andi	s9, t2, 3
	addw	t2, s8, s9
	andi	t2, t2, 3
	add	s8, t3, s7
	subw	t2, t2, s9
	addi	t3, sp, 184
	addiw	t2, t2, 1
	add	s9, t3, s7
	sw	t2, 0(s8)
	slliw	t2, t1, 3
	subw	t2, t2, t1
	subw	t2, s5, t2
	addiw	t2, t2, 3
	sw	t2, 0(s9)
	addi	t2, sp, 328
	add	s7, t2, s7
	li	t2, 0
	sw	t2, 0(s7)
	slliw	t2, s10, 2
	addw	t2, t2, s10
	addw	s11, s3, t2
	sraiw	t2, s11, 31
	andi	t2, t2, 3
	addw	t3, s11, t2
	andi	t3, t3, 3
	subw	t2, t3, t2
	addiw	t2, t2, 1
	sw	t2, 4(s8)
	slliw	t2, s10, 3
	subw	t2, t2, s10
	slliw	t3, s6, 3
	subw	t2, s5, t2
	addiw	s10, t1, 2
	addiw	t2, t2, 3
	subw	t3, t3, s6
	sw	t2, 4(s9)
	subw	t3, s5, t3
	sw	a3, 4(s7)
	slliw	a3, s10, 2
	addw	a3, a3, s10
	addiw	t3, t3, 3
	addw	s11, s3, a3
	sraiw	a3, s11, 31
	li	t2, 0
	andi	a3, a3, 3
	addiw	t1, t1, 4
	addw	a4, s11, a3
	andi	a4, a4, 3
	subw	a3, a4, a3
	addiw	a3, a3, 1
	sw	a3, 8(s8)
	slliw	a3, s10, 3
	subw	a3, a3, s10
	subw	a3, s5, a3
	addiw	a3, a3, 3
	sw	a3, 8(s9)
	li	a3, 0
	sw	a3, 8(s7)
	slliw	a3, s6, 2
	addw	a3, a3, s6
	addw	s10, s3, a3
	sraiw	a3, s10, 31
	andi	s11, a3, 3
	addw	a3, s10, s11
	andi	a3, a3, 3
	subw	a3, a3, s11
	addiw	a3, a3, 1
	sw	a3, 12(s8)
	sw	t3, 12(s9)
	sw	t2, 12(s7)
	j	.run_extra_summary.bb4
.run_extra_summary.bb6:
	addiw	t0, t0, 1
	blt	t0, s1, .run_extra_summary.bb2	# rotated-loop-branch
.run_extra_summary.bb3:
	li	s3, 0
.run_extra_summary.bb7:
	bge	s3, s2, .run_extra_summary.bb9
.run_extra_summary.bb8:
	addiw	s4, s3, 1
	mv	a0, s1	# call-arg-copy
	mv	a4, s4	# call-arg-copy
	addi	a1, sp, 40
	addi	a2, sp, 184
	addi	a3, sp, 328
	call	advance_rows_extra
	addiw	a4, s3, 2
	mv	a0, s1	# call-arg-copy
	addi	a1, sp, 40
	addi	a2, sp, 328
	addi	a3, sp, 184
	call	advance_rows_extra
	addiw	a4, s3, 3
	mv	a0, s1	# call-arg-copy
	addi	a1, sp, 40
	addi	a2, sp, 184
	addi	a3, sp, 328
	call	advance_rows_extra
	addiw	a4, s3, 4
	mv	a0, s1	# call-arg-copy
	addi	a1, sp, 40
	addi	a2, sp, 328
	addi	a3, sp, 184
	call	advance_rows_extra
	mv	s3, s4	# phi-copy
	blt	s3, s2, .run_extra_summary.bb8	# rotated-loop-branch
.run_extra_summary.bb9:
	li	t0, 0
	li	t1, 0
.run_extra_summary.bb10:
	bge	t1, s1, .run_extra_summary.bb12
.run_extra_summary.bb11:
	slliw	t2, t1, 4
	slliw	t3, t1, 3
	addw	s2, t2, t3
	li	t2, 0
.run_extra_summary.bb13:
	bge	t2, s1, .run_extra_summary.bb15
.run_extra_summary.bb18:
	addiw	t3, t2, 3
	blt	t3, s1, .run_extra_summary.bb19
.run_extra_summary.bb14:
	slliw	t3, t2, 2
	addi	a2, sp, 184
	addw	t3, s2, t3
	addiw	t2, t2, 1
	add	t3, a2, t3
	lw	t3, 0(t3)
	addw	t0, t0, t3
	j	.run_extra_summary.bb13
.run_extra_summary.bb19:
	slliw	t3, t2, 2
	addi	a2, sp, 184
	addw	t3, s2, t3
	addiw	t2, t2, 4
	add	s3, a2, t3
	lw	t3, 0(s3)
	lw	a2, 4(s3)
	addw	t0, t0, t3
	addw	t0, t0, a2
	lw	a2, 8(s3)
	lw	t3, 12(s3)
	addw	t0, t0, a2
	addw	t0, t0, t3
	j	.run_extra_summary.bb13
.run_extra_summary.bb15:
	addiw	t1, t1, 1
	blt	t1, s1, .run_extra_summary.bb11	# rotated-loop-branch
.run_extra_summary.bb12:
	mv	a0, t0
	ld	s11, 560(sp)
	ld	s10, 552(sp)
	ld	s9, 544(sp)
	ld	s8, 536(sp)
	ld	s7, 528(sp)
	ld	s6, 520(sp)
	ld	s5, 512(sp)
	ld	s4, 504(sp)
	ld	s3, 496(sp)
	ld	s2, 488(sp)
	ld	s1, 480(sp)
	ld	ra, 472(sp)
	addi	sp, sp, 576
	ret
	.size run_extra_summary, .-run_extra_summary

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	li	t5, 4
	sd	t5, 0(sp)
	li	t5, 2
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	run_extra_summary
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
