
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
	addi	sp, sp, -224
	sd	ra, 120(sp)
	sd	s1, 128(sp)
	sd	s2, 136(sp)
	sd	s3, 144(sp)
	sd	s4, 152(sp)
	sd	s5, 160(sp)
	sd	s6, 168(sp)
	sd	s7, 176(sp)
	sd	s8, 184(sp)
	sd	s9, 192(sp)
	sd	s10, 200(sp)
	sd	s11, 208(sp)
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
.main.bb25:
	bge	t2, s6, .main.bb32
.main.bb42:
	addiw	s7, t2, 3
	blt	s7, s6, .main.bb43
.main.bb26:
	addw	t3, s4, t2
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	s7, s5, t3
	lw	t3, 0(s7)
	subw	t3, t3, t1
	sw	t3, 0(s7)
	j	.main.bb25
.main.bb43:
	addw	t3, s4, t2
	slliw	t3, t3, 2
	add	s8, s5, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addiw	t3, t2, 1
	addw	t3, s4, t3
	slliw	t3, t3, 2
	add	s8, s5, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addiw	t3, t2, 2
	addw	t3, s4, t3
	addiw	t2, t2, 4
	slliw	t3, t3, 2
	add	s8, s5, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addw	t3, s4, s7
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
	sd	t0, 16(sp)
.main.bb7:
	ld	t5, 16(sp)
	ld	t6, 8(sp)
	bge	t5, t6, .main.bb9
.main.bb8:
	li	t1, 0
.main.bb10:
	bge	t1, s1, .main.bb12
.main.bb11:
	mulw	s3, t1, s1
	slliw	t2, s3, 2
	la	t3, mapped_values
	add	t2, t3, t2
	addiw	s4, t1, -2
	li	t3, 0
	slt	t3, s4, t3
	xori	t3, t3, 1
	slt	a2, s4, s1
	and	t5, t3, a2
	sd	t5, 48(sp)
	mulw	t5, s4, s1
	sd	t5, 80(sp)
	addiw	t5, t1, 1
	sd	t5, 24(sp)
	addiw	s7, t5, -2
	li	t3, 0
	slt	t3, s7, t3
	xori	t3, t3, 1
	slt	a2, s7, s1
	and	s8, t3, a2
	mulw	s7, s7, s1
	addiw	t3, t1, 3
	addiw	s9, t3, -2
	slt	t5, s9, s1
	sd	t5, 56(sp)
	mulw	t5, s9, s1
	sd	t5, 72(sp)
	addiw	t1, t1, 4
	addiw	s11, t1, -2
	slt	t5, s11, s1
	li	t3, 0
	sd	t5, 64(sp)
	mulw	t5, s11, s1
	sd	t5, 88(sp)
	la	a2, input_values	# loop.addr.hoisted
	addiw	a3, t3, -2	# loop.addr.init
	addw	a3, s7, a3	# loop.addr.init
	slliw	a3, a3, 2	# loop.addr.init
	add	a3, a2, a3	# loop.addr.init
	addiw	a4, t3, 1	# loop.addr.init
	addiw	a4, a4, -2	# loop.addr.init
	addw	a4, t5, a4	# loop.addr.init
	slliw	a4, a4, 2	# loop.addr.init
	add	a4, a2, a4	# loop.addr.init
	addiw	a5, t3, 1	# loop.addr.init
	addiw	a5, a5, -2	# loop.addr.init
	addw	a5, s3, a5	# loop.addr.init
	slliw	a5, a5, 2	# loop.addr.init
	add	a5, a2, a5	# loop.addr.init
	addiw	a6, t3, -2	# loop.addr.init
	ld	t5, 80(sp)
	addw	a6, t5, a6	# loop.addr.init
	slliw	a6, a6, 2	# loop.addr.init
	add	a6, a2, a6	# loop.addr.init
	addiw	a7, t3, 3	# loop.addr.init
	addiw	a7, a7, -2	# loop.addr.init
	ld	t5, 72(sp)
	addw	a7, t5, a7	# loop.addr.init
	slliw	a7, a7, 2	# loop.addr.init
	add	a7, a2, a7	# loop.addr.init
	sd	t2, 32(sp)
	sd	a6, 104(sp)
	sd	a7, 112(sp)
.main.bb13:
	bge	t3, s1, .main.bb15
.main.bb14:
	addiw	t4, t3, -2
	li	a0, 0
	slt	a0, t4, a0
	xori	a0, a0, 1
	ld	t5, 48(sp)
	and	a1, t5, a0
	slt	s2, t4, s1
	and	a1, a1, s2
	beq	a1, zero, .main.bb87
.main.bb44:
	ld	t5, 80(sp)
	addw	a1, t5, t4
	ld	t5, 104(sp)
	lw	a1, 0(t5)
	subw	a1, zero, a1
.main.bb45:
	addiw	t5, t3, 1
	sd	t5, 40(sp)
	addiw	s6, t5, -2
	li	t2, 0
	slt	t2, s6, t2
	xori	t2, t2, 1
	slt	t0, s6, s1
.main.bb46:
	ld	t5, 48(sp)
	andi	s5, t5, 1
	mv	t1, s5
	bne	t1, zero, .main.bb47
.main.bb88:
	mv	t1, a1	# phi-copy
.main.bb48:
	addiw	a1, t3, 3
	addiw	s10, a1, -2
	slt	a1, s10, s1
	and	s9, s5, a1
	bne	s9, zero, .main.bb49
.main.bb89:
.main.bb50:
	addiw	a6, t3, 4
	addiw	s4, a6, -2
	slt	s5, s4, s1
.main.bb51:
	and	a6, s8, a0
	li	a7, 0
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	a6, a6, s2
	bne	a6, zero, .main.bb52
.main.bb90:
.main.bb53:
	and	a6, s8, t2
	li	a7, 0
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	a6, a6, t0
	bne	a6, zero, .main.bb54
.main.bb91:
.main.bb55:
	andi	s9, s8, 1
.main.bb56:
	and	a6, s9, a1
	bne	a6, zero, .main.bb57
.main.bb92:
.main.bb58:
	and	a6, s9, s5
	bne	a6, zero, .main.bb59
.main.bb93:
	j	.main.bb61
.main.bb60:
.main.bb61:
	andi	a6, t2, 1
	and	a6, a6, t0
	bne	a6, zero, .main.bb62
.main.bb94:
.main.bb63:
	lw	a6, 4(a5)
	subw	s9, t1, a6
	j	.main.bb65
.main.bb64:
.main.bb65:
	andi	t1, s5, 1
	bne	t1, zero, .main.bb66
.main.bb95:
	mv	t1, s9	# phi-copy
.main.bb67:
	ld	t5, 56(sp)
	and	a6, t5, a0
	li	a7, 0
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	a6, a6, s2
	bne	a6, zero, .main.bb68
.main.bb96:
	j	.main.bb70
.main.bb69:
.main.bb70:
	ld	t5, 56(sp)
	andi	s9, t5, 1
	mv	a6, s9
	bne	a6, zero, .main.bb71
.main.bb97:
.main.bb72:
	and	t3, s9, a1
	bne	t3, zero, .main.bb73
.main.bb98:
	j	.main.bb75
.main.bb74:
.main.bb75:
	ld	t5, 64(sp)
	and	t3, t5, a0
	li	a6, 0
	subw	t3, t3, a6
	sltu	t3, zero, t3
	and	t3, t3, s2
	bne	t3, zero, .main.bb76
.main.bb99:
.main.bb77:
	ld	t5, 64(sp)
	and	t2, t5, t2
	li	t3, 0
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t0, t2, t0
	bne	t0, zero, .main.bb78
.main.bb100:
	mv	t0, t1	# phi-copy
.main.bb79:
	ld	t5, 64(sp)
	andi	s2, t5, 1
.main.bb80:
	and	t1, s2, a1
	bne	t1, zero, .main.bb81
.main.bb101:
.main.bb82:
	and	t1, s2, s5
	bne	t1, zero, .main.bb83
.main.bb102:
.main.bb84:
	ld	t5, 32(sp)
	sw	t0, 0(t5)
	ld	t5, 32(sp)
	addi	t0, t5, 4
	addi	t1, a3, 4	# loop.addr.bump
	addi	t2, a4, 4	# loop.addr.bump
	addi	a5, a5, 4	# loop.addr.bump
	ld	t5, 104(sp)
	addi	a3, t5, 4	# loop.addr.bump
	ld	t5, 112(sp)
	addi	a4, t5, 4	# loop.addr.bump
	sd	t0, 32(sp)
	ld	t3, 40(sp)
	sd	a3, 104(sp)
	mv	a3, t1	# phi-copy
	sd	a4, 112(sp)
	mv	a4, t2	# phi-copy
	j	.main.bb13
.main.bb49:
	ld	t5, 80(sp)
	addw	s4, t5, s10
	slliw	s11, s4, 2
	add	t5, a2, s11
	sd	t5, 96(sp)
	ld	t5, 104(sp)
	lw	a6, 12(t5)
	subw	t1, t1, a6
	j	.main.bb50
.main.bb54:
	lw	a6, 4(a3)
	subw	t1, t1, a6
	j	.main.bb55
.main.bb59:
	lw	a6, 16(a3)
	subw	t1, t1, a6
	j	.main.bb61
.main.bb68:
	ld	t5, 72(sp)
	addw	a6, t5, t4
	ld	t5, 112(sp)
	lw	a6, -12(t5)
	subw	t1, t1, a6
	j	.main.bb70
.main.bb73:
	ld	t5, 72(sp)
	addw	t3, t5, s10
	ld	t5, 112(sp)
	lw	t3, 0(t5)
	subw	t1, t1, t3
	j	.main.bb75
.main.bb78:
	ld	t5, 88(sp)
	addw	t0, t5, s6
	lw	t0, 0(a4)
	subw	t0, t1, t0
	j	.main.bb79
.main.bb83:
	ld	t5, 88(sp)
	addw	t1, t5, s4
	lw	t1, 12(a4)
	subw	t0, t0, t1
	j	.main.bb84
.main.bb47:
	ld	t5, 80(sp)
	addw	t1, t5, t3
	ld	t5, 104(sp)
	lw	t1, 8(t5)
	addw	t1, a1, t1
	j	.main.bb48
.main.bb52:
	addw	a6, s7, t4
	lw	a6, 0(a3)
	addw	t1, t1, a6
	j	.main.bb53
.main.bb57:
	lw	a6, 12(a3)
	addw	t1, t1, a6
	j	.main.bb58
.main.bb62:
	lw	a6, 0(a5)
	addw	t1, t1, a6
	j	.main.bb63
.main.bb66:
	lw	t1, 12(a5)
	addw	t1, s9, t1
	j	.main.bb67
.main.bb71:
	ld	t5, 72(sp)
	addw	t3, t5, t3
	ld	t5, 112(sp)
	lw	t3, -4(t5)
	addw	t1, t1, t3
	j	.main.bb72
.main.bb76:
	ld	t5, 88(sp)
	addw	t3, t5, t4
	lw	t3, -4(a4)
	addw	t1, t1, t3
	j	.main.bb77
.main.bb81:
	ld	t5, 88(sp)
	addw	t1, t5, s10
	lw	t1, 8(a4)
	addw	t0, t0, t1
	j	.main.bb82
.main.bb87:
	li	a1, 0
	j	.main.bb45
.main.bb15:
	ld	t1, 24(sp)
	j	.main.bb10
.main.bb22:
	bge	t2, s1, .main.bb24
.main.bb40:
	addiw	s5, t2, 3
	blt	s5, s1, .main.bb41
.main.bb23:
	addw	t3, s4, t2
	la	a2, mapped_values
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, a2, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	j	.main.bb22
.main.bb41:
	addw	t3, s4, t2
	la	s6, mapped_values
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addiw	t3, t2, 1
	addw	t3, s4, t3
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addiw	t3, t2, 2
	addw	t3, s4, t3
	addiw	t2, t2, 4
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	addw	t3, s4, s5
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
	sd	t0, 16(sp)
	j	.main.bb7
.main.bb27:
	addiw	t0, t0, 1
	bge	t0, s1, .main.bb21	# rotated-loop-branch
.main.bb20:
	mulw	s4, t0, s1
	li	t1, 0
	li	t2, 0
	j	.main.bb22
.main.bb21:
	li	t0, 0
	li	t1, 0
.main.bb28:
	bge	t1, s3, .main.bb86
.main.bb29:
	lw	t2, 0(s2)
	addiw	t1, t1, 1
	addw	t0, t0, t2
	addi	s2, s2, 4
	blt	t1, s3, .main.bb29	# rotated-loop-branch
.main.bb86:
	mv	s1, t0	# phi-copy
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
	ld	s11, 208(sp)
	ld	s10, 200(sp)
	ld	s9, 192(sp)
	ld	s8, 184(sp)
	ld	s7, 176(sp)
	ld	s6, 168(sp)
	ld	s5, 160(sp)
	ld	s4, 152(sp)
	ld	s3, 144(sp)
	ld	s2, 136(sp)
	ld	s1, 128(sp)
	ld	ra, 120(sp)
	addi	sp, sp, 224
	ret
.main.bb9:
	mulw	s3, s1, s1
	la	s2, mapped_values
	li	t0, 0
	li	t1, 0
	mv	s4, s2	# phi-copy
.main.bb16:
	bge	t0, s3, .main.bb18
.main.bb17:
	lw	s5, 0(s4)
	addiw	t0, t0, 1
	mulw	t2, s5, s5
	slliw	t3, s5, 1
	addw	t3, t3, s5
	addw	t2, t2, t3
	addiw	s5, t2, -7
	li	t2, 354224107
	mul	t2, s5, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	sraiw	t2, t2, 3	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	li	t3, 97
	mulw	t2, t2, t3	# const-rem.mul
	subw	s5, s5, t2	# const-rem.sub
	sw	s5, 0(s4)
	addw	t1, t1, s5
	addi	s4, s4, 4
	blt	t0, s3, .main.bb17	# rotated-loop-branch
.main.bb18:
	li	t0, 1
	subw	t0, t0, s1
	mulw	t0, t0, t1
	blt	s1, zero, .main.bb31
.main.bb85:
	mv	s1, t0	# phi-copy
	j	.main.bb30
.main.bb31:
	li	t0, 0
.main.bb19:
	bge	t0, s1, .main.bb21
	j	.main.bb20
	.size main, .-main
