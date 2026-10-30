
  .section .rodata
  .globl DIGITS
  .align 2
  .type  DIGITS, @object
  .size  DIGITS, 4
DIGITS:
  .word 16

  .bss
  .globl values_base16
  .align 2
  .type  values_base16, @object
  .size  values_base16, 128
values_base16:
  .zero 128

  .text

  .globl sort_by_digits_base16
  .type  sort_by_digits_base16, @function
sort_by_digits_base16:
.sort_by_digits_base16.bb0:
	addi	sp, sp, -304
	sd	ra, 232(sp)
	sd	s1, 240(sp)
	sd	s2, 248(sp)
	sd	s3, 256(sp)
	sd	s4, 264(sp)
	sd	s5, 272(sp)
	sd	s6, 280(sp)
	sd	s7, 288(sp)
	sd	s8, 296(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	addi	t5, sp, 40
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	addi	t5, sp, 104
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	addi	t5, sp, 168
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	li	t0, -1
	subw	t0, s1, t0
	sltiu	t0, t0, 1
	addiw	t1, s3, 1
	slt	t1, t1, s4
	xori	t1, t1, 1
	or	t0, t0, t1
	bne	t0, zero, .sort_by_digits_base16.bb1
	j	.sort_by_digits_base16.bb3
.sort_by_digits_base16.bb2:
.sort_by_digits_base16.bb1:
	ld	s8, 296(sp)
	ld	s7, 288(sp)
	ld	s6, 280(sp)
	ld	s5, 272(sp)
	ld	s4, 264(sp)
	ld	s3, 256(sp)
	ld	s2, 248(sp)
	ld	s1, 240(sp)
	ld	ra, 232(sp)
	addi	sp, sp, 304
	ret
.sort_by_digits_base16.bb27:
	bge	a3, s1, .sort_by_digits_base16.bb29
.sort_by_digits_base16.bb34:
	addiw	a4, a3, 3
	blt	a4, s1, .sort_by_digits_base16.bb35
.sort_by_digits_base16.bb28:
	sraiw	a4, s4, 31
	addiw	a3, a3, 1
	andi	a4, a4, 15
	addw	a4, s4, a4
	sraiw	s4, a4, 4
	j	.sort_by_digits_base16.bb27
.sort_by_digits_base16.bb35:
	sraiw	a4, s4, 31
	addiw	a3, a3, 4
	andi	a4, a4, 15
	addw	a4, s4, a4
	sraiw	s4, a4, 4
	sraiw	a4, s4, 31
	andi	a4, a4, 15
	addw	a4, s4, a4
	sraiw	s4, a4, 4
	sraiw	a4, s4, 31
	andi	a4, a4, 15
	addw	a4, s4, a4
	sraiw	s4, a4, 4
	sraiw	a4, s4, 31
	andi	a4, a4, 15
	addw	a4, s4, a4
	sraiw	s4, a4, 4
	j	.sort_by_digits_base16.bb27
.sort_by_digits_base16.bb29:
	sraiw	a3, s4, 31
	andi	s5, a3, 15
	addw	a3, s4, s5
	andi	a3, a3, 15
	subw	s4, a3, s5
	beq	s4, t1, .sort_by_digits_base16.bb18
.sort_by_digits_base16.bb17:
	slliw	a3, s4, 2
	addi	a4, sp, 40
	add	s4, a4, a3
	lw	s5, 0(s4)
	slliw	a3, s5, 2
	add	a3, s2, a3
	lw	a4, 0(a3)
	sw	a2, 0(a3)
	addiw	a2, s5, 1
	sw	a2, 0(s4)
	mv	a2, a4	# phi-copy
.sort_by_digits_base16.bb16:
	li	a3, 0
	mv	s4, a2	# phi-copy
	j	.sort_by_digits_base16.bb27
.sort_by_digits_base16.bb18:
	lw	s4, 0(t0)
	slliw	a3, s4, 2
	add	a3, s2, a3
	sw	a2, 0(a3)
	addiw	a2, s4, 1
	sw	a2, 0(t0)
.sort_by_digits_base16.bb13:
	lw	s4, 0(t0)
	bge	s4, t3, .sort_by_digits_base16.bb15
.sort_by_digits_base16.bb14:
	slliw	a2, s4, 2
	add	a2, s2, a2
	lw	a2, 0(a2)
	j	.sort_by_digits_base16.bb16
.sort_by_digits_base16.bb24:
	bge	t2, s1, .sort_by_digits_base16.bb26
.sort_by_digits_base16.bb32:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_base16.bb33
.sort_by_digits_base16.bb25:
	sraiw	t3, t1, 31
	addiw	t2, t2, 1
	andi	t3, t3, 15
	addw	t1, t1, t3
	sraiw	t1, t1, 4
	j	.sort_by_digits_base16.bb24
.sort_by_digits_base16.bb33:
	sraiw	t3, t1, 31
	addiw	t2, t2, 4
	andi	t3, t3, 15
	addw	t1, t1, t3
	sraiw	s6, t1, 4
	sraiw	t1, s6, 31
	andi	t1, t1, 15
	addw	t1, s6, t1
	sraiw	s6, t1, 4
	sraiw	t1, s6, 31
	andi	t1, t1, 15
	addw	t1, s6, t1
	sraiw	s6, t1, 4
	sraiw	t1, s6, 31
	andi	t1, t1, 15
	addw	t1, s6, t1
	sraiw	t1, t1, 4
	j	.sort_by_digits_base16.bb24
.sort_by_digits_base16.bb26:
	sraiw	t2, t1, 31
	addi	t0, t0, 4
	andi	s6, t2, 15
	addw	t1, t1, s6
	addi	t2, sp, 168
	andi	t1, t1, 15
	subw	t1, t1, s6
	slliw	t1, t1, 2
	add	s6, t2, t1
	lw	t1, 0(s6)
	addiw	t1, t1, 1
	sw	t1, 0(s6)
	addiw	s5, s5, 1
	bge	s5, s4, .sort_by_digits_base16.bb6	# rotated-loop-branch
.sort_by_digits_base16.bb5:
	lw	t1, 0(t0)
	li	t2, 0
	j	.sort_by_digits_base16.bb24
.sort_by_digits_base16.bb15:
	addiw	t1, t1, 1
	addi	t0, t0, 4
	addi	t2, t2, 4
.sort_by_digits_base16.bb10:
	li	t3, 16
	bge	t1, t3, .sort_by_digits_base16.bb12
.sort_by_digits_base16.bb11:
	lw	t3, 0(t2)
	j	.sort_by_digits_base16.bb13
.sort_by_digits_base16.bb7:
	li	t3, 16
	bge	t1, t3, .sort_by_digits_base16.bb9
.sort_by_digits_base16.bb30:
	addiw	t3, t1, 3
	li	a2, 16
	blt	t3, a2, .sort_by_digits_base16.bb31
.sort_by_digits_base16.bb8:
	addiw	t3, t1, -1
	addi	a2, sp, 104
	slliw	t3, t3, 2
	slliw	s6, t1, 2
	add	s4, a2, t3
	addiw	t1, t1, 1
	lw	s5, 0(s4)
	addi	t3, sp, 40
	add	t3, t3, s6
	sw	s5, 0(t3)
	addi	t3, sp, 168
	add	t3, t3, s6
	lw	t3, 0(t3)
	addw	t3, s5, t3
	sw	t3, 4(s4)
	j	.sort_by_digits_base16.bb7
.sort_by_digits_base16.bb31:
	addiw	t3, t1, -1
	addi	a2, sp, 104
	slliw	t3, t3, 2
	slliw	s6, t1, 2
	add	s4, a2, t3
	addiw	t1, t1, 4
	lw	s5, 0(s4)
	addi	t3, sp, 40
	add	s7, t3, s6
	addi	t3, sp, 168
	sw	s5, 0(s7)
	add	s6, t3, s6
	lw	t3, 0(s6)
	addw	s5, s5, t3
	sw	s5, 4(s4)
	sw	s5, 4(s7)
	lw	a4, 4(s6)
	addw	s5, s5, a4
	sw	s5, 8(s4)
	sw	s5, 8(s7)
	lw	a4, 8(s6)
	addw	s5, s5, a4
	sw	s5, 12(s4)
	sw	s5, 12(s7)
	lw	a2, 12(s6)
	addw	a2, s5, a2
	sw	a2, 16(s4)
	j	.sort_by_digits_base16.bb7
.sort_by_digits_base16.bb9:
	li	t1, 0
	j	.sort_by_digits_base16.bb10
.sort_by_digits_base16.bb6:
	addi	t1, sp, 40
	addi	t3, sp, 104
	sw	s3, 0(t1)
	addi	t0, sp, 40
	addi	t2, sp, 104
	addi	t1, sp, 168
	lw	t1, 0(t1)
	addw	t1, s3, t1
	sw	t1, 0(t3)
	li	t1, 1
	j	.sort_by_digits_base16.bb7
.sort_by_digits_base16.bb20:
	blt	zero, s5, .sort_by_digits_base16.bb22
.sort_by_digits_base16.bb23:
	slliw	s6, s5, 2
	addi	t0, sp, 40
	add	t0, t0, s6
	lw	s7, 0(t0)
	addi	t0, sp, 104
	add	t0, t0, s6
	lw	s6, 0(t0)
	addiw	t0, s7, 1
	slt	t0, t0, s6
	xori	t0, t0, 1
	or	t0, s3, t0
	beq	t0, zero, .sort_by_digits_base16.bb36
.sort_by_digits_base16.bb37:
	addiw	s5, s5, 1
	blt	s5, s4, .sort_by_digits_base16.bb20	# rotated-loop-branch
	j	.sort_by_digits_base16.bb1	# rotated-loop-exit
.sort_by_digits_base16.bb21:
	j	.sort_by_digits_base16.bb1
.sort_by_digits_base16.bb22:
	addiw	t0, s5, -1
	addi	t1, sp, 104
	slliw	t0, t0, 2
	slliw	s8, s5, 2
	add	s6, t1, t0
	lw	s7, 0(s6)
	addi	t0, sp, 40
	add	t0, t0, s8
	sw	s7, 0(t0)
	addi	t0, sp, 168
	add	t0, t0, s8
	lw	t0, 0(t0)
	addw	t0, s7, t0
	sw	t0, 4(s6)
	j	.sort_by_digits_base16.bb23
.sort_by_digits_base16.bb36:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s7	# call-arg-copy
	mv	a3, s6	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_base16
	j	.sort_by_digits_base16.bb37
.sort_by_digits_base16.bb12:
	addi	t0, sp, 40
	addi	t1, sp, 104
	sw	s3, 0(t0)
	addiw	s1, s1, -1
	addi	t0, sp, 168
	li	s4, 16
	lw	t0, 0(t0)
	addw	t0, s3, t0
	sw	t0, 0(t1)
	li	t0, -1
	subw	t0, s1, t0
	sltiu	s3, t0, 1
	li	s5, 0
.sort_by_digits_base16.bb19:
	bge	s5, s4, .sort_by_digits_base16.bb1
	j	.sort_by_digits_base16.bb20
.sort_by_digits_base16.bb3:
	slliw	t0, s3, 2
	add	t0, s2, t0
	mv	s5, s3	# phi-copy
.sort_by_digits_base16.bb4:
	bge	s5, s4, .sort_by_digits_base16.bb6
	j	.sort_by_digits_base16.bb5
	.size sort_by_digits_base16, .-sort_by_digits_base16

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
	li	t1, 32
	bge	t0, t1, .main.bb3
.main.bb4:
	li	t1, 197
	addiw	s1, t0, 3
	mulw	s2, t0, t1
	li	t1, 32
	blt	s1, t1, .main.bb5
.main.bb2:
	addiw	s1, s2, 53
	li	t2, 4095
	sraiw	t1, s1, 31
	la	t3, values_base16
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
	la	t3, values_base16
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
	la	t5, values_base16
	sd	t5, 8(sp)
	li	t5, 0
	sd	t5, 16(sp)
	li	t5, 32
	sd	t5, 24(sp)
	li	t5, 23
	sd	t5, 32(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	call	sort_by_digits_base16
	la	t0, values_base16
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
