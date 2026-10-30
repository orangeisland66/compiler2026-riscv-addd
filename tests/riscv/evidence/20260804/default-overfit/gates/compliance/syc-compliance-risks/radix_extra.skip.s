
  .section .rodata
  .globl DIGITS
  .align 2
  .type  DIGITS, @object
  .size  DIGITS, 4
DIGITS:
  .word 8

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
	bne	t0, zero, .sort_by_digits_extra.bb1
	j	.sort_by_digits_extra.bb3
.sort_by_digits_extra.bb2:
.sort_by_digits_extra.bb1:
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
.sort_by_digits_extra.bb29:
	bge	t2, s1, .sort_by_digits_extra.bb31
.sort_by_digits_extra.bb32:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb33
.sort_by_digits_extra.bb30:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra.bb29
.sort_by_digits_extra.bb33:
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
	j	.sort_by_digits_extra.bb29
.sort_by_digits_extra.bb31:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	beq	s4, zero, .sort_by_digits_extra.bb28
.sort_by_digits_extra.bb27:
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
.sort_by_digits_extra.bb26:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra.bb29
.sort_by_digits_extra.bb40:
	bge	t2, s1, .sort_by_digits_extra.bb42
.sort_by_digits_extra.bb43:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb44
.sort_by_digits_extra.bb41:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra.bb40
.sort_by_digits_extra.bb44:
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
	j	.sort_by_digits_extra.bb40
.sort_by_digits_extra.bb42:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 1
	beq	s4, t2, .sort_by_digits_extra.bb39
.sort_by_digits_extra.bb38:
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
.sort_by_digits_extra.bb37:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra.bb40
.sort_by_digits_extra.bb51:
	bge	t2, s1, .sort_by_digits_extra.bb53
.sort_by_digits_extra.bb54:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb55
.sort_by_digits_extra.bb52:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra.bb51
.sort_by_digits_extra.bb55:
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
	j	.sort_by_digits_extra.bb51
.sort_by_digits_extra.bb53:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 2
	beq	s4, t2, .sort_by_digits_extra.bb50
.sort_by_digits_extra.bb49:
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
.sort_by_digits_extra.bb48:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra.bb51
.sort_by_digits_extra.bb62:
	bge	t2, s1, .sort_by_digits_extra.bb64
.sort_by_digits_extra.bb65:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb66
.sort_by_digits_extra.bb63:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra.bb62
.sort_by_digits_extra.bb66:
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
	j	.sort_by_digits_extra.bb62
.sort_by_digits_extra.bb64:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 3
	beq	s4, t2, .sort_by_digits_extra.bb61
.sort_by_digits_extra.bb60:
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
.sort_by_digits_extra.bb59:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra.bb62
.sort_by_digits_extra.bb73:
	bge	t2, s1, .sort_by_digits_extra.bb75
.sort_by_digits_extra.bb76:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb77
.sort_by_digits_extra.bb74:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra.bb73
.sort_by_digits_extra.bb77:
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
	j	.sort_by_digits_extra.bb73
.sort_by_digits_extra.bb75:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 4
	beq	s4, t2, .sort_by_digits_extra.bb72
.sort_by_digits_extra.bb71:
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
.sort_by_digits_extra.bb70:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra.bb73
.sort_by_digits_extra.bb84:
	bge	t2, s1, .sort_by_digits_extra.bb86
.sort_by_digits_extra.bb87:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb88
.sort_by_digits_extra.bb85:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra.bb84
.sort_by_digits_extra.bb88:
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
	j	.sort_by_digits_extra.bb84
.sort_by_digits_extra.bb86:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 5
	beq	s4, t2, .sort_by_digits_extra.bb83
.sort_by_digits_extra.bb82:
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
.sort_by_digits_extra.bb81:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra.bb84
.sort_by_digits_extra.bb95:
	bge	t2, s1, .sort_by_digits_extra.bb97
.sort_by_digits_extra.bb98:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb99
.sort_by_digits_extra.bb96:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra.bb95
.sort_by_digits_extra.bb99:
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
	j	.sort_by_digits_extra.bb95
.sort_by_digits_extra.bb97:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 6
	beq	s4, t2, .sort_by_digits_extra.bb94
.sort_by_digits_extra.bb93:
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
.sort_by_digits_extra.bb92:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra.bb95
.sort_by_digits_extra.bb106:
	bge	t2, s1, .sort_by_digits_extra.bb108
.sort_by_digits_extra.bb109:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb110
.sort_by_digits_extra.bb107:
	sraiw	t3, s4, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t3, s4, t3
	sraiw	s4, t3, 3
	j	.sort_by_digits_extra.bb106
.sort_by_digits_extra.bb110:
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
	j	.sort_by_digits_extra.bb106
.sort_by_digits_extra.bb108:
	sraiw	t2, s4, 31
	andi	s6, t2, 7
	addw	t2, s4, s6
	andi	t2, t2, 7
	subw	s4, t2, s6
	li	t2, 7
	beq	s4, t2, .sort_by_digits_extra.bb105
.sort_by_digits_extra.bb104:
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
.sort_by_digits_extra.bb103:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_extra.bb106
.sort_by_digits_extra.bb7:
	bge	t0, t2, .sort_by_digits_extra.bb13
.sort_by_digits_extra.bb19:
	addiw	t3, t0, 3
	blt	t3, t2, .sort_by_digits_extra.bb20
.sort_by_digits_extra.bb8:
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
	j	.sort_by_digits_extra.bb7
.sort_by_digits_extra.bb20:
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
	j	.sort_by_digits_extra.bb7
.sort_by_digits_extra.bb10:
	bge	t2, s1, .sort_by_digits_extra.bb12
.sort_by_digits_extra.bb21:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_extra.bb22
.sort_by_digits_extra.bb11:
	sraiw	t3, t1, 31
	addiw	t2, t2, 1
	andi	t3, t3, 7
	addw	t1, t1, t3
	sraiw	t1, t1, 3
	j	.sort_by_digits_extra.bb10
.sort_by_digits_extra.bb22:
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
	j	.sort_by_digits_extra.bb10
.sort_by_digits_extra.bb12:
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
	bge	s6, s4, .sort_by_digits_extra.bb6	# rotated-loop-branch
.sort_by_digits_extra.bb5:
	lw	t1, 0(t0)
	li	t2, 0
	j	.sort_by_digits_extra.bb10
.sort_by_digits_extra.bb18:
.sort_by_digits_extra.bb13:
	addiw	s4, t0, 4
	bge	t0, t1, .sort_by_digits_extra.bb9
.sort_by_digits_extra.bb14:
	li	t2, 8
	bge	s4, t2, .sort_by_digits_extra.bb15
.sort_by_digits_extra.bb16:
	mv	t2, s4	# phi-copy
	j	.sort_by_digits_extra.bb7
.sort_by_digits_extra.bb17:
	j	.sort_by_digits_extra.bb7
.sort_by_digits_extra.bb28:
	addi	s4, sp, 40
	lw	s6, 0(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 0(s4)
.sort_by_digits_extra.bb23:
	addi	t1, sp, 40
	lw	s4, 0(t1)
	bge	s4, t0, .sort_by_digits_extra.bb25
.sort_by_digits_extra.bb24:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra.bb26
.sort_by_digits_extra.bb39:
	addi	s4, sp, 40
	lw	s6, 4(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 4(s4)
.sort_by_digits_extra.bb34:
	addi	t1, sp, 40
	lw	s4, 4(t1)
	bge	s4, t0, .sort_by_digits_extra.bb36
.sort_by_digits_extra.bb35:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra.bb37
.sort_by_digits_extra.bb50:
	addi	s4, sp, 40
	lw	s6, 8(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 8(s4)
.sort_by_digits_extra.bb45:
	addi	t1, sp, 40
	lw	s4, 8(t1)
	bge	s4, t0, .sort_by_digits_extra.bb47
.sort_by_digits_extra.bb46:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra.bb48
.sort_by_digits_extra.bb61:
	addi	s4, sp, 40
	lw	s6, 12(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 12(s4)
.sort_by_digits_extra.bb56:
	addi	t1, sp, 40
	lw	s4, 12(t1)
	bge	s4, t0, .sort_by_digits_extra.bb58
.sort_by_digits_extra.bb57:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra.bb59
.sort_by_digits_extra.bb72:
	addi	s4, sp, 40
	lw	s6, 16(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 16(s4)
.sort_by_digits_extra.bb67:
	addi	t1, sp, 40
	lw	s4, 16(t1)
	bge	s4, t0, .sort_by_digits_extra.bb69
.sort_by_digits_extra.bb68:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra.bb70
.sort_by_digits_extra.bb83:
	addi	s4, sp, 40
	lw	s6, 20(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 20(s4)
.sort_by_digits_extra.bb78:
	addi	t1, sp, 40
	lw	s4, 20(t1)
	bge	s4, t0, .sort_by_digits_extra.bb80
.sort_by_digits_extra.bb79:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra.bb81
.sort_by_digits_extra.bb94:
	addi	s4, sp, 40
	lw	s6, 24(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 24(s4)
.sort_by_digits_extra.bb89:
	addi	t1, sp, 40
	lw	s4, 24(t1)
	bge	s4, t0, .sort_by_digits_extra.bb91
.sort_by_digits_extra.bb90:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra.bb92
.sort_by_digits_extra.bb105:
	addi	s4, sp, 40
	lw	s6, 28(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 28(s4)
.sort_by_digits_extra.bb100:
	addi	t1, sp, 40
	lw	s4, 28(t1)
	bge	s4, t0, .sort_by_digits_extra.bb102
.sort_by_digits_extra.bb101:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_extra.bb103
.sort_by_digits_extra.bb15:
	li	t2, 8
	j	.sort_by_digits_extra.bb7
.sort_by_digits_extra.bb102:
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
	beq	t1, zero, .sort_by_digits_extra.bb111
.sort_by_digits_extra.bb112:
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
	call	sort_by_digits_extra
	lw	a2, 4(s3)
	sw	a2, 8(s4)
	lw	t1, 8(s5)
	addw	t1, a2, t1
	sw	t1, 8(s3)
	lw	a3, 8(s3)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra
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
	call	sort_by_digits_extra
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
	call	sort_by_digits_extra
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
	call	sort_by_digits_extra
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
	call	sort_by_digits_extra
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
	call	sort_by_digits_extra
	j	.sort_by_digits_extra.bb1
.sort_by_digits_extra.bb6:
	addi	t0, sp, 40
	addi	t1, sp, 72
	sw	s3, 0(t0)
	addi	t0, sp, 104
	lw	t0, 0(t0)
	addw	t0, s3, t0
	sw	t0, 0(t1)
	li	t0, 1
	li	t1, 8
	j	.sort_by_digits_extra.bb13
.sort_by_digits_extra.bb9:
	addi	t0, sp, 72
	lw	t0, 0(t0)
	j	.sort_by_digits_extra.bb23
.sort_by_digits_extra.bb25:
	addi	t0, sp, 72
	lw	t0, 4(t0)
	j	.sort_by_digits_extra.bb34
.sort_by_digits_extra.bb36:
	addi	t0, sp, 72
	lw	t0, 8(t0)
	j	.sort_by_digits_extra.bb45
.sort_by_digits_extra.bb47:
	addi	t0, sp, 72
	lw	t0, 12(t0)
	j	.sort_by_digits_extra.bb56
.sort_by_digits_extra.bb58:
	addi	t0, sp, 72
	lw	t0, 16(t0)
	j	.sort_by_digits_extra.bb67
.sort_by_digits_extra.bb69:
	addi	t0, sp, 72
	lw	t0, 20(t0)
	j	.sort_by_digits_extra.bb78
.sort_by_digits_extra.bb80:
	addi	t0, sp, 72
	lw	t0, 24(t0)
	j	.sort_by_digits_extra.bb89
.sort_by_digits_extra.bb91:
	addi	t0, sp, 72
	lw	t0, 28(t0)
	j	.sort_by_digits_extra.bb100
.sort_by_digits_extra.bb111:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	mv	a3, t0	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_extra
	j	.sort_by_digits_extra.bb112
.sort_by_digits_extra.bb3:
	slliw	t0, s3, 2
	add	t0, s2, t0
	mv	s6, s3	# phi-copy
.sort_by_digits_extra.bb4:
	bge	s6, s4, .sort_by_digits_extra.bb6
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
