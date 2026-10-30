
  .section .rodata
  .globl DIGITS
  .align 2
  .type  DIGITS, @object
  .size  DIGITS, 4
DIGITS:
  .word 2

  .bss
  .globl values_base2
  .align 2
  .type  values_base2, @object
  .size  values_base2, 64
values_base2:
  .zero 64

  .text

  .globl sort_by_digits_base2
  .type  sort_by_digits_base2, @function
sort_by_digits_base2:
.sort_by_digits_base2.bb0:
	addi	sp, sp, -128
	sd	ra, 56(sp)
	sd	s1, 64(sp)
	sd	s2, 72(sp)
	sd	s3, 80(sp)
	sd	s4, 88(sp)
	sd	s5, 96(sp)
	sd	s6, 104(sp)
	sd	s7, 112(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	addi	t5, sp, 40
	sd	zero, 0(t5)	# memset-zero
	addi	t5, sp, 48
	sd	zero, 0(t5)	# memset-zero
	li	t0, -1
	subw	t0, s1, t0
	sltiu	t0, t0, 1
	addiw	s5, s3, 1
	slt	t1, s5, s4
	xori	t1, t1, 1
	or	t0, t0, t1
	bne	t0, zero, .sort_by_digits_base2.bb1
	j	.sort_by_digits_base2.bb3
.sort_by_digits_base2.bb2:
.sort_by_digits_base2.bb1:
	ld	s7, 112(sp)
	ld	s6, 104(sp)
	ld	s5, 96(sp)
	ld	s4, 88(sp)
	ld	s3, 80(sp)
	ld	s2, 72(sp)
	ld	s1, 64(sp)
	ld	ra, 56(sp)
	addi	sp, sp, 128
	ret
.sort_by_digits_base2.bb18:
	bge	t2, s1, .sort_by_digits_base2.bb20
.sort_by_digits_base2.bb21:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_base2.bb22
.sort_by_digits_base2.bb19:
	addiw	t2, t2, 1
	srliw	t3, s6, 31	# const-div.pow2.bias-fold
	addw	t3, s6, t3
	sraiw	s6, t3, 1
	j	.sort_by_digits_base2.bb18
.sort_by_digits_base2.bb22:
	addiw	t2, t2, 4
	srliw	t3, s6, 31	# const-div.pow2.bias-fold
	addw	t3, s6, t3
	sraiw	s6, t3, 1
	srliw	t3, s6, 31	# const-div.pow2.bias-fold
	addw	t3, s6, t3
	sraiw	s6, t3, 1
	srliw	t3, s6, 31	# const-div.pow2.bias-fold
	addw	t3, s6, t3
	sraiw	s6, t3, 1
	srliw	t3, s6, 31	# const-div.pow2.bias-fold
	addw	t3, s6, t3
	sraiw	s6, t3, 1
	j	.sort_by_digits_base2.bb18
.sort_by_digits_base2.bb20:
	andi	t3, s6, 1
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	t2, s6, s7
	andi	t2, t2, 1
	subw	t2, t2, s7
	beq	t3, zero, .sort_by_digits_base2.bb17
.sort_by_digits_base2.bb16:
	slliw	t2, t2, 2
	addi	t3, sp, 40
	add	s6, t3, t2
	lw	s7, 0(s6)
	slliw	t2, s7, 2
	add	t2, s2, t2
	lw	t3, 0(t2)
	sw	t1, 0(t2)
	addiw	t1, s7, 1
	sw	t1, 0(s6)
	mv	t1, t3	# phi-copy
.sort_by_digits_base2.bb15:
	li	t2, 0
	mv	s6, t1	# phi-copy
	j	.sort_by_digits_base2.bb18
.sort_by_digits_base2.bb29:
	bge	t2, s1, .sort_by_digits_base2.bb31
.sort_by_digits_base2.bb32:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_base2.bb33
.sort_by_digits_base2.bb30:
	addiw	t2, t2, 1
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	s4, t3, 1
	j	.sort_by_digits_base2.bb29
.sort_by_digits_base2.bb33:
	addiw	t2, t2, 4
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	s4, t3, 1
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	s4, t3, 1
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	s4, t3, 1
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	s4, t3, 1
	j	.sort_by_digits_base2.bb29
.sort_by_digits_base2.bb31:
	andi	t3, s4, 1
	srliw	s6, s4, 31	# const-div.pow2.bias-fold
	addw	t2, s4, s6
	andi	t2, t2, 1
	subw	t2, t2, s6
	bne	t3, zero, .sort_by_digits_base2.bb28
.sort_by_digits_base2.bb27:
	slliw	t2, t2, 2
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
.sort_by_digits_base2.bb26:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.sort_by_digits_base2.bb29
.sort_by_digits_base2.bb7:
	bge	t2, s1, .sort_by_digits_base2.bb9
.sort_by_digits_base2.bb10:
	addiw	t3, t2, 3
	blt	t3, s1, .sort_by_digits_base2.bb11
.sort_by_digits_base2.bb8:
	addiw	t2, t2, 1
	srliw	t3, t1, 31	# const-div.pow2.bias-fold
	addw	t1, t1, t3
	sraiw	t1, t1, 1
	j	.sort_by_digits_base2.bb7
.sort_by_digits_base2.bb11:
	addiw	t2, t2, 4
	srliw	t3, t1, 31	# const-div.pow2.bias-fold
	addw	t1, t1, t3
	sraiw	s7, t1, 1
	srliw	t1, s7, 31	# const-div.pow2.bias-fold
	addw	t1, s7, t1
	sraiw	s7, t1, 1
	srliw	t1, s7, 31	# const-div.pow2.bias-fold
	addw	t1, s7, t1
	sraiw	s7, t1, 1
	srliw	t1, s7, 31	# const-div.pow2.bias-fold
	addw	t1, s7, t1
	sraiw	t1, t1, 1
	j	.sort_by_digits_base2.bb7
.sort_by_digits_base2.bb9:
	addi	t0, t0, 4
	srliw	s7, t1, 31	# const-div.pow2.bias-fold
	addw	t1, t1, s7
	addi	t2, sp, 48
	andi	t1, t1, 1
	subw	t1, t1, s7
	slliw	t1, t1, 2
	add	s7, t2, t1
	lw	t1, 0(s7)
	addiw	t1, t1, 1
	sw	t1, 0(s7)
	addiw	s6, s6, 1
	bge	s6, s4, .sort_by_digits_base2.bb6	# rotated-loop-branch
.sort_by_digits_base2.bb5:
	lw	t1, 0(t0)
	li	t2, 0
	j	.sort_by_digits_base2.bb7
.sort_by_digits_base2.bb17:
	addi	s6, sp, 40
	lw	s7, 0(s6)
	slliw	t2, s7, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s7, 1
	sw	t1, 0(s6)
.sort_by_digits_base2.bb12:
	addi	t1, sp, 40
	lw	s6, 0(t1)
	bge	s6, s4, .sort_by_digits_base2.bb23
.sort_by_digits_base2.bb13:
	slliw	t1, s6, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_base2.bb15
.sort_by_digits_base2.bb28:
	addi	s4, sp, 40
	lw	s6, 4(s4)
	slliw	t2, s6, 2
	add	t2, s2, t2
	sw	t1, 0(t2)
	addiw	t1, s6, 1
	sw	t1, 4(s4)
.sort_by_digits_base2.bb23:
	addi	t1, sp, 40
	lw	s4, 4(t1)
	bge	s4, t0, .sort_by_digits_base2.bb25
.sort_by_digits_base2.bb24:
	slliw	t1, s4, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	j	.sort_by_digits_base2.bb26
.sort_by_digits_base2.bb25:
	addi	t0, sp, 40
	addiw	s1, s1, -1
	sw	s3, 0(t0)
	addi	t0, sp, 48
	lw	t0, 0(t0)
	addw	s4, s3, t0
	li	t0, -1
	slt	t1, s5, s4
	subw	t0, s1, t0
	xori	t1, t1, 1
	sltiu	t0, t0, 1
	or	t0, t0, t1
	beq	t0, zero, .sort_by_digits_base2.bb34
.sort_by_digits_base2.bb35:
	addi	t0, sp, 40
	sw	s4, 4(t0)
	addi	t0, sp, 48
	lw	t0, 4(t0)
	addw	a3, s4, t0
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s4	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_base2
	j	.sort_by_digits_base2.bb1
.sort_by_digits_base2.bb6:
	addi	t0, sp, 40
	addi	t1, sp, 48
	sw	s3, 0(t0)
	lw	t2, 0(t1)
	addw	s4, s3, t2
	sw	s4, 4(t0)
	lw	t0, 4(t1)
	addw	t0, s4, t0
	j	.sort_by_digits_base2.bb12
.sort_by_digits_base2.bb14:
	j	.sort_by_digits_base2.bb23
.sort_by_digits_base2.bb34:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	mv	a3, s4	# call-arg-copy
	li	a4, 0
	call	sort_by_digits_base2
	j	.sort_by_digits_base2.bb35
.sort_by_digits_base2.bb3:
	slliw	t0, s3, 2
	add	t0, s2, t0
	mv	s6, s3	# phi-copy
.sort_by_digits_base2.bb4:
	bge	s6, s4, .sort_by_digits_base2.bb6
	j	.sort_by_digits_base2.bb5
	.size sort_by_digits_base2, .-sort_by_digits_base2

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	li	t0, 3
	la	s1, values_base2
	sw	t0, 0(s1)
	li	t0, 10
	sw	t0, 4(s1)
	li	t0, 1
	sw	t0, 8(s1)
	li	t0, 8
	sw	t0, 12(s1)
	li	t0, 15
	sw	t0, 16(s1)
	li	t0, 6
	sw	t0, 20(s1)
	li	t0, 13
	sw	t0, 24(s1)
	li	t0, 4
	sw	t0, 28(s1)
	li	t0, 11
	sw	t0, 32(s1)
	li	t0, 2
	sw	t0, 36(s1)
	li	t0, 9
	sw	t0, 40(s1)
	li	t0, 0
	sw	t0, 44(s1)
	li	t0, 7
	sw	t0, 48(s1)
	li	t0, 14
	sw	t0, 52(s1)
	li	t0, 5
	sw	t0, 56(s1)
	li	t0, 12
	sw	t0, 60(s1)
	li	t5, 3
	sd	t5, 0(sp)
	la	t5, values_base2
	sd	t5, 8(sp)
	li	t5, 0
	sd	t5, 16(sp)
	li	t5, 16
	sd	t5, 24(sp)
	li	t5, 11
	sd	t5, 32(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	call	sort_by_digits_base2
	lw	t5, 0(s1)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main
