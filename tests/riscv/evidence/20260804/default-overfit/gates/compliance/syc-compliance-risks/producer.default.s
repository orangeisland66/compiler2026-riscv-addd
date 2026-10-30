
  .section .rodata
  .globl KSIZE
  .align 2
  .type  KSIZE, @object
  .size  KSIZE, 4
KSIZE:
  .word 5

  .bss
  .globl bound
  .align 2
  .type  bound, @object
  .size  bound, 4
bound:
  .zero 4

  .bss
  .globl repeat_bound
  .align 2
  .type  repeat_bound, @object
  .size  repeat_bound, 4
repeat_bound:
  .zero 4

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
	sd	ra, 80(sp)
	sd	s1, 88(sp)
	sd	s2, 96(sp)
	sd	s3, 104(sp)
	sd	s4, 112(sp)
	sd	s5, 120(sp)
	sd	s6, 128(sp)
	sd	s7, 136(sp)
	sd	s8, 144(sp)
	sd	s9, 152(sp)
	sd	s10, 160(sp)
	sd	s11, 168(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	t5, a0
	sd	t5, 8(sp)
	la	t0, repeat_bound
	sw	t5, 0(t0)
	li	t0, 0
	la	s3, input_values	# loop.addr.hoisted
.main.bb1:
	bge	t0, s1, .main.bb3
.main.bb2:
	mulw	s2, t0, s1
	li	t1, 0
.main.bb4:
	bge	t1, s1, .main.bb6
.main.bb38:
	addiw	s4, t1, 3
	blt	s4, s1, .main.bb39
.main.bb5:
	addw	t3, s2, t1
	addw	t2, t0, t1
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s3, t3
	addiw	t1, t1, 1
	sw	t2, 0(t3)
	j	.main.bb4
.main.bb13:
	bge	a6, s1, .main.bb15
.main.bb77:
	addiw	a7, t1, 2
	slt	t4, a7, s1
	ld	t5, 24(sp)
	and	t4, t5, t4
	addiw	a0, a6, -2
	li	a1, 0
	slt	a1, a0, a1
	xori	a1, a1, 1
	and	t4, t4, a1
	addiw	t3, a6, 2
	slt	s2, t3, s1
	and	t4, t4, s2
	bnez	t4, .main.bb78
.main.bb14:
	ld	t5, 32(sp)
	and	t3, t5, a1
	li	a7, 0
	subw	t3, t3, a7
	sltu	t3, zero, t3
	slt	s2, a0, s1
	and	t3, t3, s2
	beq	t3, zero, .main.bb83
.main.bb44:
	addw	t3, s5, a0
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	subw	t3, zero, t3
.main.bb45:
	addiw	t5, a6, 1
	sd	t5, 40(sp)
	addiw	t4, t5, -2
	li	t0, 0
	slt	t0, t4, t0
	xori	t0, t0, 1
	slt	s6, t4, s1
	ld	t5, 32(sp)
	andi	s7, t5, 1
	mv	a7, s7
	bne	a7, zero, .main.bb46
.main.bb84:
.main.bb47:
	addiw	a7, a6, 3
	addiw	a7, a7, -2
	slt	s8, a7, s1
	and	a2, s7, s8
	bne	a2, zero, .main.bb48
.main.bb85:
.main.bb49:
	and	t2, s10, a1
	li	a5, 0
	addiw	a2, a6, 4
	subw	t2, t2, a5
	addiw	s7, a2, -2
	sltu	t2, zero, t2
	slt	a2, s7, s1
	and	t2, t2, s2
	bne	t2, zero, .main.bb50
.main.bb86:
	mv	t2, t3	# phi-copy
.main.bb51:
	and	t3, s10, t0
	li	a5, 0
	subw	t3, t3, a5
	sltu	t3, zero, t3
	and	t3, t3, s6
	bne	t3, zero, .main.bb52
.main.bb87:
.main.bb53:
	andi	t3, s10, 1
	and	a5, t3, s8
	bne	a5, zero, .main.bb54
.main.bb88:
.main.bb55:
	and	t3, t3, a2
	bne	t3, zero, .main.bb56
.main.bb89:
.main.bb57:
	andi	t3, t0, 1
	and	t3, t3, s6
	bne	t3, zero, .main.bb58
.main.bb90:
.main.bb59:
	addw	t3, s4, a6
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
.main.bb60:
	andi	t3, a2, 1
	bne	t3, zero, .main.bb61
.main.bb91:
.main.bb62:
	and	t3, a3, a1
	li	a5, 0
	subw	t3, t3, a5
	sltu	t3, zero, t3
	and	t3, t3, s2
	bne	t3, zero, .main.bb63
.main.bb92:
.main.bb64:
	andi	t3, a3, 1
	mv	a5, t3
	bne	a5, zero, .main.bb65
.main.bb93:
.main.bb66:
	and	t3, t3, s8
	bne	t3, zero, .main.bb67
.main.bb94:
.main.bb68:
	ld	t5, 72(sp)
	and	t3, t5, a1
	li	a5, 0
	subw	t3, t3, a5
	sltu	t3, zero, t3
	and	t3, t3, s2
	bne	t3, zero, .main.bb69
.main.bb95:
.main.bb70:
	ld	t5, 72(sp)
	and	t0, t5, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
	and	t0, t0, s6
	bne	t0, zero, .main.bb71
.main.bb96:
	mv	t0, t2	# phi-copy
.main.bb72:
	ld	t5, 72(sp)
	andi	s2, t5, 1
	and	t2, s2, s8
	bne	t2, zero, .main.bb73
.main.bb97:
.main.bb74:
	and	t2, s2, a2
	bne	t2, zero, .main.bb75
.main.bb98:
.main.bb76:
	ld	t5, 56(sp)
	addi	t2, t5, 4
	mulw	t3, t0, t0
	slliw	a2, t0, 1
	addw	t0, a2, t0
	addw	t0, t3, t0
	addiw	s2, t0, -7
	li	t0, 354224107
	mul	t0, s2, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 3	# const-div.shift
	srliw	t3, t0, 31	# const-div.sign
	addw	t0, t0, t3	# const-div.round
	li	t3, 97
	mulw	t0, t0, t3	# const-rem.mul
	subw	t0, s2, t0	# const-rem.sub
	ld	t5, 64(sp)
	addw	t0, t5, t0
	sd	t0, 64(sp)
	ld	a6, 40(sp)
	sd	t2, 56(sp)
	j	.main.bb13
.main.bb78:
	addw	t0, s5, a0
	slliw	t0, t0, 2
	add	t0, s3, t0
	lw	t0, 0(t0)
	addw	t2, s5, a6
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t2, t0
	addiw	s2, a6, 1
	addw	t2, s5, s2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addiw	t2, t1, -1
	mulw	s6, t2, s1
	addw	t2, s6, a0
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addiw	s7, a6, -1
	addw	t2, s6, s7
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s6, s2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s6, t3
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s4, s7
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s4, a6
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s4, t3
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	ld	t5, 48(sp)
	mulw	s6, t5, s1
	addw	t2, s6, a0
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s6, a6
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s6, s2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	mulw	s6, a7, s1
	addw	t2, s6, a0
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s6, s7
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s6, s2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s6, t3
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	s6, t0, t2
	mulw	t0, s6, s6
	slliw	t2, s6, 1
	addw	t2, t2, s6
	addw	t0, t0, t2
	addiw	s6, t0, -7
	li	t0, 354224107
	mul	t0, s6, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 3	# const-div.shift
	srliw	t2, t0, 31	# const-div.sign
	addw	t0, t0, t2	# const-div.round
	li	t2, 97
	mulw	t0, t0, t2	# const-rem.mul
	subw	t0, s6, t0	# const-rem.sub
	ld	t5, 64(sp)
	addw	t0, t5, t0
	ld	t5, 56(sp)
	addi	t2, t5, 4
	sd	t0, 64(sp)
	mv	a6, s2	# phi-copy
	sd	t2, 56(sp)
	j	.main.bb13
.main.bb46:
	addw	a7, s5, a6
	slliw	a7, a7, 2
	add	a7, s3, a7
	lw	a7, 0(a7)
	addw	t3, t3, a7
	j	.main.bb47
.main.bb48:
	addw	a2, s5, a7
	slliw	a2, a2, 2
	add	a2, s3, a2
	lw	a2, 0(a2)
	subw	t3, t3, a2
	j	.main.bb49
.main.bb50:
	addw	t2, s9, a0
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t2, t3, t2
	j	.main.bb51
.main.bb52:
	addw	t3, s9, t4
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
	j	.main.bb53
.main.bb54:
	addw	a5, s9, a7
	slliw	a5, a5, 2
	add	a5, s3, a5
	lw	a5, 0(a5)
	addw	t2, t2, a5
	j	.main.bb55
.main.bb56:
	addw	t3, s9, s7
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
	j	.main.bb57
.main.bb58:
	addw	t3, s4, t4
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	addw	t2, t2, t3
	j	.main.bb59
.main.bb63:
	addw	t3, s11, a0
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
	j	.main.bb64
.main.bb65:
	addw	a5, s11, a6
	slliw	a5, a5, 2
	add	a5, s3, a5
	lw	a5, 0(a5)
	addw	t2, t2, a5
	j	.main.bb66
.main.bb67:
	addw	t3, s11, a7
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
	j	.main.bb68
.main.bb69:
	addw	t3, a4, a0
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	addw	t2, t2, t3
	j	.main.bb70
.main.bb71:
	addw	t0, a4, t4
	slliw	t0, t0, 2
	add	t0, s3, t0
	lw	t0, 0(t0)
	subw	t0, t2, t0
	j	.main.bb72
.main.bb73:
	addw	t2, a4, a7
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	j	.main.bb74
.main.bb75:
	addw	t2, a4, s7
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	j	.main.bb76
.main.bb61:
	addw	t3, s4, s7
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	addw	t2, t2, t3
	j	.main.bb62
.main.bb83:
	li	t3, 0
	j	.main.bb45
.main.bb15:
	ld	t2, 64(sp)
	ld	t1, 48(sp)
.main.bb10:
	bge	t1, s1, .main.bb12
.main.bb11:
	mulw	s4, t1, s1
	slliw	a2, s4, 2
	la	t5, mapped_values	# remat
	add	a2, t5, a2
	addiw	s5, t1, -2
	li	a3, 0
	slt	a3, s5, a3
	xori	t5, a3, 1
	sd	t5, 24(sp)
	slt	a3, s5, s1
	and	t5, t5, a3
	sd	t5, 32(sp)
	mulw	s5, s5, s1
	addiw	t5, t1, 1
	sd	t5, 48(sp)
	addiw	s9, t5, -2
	li	a3, 0
	slt	a3, s9, a3
	xori	a3, a3, 1
	slt	a4, s9, s1
	and	s10, a3, a4
	mulw	s9, s9, s1
	addiw	a3, t1, 3
	addiw	s11, a3, -2
	slt	a3, s11, s1
	mulw	s11, s11, s1
	addiw	a4, t1, 4
	addiw	a4, a4, -2
	slt	t5, a4, s1
	li	a6, 0
	sd	t5, 72(sp)
	mulw	a4, a4, s1
	sd	t2, 64(sp)
	sd	a2, 56(sp)
	j	.main.bb13
.main.bb25:
	bge	t2, s6, .main.bb32
.main.bb42:
	addiw	s7, t2, 3
	blt	s7, s6, .main.bb43
.main.bb26:
	addw	t3, s2, t2
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	s7, s5, t3
	lw	t3, 0(s7)
	subw	t3, t3, t1
	sw	t3, 0(s7)
	j	.main.bb25
.main.bb43:
	addw	t3, s2, t2
	slliw	t3, t3, 2
	add	s8, s5, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addiw	t3, t2, 1
	addw	t3, s2, t3
	slliw	t3, t3, 2
	add	s8, s5, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addiw	t3, t2, 2
	addw	t3, s2, t3
	addiw	t2, t2, 4
	slliw	t3, t3, 2
	add	s8, s5, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addw	t3, s2, s7
	slliw	t3, t3, 2
	add	s7, s5, t3
	lw	t3, 0(s7)
	subw	t3, t3, t1
	sw	t3, 0(s7)
	j	.main.bb25
.main.bb37:
.main.bb32:
	addiw	s6, t2, 8
	bge	t2, s1, .main.bb27
.main.bb33:
	bge	s6, s1, .main.bb34
.main.bb35:
	j	.main.bb25
.main.bb36:
	j	.main.bb25
.main.bb39:
	addw	t3, s2, t1
	addw	t2, t0, t1
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s3, t3
	addiw	s5, t1, 1
	sw	t2, 0(t3)
	addw	t3, s2, s5
	addw	t2, t0, s5
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s3, t3
	addiw	s5, t1, 2
	sw	t2, 0(t3)
	addiw	t1, t1, 4
	addw	t3, s2, s5
	addw	t2, t0, s5
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s3, t3
	sw	t2, 0(t3)
	addw	t3, s2, s4
	addw	t2, t0, s4
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, s3, t3
	sw	t2, 0(t3)
	j	.main.bb4
.main.bb34:
	mv	s6, s1	# phi-copy
	j	.main.bb25
.main.bb6:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t5, 29
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 1
	li	t1, 0
	sd	t0, 16(sp)
.main.bb7:
	ld	t5, 16(sp)
	ld	t6, 8(sp)
	bge	t5, t6, .main.bb9
.main.bb8:
	li	t1, 0
	li	t2, 0
	la	s3, input_values	# loop.addr.hoisted
	j	.main.bb10
.main.bb22:
	bge	t2, s1, .main.bb24
.main.bb40:
	addiw	s5, t2, 3
	blt	s5, s1, .main.bb41
.main.bb23:
	addw	t3, s2, t2
	la	a2, mapped_values
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, a2, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	j	.main.bb22
.main.bb41:
	addw	t3, s2, t2
	la	s6, mapped_values
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addiw	t3, t2, 1
	addw	t3, s2, t3
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addiw	t3, t2, 2
	addw	t3, s2, t3
	addiw	t2, t2, 4
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addw	t3, s2, s5
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	j	.main.bb22
.main.bb24:
	li	t2, 0
	la	s5, mapped_values	# loop.addr.hoisted
	j	.main.bb32
.main.bb12:
	ld	t5, 16(sp)
	addiw	t0, t5, 1
	mv	t1, t2	# phi-copy
	sd	t0, 16(sp)
	j	.main.bb7
.main.bb27:
	addiw	t0, t0, 1
	bge	t0, s1, .main.bb21	# rotated-loop-branch
.main.bb20:
	mulw	s2, t0, s1
	li	t1, 0
	li	t2, 0
	j	.main.bb22
.main.bb9:
	la	s3, mapped_values
	mulw	s4, s1, s1
	li	t0, 1
	ld	t5, 8(sp)
	slt	t0, t0, t5
	li	t2, 0
	slt	t2, s1, t2
	xori	s2, t2, 1
	and	t0, t0, s2
	li	t2, 1
	subw	s5, t2, s1
	mulw	t1, s5, t1
	beqz	t0, .main.bb79
.main.bb80:
	mv	s1, t1	# phi-copy
.main.bb30:
	li	a0, 94
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s11, 168(sp)
	ld	s10, 160(sp)
	ld	s9, 152(sp)
	ld	s8, 144(sp)
	ld	s7, 136(sp)
	ld	s6, 128(sp)
	ld	s5, 120(sp)
	ld	s4, 112(sp)
	ld	s3, 104(sp)
	ld	s2, 96(sp)
	ld	s1, 88(sp)
	ld	ra, 80(sp)
	addi	sp, sp, 176
	ret
.main.bb21:
	li	t0, 0
	li	t1, 0
.main.bb28:
	bge	t1, s4, .main.bb82
.main.bb29:
	lw	t2, 0(s3)
	addiw	t1, t1, 1
	addw	t0, t0, t2
	addi	s3, s3, 4
	blt	t1, s4, .main.bb29	# rotated-loop-branch
.main.bb82:
	mv	s1, t0	# phi-copy
	j	.main.bb30
.main.bb17:
	lw	s7, 0(s6)
	addiw	t0, t0, 1
	mulw	t2, s7, s7
	slliw	t3, s7, 1
	addw	t3, t3, s7
	addw	t2, t2, t3
	addiw	s7, t2, -7
	li	t2, 354224107
	mul	t2, s7, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	sraiw	t2, t2, 3	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	li	t3, 97
	mulw	t2, t2, t3	# const-rem.mul
	subw	s7, s7, t2	# const-rem.sub
	sw	s7, 0(s6)
	addw	t1, t1, s7
	addi	s6, s6, 4
	blt	t0, s4, .main.bb17	# rotated-loop-branch
.main.bb18:
	mulw	t0, s5, t1
	beqz	s2, .main.bb31
.main.bb81:
	mv	s1, t0	# phi-copy
	j	.main.bb30
.main.bb31:
	li	t0, 0
.main.bb19:
	bge	t0, s1, .main.bb21
	j	.main.bb20
.main.bb79:
	li	t0, 0
	li	t1, 0
	mv	s6, s3	# phi-copy
.main.bb16:
	bge	t0, s4, .main.bb18
	j	.main.bb17
	.size main, .-main
