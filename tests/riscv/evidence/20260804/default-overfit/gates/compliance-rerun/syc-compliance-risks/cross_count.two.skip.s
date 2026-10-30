
  .section .rodata
  .globl SIDE
  .align 2
  .type  SIDE, @object
  .size  SIDE, 4
SIDE:
  .word 6

  .text

  .globl run_two
  .type  run_two, @function
run_two:
.run_two.bb0:
	addi	sp, sp, -496
	sd	s1, 432(sp)
	sd	s2, 440(sp)
	sd	s3, 448(sp)
	sd	s4, 456(sp)
	sd	s5, 464(sp)
	sd	s6, 472(sp)
	sd	s7, 480(sp)
	li	t0, 0
	li	t1, 4
.run_two.bb1:
	bge	t0, t1, .run_two.bb3
.run_two.bb2:
	slliw	t2, t0, 1
	slliw	t3, t0, 3
	addw	s1, t2, t0
	li	a3, 0
	slliw	t2, t0, 4
	addiw	s6, s1, 5
	addw	s2, t2, t3
	li	t2, 11
	addi	t3, sp, 0
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
	addi	t3, sp, 144
	addiw	t2, s3, 3
	add	s5, t3, s2
	sw	t2, 0(s5)
	addi	t2, sp, 288
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
	blt	t0, t1, .run_two.bb2	# rotated-loop-branch
.run_two.bb3:
	li	t1, 1
	li	t0, 0
.run_two.bb4:
	bge	t0, t1, .run_two.bb6
.run_two.bb5:
	li	t2, 0
	addi	s1, sp, 288
	sw	t2, 0(s1)
	addiw	t0, t0, 1
	li	a4, 0
	sw	t2, 4(s1)
	sw	t2, 8(s1)
	sw	t2, 12(s1)
	sw	t2, 24(s1)
	sw	t2, 28(s1)
	sw	t2, 32(s1)
	sw	t2, 36(s1)
	sw	t2, 48(s1)
	sw	t2, 52(s1)
	sw	t2, 56(s1)
	sw	t2, 60(s1)
	sw	t2, 72(s1)
	sw	t2, 76(s1)
	sw	t2, 80(s1)
	sw	t2, 84(s1)
	addi	s1, sp, 144
	lw	t2, 0(s1)
	lw	t3, 4(s1)
	lw	a2, 8(s1)
	lw	a3, 12(s1)
.run_two.bb7:
	li	a5, 4
	bge	a4, a5, .run_two.bb9
.run_two.bb8:
	slliw	a5, a4, 4
	slliw	a6, a4, 3
	addw	s1, a5, a6
	addi	a5, sp, 0
	add	a5, a5, s1
	lw	s2, 0(a5)
	li	a5, 1
	bne	s2, a5, .run_two.bb11
.run_two.bb10:
	addiw	a4, a4, 1
	j	.run_two.bb7
.run_two.bb11:
	addi	a5, sp, 288
	addiw	a4, a4, 1
	add	s1, a5, s1
	lw	a5, 0(s1)
	lw	a6, 4(s1)
	mulw	a5, a5, s2
	mulw	a6, a6, s2
	addw	a5, a5, t2
	sw	a5, 0(s1)
	addw	a6, a6, t3
	sw	a6, 4(s1)
	lw	a6, 8(s1)
	lw	a5, 12(s1)
	mulw	a6, a6, s2
	mulw	a5, a5, s2
	addw	a6, a6, a2
	sw	a6, 8(s1)
	addw	a5, a5, a3
	sw	a5, 12(s1)
	j	.run_two.bb7
.run_two.bb9:
	addi	s1, sp, 144
	li	a4, 0
	lw	t2, 24(s1)
	lw	t3, 28(s1)
	lw	a2, 32(s1)
	lw	a3, 36(s1)
.run_two.bb12:
	li	a5, 4
	bge	a4, a5, .run_two.bb14
.run_two.bb13:
	slliw	a5, a4, 4
	slliw	a6, a4, 3
	addw	s1, a5, a6
	addiw	a5, s1, 4
	addi	a6, sp, 0
	add	a5, a6, a5
	lw	s2, 0(a5)
	li	a5, 1
	bne	s2, a5, .run_two.bb16
.run_two.bb15:
	addiw	a4, a4, 1
	j	.run_two.bb12
.run_two.bb16:
	addi	a5, sp, 288
	addiw	a4, a4, 1
	add	s1, a5, s1
	lw	a5, 0(s1)
	lw	a6, 4(s1)
	mulw	a5, a5, s2
	mulw	a6, a6, s2
	addw	a5, a5, t2
	sw	a5, 0(s1)
	addw	a6, a6, t3
	sw	a6, 4(s1)
	lw	a6, 8(s1)
	lw	a5, 12(s1)
	mulw	a6, a6, s2
	mulw	a5, a5, s2
	addw	a6, a6, a2
	sw	a6, 8(s1)
	addw	a5, a5, a3
	sw	a5, 12(s1)
	j	.run_two.bb12
.run_two.bb14:
	addi	s1, sp, 144
	li	a4, 0
	lw	t2, 48(s1)
	lw	t3, 52(s1)
	lw	a2, 56(s1)
	lw	a3, 60(s1)
.run_two.bb17:
	li	a5, 4
	bge	a4, a5, .run_two.bb19
.run_two.bb18:
	slliw	a5, a4, 4
	slliw	a6, a4, 3
	addw	s1, a5, a6
	addiw	a5, s1, 8
	addi	a6, sp, 0
	add	a5, a6, a5
	lw	s2, 0(a5)
	li	a5, 1
	bne	s2, a5, .run_two.bb21
.run_two.bb20:
	addiw	a4, a4, 1
	j	.run_two.bb17
.run_two.bb21:
	addi	a5, sp, 288
	addiw	a4, a4, 1
	add	s1, a5, s1
	lw	a5, 0(s1)
	lw	a6, 4(s1)
	mulw	a5, a5, s2
	mulw	a6, a6, s2
	addw	a5, a5, t2
	sw	a5, 0(s1)
	addw	a6, a6, t3
	sw	a6, 4(s1)
	lw	a6, 8(s1)
	lw	a5, 12(s1)
	mulw	a6, a6, s2
	mulw	a5, a5, s2
	addw	a6, a6, a2
	sw	a6, 8(s1)
	addw	a5, a5, a3
	sw	a5, 12(s1)
	j	.run_two.bb17
.run_two.bb19:
	addi	s1, sp, 144
	li	a4, 0
	lw	t2, 72(s1)
	lw	t3, 76(s1)
	lw	a2, 80(s1)
	lw	a3, 84(s1)
.run_two.bb22:
	li	a5, 4
	bge	a4, a5, .run_two.bb24
.run_two.bb23:
	slliw	a5, a4, 4
	slliw	a6, a4, 3
	addw	s1, a5, a6
	addiw	a5, s1, 12
	addi	a6, sp, 0
	add	a5, a6, a5
	lw	s2, 0(a5)
	li	a5, 1
	bne	s2, a5, .run_two.bb26
.run_two.bb25:
	addiw	a4, a4, 1
	j	.run_two.bb22
.run_two.bb26:
	addi	a5, sp, 288
	addiw	a4, a4, 1
	add	s1, a5, s1
	lw	a5, 0(s1)
	lw	a6, 4(s1)
	mulw	a5, a5, s2
	mulw	a6, a6, s2
	addw	a5, a5, t2
	sw	a5, 0(s1)
	addw	a6, a6, t3
	sw	a6, 4(s1)
	lw	a6, 8(s1)
	lw	a5, 12(s1)
	mulw	a6, a6, s2
	mulw	a5, a5, s2
	addw	a6, a6, a2
	sw	a6, 8(s1)
	addw	a5, a5, a3
	sw	a5, 12(s1)
	j	.run_two.bb22
.run_two.bb24:
	li	t2, 0
	addi	s1, sp, 144
	sw	t2, 0(s1)
	li	a4, 0
	sw	t2, 4(s1)
	sw	t2, 8(s1)
	sw	t2, 12(s1)
	sw	t2, 24(s1)
	sw	t2, 28(s1)
	sw	t2, 32(s1)
	sw	t2, 36(s1)
	sw	t2, 48(s1)
	sw	t2, 52(s1)
	sw	t2, 56(s1)
	sw	t2, 60(s1)
	sw	t2, 72(s1)
	sw	t2, 76(s1)
	sw	t2, 80(s1)
	sw	t2, 84(s1)
	addi	s1, sp, 288
	lw	t2, 0(s1)
	lw	t3, 4(s1)
	lw	a2, 8(s1)
	lw	a3, 12(s1)
.run_two.bb27:
	li	a5, 4
	bge	a4, a5, .run_two.bb29
.run_two.bb28:
	slliw	a5, a4, 4
	slliw	a6, a4, 3
	addw	s1, a5, a6
	addi	a5, sp, 0
	add	a5, a5, s1
	lw	s2, 0(a5)
	li	a5, 1
	bne	s2, a5, .run_two.bb31
.run_two.bb30:
	addiw	a4, a4, 1
	j	.run_two.bb27
.run_two.bb31:
	addi	a5, sp, 144
	addiw	a4, a4, 1
	add	s1, a5, s1
	lw	a5, 0(s1)
	lw	a6, 4(s1)
	mulw	a5, a5, s2
	mulw	a6, a6, s2
	addw	a5, a5, t2
	sw	a5, 0(s1)
	addw	a6, a6, t3
	sw	a6, 4(s1)
	lw	a6, 8(s1)
	lw	a5, 12(s1)
	mulw	a6, a6, s2
	mulw	a5, a5, s2
	addw	a6, a6, a2
	sw	a6, 8(s1)
	addw	a5, a5, a3
	sw	a5, 12(s1)
	j	.run_two.bb27
.run_two.bb29:
	addi	s1, sp, 288
	li	a4, 0
	lw	t2, 24(s1)
	lw	t3, 28(s1)
	lw	a2, 32(s1)
	lw	a3, 36(s1)
.run_two.bb32:
	li	a5, 4
	bge	a4, a5, .run_two.bb34
.run_two.bb33:
	slliw	a5, a4, 4
	slliw	a6, a4, 3
	addw	s1, a5, a6
	addiw	a5, s1, 4
	addi	a6, sp, 0
	add	a5, a6, a5
	lw	s2, 0(a5)
	li	a5, 1
	bne	s2, a5, .run_two.bb36
.run_two.bb35:
	addiw	a4, a4, 1
	j	.run_two.bb32
.run_two.bb36:
	addi	a5, sp, 144
	addiw	a4, a4, 1
	add	s1, a5, s1
	lw	a5, 0(s1)
	lw	a6, 4(s1)
	mulw	a5, a5, s2
	mulw	a6, a6, s2
	addw	a5, a5, t2
	sw	a5, 0(s1)
	addw	a6, a6, t3
	sw	a6, 4(s1)
	lw	a6, 8(s1)
	lw	a5, 12(s1)
	mulw	a6, a6, s2
	mulw	a5, a5, s2
	addw	a6, a6, a2
	sw	a6, 8(s1)
	addw	a5, a5, a3
	sw	a5, 12(s1)
	j	.run_two.bb32
.run_two.bb34:
	addi	s1, sp, 288
	li	a4, 0
	lw	t2, 48(s1)
	lw	t3, 52(s1)
	lw	a2, 56(s1)
	lw	a3, 60(s1)
.run_two.bb37:
	li	a5, 4
	bge	a4, a5, .run_two.bb39
.run_two.bb38:
	slliw	a5, a4, 4
	slliw	a6, a4, 3
	addw	s1, a5, a6
	addiw	a5, s1, 8
	addi	a6, sp, 0
	add	a5, a6, a5
	lw	s2, 0(a5)
	li	a5, 1
	bne	s2, a5, .run_two.bb41
.run_two.bb40:
	addiw	a4, a4, 1
	j	.run_two.bb37
.run_two.bb41:
	addi	a5, sp, 144
	addiw	a4, a4, 1
	add	s1, a5, s1
	lw	a5, 0(s1)
	lw	a6, 4(s1)
	mulw	a5, a5, s2
	mulw	a6, a6, s2
	addw	a5, a5, t2
	sw	a5, 0(s1)
	addw	a6, a6, t3
	sw	a6, 4(s1)
	lw	a6, 8(s1)
	lw	a5, 12(s1)
	mulw	a6, a6, s2
	mulw	a5, a5, s2
	addw	a6, a6, a2
	sw	a6, 8(s1)
	addw	a5, a5, a3
	sw	a5, 12(s1)
	j	.run_two.bb37
.run_two.bb39:
	addi	s1, sp, 288
	li	a4, 0
	lw	t2, 72(s1)
	lw	t3, 76(s1)
	lw	a2, 80(s1)
	lw	a3, 84(s1)
.run_two.bb42:
	li	a5, 4
	bge	a4, a5, .run_two.bb44
.run_two.bb43:
	slliw	a5, a4, 4
	slliw	a6, a4, 3
	addw	s1, a5, a6
	addiw	a5, s1, 12
	addi	a6, sp, 0
	add	a5, a6, a5
	lw	s2, 0(a5)
	li	a5, 1
	bne	s2, a5, .run_two.bb46
.run_two.bb45:
	addiw	a4, a4, 1
	j	.run_two.bb42
.run_two.bb46:
	addi	a5, sp, 144
	addiw	a4, a4, 1
	add	s1, a5, s1
	lw	a5, 0(s1)
	lw	a6, 4(s1)
	mulw	a5, a5, s2
	mulw	a6, a6, s2
	addw	a5, a5, t2
	sw	a5, 0(s1)
	addw	a6, a6, t3
	sw	a6, 4(s1)
	lw	a6, 8(s1)
	lw	a5, 12(s1)
	mulw	a6, a6, s2
	mulw	a5, a5, s2
	addw	a6, a6, a2
	sw	a6, 8(s1)
	addw	a5, a5, a3
	sw	a5, 12(s1)
	j	.run_two.bb42
.run_two.bb44:
	blt	t0, t1, .run_two.bb5	# rotated-loop-branch
.run_two.bb6:
	addi	t0, sp, 144
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
	ld	s7, 480(sp)
	ld	s6, 472(sp)
	ld	s5, 464(sp)
	ld	s4, 456(sp)
	ld	s3, 448(sp)
	ld	s2, 440(sp)
	ld	s1, 432(sp)
	addi	sp, sp, 496
	ret
	.size run_two, .-run_two

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	call	run_two
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
