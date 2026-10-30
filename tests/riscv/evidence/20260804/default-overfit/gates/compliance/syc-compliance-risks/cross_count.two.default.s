
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
	addi	sp, sp, -640
	sd	s1, 544(sp)
	sd	s2, 552(sp)
	sd	s3, 560(sp)
	sd	s4, 568(sp)
	sd	s5, 576(sp)
	sd	s6, 584(sp)
	sd	s7, 592(sp)
	sd	s8, 600(sp)
	sd	s9, 608(sp)
	sd	s10, 616(sp)
	sd	s11, 624(sp)
	li	t0, 0
	li	t1, 4
.run_two.bb1:
	bge	t0, t1, .run_two.bb3
.run_two.bb2:
	slliw	t2, t0, 1
	slliw	t3, t0, 3
	addw	s1, t2, t0
	slliw	t2, t0, 4
	addiw	s5, s1, 5
	addw	s2, t2, t3
	li	t2, 11
	addi	t3, sp, 256
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
	addi	t3, sp, 400
	addiw	t2, s3, 3
	add	s2, t3, s2
	sw	t2, 0(s2)
	sraiw	t2, s5, 31
	andi	s6, t2, 3
	addw	t2, s5, s6
	andi	t2, t2, 3
	addiw	s5, s1, 10
	subw	t2, t2, s6
	addiw	s1, s1, 15
	addiw	t2, t2, 1
	sw	t2, 4(s4)
	addiw	t2, s3, -7
	addiw	t2, t2, 3
	sw	t2, 4(s2)
	sraiw	t2, s5, 31
	andi	s6, t2, 3
	addw	t2, s5, s6
	andi	t2, t2, 3
	subw	t2, t2, s6
	addiw	t2, t2, 1
	sw	t2, 8(s4)
	addiw	t2, s3, -14
	addiw	t2, t2, 3
	sw	t2, 8(s2)
	sraiw	t2, s1, 31
	andi	s5, t2, 3
	addw	t2, s1, s5
	andi	t2, t2, 3
	subw	t2, t2, s5
	addiw	t2, t2, 1
	sw	t2, 12(s4)
	addiw	t2, s3, -21
	addiw	t2, t2, 3
	sw	t2, 12(s2)
	blt	t0, t1, .run_two.bb2	# rotated-loop-branch
.run_two.bb3:
	addi	t0, sp, 400
	lw	t1, 0(t0)
	lw	t2, 4(t0)
	addw	t1, t1, t2
	lw	t2, 8(t0)
	addw	t1, t1, t2
	lw	t2, 12(t0)
	addw	t5, t1, t2
	sd	t5, 0(sp)
	lw	t2, 24(t0)
	lw	t3, 28(t0)
	addw	t2, t2, t3
	lw	t3, 32(t0)
	addw	t2, t2, t3
	lw	t3, 36(t0)
	addw	t5, t2, t3
	sd	t5, 8(sp)
	lw	t3, 48(t0)
	lw	a2, 52(t0)
	addw	t3, t3, a2
	lw	a2, 56(t0)
	addw	t3, t3, a2
	lw	a2, 60(t0)
	addw	t5, t3, a2
	sd	t5, 16(sp)
	lw	a2, 72(t0)
	lw	a3, 76(t0)
	addw	a2, a2, a3
	lw	a3, 80(t0)
	addw	a2, a2, a3
	lw	t0, 84(t0)
	addw	s6, a2, t0
	addi	a2, sp, 256
	lw	a3, 48(a2)
	lw	a4, 72(a2)
	lw	s1, 28(a2)
	lw	a5, 24(a2)
	lw	s2, 76(a2)
	lw	t5, 56(a2)
	sd	t5, 184(sp)
	lw	t5, 80(a2)
	sd	t5, 216(sp)
	lw	t5, 60(a2)
	sd	t5, 208(sp)
	lw	t5, 84(a2)
	sd	t5, 104(sp)
	lw	a6, 0(a2)
	lw	t5, 4(a2)
	sd	t5, 232(sp)
	lw	t5, 36(a2)
	sd	t5, 200(sp)
	lw	s9, 8(a2)
	lw	t5, 12(a2)
	sd	t5, 192(sp)
	lw	t5, 32(a2)
	sd	t5, 176(sp)
	lw	a2, 52(a2)
	li	a7, 1
	subw	a6, a6, a7
	sltu	t5, zero, a6
	sd	t5, 224(sp)
	subw	a5, a5, a7
	sltu	t5, zero, a5
	sd	t5, 144(sp)
	subw	a3, a3, a7
	sltu	t5, zero, a3
	sd	t5, 128(sp)
	subw	a4, a4, a7
	sltu	t5, zero, a4
	sd	t5, 136(sp)
	ld	t5, 232(sp)
	subw	a7, t5, a7
	sltu	t5, zero, a7
	sd	t5, 152(sp)
	li	t4, 1
	subw	t4, s1, t4
	sltu	t5, zero, t4
	sd	t5, 160(sp)
	li	a0, 1
	subw	a0, a2, a0
	sltu	t5, zero, a0
	sd	t5, 168(sp)
	li	a1, 1
	subw	a1, s2, a1
	sltu	t5, zero, a1
	sd	t5, 96(sp)
	li	t1, 1
	subw	t1, s9, t1
	sltu	t5, zero, t1
	sd	t5, 88(sp)
	li	t2, 1
	ld	t5, 176(sp)
	subw	t2, t5, t2
	sltu	t5, zero, t2
	sd	t5, 80(sp)
	li	t3, 1
	ld	t5, 184(sp)
	subw	t3, t5, t3
	sltu	t5, zero, t3
	sd	t5, 72(sp)
	li	t0, 1
	ld	t5, 216(sp)
	subw	t0, t5, t0
	sltu	t5, zero, t0
	sd	t5, 32(sp)
	li	t0, 1
	ld	t5, 192(sp)
	subw	t0, t5, t0
	sltu	t5, zero, t0
	sd	t5, 40(sp)
	li	t0, 1
	ld	t5, 200(sp)
	subw	t0, t5, t0
	sltu	t5, zero, t0
	sd	t5, 48(sp)
	li	t0, 1
	ld	t5, 208(sp)
	subw	t0, t5, t0
	sltu	t5, zero, t0
	sd	t5, 56(sp)
	li	t0, 1
	ld	t5, 104(sp)
	subw	t0, t5, t0
	sltu	t5, zero, t0
	sd	t5, 64(sp)
	li	t0, 0
	li	t5, 1
	sd	t5, 112(sp)
	sd	t0, 120(sp)
	ld	t2, 0(sp)
	ld	t1, 8(sp)
	ld	a1, 16(sp)
.run_two.bb4:
	ld	t5, 120(sp)
	ld	t6, 112(sp)
	bge	t5, t6, .run_two.bb6
.run_two.bb5:
	ld	t5, 224(sp)
	mulw	t3, t5, t2
.run_two.bb7:
	ld	t5, 144(sp)
	mulw	t0, t5, t2
.run_two.bb8:
	ld	t5, 128(sp)
	mulw	a3, t5, t2
.run_two.bb9:
	ld	t5, 136(sp)
	mulw	t2, t5, t2
.run_two.bb10:
	ld	t5, 232(sp)
	mulw	a4, t3, t5
	addw	a4, a4, t1
	li	a5, 0
	ld	t5, 152(sp)
	subw	a5, a5, t5
	xor	a4, t3, a4
	and	a4, a4, a5
	xor	t3, t3, a4
.run_two.bb11:
	mulw	a4, t0, s1
	addw	a4, a4, t1
	li	a7, 0
	ld	t5, 160(sp)
	subw	a7, a7, t5
	xor	a4, t0, a4
	and	a4, a4, a7
	xor	t0, t0, a4
.run_two.bb12:
	mulw	a4, a3, a2
	addw	a4, a4, t1
	li	t4, 0
	ld	t5, 168(sp)
	subw	t4, t4, t5
	xor	a4, a3, a4
	and	a4, a4, t4
	xor	a3, a3, a4
.run_two.bb13:
	mulw	a4, t2, s2
	addw	t1, a4, t1
	li	a4, 0
	ld	t5, 96(sp)
	subw	a4, a4, t5
	xor	t1, t2, t1
	and	t1, t1, a4
	xor	t1, t2, t1
.run_two.bb14:
	mulw	t2, t3, s9
	addw	t2, t2, a1
	li	a0, 0
	ld	t5, 88(sp)
	subw	a0, a0, t5
	xor	t2, t3, t2
	and	t2, t2, a0
	xor	t2, t3, t2
.run_two.bb15:
	ld	t5, 176(sp)
	mulw	t3, t0, t5
	addw	t3, t3, a1
	li	s11, 0
	ld	t5, 80(sp)
	subw	s11, s11, t5
	xor	t3, t0, t3
	and	t3, t3, s11
	xor	t0, t0, t3
.run_two.bb16:
	ld	t5, 184(sp)
	mulw	t3, a3, t5
	addw	t3, t3, a1
	li	s3, 0
	ld	t5, 72(sp)
	subw	s3, s3, t5
	xor	t3, a3, t3
	and	t3, t3, s3
	xor	t3, a3, t3
.run_two.bb17:
	ld	t5, 216(sp)
	mulw	a3, t1, t5
	addw	a3, a3, a1
	li	a1, 0
	ld	t5, 32(sp)
	subw	a1, a1, t5
	xor	a3, t1, a3
	and	a3, a3, a1
	xor	t1, t1, a3
.run_two.bb18:
	ld	t5, 192(sp)
	mulw	a3, t2, t5
	addw	a3, a3, s6
	li	s10, 0
	ld	t5, 40(sp)
	subw	s10, s10, t5
	xor	a3, t2, a3
	and	a3, a3, s10
	xor	t2, t2, a3
.run_two.bb19:
	ld	t5, 200(sp)
	mulw	a3, t0, t5
	addw	a3, a3, s6
	li	s8, 0
	ld	t5, 48(sp)
	subw	s8, s8, t5
	xor	a3, t0, a3
	and	a3, a3, s8
	xor	t0, t0, a3
.run_two.bb20:
	ld	t5, 208(sp)
	mulw	a3, t3, t5
	addw	a3, a3, s6
	li	s5, 0
	ld	t5, 56(sp)
	subw	t5, s5, t5
	sd	t5, 248(sp)
	xor	a3, t3, a3
	and	a3, a3, t5
	xor	t3, t3, a3
.run_two.bb21:
	ld	t5, 104(sp)
	mulw	a3, t1, t5
	addw	a3, a3, s6
	li	s4, 0
	ld	t5, 64(sp)
	subw	t5, s4, t5
	sd	t5, 240(sp)
	xor	a3, t1, a3
	and	a3, a3, t5
	xor	s6, t1, a3
.run_two.bb22:
	ld	t5, 224(sp)
	mulw	t1, t5, t2
.run_two.bb23:
	ld	t5, 144(sp)
	mulw	a3, t5, t2
.run_two.bb24:
	ld	t5, 128(sp)
	mulw	a6, t5, t2
.run_two.bb25:
	ld	t5, 136(sp)
	mulw	t2, t5, t2
.run_two.bb26:
	ld	t5, 232(sp)
	mulw	s7, t1, t5
	addw	s4, s7, t0
	xor	s5, t1, s4
	and	a5, s5, a5
	xor	s4, t1, a5
.run_two.bb27:
	mulw	t1, a3, s1
	addw	t1, t1, t0
	xor	t1, a3, t1
	and	t1, t1, a7
	xor	s5, a3, t1
.run_two.bb28:
	mulw	t1, a6, a2
	addw	t1, t1, t0
	xor	t1, a6, t1
	and	t1, t1, t4
	xor	s7, a6, t1
.run_two.bb29:
	mulw	t1, t2, s2
	addw	t0, t1, t0
	xor	t0, t2, t0
	and	t0, t0, a4
	xor	t0, t2, t0
.run_two.bb30:
	mulw	t1, s4, s9
	addw	t1, t1, t3
	xor	t1, s4, t1
	and	t1, t1, a0
	xor	s4, s4, t1
.run_two.bb31:
	ld	t5, 176(sp)
	mulw	t1, s5, t5
	addw	t1, t1, t3
	xor	t1, s5, t1
	and	t1, t1, s11
	xor	s5, s5, t1
.run_two.bb32:
	ld	t5, 184(sp)
	mulw	t1, s7, t5
	addw	t1, t1, t3
	xor	t1, s7, t1
	and	t1, t1, s3
	xor	s3, s7, t1
.run_two.bb33:
	ld	t5, 216(sp)
	mulw	t1, t0, t5
	addw	t1, t1, t3
	xor	t1, t0, t1
	and	t1, t1, a1
	xor	s7, t0, t1
.run_two.bb34:
	ld	t5, 192(sp)
	mulw	t0, s4, t5
	addw	t0, t0, s6
	xor	t0, s4, t0
	and	t0, t0, s10
	xor	t0, s4, t0
.run_two.bb35:
	ld	t5, 200(sp)
	mulw	t1, s5, t5
	addw	t1, t1, s6
	xor	t1, s5, t1
	and	t1, t1, s8
	xor	t1, s5, t1
.run_two.bb36:
	ld	t5, 208(sp)
	mulw	t2, s3, t5
	addw	t2, t2, s6
	xor	t2, s3, t2
	ld	t5, 248(sp)
	and	t2, t2, t5
	xor	t2, s3, t2
.run_two.bb37:
	ld	t5, 104(sp)
	mulw	t3, s7, t5
	addw	t3, t3, s6
	xor	t3, s7, t3
	ld	t5, 240(sp)
	and	t3, t3, t5
	xor	t3, s7, t3
.run_two.bb38:
	ld	t5, 120(sp)
	addiw	a3, t5, 1
	sd	a3, 120(sp)
	mv	a1, t2	# phi-copy
	mv	t2, t0	# phi-copy
	mv	s6, t3	# phi-copy
	j	.run_two.bb4
.run_two.bb6:
	addw	t0, t2, t1
	addw	t0, t0, a1
	addw	a0, t0, s6
	ld	s11, 624(sp)
	ld	s10, 616(sp)
	ld	s9, 608(sp)
	ld	s8, 600(sp)
	ld	s7, 592(sp)
	ld	s6, 584(sp)
	ld	s5, 576(sp)
	ld	s4, 568(sp)
	ld	s3, 560(sp)
	ld	s2, 552(sp)
	ld	s1, 544(sp)
	addi	sp, sp, 640
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
