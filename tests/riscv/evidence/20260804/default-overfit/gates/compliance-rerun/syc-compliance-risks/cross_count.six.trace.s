
  .section .rodata
  .globl SIDE
  .align 2
  .type  SIDE, @object
  .size  SIDE, 4
SIDE:
  .word 6

  .text

  .globl run_six
  .type  run_six, @function
run_six:
.run_six.bb0:
	addi	sp, sp, -624
	sd	s1, 528(sp)
	sd	s2, 536(sp)
	sd	s3, 544(sp)
	sd	s4, 552(sp)
	sd	s5, 560(sp)
	sd	s6, 568(sp)
	sd	s7, 576(sp)
	sd	s8, 584(sp)
	sd	s9, 592(sp)
	sd	s10, 600(sp)
	sd	s11, 608(sp)
	li	t0, 0
	li	t1, 4
.run_six.bb1:
	bge	t0, t1, .run_six.bb3
.run_six.bb2:
	slliw	t2, t0, 1
	slliw	t3, t0, 3
	addw	s1, t2, t0
	slliw	t2, t0, 4
	addiw	s5, s1, 5
	addw	s2, t2, t3
	li	t2, 11
	addi	t3, sp, 240
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
	addi	t3, sp, 384
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
	blt	t0, t1, .run_six.bb2	# rotated-loop-branch
.run_six.bb3:
	addi	t0, sp, 384
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
	addw	s8, a2, t0
	addi	a2, sp, 240
	lw	a3, 0(a2)
	lw	a4, 48(a2)
	lw	s1, 28(a2)
	lw	t5, 32(a2)
	sd	t5, 176(sp)
	lw	t5, 12(a2)
	sd	t5, 192(sp)
	lw	t5, 36(a2)
	sd	t5, 200(sp)
	lw	a5, 72(a2)
	lw	s5, 8(a2)
	lw	t5, 4(a2)
	sd	t5, 144(sp)
	lw	t5, 60(a2)
	sd	t5, 208(sp)
	lw	t5, 84(a2)
	sd	t5, 104(sp)
	lw	s9, 52(a2)
	lw	a6, 24(a2)
	lw	s10, 76(a2)
	lw	s11, 80(a2)
	lw	t5, 56(a2)
	sd	t5, 184(sp)
	li	a7, 1
	subw	a3, a3, a7
	sltu	t5, zero, a3
	sd	t5, 232(sp)
	subw	a6, a6, a7
	sltu	t5, zero, a6
	sd	t5, 224(sp)
	subw	a4, a4, a7
	sltu	t5, zero, a4
	sd	t5, 128(sp)
	subw	a5, a5, a7
	sltu	t5, zero, a5
	sd	t5, 136(sp)
	ld	t5, 144(sp)
	subw	a7, t5, a7
	sltu	t5, zero, a7
	sd	t5, 152(sp)
	li	t4, 1
	subw	t4, s1, t4
	sltu	t5, zero, t4
	sd	t5, 160(sp)
	li	a0, 1
	subw	a0, s9, a0
	sltu	t5, zero, a0
	sd	t5, 168(sp)
	li	a1, 1
	subw	a1, s10, a1
	sltu	t5, zero, a1
	sd	t5, 96(sp)
	li	t1, 1
	subw	t1, s5, t1
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
	subw	t0, s11, t0
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
.run_six.bb4:
	ld	t5, 120(sp)
	ld	t6, 112(sp)
	bge	t5, t6, .run_six.bb6
.run_six.bb5:
	ld	t5, 232(sp)
	mulw	t3, t5, t2
.run_six.bb7:
	ld	t5, 224(sp)
	mulw	t0, t5, t2
.run_six.bb8:
	ld	t5, 128(sp)
	mulw	a4, t5, t2
.run_six.bb9:
	ld	t5, 136(sp)
	mulw	t2, t5, t2
.run_six.bb10:
	ld	t5, 144(sp)
	mulw	a5, t3, t5
	addw	a5, a5, t1
	li	s6, 0
	ld	t5, 152(sp)
	subw	s6, s6, t5
	xor	a5, t3, a5
	and	a5, a5, s6
	xor	t3, t3, a5
.run_six.bb11:
	mulw	a5, t0, s1
	addw	a5, a5, t1
	li	a7, 0
	ld	t5, 160(sp)
	subw	a7, a7, t5
	xor	a5, t0, a5
	and	a5, a5, a7
	xor	t0, t0, a5
.run_six.bb12:
	mulw	a5, a4, s9
	addw	a5, a5, t1
	li	t4, 0
	ld	t5, 168(sp)
	subw	t4, t4, t5
	xor	a5, a4, a5
	and	a5, a5, t4
	xor	a4, a4, a5
.run_six.bb13:
	mulw	a5, t2, s10
	addw	t1, a5, t1
	li	a5, 0
	ld	t5, 96(sp)
	subw	a5, a5, t5
	xor	t1, t2, t1
	and	t1, t1, a5
	xor	t1, t2, t1
.run_six.bb14:
	mulw	t2, t3, s5
	addw	t2, t2, a1
	li	a0, 0
	ld	t5, 88(sp)
	subw	a0, a0, t5
	xor	t2, t3, t2
	and	t2, t2, a0
	xor	t2, t3, t2
.run_six.bb15:
	ld	t5, 176(sp)
	mulw	t3, t0, t5
	addw	t3, t3, a1
	li	s2, 0
	ld	t5, 80(sp)
	subw	s2, s2, t5
	xor	t3, t0, t3
	and	t3, t3, s2
	xor	t0, t0, t3
.run_six.bb16:
	ld	t5, 184(sp)
	mulw	t3, a4, t5
	addw	t3, t3, a1
	li	a2, 0
	ld	t5, 72(sp)
	subw	a2, a2, t5
	xor	t3, a4, t3
	and	t3, t3, a2
	xor	t3, a4, t3
.run_six.bb17:
	mulw	a4, t1, s11
	addw	a4, a4, a1
	li	a1, 0
	ld	t5, 32(sp)
	subw	a1, a1, t5
	xor	a4, t1, a4
	and	a4, a4, a1
	xor	t1, t1, a4
.run_six.bb18:
	ld	t5, 192(sp)
	mulw	a4, t2, t5
	addw	a4, a4, s8
	li	s3, 0
	ld	t5, 40(sp)
	subw	s3, s3, t5
	xor	a4, t2, a4
	and	a4, a4, s3
	xor	t2, t2, a4
.run_six.bb19:
	ld	t5, 200(sp)
	mulw	a4, t0, t5
	addw	a4, a4, s8
	li	s4, 0
	ld	t5, 48(sp)
	subw	s4, s4, t5
	xor	a4, t0, a4
	and	a4, a4, s4
	xor	t0, t0, a4
.run_six.bb20:
	ld	t5, 208(sp)
	mulw	a4, t3, t5
	addw	a4, a4, s8
	li	s7, 0
	ld	t5, 56(sp)
	subw	t5, s7, t5
	sd	t5, 216(sp)
	xor	a4, t3, a4
	and	a4, a4, t5
	xor	t3, t3, a4
.run_six.bb21:
	ld	t5, 104(sp)
	mulw	a4, t1, t5
	addw	a4, a4, s8
	li	s7, 0
	ld	t5, 64(sp)
	subw	s7, s7, t5
	xor	a4, t1, a4
	and	a4, a4, s7
	xor	s8, t1, a4
.run_six.bb22:
	ld	t5, 232(sp)
	mulw	t1, t5, t2
.run_six.bb23:
	ld	t5, 224(sp)
	mulw	a4, t5, t2
.run_six.bb24:
	ld	t5, 128(sp)
	mulw	a6, t5, t2
.run_six.bb25:
	ld	t5, 136(sp)
	mulw	t2, t5, t2
.run_six.bb26:
	ld	t5, 144(sp)
	mulw	a3, t1, t5
	addw	a3, a3, t0
	xor	a3, t1, a3
	and	a3, a3, s6
	xor	t1, t1, a3
.run_six.bb27:
	mulw	a3, a4, s1
	addw	a3, a3, t0
	xor	a3, a4, a3
	and	a3, a3, a7
	xor	a3, a4, a3
.run_six.bb28:
	mulw	a4, a6, s9
	addw	a4, a4, t0
	xor	a4, a6, a4
	and	a4, a4, t4
	xor	a4, a6, a4
.run_six.bb29:
	mulw	a6, t2, s10
	addw	t0, a6, t0
	xor	t0, t2, t0
	and	t0, t0, a5
	xor	t0, t2, t0
.run_six.bb30:
	mulw	t2, t1, s5
	addw	t2, t2, t3
	xor	t2, t1, t2
	and	t2, t2, a0
	xor	t1, t1, t2
.run_six.bb31:
	ld	t5, 176(sp)
	mulw	t2, a3, t5
	addw	t2, t2, t3
	xor	t2, a3, t2
	and	t2, t2, s2
	xor	t2, a3, t2
.run_six.bb32:
	ld	t5, 184(sp)
	mulw	a3, a4, t5
	addw	a3, a3, t3
	xor	a3, a4, a3
	and	a3, a3, a2
	xor	a3, a4, a3
.run_six.bb33:
	mulw	a4, t0, s11
	addw	t3, a4, t3
	xor	t3, t0, t3
	and	t3, t3, a1
	xor	t0, t0, t3
.run_six.bb34:
	ld	t5, 192(sp)
	mulw	t3, t1, t5
	addw	t3, t3, s8
	xor	t3, t1, t3
	and	t3, t3, s3
	xor	t1, t1, t3
.run_six.bb35:
	ld	t5, 200(sp)
	mulw	t3, t2, t5
	addw	t3, t3, s8
	xor	t3, t2, t3
	and	t3, t3, s4
	xor	t2, t2, t3
.run_six.bb36:
	ld	t5, 208(sp)
	mulw	t3, a3, t5
	addw	t3, t3, s8
	xor	t3, a3, t3
	ld	t5, 216(sp)
	and	t3, t3, t5
	xor	t3, a3, t3
.run_six.bb37:
	ld	t5, 104(sp)
	mulw	a3, t0, t5
	addw	a3, a3, s8
	xor	a3, t0, a3
	and	a3, a3, s7
	xor	s8, t0, a3
.run_six.bb38:
	ld	t5, 232(sp)
	mulw	t0, t5, t1
.run_six.bb39:
	ld	t5, 224(sp)
	mulw	a3, t5, t1
.run_six.bb40:
	ld	t5, 128(sp)
	mulw	a4, t5, t1
.run_six.bb41:
	ld	t5, 136(sp)
	mulw	t1, t5, t1
.run_six.bb42:
	ld	t5, 144(sp)
	mulw	a6, t0, t5
	addw	a6, a6, t2
	xor	a6, t0, a6
	and	a6, a6, s6
	xor	t0, t0, a6
.run_six.bb43:
	mulw	a6, a3, s1
	addw	a6, a6, t2
	xor	a6, a3, a6
	and	a6, a6, a7
	xor	a3, a3, a6
.run_six.bb44:
	mulw	a6, a4, s9
	addw	a6, a6, t2
	xor	a6, a4, a6
	and	a6, a6, t4
	xor	a4, a4, a6
.run_six.bb45:
	mulw	a6, t1, s10
	addw	t2, a6, t2
	xor	t2, t1, t2
	and	t2, t2, a5
	xor	t1, t1, t2
.run_six.bb46:
	mulw	t2, t0, s5
	addw	t2, t2, t3
	xor	t2, t0, t2
	and	t2, t2, a0
	xor	t0, t0, t2
.run_six.bb47:
	ld	t5, 176(sp)
	mulw	t2, a3, t5
	addw	t2, t2, t3
	xor	t2, a3, t2
	and	t2, t2, s2
	xor	t2, a3, t2
.run_six.bb48:
	ld	t5, 184(sp)
	mulw	a3, a4, t5
	addw	a3, a3, t3
	xor	a3, a4, a3
	and	a3, a3, a2
	xor	a3, a4, a3
.run_six.bb49:
	mulw	a4, t1, s11
	addw	t3, a4, t3
	xor	t3, t1, t3
	and	t3, t3, a1
	xor	t1, t1, t3
.run_six.bb50:
	ld	t5, 192(sp)
	mulw	t3, t0, t5
	addw	t3, t3, s8
	xor	t3, t0, t3
	and	t3, t3, s3
	xor	t0, t0, t3
.run_six.bb51:
	ld	t5, 200(sp)
	mulw	t3, t2, t5
	addw	t3, t3, s8
	xor	t3, t2, t3
	and	t3, t3, s4
	xor	t2, t2, t3
.run_six.bb52:
	ld	t5, 208(sp)
	mulw	t3, a3, t5
	addw	t3, t3, s8
	xor	t3, a3, t3
	ld	t5, 216(sp)
	and	t3, t3, t5
	xor	t3, a3, t3
.run_six.bb53:
	ld	t5, 104(sp)
	mulw	a3, t1, t5
	addw	a3, a3, s8
	xor	a3, t1, a3
	and	a3, a3, s7
	xor	s8, t1, a3
.run_six.bb54:
	ld	t5, 232(sp)
	mulw	t1, t5, t0
.run_six.bb55:
	ld	t5, 224(sp)
	mulw	a3, t5, t0
.run_six.bb56:
	ld	t5, 128(sp)
	mulw	a4, t5, t0
.run_six.bb57:
	ld	t5, 136(sp)
	mulw	t0, t5, t0
.run_six.bb58:
	ld	t5, 144(sp)
	mulw	a6, t1, t5
	addw	a6, a6, t2
	xor	a6, t1, a6
	and	a6, a6, s6
	xor	t1, t1, a6
.run_six.bb59:
	mulw	a6, a3, s1
	addw	a6, a6, t2
	xor	a6, a3, a6
	and	a6, a6, a7
	xor	a3, a3, a6
.run_six.bb60:
	mulw	a6, a4, s9
	addw	a6, a6, t2
	xor	a6, a4, a6
	and	a6, a6, t4
	xor	a4, a4, a6
.run_six.bb61:
	mulw	a6, t0, s10
	addw	t2, a6, t2
	xor	t2, t0, t2
	and	t2, t2, a5
	xor	t0, t0, t2
.run_six.bb62:
	mulw	t2, t1, s5
	addw	t2, t2, t3
	xor	t2, t1, t2
	and	t2, t2, a0
	xor	t1, t1, t2
.run_six.bb63:
	ld	t5, 176(sp)
	mulw	t2, a3, t5
	addw	t2, t2, t3
	xor	t2, a3, t2
	and	t2, t2, s2
	xor	t2, a3, t2
.run_six.bb64:
	ld	t5, 184(sp)
	mulw	a3, a4, t5
	addw	a3, a3, t3
	xor	a3, a4, a3
	and	a3, a3, a2
	xor	a3, a4, a3
.run_six.bb65:
	mulw	a4, t0, s11
	addw	t3, a4, t3
	xor	t3, t0, t3
	and	t3, t3, a1
	xor	t0, t0, t3
.run_six.bb66:
	ld	t5, 192(sp)
	mulw	t3, t1, t5
	addw	t3, t3, s8
	xor	t3, t1, t3
	and	t3, t3, s3
	xor	t1, t1, t3
.run_six.bb67:
	ld	t5, 200(sp)
	mulw	t3, t2, t5
	addw	t3, t3, s8
	xor	t3, t2, t3
	and	t3, t3, s4
	xor	t2, t2, t3
.run_six.bb68:
	ld	t5, 208(sp)
	mulw	t3, a3, t5
	addw	t3, t3, s8
	xor	t3, a3, t3
	ld	t5, 216(sp)
	and	t3, t3, t5
	xor	t3, a3, t3
.run_six.bb69:
	ld	t5, 104(sp)
	mulw	a3, t0, t5
	addw	a3, a3, s8
	xor	a3, t0, a3
	and	a3, a3, s7
	xor	s8, t0, a3
.run_six.bb70:
	ld	t5, 232(sp)
	mulw	t0, t5, t1
.run_six.bb71:
	ld	t5, 224(sp)
	mulw	a3, t5, t1
.run_six.bb72:
	ld	t5, 128(sp)
	mulw	a4, t5, t1
.run_six.bb73:
	ld	t5, 136(sp)
	mulw	t1, t5, t1
.run_six.bb74:
	ld	t5, 144(sp)
	mulw	a6, t0, t5
	addw	a6, a6, t2
	xor	a6, t0, a6
	and	a6, a6, s6
	xor	t0, t0, a6
.run_six.bb75:
	mulw	a6, a3, s1
	addw	a6, a6, t2
	xor	a6, a3, a6
	and	a6, a6, a7
	xor	a3, a3, a6
.run_six.bb76:
	mulw	a6, a4, s9
	addw	a6, a6, t2
	xor	a6, a4, a6
	and	a6, a6, t4
	xor	a4, a4, a6
.run_six.bb77:
	mulw	a6, t1, s10
	addw	t2, a6, t2
	xor	t2, t1, t2
	and	t2, t2, a5
	xor	t1, t1, t2
.run_six.bb78:
	mulw	t2, t0, s5
	addw	t2, t2, t3
	xor	t2, t0, t2
	and	t2, t2, a0
	xor	t0, t0, t2
.run_six.bb79:
	ld	t5, 176(sp)
	mulw	t2, a3, t5
	addw	t2, t2, t3
	xor	t2, a3, t2
	and	t2, t2, s2
	xor	t2, a3, t2
.run_six.bb80:
	ld	t5, 184(sp)
	mulw	a3, a4, t5
	addw	a3, a3, t3
	xor	a3, a4, a3
	and	a3, a3, a2
	xor	a3, a4, a3
.run_six.bb81:
	mulw	a4, t1, s11
	addw	t3, a4, t3
	xor	t3, t1, t3
	and	t3, t3, a1
	xor	t1, t1, t3
.run_six.bb82:
	ld	t5, 192(sp)
	mulw	t3, t0, t5
	addw	t3, t3, s8
	xor	t3, t0, t3
	and	t3, t3, s3
	xor	t0, t0, t3
.run_six.bb83:
	ld	t5, 200(sp)
	mulw	t3, t2, t5
	addw	t3, t3, s8
	xor	t3, t2, t3
	and	t3, t3, s4
	xor	t2, t2, t3
.run_six.bb84:
	ld	t5, 208(sp)
	mulw	t3, a3, t5
	addw	t3, t3, s8
	xor	t3, a3, t3
	ld	t5, 216(sp)
	and	t3, t3, t5
	xor	t3, a3, t3
.run_six.bb85:
	ld	t5, 104(sp)
	mulw	a3, t1, t5
	addw	a3, a3, s8
	xor	a3, t1, a3
	and	a3, a3, s7
	xor	s8, t1, a3
.run_six.bb86:
	ld	t5, 232(sp)
	mulw	t1, t5, t0
.run_six.bb87:
	ld	t5, 224(sp)
	mulw	a3, t5, t0
.run_six.bb88:
	ld	t5, 128(sp)
	mulw	a4, t5, t0
.run_six.bb89:
	ld	t5, 136(sp)
	mulw	t0, t5, t0
.run_six.bb90:
	ld	t5, 144(sp)
	mulw	a6, t1, t5
	addw	a6, a6, t2
	xor	a6, t1, a6
	and	a6, a6, s6
	xor	s6, t1, a6
.run_six.bb91:
	mulw	t1, a3, s1
	addw	t1, t1, t2
	xor	t1, a3, t1
	and	t1, t1, a7
	xor	t1, a3, t1
.run_six.bb92:
	mulw	a3, a4, s9
	addw	a3, a3, t2
	xor	a3, a4, a3
	and	a3, a3, t4
	xor	a3, a4, a3
.run_six.bb93:
	mulw	a4, t0, s10
	addw	t2, a4, t2
	xor	t2, t0, t2
	and	t2, t2, a5
	xor	t0, t0, t2
.run_six.bb94:
	mulw	t2, s6, s5
	addw	t2, t2, t3
	xor	t2, s6, t2
	and	t2, t2, a0
	xor	s6, s6, t2
.run_six.bb95:
	ld	t5, 176(sp)
	mulw	t2, t1, t5
	addw	t2, t2, t3
	xor	t2, t1, t2
	and	t2, t2, s2
	xor	s2, t1, t2
.run_six.bb96:
	ld	t5, 184(sp)
	mulw	t1, a3, t5
	addw	t1, t1, t3
	xor	t1, a3, t1
	and	t1, t1, a2
	xor	t1, a3, t1
.run_six.bb97:
	mulw	t2, t0, s11
	addw	t2, t2, t3
	xor	t2, t0, t2
	and	t2, t2, a1
	xor	t0, t0, t2
.run_six.bb98:
	ld	t5, 192(sp)
	mulw	t2, s6, t5
	addw	t2, t2, s8
	xor	t2, s6, t2
	and	t2, t2, s3
	xor	t2, s6, t2
.run_six.bb99:
	ld	t5, 200(sp)
	mulw	t3, s2, t5
	addw	t3, t3, s8
	xor	t3, s2, t3
	and	t3, t3, s4
	xor	t3, s2, t3
.run_six.bb100:
	ld	t5, 208(sp)
	mulw	a2, t1, t5
	addw	a2, a2, s8
	xor	a2, t1, a2
	ld	t5, 216(sp)
	and	a2, a2, t5
	xor	t1, t1, a2
.run_six.bb101:
	ld	t5, 104(sp)
	mulw	a2, t0, t5
	addw	a2, a2, s8
	xor	a2, t0, a2
	and	a2, a2, s7
	xor	t0, t0, a2
.run_six.bb102:
	ld	t5, 120(sp)
	addiw	a2, t5, 1
	sd	a2, 120(sp)
	mv	a1, t1	# phi-copy
	mv	t1, t3	# phi-copy
	mv	s8, t0	# phi-copy
	j	.run_six.bb4
.run_six.bb6:
	addw	t0, t2, t1
	addw	t0, t0, a1
	addw	a0, t0, s8
	ld	s11, 608(sp)
	ld	s10, 600(sp)
	ld	s9, 592(sp)
	ld	s8, 584(sp)
	ld	s7, 576(sp)
	ld	s6, 568(sp)
	ld	s5, 560(sp)
	ld	s4, 552(sp)
	ld	s3, 544(sp)
	ld	s2, 536(sp)
	ld	s1, 528(sp)
	addi	sp, sp, 624
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
