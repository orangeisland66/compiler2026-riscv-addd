
  .section .rodata
  .globl SIDE
  .align 2
  .type  SIDE, @object
  .size  SIDE, 4
SIDE:
  .word 6

  .text

  .globl run_extra_summary
  .type  run_extra_summary, @function
run_extra_summary:
.run_extra_summary.bb0:
	addi	sp, sp, -416
	sd	s1, 336(sp)
	sd	s2, 344(sp)
	sd	s3, 352(sp)
	sd	s4, 360(sp)
	sd	s5, 368(sp)
	sd	s6, 376(sp)
	sd	s7, 384(sp)
	sd	s8, 392(sp)
	sd	s9, 400(sp)
	sd	s10, 408(sp)
	mv	s1, a0
	mv	t0, a1
	li	t1, 0
.run_extra_summary.bb1:
	bge	t1, s1, .run_extra_summary.bb3
.run_extra_summary.bb2:
	slliw	t2, t1, 1
	slliw	t3, t1, 3
	addw	s2, t2, t1
	slliw	t2, t1, 4
	addw	s3, t2, t3
	li	t2, 11
	mulw	s4, t1, t2
	li	t2, 0
.run_extra_summary.bb4:
	bge	t2, s1, .run_extra_summary.bb6
.run_extra_summary.bb63:
	addiw	s5, t2, 3
	blt	s5, s1, .run_extra_summary.bb64
.run_extra_summary.bb5:
	slliw	s5, t2, 2
	addi	a2, sp, 48
	addw	t3, s5, t2
	addw	s6, s2, t3
	addw	s5, s3, s5
	sraiw	t3, s6, 31
	add	a2, a2, s5
	andi	s7, t3, 3
	addw	t3, s6, s7
	andi	t3, t3, 3
	subw	t3, t3, s7
	addiw	t3, t3, 1
	sw	t3, 0(a2)
	slliw	t3, t2, 3
	addi	a2, sp, 192
	subw	t3, t3, t2
	add	a2, a2, s5
	subw	t3, s4, t3
	addiw	t2, t2, 1
	addiw	t3, t3, 3
	sw	t3, 0(a2)
	j	.run_extra_summary.bb4
.run_extra_summary.bb64:
	slliw	s6, t2, 2
	addi	a2, sp, 48
	addw	t3, s6, t2
	addw	s7, s2, t3
	addw	s6, s3, s6
	sraiw	t3, s7, 31
	andi	s8, t3, 3
	addw	t3, s7, s8
	andi	t3, t3, 3
	add	s7, a2, s6
	subw	t3, t3, s8
	addi	a2, sp, 192
	addiw	t3, t3, 1
	add	s6, a2, s6
	sw	t3, 0(s7)
	addiw	s8, t2, 1
	slliw	t3, t2, 3
	subw	t3, t3, t2
	subw	t3, s4, t3
	addiw	t3, t3, 3
	sw	t3, 0(s6)
	slliw	t3, s8, 2
	addw	t3, t3, s8
	addw	s9, s2, t3
	sraiw	t3, s9, 31
	andi	s10, t3, 3
	addw	t3, s9, s10
	andi	t3, t3, 3
	subw	t3, t3, s10
	addiw	t3, t3, 1
	sw	t3, 4(s7)
	slliw	t3, s8, 3
	subw	t3, t3, s8
	subw	t3, s4, t3
	addiw	s8, t2, 2
	addiw	t3, t3, 3
	addiw	t2, t2, 4
	sw	t3, 4(s6)
	slliw	t3, s8, 2
	addw	t3, t3, s8
	addw	s9, s2, t3
	sraiw	t3, s9, 31
	andi	s10, t3, 3
	addw	t3, s9, s10
	andi	t3, t3, 3
	subw	t3, t3, s10
	addiw	t3, t3, 1
	sw	t3, 8(s7)
	slliw	t3, s8, 3
	subw	t3, t3, s8
	subw	t3, s4, t3
	addiw	t3, t3, 3
	sw	t3, 8(s6)
	slliw	t3, s5, 2
	addw	t3, t3, s5
	addw	s8, s2, t3
	sraiw	t3, s8, 31
	andi	s9, t3, 3
	addw	t3, s8, s9
	andi	t3, t3, 3
	subw	t3, t3, s9
	addiw	t3, t3, 1
	sw	t3, 12(s7)
	slliw	t3, s5, 3
	subw	t3, t3, s5
	subw	t3, s4, t3
	addiw	t3, t3, 3
	sw	t3, 12(s6)
	j	.run_extra_summary.bb4
.run_extra_summary.bb6:
	addiw	t1, t1, 1
	blt	t1, s1, .run_extra_summary.bb2	# rotated-loop-branch
.run_extra_summary.bb3:
	addi	s2, sp, 0
	li	t1, 0
	mv	s3, s2	# phi-copy
.run_extra_summary.bb7:
	bge	t1, s1, .run_extra_summary.bb9
.run_extra_summary.bb8:
	slliw	t2, t1, 4
	slliw	t3, t1, 3
	addw	s4, t2, t3
	li	t2, 0
	li	t3, 0
.run_extra_summary.bb10:
	bge	t2, s1, .run_extra_summary.bb12
.run_extra_summary.bb65:
	addiw	a2, t2, 3
	blt	a2, s1, .run_extra_summary.bb66
.run_extra_summary.bb11:
	slliw	a2, t2, 2
	addi	a3, sp, 192
	addw	a2, s4, a2
	addiw	t2, t2, 1
	add	a2, a3, a2
	lw	a2, 0(a2)
	addw	t3, t3, a2
	j	.run_extra_summary.bb10
.run_extra_summary.bb66:
	slliw	a2, t2, 2
	addi	a3, sp, 192
	addw	a2, s4, a2
	addiw	t2, t2, 4
	add	s5, a3, a2
	lw	a2, 0(s5)
	lw	a3, 4(s5)
	addw	t3, t3, a2
	addw	t3, t3, a3
	lw	a3, 8(s5)
	lw	a2, 12(s5)
	addw	t3, t3, a3
	addw	t3, t3, a2
	j	.run_extra_summary.bb10
.run_extra_summary.bb12:
	sw	t3, 0(s3)
	addiw	t1, t1, 1
	addi	s3, s3, 4
	blt	t1, s1, .run_extra_summary.bb8	# rotated-loop-branch
.run_extra_summary.bb9:
	addi	s3, sp, 24
	li	t1, 0
.run_extra_summary.bb13:
	bge	t1, t0, .run_extra_summary.bb15
.run_extra_summary.bb14:
	li	t2, 0
.run_extra_summary.bb16:
	bge	t2, s1, .run_extra_summary.bb18
.run_extra_summary.bb67:
	addiw	t3, t2, 3
	blt	t3, s1, .run_extra_summary.bb68
.run_extra_summary.bb17:
	slliw	t3, t2, 2
	addi	a2, sp, 24
	add	t3, a2, t3
	addiw	t2, t2, 1
	li	a2, 0
	sw	a2, 0(t3)
	j	.run_extra_summary.bb16
.run_extra_summary.bb68:
	slliw	t3, t2, 2
	addi	a2, sp, 24
	add	s4, a2, t3
	addiw	t2, t2, 4
	li	t3, 0
	li	a2, 0
	sw	t3, 0(s4)
	sw	a2, 4(s4)
	sw	a2, 8(s4)
	sw	t3, 12(s4)
	j	.run_extra_summary.bb16
.run_extra_summary.bb18:
	li	t2, 0
	mv	s4, s2	# phi-copy
.run_extra_summary.bb19:
	bge	t2, s1, .run_extra_summary.bb21
.run_extra_summary.bb20:
	lw	a2, 0(s4)
	slliw	t3, t2, 2
	li	a3, 0
	mv	s5, s3	# phi-copy
.run_extra_summary.bb22:
	bge	a3, s1, .run_extra_summary.bb24
.run_extra_summary.bb23:
	slliw	a4, a3, 4
	slliw	a5, a3, 3
	addw	a4, a4, a5
	addw	a4, a4, t3
	addi	a5, sp, 48
	add	a4, a5, a4
	lw	s6, 0(a4)
	li	a4, 1
	bne	s6, a4, .run_extra_summary.bb25
.run_extra_summary.bb26:
	addiw	a3, a3, 1
	addi	s5, s5, 4
	j	.run_extra_summary.bb22
.run_extra_summary.bb25:
	lw	a4, 0(s5)
	mulw	a4, a4, s6
	addw	a4, a4, a2
	sw	a4, 0(s5)
	j	.run_extra_summary.bb26
.run_extra_summary.bb24:
	addiw	t2, t2, 1
	addi	s4, s4, 4
	j	.run_extra_summary.bb19
.run_extra_summary.bb27:
	bge	t2, s1, .run_extra_summary.bb29
.run_extra_summary.bb69:
	addiw	t3, t2, 3
	blt	t3, s1, .run_extra_summary.bb70
.run_extra_summary.bb28:
	slliw	t3, t2, 2
	addi	a2, sp, 0
	add	t3, a2, t3
	addiw	t2, t2, 1
	li	a2, 0
	sw	a2, 0(t3)
	j	.run_extra_summary.bb27
.run_extra_summary.bb70:
	slliw	t3, t2, 2
	addi	a2, sp, 0
	add	s4, a2, t3
	addiw	t2, t2, 4
	li	t3, 0
	li	a2, 0
	sw	t3, 0(s4)
	sw	a2, 4(s4)
	sw	a2, 8(s4)
	sw	t3, 12(s4)
	j	.run_extra_summary.bb27
.run_extra_summary.bb29:
	li	t2, 0
	mv	s4, s3	# phi-copy
.run_extra_summary.bb30:
	bge	t2, s1, .run_extra_summary.bb32
.run_extra_summary.bb31:
	lw	a2, 0(s4)
	slliw	t3, t2, 2
	li	a3, 0
	mv	s5, s2	# phi-copy
.run_extra_summary.bb33:
	bge	a3, s1, .run_extra_summary.bb35
.run_extra_summary.bb34:
	slliw	a4, a3, 4
	slliw	a5, a3, 3
	addw	a4, a4, a5
	addw	a4, a4, t3
	addi	a5, sp, 48
	add	a4, a5, a4
	lw	s6, 0(a4)
	li	a4, 1
	bne	s6, a4, .run_extra_summary.bb36
.run_extra_summary.bb37:
	addiw	a3, a3, 1
	addi	s5, s5, 4
	j	.run_extra_summary.bb33
.run_extra_summary.bb36:
	lw	a4, 0(s5)
	mulw	a4, a4, s6
	addw	a4, a4, a2
	sw	a4, 0(s5)
	j	.run_extra_summary.bb37
.run_extra_summary.bb35:
	addiw	t2, t2, 1
	addi	s4, s4, 4
	j	.run_extra_summary.bb30
.run_extra_summary.bb38:
	bge	t2, s1, .run_extra_summary.bb40
.run_extra_summary.bb71:
	addiw	t3, t2, 3
	blt	t3, s1, .run_extra_summary.bb72
.run_extra_summary.bb39:
	slliw	t3, t2, 2
	addi	a2, sp, 24
	add	t3, a2, t3
	addiw	t2, t2, 1
	li	a2, 0
	sw	a2, 0(t3)
	j	.run_extra_summary.bb38
.run_extra_summary.bb72:
	slliw	t3, t2, 2
	addi	a2, sp, 24
	add	s4, a2, t3
	addiw	t2, t2, 4
	li	t3, 0
	li	a2, 0
	sw	t3, 0(s4)
	sw	a2, 4(s4)
	sw	a2, 8(s4)
	sw	t3, 12(s4)
	j	.run_extra_summary.bb38
.run_extra_summary.bb40:
	li	t2, 0
	mv	s4, s2	# phi-copy
.run_extra_summary.bb41:
	bge	t2, s1, .run_extra_summary.bb43
.run_extra_summary.bb42:
	lw	a2, 0(s4)
	slliw	t3, t2, 2
	li	a3, 0
	mv	s5, s3	# phi-copy
.run_extra_summary.bb44:
	bge	a3, s1, .run_extra_summary.bb46
.run_extra_summary.bb45:
	slliw	a4, a3, 4
	slliw	a5, a3, 3
	addw	a4, a4, a5
	addw	a4, a4, t3
	addi	a5, sp, 48
	add	a4, a5, a4
	lw	s6, 0(a4)
	li	a4, 1
	bne	s6, a4, .run_extra_summary.bb47
.run_extra_summary.bb48:
	addiw	a3, a3, 1
	addi	s5, s5, 4
	j	.run_extra_summary.bb44
.run_extra_summary.bb47:
	lw	a4, 0(s5)
	mulw	a4, a4, s6
	addw	a4, a4, a2
	sw	a4, 0(s5)
	j	.run_extra_summary.bb48
.run_extra_summary.bb46:
	addiw	t2, t2, 1
	addi	s4, s4, 4
	j	.run_extra_summary.bb41
.run_extra_summary.bb49:
	bge	t2, s1, .run_extra_summary.bb51
.run_extra_summary.bb73:
	addiw	t3, t2, 3
	blt	t3, s1, .run_extra_summary.bb74
.run_extra_summary.bb50:
	slliw	t3, t2, 2
	addi	a2, sp, 0
	add	t3, a2, t3
	addiw	t2, t2, 1
	li	a2, 0
	sw	a2, 0(t3)
	j	.run_extra_summary.bb49
.run_extra_summary.bb74:
	slliw	t3, t2, 2
	addi	a2, sp, 0
	add	s4, a2, t3
	addiw	t2, t2, 4
	li	t3, 0
	li	a2, 0
	sw	t3, 0(s4)
	sw	a2, 4(s4)
	sw	a2, 8(s4)
	sw	t3, 12(s4)
	j	.run_extra_summary.bb49
.run_extra_summary.bb51:
	li	t2, 0
	mv	s4, s3	# phi-copy
.run_extra_summary.bb52:
	bge	t2, s1, .run_extra_summary.bb54
.run_extra_summary.bb53:
	lw	a2, 0(s4)
	slliw	t3, t2, 2
	li	a3, 0
	mv	s5, s2	# phi-copy
.run_extra_summary.bb55:
	bge	a3, s1, .run_extra_summary.bb57
.run_extra_summary.bb56:
	slliw	a4, a3, 4
	slliw	a5, a3, 3
	addw	a4, a4, a5
	addw	a4, a4, t3
	addi	a5, sp, 48
	add	a4, a5, a4
	lw	s6, 0(a4)
	li	a4, 1
	bne	s6, a4, .run_extra_summary.bb58
.run_extra_summary.bb59:
	addiw	a3, a3, 1
	addi	s5, s5, 4
	j	.run_extra_summary.bb55
.run_extra_summary.bb58:
	lw	a4, 0(s5)
	mulw	a4, a4, s6
	addw	a4, a4, a2
	sw	a4, 0(s5)
	j	.run_extra_summary.bb59
.run_extra_summary.bb57:
	addiw	t2, t2, 1
	addi	s4, s4, 4
	j	.run_extra_summary.bb52
.run_extra_summary.bb21:
	li	t2, 0
	j	.run_extra_summary.bb27
.run_extra_summary.bb32:
	li	t2, 0
	j	.run_extra_summary.bb38
.run_extra_summary.bb43:
	li	t2, 0
	j	.run_extra_summary.bb49
.run_extra_summary.bb54:
	addiw	t1, t1, 1
	blt	t1, t0, .run_extra_summary.bb14	# rotated-loop-branch
.run_extra_summary.bb15:
	li	t0, 0
	li	t1, 0
.run_extra_summary.bb60:
	bge	t1, s1, .run_extra_summary.bb62
.run_extra_summary.bb75:
	addiw	t2, t1, 3
	blt	t2, s1, .run_extra_summary.bb76
.run_extra_summary.bb61:
	slliw	t2, t1, 2
	addi	t3, sp, 0
	add	t2, t3, t2
	addiw	t1, t1, 1
	lw	t2, 0(t2)
	addw	t0, t0, t2
	j	.run_extra_summary.bb60
.run_extra_summary.bb76:
	slliw	t2, t1, 2
	addi	t3, sp, 0
	add	s2, t3, t2
	addiw	t1, t1, 4
	lw	t2, 0(s2)
	lw	t3, 4(s2)
	addw	t0, t0, t2
	addw	t0, t0, t3
	lw	t3, 8(s2)
	lw	t2, 12(s2)
	addw	t0, t0, t3
	addw	t0, t0, t2
	j	.run_extra_summary.bb60
.run_extra_summary.bb62:
	mv	a0, t0
	ld	s10, 408(sp)
	ld	s9, 400(sp)
	ld	s8, 392(sp)
	ld	s7, 384(sp)
	ld	s6, 376(sp)
	ld	s5, 368(sp)
	ld	s4, 360(sp)
	ld	s3, 352(sp)
	ld	s2, 344(sp)
	ld	s1, 336(sp)
	addi	sp, sp, 416
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
