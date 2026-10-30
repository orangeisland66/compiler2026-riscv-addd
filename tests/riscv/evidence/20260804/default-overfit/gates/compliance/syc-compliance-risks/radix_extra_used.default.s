
  .section .rodata
  .globl DIGITS
  .align 2
  .type  DIGITS, @object
  .size  DIGITS, 4
DIGITS:
  .word 8

  .bss
  .globl values_extra_used
  .align 2
  .type  values_extra_used, @object
  .size  values_extra_used, 256
values_extra_used:
  .zero 256

  .text

  .globl sort_by_digits_extra_used
  .type  sort_by_digits_extra_used, @function
sort_by_digits_extra_used:
.sort_by_digits_extra_used.bb0:
	addi	sp, sp, -208
	sd	ra, 136(sp)
	sd	s1, 144(sp)
	sd	s2, 152(sp)
	sd	s3, 160(sp)
	sd	s4, 168(sp)
	sd	s5, 176(sp)
	sd	s6, 184(sp)
	sd	s7, 192(sp)
	sd	s8, 200(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	addi	t5, sp, 40
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	addi	t5, sp, 72
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	addi	t5, sp, 104
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	li	t0, -1
	subw	t0, s1, t0
	sltiu	t0, t0, 1
	addiw	s5, s3, 1
	slt	t1, s5, s4
	xori	t1, t1, 1
	or	t0, t0, t1
	bne	t0, zero, .sort_by_digits_extra_used.bb1
	j	.sort_by_digits_extra_used.bb3
.sort_by_digits_extra_used.bb2:
.sort_by_digits_extra_used.bb1:
	ld	s8, 200(sp)
	ld	s7, 192(sp)
	ld	s6, 184(sp)
	ld	s5, 176(sp)
	ld	s4, 168(sp)
	ld	s3, 160(sp)
	ld	s2, 152(sp)
	ld	s1, 144(sp)
	ld	ra, 136(sp)
	addi	sp, sp, 208
	ret
.sort_by_digits_extra_used.bb29:
	bge	t2, s1, .sort_by_digits_extra_used.bb31
.sort_by_digits_extra_used.bb32:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb33
.sort_by_digits_extra_used.bb30:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb29
.sort_by_digits_extra_used.bb33:
	sraiw	t3, s4, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb29
.sort_by_digits_extra_used.bb31:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	beq	s4, zero, .sort_by_digits_extra_used.bb28
.sort_by_digits_extra_used.bb27:
	slliw	t2, s4, 2
	addi	t3, sp, 40
	add	s4, t3, t2
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
	mv	t1, t3	# phi-copy
.sort_by_digits_extra_used.bb26:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra_used.bb29
.sort_by_digits_extra_used.bb40:
	bge	t2, s1, .sort_by_digits_extra_used.bb42
.sort_by_digits_extra_used.bb43:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb44
.sort_by_digits_extra_used.bb41:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb40
.sort_by_digits_extra_used.bb44:
	sraiw	t3, s4, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb40
.sort_by_digits_extra_used.bb42:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 1
	beq	s4, t2, .sort_by_digits_extra_used.bb39
.sort_by_digits_extra_used.bb38:
	slliw	t2, s4, 2
	addi	t3, sp, 40
	add	s4, t3, t2
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
	mv	t1, t3	# phi-copy
.sort_by_digits_extra_used.bb37:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra_used.bb40
.sort_by_digits_extra_used.bb51:
	bge	t2, s1, .sort_by_digits_extra_used.bb53
.sort_by_digits_extra_used.bb54:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb55
.sort_by_digits_extra_used.bb52:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb51
.sort_by_digits_extra_used.bb55:
	sraiw	t3, s4, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb51
.sort_by_digits_extra_used.bb53:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 2
	beq	s4, t2, .sort_by_digits_extra_used.bb50
.sort_by_digits_extra_used.bb49:
	slliw	t2, s4, 2
	addi	t3, sp, 40
	add	s4, t3, t2
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
	mv	t1, t3	# phi-copy
.sort_by_digits_extra_used.bb48:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra_used.bb51
.sort_by_digits_extra_used.bb62:
	bge	t2, s1, .sort_by_digits_extra_used.bb64
.sort_by_digits_extra_used.bb65:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb66
.sort_by_digits_extra_used.bb63:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb62
.sort_by_digits_extra_used.bb66:
	sraiw	t3, s4, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb62
.sort_by_digits_extra_used.bb64:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 3
	beq	s4, t2, .sort_by_digits_extra_used.bb61
.sort_by_digits_extra_used.bb60:
	slliw	t2, s4, 2
	addi	t3, sp, 40
	add	s4, t3, t2
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
	mv	t1, t3	# phi-copy
.sort_by_digits_extra_used.bb59:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra_used.bb62
.sort_by_digits_extra_used.bb73:
	bge	t2, s1, .sort_by_digits_extra_used.bb75
.sort_by_digits_extra_used.bb76:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb77
.sort_by_digits_extra_used.bb74:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb73
.sort_by_digits_extra_used.bb77:
	sraiw	t3, s4, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb73
.sort_by_digits_extra_used.bb75:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 4
	beq	s4, t2, .sort_by_digits_extra_used.bb72
.sort_by_digits_extra_used.bb71:
	slliw	t2, s4, 2
	addi	t3, sp, 40
	add	s4, t3, t2
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
	mv	t1, t3	# phi-copy
.sort_by_digits_extra_used.bb70:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra_used.bb73
.sort_by_digits_extra_used.bb84:
	bge	t2, s1, .sort_by_digits_extra_used.bb86
.sort_by_digits_extra_used.bb87:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb88
.sort_by_digits_extra_used.bb85:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb84
.sort_by_digits_extra_used.bb88:
	sraiw	t3, s4, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb84
.sort_by_digits_extra_used.bb86:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 5
	beq	s4, t2, .sort_by_digits_extra_used.bb83
.sort_by_digits_extra_used.bb82:
	slliw	t2, s4, 2
	addi	t3, sp, 40
	add	s4, t3, t2
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
	mv	t1, t3	# phi-copy
.sort_by_digits_extra_used.bb81:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra_used.bb84
.sort_by_digits_extra_used.bb95:
	bge	t2, s1, .sort_by_digits_extra_used.bb97
.sort_by_digits_extra_used.bb98:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb99
.sort_by_digits_extra_used.bb96:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb95
.sort_by_digits_extra_used.bb99:
	sraiw	t3, s4, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb95
.sort_by_digits_extra_used.bb97:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 6
	beq	s4, t2, .sort_by_digits_extra_used.bb94
.sort_by_digits_extra_used.bb93:
	slliw	t2, s4, 2
	addi	t3, sp, 40
	add	s4, t3, t2
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
	mv	t1, t3	# phi-copy
.sort_by_digits_extra_used.bb92:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra_used.bb95
.sort_by_digits_extra_used.bb106:
	bge	t2, s1, .sort_by_digits_extra_used.bb108
.sort_by_digits_extra_used.bb109:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb110
.sort_by_digits_extra_used.bb107:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb106
.sort_by_digits_extra_used.bb110:
	sraiw	t3, s4, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	sraiw	t3, s4, 31
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra_used.bb106
.sort_by_digits_extra_used.bb108:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 7
	beq	s4, t2, .sort_by_digits_extra_used.bb105
.sort_by_digits_extra_used.bb104:
	slliw	t2, s4, 2
	addi	t3, sp, 40
	add	s4, t3, t2
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
	mv	t1, t3	# phi-copy
.sort_by_digits_extra_used.bb103:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra_used.bb106
.sort_by_digits_extra_used.bb7:
	bge	t0, t2, .sort_by_digits_extra_used.bb13
.sort_by_digits_extra_used.bb19:
	addiw	t3, t0, 3
	blt	t3, t2, .sort_by_digits_extra_used.bb20
.sort_by_digits_extra_used.bb8:
	addiw	t3, t0, -1
	addi	a2, sp, 72
	slliw	t3, t3, 2
	slliw	s7, t0, 2
	add	s4, a2, t3
	addiw	t0, t0, 1
	lw	s6, 0(s4)
	addi	t3, sp, 40
	add	t3, t3, s7
	sw	s6, 0(t3)
	addi	t3, sp, 104
	add	t3, t3, s7
	lw	t3, 0(t3)
	addw	t3, s6, t3
	sw	t3, 4(s4)
	j	.sort_by_digits_extra_used.bb7
.sort_by_digits_extra_used.bb20:
	addiw	t3, t0, -1
	addi	a2, sp, 72
	slliw	t3, t3, 2
	slliw	s7, t0, 2
	add	s4, a2, t3
	addiw	t0, t0, 4
	lw	s6, 0(s4)
	addi	t3, sp, 40
	add	s8, t3, s7
	addi	t3, sp, 104
	sw	s6, 0(s8)
	add	s7, t3, s7
	lw	t3, 0(s7)
	addw	s6, s6, t3
	sw	s6, 4(s4)
	sw	s6, 4(s8)
	lw	a4, 4(s7)
	addw	s6, s6, a4
	sw	s6, 8(s4)
	sw	s6, 8(s8)
	lw	a4, 8(s7)
	addw	s6, s6, a4
	sw	s6, 12(s4)
	sw	s6, 12(s8)
	lw	a2, 12(s7)
	addw	a2, s6, a2
	sw	a2, 16(s4)
	j	.sort_by_digits_extra_used.bb7
.sort_by_digits_extra_used.bb10:
	bge	t2, s1, .sort_by_digits_extra_used.bb12
.sort_by_digits_extra_used.bb21:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra_used.bb22
.sort_by_digits_extra_used.bb11:
	sraiw	t3, t1, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t1, t1, t3
	sraiw	t1, t1, 3
	j	.sort_by_digits_extra_used.bb10
.sort_by_digits_extra_used.bb22:
	sraiw	t3, t1, 31
	addiw	t2, t2, 4
	andi	t3, t3, 7
	addw	t1, t1, t3
	sraiw	s7, t1, 3
	sraiw	t1, s7, 31
	andi	t1, t1, 7
	addw	t1, s7, t1
	sraiw	s7, t1, 3
	sraiw	t1, s7, 31
	andi	t1, t1, 7
	addw	t1, s7, t1
	sraiw	s7, t1, 3
	sraiw	t1, s7, 31
	andi	t1, t1, 7
	addw	t1, s7, t1
	sraiw	t1, t1, 3
	j	.sort_by_digits_extra_used.bb10
.sort_by_digits_extra_used.bb12:
	sraiw	t2, t1, 31
	addi	t0, t0, 4
	andi	s7, t2, 7
	addw	t1, t1, s7
	addi	t2, sp, 104
	andi	t1, t1, 7
	subw	t1, t1, s7
	slliw	t1, t1, 2
	add	s7, t2, t1
	lw	t1, 0(s7)
	addiw	t1, t1, 1
	sw	t1, 0(s7)
	addiw	s6, s6, 1
	bge	s6, s4, .sort_by_digits_extra_used.bb6	# rotated-loop-branch
.sort_by_digits_extra_used.bb5:
	lw	t1, 0(t0)
	li	t2, 0
	j	.sort_by_digits_extra_used.bb10
.sort_by_digits_extra_used.bb18:
.sort_by_digits_extra_used.bb13:
	addiw	s4, t0, 4
	bge	t0, t1, .sort_by_digits_extra_used.bb9
.sort_by_digits_extra_used.bb14:
	li	t2, 8
	bge	s4, t2, .sort_by_digits_extra_used.bb15
.sort_by_digits_extra_used.bb16:
	mv	t2, s4	# phi-copy
	j	.sort_by_digits_extra_used.bb7
.sort_by_digits_extra_used.bb17:
	j	.sort_by_digits_extra_used.bb7
.sort_by_digits_extra_used.bb28:
	addi	s4, sp, 40
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
.sort_by_digits_extra_used.bb23:
	addi	t1, sp, 40
	lw	s4, 0(t1)
	bge	s4, t0, .sort_by_digits_extra_used.bb25
.sort_by_digits_extra_used.bb24:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra_used.bb26
.sort_by_digits_extra_used.bb39:
	addi	s4, sp, 40
	lw	s6, 4(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 4(s4)
.sort_by_digits_extra_used.bb34:
	addi	t1, sp, 40
	lw	s4, 4(t1)
	bge	s4, t0, .sort_by_digits_extra_used.bb36
.sort_by_digits_extra_used.bb35:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra_used.bb37
.sort_by_digits_extra_used.bb50:
	addi	s4, sp, 40
	lw	s6, 8(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 8(s4)
.sort_by_digits_extra_used.bb45:
	addi	t1, sp, 40
	lw	s4, 8(t1)
	bge	s4, t0, .sort_by_digits_extra_used.bb47
.sort_by_digits_extra_used.bb46:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra_used.bb48
.sort_by_digits_extra_used.bb61:
	addi	s4, sp, 40
	lw	s6, 12(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 12(s4)
.sort_by_digits_extra_used.bb56:
	addi	t1, sp, 40
	lw	s4, 12(t1)
	bge	s4, t0, .sort_by_digits_extra_used.bb58
.sort_by_digits_extra_used.bb57:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra_used.bb59
.sort_by_digits_extra_used.bb72:
	addi	s4, sp, 40
	lw	s6, 16(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 16(s4)
.sort_by_digits_extra_used.bb67:
	addi	t1, sp, 40
	lw	s4, 16(t1)
	bge	s4, t0, .sort_by_digits_extra_used.bb69
.sort_by_digits_extra_used.bb68:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra_used.bb70
.sort_by_digits_extra_used.bb83:
	addi	s4, sp, 40
	lw	s6, 20(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 20(s4)
.sort_by_digits_extra_used.bb78:
	addi	t1, sp, 40
	lw	s4, 20(t1)
	bge	s4, t0, .sort_by_digits_extra_used.bb80
.sort_by_digits_extra_used.bb79:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra_used.bb81
.sort_by_digits_extra_used.bb94:
	addi	s4, sp, 40
	lw	s6, 24(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 24(s4)
.sort_by_digits_extra_used.bb89:
	addi	t1, sp, 40
	lw	s4, 24(t1)
	bge	s4, t0, .sort_by_digits_extra_used.bb91
.sort_by_digits_extra_used.bb90:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra_used.bb92
.sort_by_digits_extra_used.bb105:
	addi	s4, sp, 40
	lw	s6, 28(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 28(s4)
.sort_by_digits_extra_used.bb100:
	addi	t1, sp, 40
	lw	s4, 28(t1)
	bge	s4, t0, .sort_by_digits_extra_used.bb102
.sort_by_digits_extra_used.bb101:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra_used.bb103
.sort_by_digits_extra_used.bb15:
	li	t2, 8
	j	.sort_by_digits_extra_used.bb7
.sort_by_digits_extra_used.bb102:
	addi	t0, sp, 40
	addi	t1, sp, 72
	sw	s3, 0(t0)
	addiw	s1, s1, -1
	addi	t0, sp, 104
	lw	t0, 0(t0)
	addw	t0, s3, t0
	sw	t0, 0(t1)
	slt	t2, s5, t0
	li	t1, -1
	xori	t2, t2, 1
	subw	t1, s1, t1
	sltiu	t1, t1, 1
	or	t1, t1, t2
	beq	t1, zero, .sort_by_digits_extra_used.bb111
.sort_by_digits_extra_used.bb112:
	addi	s3, sp, 72
	lw	t0, 0(s3)
	addi	s4, sp, 40
	sw	t0, 4(s4)
	addi	s5, sp, 104
	lw	t1, 4(s5)
	addw	t1, t0, t1
	sw	t1, 4(s3)
	lw	a3, 4(s3)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, t0	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra_used
	lw	a2, 4(s3)
	sw	a2, 8(s4)
	lw	t1, 8(s5)
	addw	t1, a2, t1
	sw	t1, 8(s3)
	lw	a3, 8(s3)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra_used
	lw	t0, 8(s3)
	sw	t0, 12(s4)
	lw	t1, 12(s5)
	addw	t1, t0, t1
	sw	t1, 12(s3)
	lw	t1, 12(s3)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, t0	# call-arg-copy
	mv	a3, t1	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra_used
	lw	t0, 12(s3)
	sw	t0, 16(s4)
	lw	t1, 16(s5)
	addw	t1, t0, t1
	sw	t1, 16(s3)
	lw	t1, 16(s3)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, t0	# call-arg-copy
	mv	a3, t1	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra_used
	lw	t0, 16(s3)
	sw	t0, 20(s4)
	lw	t1, 20(s5)
	addw	t1, t0, t1
	sw	t1, 20(s3)
	lw	t1, 20(s3)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, t0	# call-arg-copy
	mv	a3, t1	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra_used
	lw	t0, 20(s3)
	sw	t0, 24(s4)
	lw	t1, 24(s5)
	addw	t1, t0, t1
	sw	t1, 24(s3)
	lw	t1, 24(s3)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, t0	# call-arg-copy
	mv	a3, t1	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra_used
	lw	t0, 24(s3)
	sw	t0, 28(s4)
	lw	t1, 28(s5)
	addw	t1, t0, t1
	sw	t1, 28(s3)
	lw	t1, 28(s3)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, t0	# call-arg-copy
	mv	a3, t1	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra_used
	j	.sort_by_digits_extra_used.bb1
.sort_by_digits_extra_used.bb6:
	addi	t0, sp, 40
	addi	t1, sp, 72
	sw	s3, 0(t0)
	addi	t0, sp, 104
	lw	t0, 0(t0)
	addw	t0, s3, t0
	sw	t0, 0(t1)
	li	t0, 1
	li	t1, 8
	j	.sort_by_digits_extra_used.bb13
.sort_by_digits_extra_used.bb9:
	addi	t0, sp, 72
	lw	t0, 0(t0)
	j	.sort_by_digits_extra_used.bb23
.sort_by_digits_extra_used.bb25:
	addi	t0, sp, 72
	lw	t0, 4(t0)
	j	.sort_by_digits_extra_used.bb34
.sort_by_digits_extra_used.bb36:
	addi	t0, sp, 72
	lw	t0, 8(t0)
	j	.sort_by_digits_extra_used.bb45
.sort_by_digits_extra_used.bb47:
	addi	t0, sp, 72
	lw	t0, 12(t0)
	j	.sort_by_digits_extra_used.bb56
.sort_by_digits_extra_used.bb58:
	addi	t0, sp, 72
	lw	t0, 16(t0)
	j	.sort_by_digits_extra_used.bb67
.sort_by_digits_extra_used.bb69:
	addi	t0, sp, 72
	lw	t0, 20(t0)
	j	.sort_by_digits_extra_used.bb78
.sort_by_digits_extra_used.bb80:
	addi	t0, sp, 72
	lw	t0, 24(t0)
	j	.sort_by_digits_extra_used.bb89
.sort_by_digits_extra_used.bb91:
	addi	t0, sp, 72
	lw	t0, 28(t0)
	j	.sort_by_digits_extra_used.bb100
.sort_by_digits_extra_used.bb111:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	mv	a3, t0	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra_used
	j	.sort_by_digits_extra_used.bb112
.sort_by_digits_extra_used.bb3:
	slliw	t0, s3, 2
	add	t0, s2, t0
	mv	s6, s3	# phi-copy
.sort_by_digits_extra_used.bb4:
	bge	s6, s4, .sort_by_digits_extra_used.bb6
	j	.sort_by_digits_extra_used.bb5
	.size sort_by_digits_extra_used, .-sort_by_digits_extra_used

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	sd	s2, 56(sp)
	sd	s3, 64(sp)
	sd	s4, 72(sp)
	li	t0, 0
.main.bb1:
	li	t1, 64
	bge	t0, t1, .main.bb3
.main.bb4:
	li	t1, 197
	addiw	s1, t0, 3
	mulw	s2, t0, t1
	li	t1, 64
	blt	s1, t1, .main.bb5
.main.bb2:
	addiw	s1, s2, 53
	li	t2, 4095
	sraiw	t1, s1, 31
	la	t3, values_extra_used
	and	s2, t1, t2
	addw	t1, s1, s2
	and	t1, t1, t2
	slliw	t2, t0, 2
	subw	t1, t1, s2
	add	t2, t3, t2
	addiw	t0, t0, 1
	sw	t1, 0(t2)
	j	.main.bb1
.main.bb5:
	addiw	s2, s2, 53
	li	t2, 4095
	sraiw	t1, s2, 31
	la	t3, values_extra_used
	and	s3, t1, t2
	addw	t1, s2, s3
	and	t1, t1, t2
	slliw	t2, t0, 2
	subw	t1, t1, s3
	add	s2, t3, t2
	sw	t1, 0(s2)
	li	t2, 197
	addiw	t1, t0, 1
	li	t3, 197
	mulw	t1, t1, t2
	addiw	s3, t1, 53
	li	t2, 4095
	sraiw	t1, s3, 31
	and	s4, t1, t2
	addw	t1, s3, s4
	and	t1, t1, t2
	subw	t1, t1, s4
	sw	t1, 4(s2)
	addiw	t1, t0, 2
	mulw	t1, t1, t3
	addiw	t0, t0, 4
	addiw	s3, t1, 53
	li	t3, 4095
	sraiw	t1, s3, 31
	and	s4, t1, t3
	addw	t1, s3, s4
	and	t1, t1, t3
	subw	t1, t1, s4
	sw	t1, 8(s2)
	li	t1, 197
	mulw	t1, s1, t1
	addiw	s1, t1, 53
	sraiw	t1, s1, 31
	and	s3, t1, t3
	addw	t1, s1, s3
	and	t1, t1, t3
	subw	t1, t1, s3
	sw	t1, 12(s2)
	j	.main.bb1
.main.bb3:
	li	t5, 3
	sd	t5, 0(sp)
	la	t5, values_extra_used
	sd	t5, 8(sp)
	li	t5, 0
	sd	t5, 16(sp)
	li	t5, 64
	sd	t5, 24(sp)
	li	t5, 0
	sd	t5, 32(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	call	sort_by_digits_extra_used
	la	t0, values_extra_used
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s4, 72(sp)
	ld	s3, 64(sp)
	ld	s2, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
