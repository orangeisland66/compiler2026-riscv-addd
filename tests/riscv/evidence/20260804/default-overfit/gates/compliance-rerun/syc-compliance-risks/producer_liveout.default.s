
  .section .rodata
  .globl KSIZE
  .align 2
  .type  KSIZE, @object
  .size  KSIZE, 4
KSIZE:
  .word 5

  .bss
  .globl input_values
  .align 2
  .type  input_values, @object
  .size  input_values, 256
input_values:
  .zero 256

  .bss
  .globl mapped_values
  .align 2
  .type  mapped_values, @object
  .size  mapped_values, 256
mapped_values:
  .zero 256

  .bss
  .globl kernel
  .align 2
  .type  kernel, @object
  .size  kernel, 100
kernel:
  .zero 100

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -176
	sd	ra, 72(sp)
	sd	s1, 80(sp)
	sd	s2, 88(sp)
	sd	s3, 96(sp)
	sd	s4, 104(sp)
	sd	s5, 112(sp)
	sd	s6, 120(sp)
	sd	s7, 128(sp)
	sd	s8, 136(sp)
	sd	s9, 144(sp)
	sd	s10, 152(sp)
	sd	s11, 160(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	t5, a0
	sd	t5, 8(sp)
	li	t1, 0
	la	s2, input_values	# loop.addr.hoisted
.main.bb1:
	bge	t1, s1, .main.bb3
.main.bb2:
	mulw	s3, t1, s1
	li	t0, 0
.main.bb4:
	bge	t0, s1, .main.bb6
.main.bb38:
	addiw	s4, t0, 3
	blt	s4, s1, .main.bb39
.main.bb5:
	addw	t3, s3, t0
	addw	t2, t1, t0
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s2, t3
	addiw	t0, t0, 1
	sw	t2, 0(t3)
	j	.main.bb4
.main.bb13:
	bge	a4, s1, .main.bb15
.main.bb77:
	addiw	a5, t2, 2
	slt	a6, a5, s1
	ld	t5, 24(sp)
	and	a6, t5, a6
	addiw	a7, a4, -2
	li	t4, 0
	slt	t4, a7, t4
	xori	t4, t4, 1
	and	a6, a6, t4
	addiw	a0, a4, 2
	slt	a1, a0, s1
	and	a6, a6, a1
	bnez	a6, .main.bb78
.main.bb14:
	ld	t5, 32(sp)
	and	a5, t5, t4
	li	a6, 0
	subw	a5, a5, a6
	sltu	a5, zero, a5
	slt	a6, a7, s1
	and	a5, a5, a6
	beq	a5, zero, .main.bb81
.main.bb44:
	addw	a5, s4, a7
	slliw	a5, a5, 2
	add	a5, s2, a5
	lw	a5, 0(a5)
	subw	a5, zero, a5
.main.bb45:
	addiw	t5, a4, 1
	sd	t5, 40(sp)
	addiw	a1, t5, -2
	li	t0, 0
	slt	t0, a1, t0
	xori	t0, t0, 1
	slt	t1, a1, s1
	ld	t5, 32(sp)
	andi	s5, t5, 1
	mv	a0, s5
	bne	a0, zero, .main.bb46
.main.bb82:
.main.bb47:
	addiw	a0, a4, 3
	addiw	s6, a0, -2
	slt	a0, s6, s1
	and	s7, s5, a0
	bne	s7, zero, .main.bb48
.main.bb83:
	mv	a3, a5	# phi-copy
.main.bb49:
	addiw	a5, a4, 4
	li	t3, 0
	addiw	s5, a5, -2
	and	a5, s9, t4
	slt	s7, s5, s1
	subw	t3, a5, t3
	sltu	t3, zero, t3
	and	t3, t3, a6
	bne	t3, zero, .main.bb50
.main.bb84:
	mv	t3, a3	# phi-copy
.main.bb51:
	and	a3, s9, t0
	li	a5, 0
	subw	a3, a3, a5
	sltu	a3, zero, a3
	and	a3, a3, t1
	bne	a3, zero, .main.bb52
.main.bb85:
.main.bb53:
	andi	a3, s9, 1
	and	a5, a3, a0
	bne	a5, zero, .main.bb54
.main.bb86:
.main.bb55:
	and	a3, a3, s7
	bne	a3, zero, .main.bb56
.main.bb87:
.main.bb57:
	andi	a3, t0, 1
	and	a3, a3, t1
	bne	a3, zero, .main.bb58
.main.bb88:
.main.bb59:
	addw	a3, s3, a4
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	subw	t3, t3, a3
.main.bb60:
	andi	a3, s7, 1
	bne	a3, zero, .main.bb61
.main.bb89:
.main.bb62:
	and	a3, s11, t4
	li	a5, 0
	subw	a3, a3, a5
	sltu	a3, zero, a3
	and	a3, a3, a6
	bne	a3, zero, .main.bb63
.main.bb90:
.main.bb64:
	andi	a3, s11, 1
	mv	a5, a3
	bne	a5, zero, .main.bb65
.main.bb91:
.main.bb66:
	and	a3, a3, a0
	bne	a3, zero, .main.bb67
.main.bb92:
.main.bb68:
	ld	t5, 56(sp)
	and	a3, t5, t4
	li	a4, 0
	subw	a3, a3, a4
	sltu	a3, zero, a3
	and	a3, a3, a6
	bne	a3, zero, .main.bb69
.main.bb93:
.main.bb70:
	ld	t5, 56(sp)
	and	t0, t5, t0
	li	a3, 0
	subw	t0, t0, a3
	sltu	t0, zero, t0
	and	t0, t0, t1
	bne	t0, zero, .main.bb71
.main.bb94:
	mv	t0, t3	# phi-copy
.main.bb72:
	ld	t5, 56(sp)
	andi	t1, t5, 1
	and	t3, t1, a0
	bne	t3, zero, .main.bb73
.main.bb95:
.main.bb74:
	and	t1, t1, s7
	bne	t1, zero, .main.bb75
.main.bb96:
.main.bb76:
	ld	t5, 64(sp)
	sw	t0, 0(t5)
	ld	t5, 64(sp)
	addi	t0, t5, 4
	ld	a4, 40(sp)
	sd	t0, 64(sp)
	j	.main.bb13
.main.bb78:
	addw	t0, s4, a7
	slliw	t0, t0, 2
	add	t0, s2, t0
	lw	t0, 0(t0)
	addw	t1, s4, a4
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t1, t0
	addiw	s5, a4, 1
	addw	t1, s4, s5
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	addiw	t1, t2, -1
	mulw	s6, t1, s1
	addw	t1, s6, a7
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addiw	s7, a4, -1
	addw	t1, s6, s7
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	addw	t1, s6, s5
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addw	t1, s6, a0
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	addw	t1, s3, s7
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addw	t1, s3, a4
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	addw	t1, s3, a0
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	ld	t5, 48(sp)
	mulw	s6, t5, s1
	addw	t1, s6, a7
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	addw	t1, s6, a4
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addw	t1, s6, s5
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	mulw	s6, a5, s1
	addw	t1, s6, a7
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addw	t1, s6, s7
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	addw	t1, s6, s5
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addw	t1, s6, a0
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	ld	t5, 64(sp)
	sw	t0, 0(t5)
	ld	t5, 64(sp)
	addi	t0, t5, 4
	mv	a4, s5	# phi-copy
	sd	t0, 64(sp)
	j	.main.bb13
.main.bb46:
	addw	a0, s4, a4
	slliw	a0, a0, 2
	add	a0, s2, a0
	lw	a0, 0(a0)
	addw	a5, a5, a0
	j	.main.bb47
.main.bb48:
	addw	a3, s4, s6
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	subw	a3, a5, a3
	j	.main.bb49
.main.bb50:
	addw	t3, s8, a7
	slliw	t3, t3, 2
	add	t3, s2, t3
	lw	t3, 0(t3)
	addw	t3, a3, t3
	j	.main.bb51
.main.bb52:
	addw	a3, s8, a1
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	subw	t3, t3, a3
	j	.main.bb53
.main.bb54:
	addw	a5, s8, s6
	slliw	a5, a5, 2
	add	a5, s2, a5
	lw	a5, 0(a5)
	addw	t3, t3, a5
	j	.main.bb55
.main.bb56:
	addw	a3, s8, s5
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	subw	t3, t3, a3
	j	.main.bb57
.main.bb58:
	addw	a3, s3, a1
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	addw	t3, t3, a3
	j	.main.bb59
.main.bb63:
	addw	a3, s10, a7
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	subw	t3, t3, a3
	j	.main.bb64
.main.bb65:
	addw	a4, s10, a4
	slliw	a4, a4, 2
	add	a4, s2, a4
	lw	a4, 0(a4)
	addw	t3, t3, a4
	j	.main.bb66
.main.bb67:
	addw	a3, s10, s6
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	subw	t3, t3, a3
	j	.main.bb68
.main.bb69:
	addw	a3, a2, a7
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	addw	t3, t3, a3
	j	.main.bb70
.main.bb71:
	addw	t0, a2, a1
	slliw	t0, t0, 2
	add	t0, s2, t0
	lw	t0, 0(t0)
	subw	t0, t3, t0
	j	.main.bb72
.main.bb73:
	addw	t3, a2, s6
	slliw	t3, t3, 2
	add	t3, s2, t3
	lw	t3, 0(t3)
	addw	t0, t0, t3
	j	.main.bb74
.main.bb75:
	addw	t1, a2, s5
	slliw	t1, t1, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	subw	t0, t0, t1
	j	.main.bb76
.main.bb61:
	addw	a3, s3, s5
	slliw	a3, a3, 2
	add	a3, s2, a3
	lw	a3, 0(a3)
	addw	t3, t3, a3
	j	.main.bb62
.main.bb81:
	li	a5, 0
	j	.main.bb45
.main.bb15:
	ld	t2, 48(sp)
.main.bb10:
	bge	t2, s1, .main.bb12
.main.bb11:
	mulw	s3, t2, s1
	slliw	t3, s3, 2
	la	a2, mapped_values
	add	t3, a2, t3
	addiw	s4, t2, -2
	li	a2, 0
	slt	a2, s4, a2
	xori	t5, a2, 1
	sd	t5, 24(sp)
	slt	a2, s4, s1
	and	t5, t5, a2
	sd	t5, 32(sp)
	mulw	s4, s4, s1
	addiw	t5, t2, 1
	sd	t5, 48(sp)
	addiw	s8, t5, -2
	li	a2, 0
	slt	a2, s8, a2
	xori	a2, a2, 1
	slt	a3, s8, s1
	and	s9, a2, a3
	mulw	s8, s8, s1
	addiw	a2, t2, 3
	addiw	s10, a2, -2
	slt	s11, s10, s1
	mulw	s10, s10, s1
	addiw	a2, t2, 4
	addiw	a2, a2, -2
	slt	t5, a2, s1
	li	a4, 0
	sd	t5, 56(sp)
	mulw	a2, a2, s1
	sd	t3, 64(sp)
	j	.main.bb13
.main.bb25:
	bge	t3, s6, .main.bb32
.main.bb42:
	addiw	s7, t3, 3
	blt	s7, s6, .main.bb43
.main.bb26:
	addw	a2, s4, t3
	slliw	a2, a2, 2
	addiw	t3, t3, 1
	add	s7, s5, a2
	lw	a2, 0(s7)
	subw	a2, a2, t2
	sw	a2, 0(s7)
	j	.main.bb25
.main.bb43:
	addw	a2, s4, t3
	slliw	a2, a2, 2
	add	s8, s5, a2
	lw	a2, 0(s8)
	subw	a2, a2, t2
	sw	a2, 0(s8)
	addiw	a2, t3, 1
	addw	a2, s4, a2
	slliw	a2, a2, 2
	add	s8, s5, a2
	lw	a2, 0(s8)
	subw	a2, a2, t2
	sw	a2, 0(s8)
	addiw	a2, t3, 2
	addw	a2, s4, a2
	addiw	t3, t3, 4
	slliw	a2, a2, 2
	add	s8, s5, a2
	lw	a2, 0(s8)
	subw	a2, a2, t2
	sw	a2, 0(s8)
	addw	a2, s4, s7
	slliw	a2, a2, 2
	add	s7, s5, a2
	lw	a2, 0(s7)
	subw	a2, a2, t2
	sw	a2, 0(s7)
	j	.main.bb25
.main.bb37:
.main.bb32:
	addiw	s6, t3, 8
	bge	t3, s1, .main.bb27
.main.bb33:
	bge	s6, s1, .main.bb34
.main.bb35:
	j	.main.bb25
.main.bb36:
	j	.main.bb25
.main.bb39:
	addw	t3, s3, t0
	addw	t2, t1, t0
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s2, t3
	addiw	s5, t0, 1
	sw	t2, 0(t3)
	addw	t3, s3, s5
	addw	t2, t1, s5
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s2, t3
	addiw	s5, t0, 2
	sw	t2, 0(t3)
	addiw	t0, t0, 4
	addw	t3, s3, s5
	addw	t2, t1, s5
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s2, t3
	sw	t2, 0(t3)
	addw	t3, s3, s4
	addw	t2, t1, s4
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s2, t3
	sw	t2, 0(t3)
	j	.main.bb4
.main.bb34:
	mv	s6, s1	# phi-copy
	j	.main.bb25
.main.bb6:
	addiw	t1, t1, 1
	blt	t1, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t1, 1
	sd	t1, 16(sp)
.main.bb7:
	ld	t5, 16(sp)
	ld	t6, 8(sp)
	bge	t5, t6, .main.bb9
.main.bb8:
	li	t2, 0
	la	s2, input_values	# loop.addr.hoisted
	j	.main.bb10
.main.bb22:
	bge	t3, s1, .main.bb24
.main.bb40:
	addiw	s5, t3, 3
	blt	s5, s1, .main.bb41
.main.bb23:
	addw	a2, s4, t3
	la	a3, mapped_values
	slliw	a2, a2, 2
	addiw	t3, t3, 1
	add	a2, a3, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	j	.main.bb22
.main.bb41:
	addw	a2, s4, t3
	la	s6, mapped_values
	slliw	a2, a2, 2
	add	a2, s6, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	addiw	a2, t3, 1
	addw	a2, s4, a2
	slliw	a2, a2, 2
	add	a2, s6, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	addiw	a2, t3, 2
	addw	a2, s4, a2
	addiw	t3, t3, 4
	slliw	a2, a2, 2
	add	a2, s6, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	addw	a2, s4, s5
	slliw	a2, a2, 2
	add	a2, s6, a2
	lw	a2, 0(a2)
	addw	t2, t2, a2
	j	.main.bb22
.main.bb24:
	li	t3, 0
	la	s5, mapped_values	# loop.addr.hoisted
	j	.main.bb32
.main.bb12:
	ld	t5, 16(sp)
	addiw	t1, t5, 1
	sd	t1, 16(sp)
	j	.main.bb7
.main.bb27:
	addiw	t1, t1, 1
	bge	t1, s1, .main.bb21	# rotated-loop-branch
.main.bb20:
	mulw	s4, t1, s1
	li	t2, 0
	li	t3, 0
	j	.main.bb22
.main.bb21:
	li	t1, 0
	li	t2, 0
.main.bb28:
	bge	t2, s2, .main.bb30
.main.bb29:
	lw	t3, 0(s3)
	addiw	t2, t2, 1
	addw	t1, t1, t3
	addi	s3, s3, 4
	blt	t2, s2, .main.bb29	# rotated-loop-branch
	j	.main.bb30	# rotated-loop-exit
.main.bb80:
.main.bb30:
	addw	a0, t1, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s11, 160(sp)
	ld	s10, 152(sp)
	ld	s9, 144(sp)
	ld	s8, 136(sp)
	ld	s7, 128(sp)
	ld	s6, 120(sp)
	ld	s5, 112(sp)
	ld	s4, 104(sp)
	ld	s3, 96(sp)
	ld	s2, 88(sp)
	ld	s1, 80(sp)
	ld	ra, 72(sp)
	addi	sp, sp, 176
	ret
.main.bb9:
	mulw	s2, s1, s1
	la	s3, mapped_values
	li	t0, 0
	li	t1, 0
	li	t2, 0
	mv	s4, s3	# phi-copy
.main.bb16:
	bge	t1, s2, .main.bb18
.main.bb17:
	lw	s5, 0(s4)
	mulw	t0, s5, s5
	slliw	t3, s5, 1
	addw	t3, t3, s5
	addw	t0, t0, t3
	addiw	s6, t0, -7
	li	t0, 354224107
	mul	t0, s6, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 3	# const-div.shift
	srliw	t3, t0, 31	# const-div.sign
	addw	t0, t0, t3	# const-div.round
	li	t3, 97
	mulw	t0, t0, t3	# const-rem.mul
	subw	s6, s6, t0	# const-rem.sub
	addiw	t1, t1, 1
	sw	s6, 0(s4)
	addw	t2, t2, s6
	addi	s4, s4, 4
	mv	t0, s5	# phi-copy
	blt	t1, s2, .main.bb17	# rotated-loop-branch
.main.bb18:
	li	t1, 1
	subw	t1, t1, s1
	mulw	t1, t1, t2
	bge	s1, zero, .main.bb30
	j	.main.bb31
.main.bb79:
	j	.main.bb30
.main.bb31:
	li	t1, 0
.main.bb19:
	bge	t1, s1, .main.bb21
	j	.main.bb20
	.size main, .-main
