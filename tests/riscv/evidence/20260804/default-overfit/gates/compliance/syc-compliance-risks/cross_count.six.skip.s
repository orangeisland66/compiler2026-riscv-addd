
  .section .rodata
  .globl SIDE
  .align 2
  .type  SIDE, @object
  .size  SIDE, 4
SIDE:
  .word 6

  .text

  .globl advance_rows_six
  .type  advance_rows_six, @function
advance_rows_six:
.advance_rows_six.bb0:
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
.advance_rows_six.bb1:
	bge	t1, s1, .advance_rows_six.bb3
.advance_rows_six.bb2:
	slliw	t2, t1, 4
	slliw	t3, t1, 3
	addw	s4, t2, t3
	li	t2, 0
.advance_rows_six.bb4:
	bge	t2, s1, .advance_rows_six.bb6
.advance_rows_six.bb24:
	addiw	t3, t2, 3
	blt	t3, s1, .advance_rows_six.bb25
.advance_rows_six.bb5:
	slliw	t3, t2, 2
	li	a2, 0
	addw	t3, s4, t3
	addiw	t2, t2, 1
	add	t3, s3, t3
	sw	a2, 0(t3)
	j	.advance_rows_six.bb4
.advance_rows_six.bb15:
	bge	a2, s6, .advance_rows_six.bb18
.advance_rows_six.bb26:
	addiw	a3, a2, 15
	blt	a3, s6, .advance_rows_six.bb27
.advance_rows_six.bb16:
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
	j	.advance_rows_six.bb15
.advance_rows_six.bb27:
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
	j	.advance_rows_six.bb15
.advance_rows_six.bb23:
.advance_rows_six.bb18:
	addiw	s6, a2, 16
	bge	a2, s1, .advance_rows_six.bb17
.advance_rows_six.bb19:
	bge	s6, s1, .advance_rows_six.bb20
.advance_rows_six.bb21:
	j	.advance_rows_six.bb15
.advance_rows_six.bb22:
	j	.advance_rows_six.bb15
.advance_rows_six.bb20:
	mv	s6, s1	# phi-copy
	j	.advance_rows_six.bb15
.advance_rows_six.bb17:
	addiw	t3, t3, 1
	addi	t2, t2, 24
.advance_rows_six.bb10:
	bge	t3, s1, .advance_rows_six.bb12
.advance_rows_six.bb11:
	slliw	a2, t3, 4
	slliw	a3, t3, 3
	addw	s5, a2, a3
	lw	a2, 0(t2)
	li	a3, 1
	bne	a2, a3, .advance_rows_six.bb14
.advance_rows_six.bb13:
	addiw	t3, t3, 1
	addi	t2, t2, 24
	j	.advance_rows_six.bb10
.advance_rows_six.bb14:
	li	a2, 0
	j	.advance_rows_six.bb18
.advance_rows_six.bb25:
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
	j	.advance_rows_six.bb4
.advance_rows_six.bb6:
	addiw	t1, t1, 1
	blt	t1, s1, .advance_rows_six.bb2	# rotated-loop-branch
.advance_rows_six.bb3:
	li	t1, 0
.advance_rows_six.bb7:
	bge	t1, s1, .advance_rows_six.bb9
.advance_rows_six.bb8:
	slliw	t2, t1, 4
	slliw	t3, t1, 3
	addw	s4, t2, t3
	slliw	t2, t1, 2
	li	t3, 0
	add	t2, t0, t2
	j	.advance_rows_six.bb10
.advance_rows_six.bb12:
	addiw	t1, t1, 1
	blt	t1, s1, .advance_rows_six.bb8	# rotated-loop-branch
.advance_rows_six.bb9:
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
	.size advance_rows_six, .-advance_rows_six

  .globl run_six
  .type  run_six, @function
run_six:
.run_six.bb0:
	addi	sp, sp, -544
	sd	ra, 472(sp)
	sd	s1, 480(sp)
	sd	s2, 488(sp)
	sd	s3, 496(sp)
	sd	s4, 504(sp)
	sd	s5, 512(sp)
	sd	s6, 520(sp)
	sd	s7, 528(sp)
	li	t0, 0
	li	t1, 4
.run_six.bb1:
	bge	t0, t1, .run_six.bb3
.run_six.bb2:
	slliw	t2, t0, 1
	slliw	t3, t0, 3
	addw	s1, t2, t0
	li	a3, 0
	slliw	t2, t0, 4
	addiw	s6, s1, 5
	addw	s2, t2, t3
	li	t2, 11
	addi	t3, sp, 40
	mulw	s3, t0, t2
	sraiw	t2, s1, 31
	addiw	t0, t0, 1
	andi	s4, t2, 3
	addw	t2, s1, s4
	andi	t2, t2, 3
	subw	t2, t2, s4
	addiw	t2, t2, 1
	add	s4, t3, s2
	sw	t2, 0(s4)
	addi	t3, sp, 184
	addiw	t2, s3, 3
	add	s5, t3, s2
	sw	t2, 0(s5)
	addi	t2, sp, 328
	add	s2, t2, s2
	li	t2, 0
	addiw	t3, s3, -21
	sw	t2, 0(s2)
	sraiw	t2, s6, 31
	addiw	t3, t3, 3
	andi	s7, t2, 3
	addw	t2, s6, s7
	andi	t2, t2, 3
	addiw	s6, s1, 10
	subw	t2, t2, s7
	addiw	s1, s1, 15
	addiw	t2, t2, 1
	sw	t2, 4(s4)
	addiw	t2, s3, -7
	addiw	t2, t2, 3
	sw	t2, 4(s5)
	sw	a3, 4(s2)
	sraiw	a3, s6, 31
	andi	s7, a3, 3
	addw	a3, s6, s7
	li	t2, 0
	andi	a3, a3, 3
	subw	a3, a3, s7
	addiw	a3, a3, 1
	sw	a3, 8(s4)
	addiw	a3, s3, -14
	addiw	a3, a3, 3
	sw	a3, 8(s5)
	li	a3, 0
	sw	a3, 8(s2)
	sraiw	a3, s1, 31
	andi	s6, a3, 3
	addw	a3, s1, s6
	andi	a3, a3, 3
	subw	a3, a3, s6
	addiw	a3, a3, 1
	sw	a3, 12(s4)
	sw	t3, 12(s5)
	sw	t2, 12(s2)
	blt	t0, t1, .run_six.bb2	# rotated-loop-branch
.run_six.bb3:
	li	s2, 0
	li	s1, 1
.run_six.bb4:
	bge	s2, s1, .run_six.bb6
.run_six.bb5:
	addiw	s3, s2, 1
	mv	a4, s3	# call-arg-copy
	li	a0, 4
	addi	a1, sp, 40
	addi	a2, sp, 184
	addi	a3, sp, 328
	call	advance_rows_six
	addiw	a4, s2, 2
	li	a0, 4
	addi	a1, sp, 40
	addi	a2, sp, 328
	addi	a3, sp, 184
	call	advance_rows_six
	addiw	a4, s2, 3
	li	a0, 4
	addi	a1, sp, 40
	addi	a2, sp, 184
	addi	a3, sp, 328
	call	advance_rows_six
	addiw	a4, s2, 4
	li	a0, 4
	addi	a1, sp, 40
	addi	a2, sp, 328
	addi	a3, sp, 184
	call	advance_rows_six
	addiw	a4, s2, 5
	li	a0, 4
	addi	a1, sp, 40
	addi	a2, sp, 184
	addi	a3, sp, 328
	call	advance_rows_six
	addiw	a4, s2, 6
	li	a0, 4
	addi	a1, sp, 40
	addi	a2, sp, 328
	addi	a3, sp, 184
	call	advance_rows_six
	mv	s2, s3	# phi-copy
	blt	s2, s1, .run_six.bb5	# rotated-loop-branch
.run_six.bb6:
	addi	t0, sp, 184
	lw	t1, 0(t0)
	lw	t2, 4(t0)
	addw	t1, t1, t2
	lw	t2, 8(t0)
	addw	t1, t1, t2
	lw	t2, 12(t0)
	addw	t1, t1, t2
	lw	t2, 24(t0)
	addw	t1, t1, t2
	lw	t2, 28(t0)
	addw	t1, t1, t2
	lw	t2, 32(t0)
	addw	t1, t1, t2
	lw	t2, 36(t0)
	addw	t1, t1, t2
	lw	t2, 48(t0)
	addw	t1, t1, t2
	lw	t2, 52(t0)
	addw	t1, t1, t2
	lw	t2, 56(t0)
	addw	t1, t1, t2
	lw	t2, 60(t0)
	addw	t1, t1, t2
	lw	t2, 72(t0)
	addw	t1, t1, t2
	lw	t2, 76(t0)
	addw	t1, t1, t2
	lw	t2, 80(t0)
	addw	t1, t1, t2
	lw	t0, 84(t0)
	addw	a0, t1, t0
	ld	s7, 528(sp)
	ld	s6, 520(sp)
	ld	s5, 512(sp)
	ld	s4, 504(sp)
	ld	s3, 496(sp)
	ld	s2, 488(sp)
	ld	s1, 480(sp)
	ld	ra, 472(sp)
	addi	sp, sp, 544
	ret
	.size run_six, .-run_six

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	call	run_six
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
