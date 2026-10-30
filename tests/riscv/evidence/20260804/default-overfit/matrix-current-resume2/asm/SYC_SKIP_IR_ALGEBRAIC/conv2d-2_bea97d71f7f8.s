
  .section .rodata
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .word 2048

  .section .rodata
  .globl KSIZE
  .align 2
  .type  KSIZE, @object
  .size  KSIZE, 4
KSIZE:
  .word 5

  .bss
  .globl state
  .align 2
  .type  state, @object
  .size  state, 4
state:
  .zero 4

  .bss
  .globl repeat_factor
  .align 2
  .type  repeat_factor, @object
  .size  repeat_factor, 4
repeat_factor:
  .zero 4

  .bss
  .globl N_eff
  .align 2
  .type  N_eff, @object
  .size  N_eff, 4
N_eff:
  .zero 4

  .bss
  .globl In
  .align 2
  .type  In, @object
  .size  In, 16777216
In:
  .zero 16777216

  .bss
  .globl Out
  .align 2
  .type  Out, @object
  .size  Out, 16777216
Out:
  .zero 16777216

  .bss
  .globl K
  .align 2
  .type  K, @object
  .size  K, 100
K:
  .zero 100

  .text

  .globl init_matrix
  .type  init_matrix, @function
init_matrix:
.init_matrix.bb0:
	addi	sp, sp, -32
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	mv	s1, a0
	la	t0, state
	lw	t0, 0(t0)
	la	t1, N_eff
	lw	s2, 0(t1)
	srliw	t1, s2, 31	# const-div.pow2.bias-fold
	addw	t1, s2, t1
	sraiw	t1, t1, 1
	li	t2, 0
.init_matrix.bb1:
	bge	t2, s2, .init_matrix.bb3
.init_matrix.bb2:
	mulw	s3, t2, s2
	bge	t2, t1, .init_matrix.bb5
.init_matrix.bb4:
	slliw	t3, s3, 2
	li	a2, 0
	add	t3, s1, t3
.init_matrix.bb7:
	bge	a2, s2, .init_matrix.bb6
.init_matrix.bb8:
	sraiw	a3, t0, 31
	li	a4, 0
	andi	s3, a3, 2047
	slt	a4, t0, a4
	addw	a3, t0, s3
	xori	a4, a4, 1
	andi	a3, a3, 2047
	li	a5, 65535
	subw	s3, a3, s3
	slt	a5, t0, a5
	li	a3, 0
	slliw	s4, s3, 7
	slt	a3, a3, s3
	and	a3, a3, a4
	li	a4, 2147483647
	and	a3, a3, a5
	subw	a4, a4, s4
	slt	a4, a4, t0
	xori	a4, a4, 1
	and	a3, a3, a4
	beqz	a3, .init_matrix.bb19
.init_matrix.bb18:
	addw	s3, t0, s4
	li	t0, -2147450879
	mul	t0, s3, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	addw	t0, t0, s3	# const-div.fixup
	sraiw	t0, t0, 15	# const-div.shift
	srliw	a3, t0, 31	# const-div.sign
	addw	t0, t0, a3	# const-div.round
	li	a3, 65535
	mulw	t0, t0, a3	# const-rem.mul
	subw	t0, s3, t0	# const-rem.sub
.init_matrix.bb17:
	li	a3, -2147450879
	mul	a3, t0, a3	# const-div.fullmul
	srai	a3, a3, 32	# const-div.mulhi
	addw	a3, a3, t0	# const-div.fixup
	sraiw	a3, a3, 15	# const-div.shift
	srliw	a4, a3, 31	# const-div.sign
	addw	a3, a3, a4	# const-div.round
	li	a4, 65535
	mulw	a3, a3, a4	# const-rem.mul
	subw	t0, t0, a3	# const-rem.sub
	sw	t0, 0(t3)
	addiw	a2, a2, 1
	addi	t3, t3, 4
	j	.init_matrix.bb7
.init_matrix.bb19:
	li	a3, 0
.init_matrix.bb15:
	bge	a3, s3, .init_matrix.bb17
.init_matrix.bb16:
	addiw	s4, t0, 128
	addiw	a3, a3, 1
	li	t0, -2147450879
	mul	t0, s4, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	addw	t0, t0, s4	# const-div.fixup
	sraiw	t0, t0, 15	# const-div.shift
	srliw	a4, t0, 31	# const-div.sign
	addw	t0, t0, a4	# const-div.round
	li	a4, 65535
	mulw	t0, t0, a4	# const-rem.mul
	subw	t0, s4, t0	# const-rem.sub
	blt	a3, s3, .init_matrix.bb16	# rotated-loop-branch
	j	.init_matrix.bb17	# rotated-loop-exit
.init_matrix.bb20:
	j	.init_matrix.bb17
.init_matrix.bb10:
	bge	t3, s2, .init_matrix.bb6
.init_matrix.bb13:
	addiw	a2, t3, 3
	blt	a2, s2, .init_matrix.bb14
.init_matrix.bb11:
	addw	a2, s3, t3
	li	a3, -1
	slliw	a2, a2, 2
	addiw	t3, t3, 1
	add	a2, s1, a2
	sw	a3, 0(a2)
	j	.init_matrix.bb10
.init_matrix.bb14:
	addw	a2, s3, t3
	addiw	s4, t3, 1
	slliw	a2, a2, 2
	li	a3, -1
	add	a2, s1, a2
	addw	t3, s3, s4
	sw	a3, 0(a2)
	slliw	t3, t3, 2
	add	t3, s1, t3
	li	a2, -1
	sw	a2, 0(t3)
	addiw	s4, s4, 1
	addw	t3, s3, s4
	slliw	t3, t3, 2
	addiw	s4, s4, 1
	add	t3, s1, t3
	sw	a2, 0(t3)
	addw	t3, s3, s4
	slliw	t3, t3, 2
	add	t3, s1, t3
	sw	a2, 0(t3)
	addiw	t3, s4, 1
	j	.init_matrix.bb10
.init_matrix.bb12:
.init_matrix.bb6:
	addiw	t2, t2, 1
	blt	t2, s2, .init_matrix.bb2	# rotated-loop-branch
.init_matrix.bb3:
	la	t1, state
	sw	t0, 0(t1)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 32
	ret
.init_matrix.bb9:
	j	.init_matrix.bb6
.init_matrix.bb5:
	li	t3, 0
	j	.init_matrix.bb10
	.size init_matrix, .-init_matrix

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -208
	sd	ra, 112(sp)
	sd	s1, 120(sp)
	sd	s2, 128(sp)
	sd	s3, 136(sp)
	sd	s4, 144(sp)
	sd	s5, 152(sp)
	sd	s6, 160(sp)
	sd	s7, 168(sp)
	sd	s8, 176(sp)
	sd	s9, 184(sp)
	sd	s10, 192(sp)
	sd	s11, 200(sp)
	call	getint
	mv	s1, a0
	la	t0, state
	sw	s1, 0(t0)
	call	getint
	mv	t5, a0
	sd	t5, 8(sp)
	li	t0, 2143297521
	mul	t0, s1, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 8	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 513
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s1, t0	# const-rem.sub
	addiw	t0, t0, 64
	la	s1, N_eff
	sw	t0, 0(s1)
	li	t5, 134
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	la	t5, In
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	init_matrix
	lw	s1, 0(s1)
	li	t0, 0
	li	t1, 0
	sd	t0, 16(sp)
.main.bb1:
	ld	t5, 16(sp)
	ld	t6, 8(sp)
	bge	t5, t6, .main.bb3
.main.bb2:
	li	t1, 0
	li	t2, 0
	la	s3, In	# loop.addr.hoisted
.main.bb4:
	bge	t1, s1, .main.bb6
.main.bb5:
	mulw	t5, t1, s1
	sd	t5, 88(sp)
	slliw	a2, t5, 2
	la	t5, Out	# remat
	add	a2, t5, a2
	addiw	s5, t1, -2
	li	a3, 0
	slt	a3, s5, a3
	xori	t5, a3, 1
	sd	t5, 24(sp)
	slt	a3, s5, s1
	and	t5, t5, a3
	sd	t5, 64(sp)
	mulw	s5, s5, s1
	addiw	t5, t1, 1
	sd	t5, 32(sp)
	addiw	s9, t5, -2
	li	a3, 0
	slt	a3, s9, a3
	xori	a3, a3, 1
	slt	a4, s9, s1
	and	s10, a3, a4
	mulw	s9, s9, s1
	addiw	t5, t1, 2
	sd	t5, 56(sp)
	addiw	a3, t5, -2
	slt	a4, a3, s1
	mulw	a3, a3, s1
	addiw	a5, t1, 3
	addiw	a5, a5, -2
	slt	t5, a5, s1
	sd	t5, 96(sp)
	mulw	a5, a5, s1
	addiw	a7, t1, 4
	addiw	a7, a7, -2
	slt	t5, a7, s1
	li	a0, 0
	sd	t5, 104(sp)
	mulw	a7, a7, s1
	sd	t2, 72(sp)
	sd	a2, 48(sp)
.main.bb7:
	bge	a0, s1, .main.bb9
.main.bb70:
	ld	t5, 56(sp)
	slt	a1, t5, s1
	ld	t5, 24(sp)
	and	a1, t5, a1
	addiw	t3, a0, -2
	li	s2, 0
	slt	t0, t3, s2
	xori	s2, t0, 1
	and	t0, a1, s2
	addiw	a1, a0, 2
	slt	s6, a1, s1
	and	t0, t0, s6
	bnez	t0, .main.bb71
.main.bb8:
	ld	t5, 64(sp)
	and	t0, t5, s2
	li	s8, 0
	subw	t0, t0, s8
	sltu	t0, zero, t0
	slt	s6, t3, s1
	and	t0, t0, s6
	beq	t0, zero, .main.bb76
.main.bb36:
	addw	t0, s5, t3
	slliw	t0, t0, 2
	add	t0, s3, t0
	lw	t0, 0(t0)
	subw	t0, zero, t0
.main.bb37:
	addiw	t5, a0, 1
	sd	t5, 40(sp)
	addiw	s8, t5, -2
	li	a2, 0
	slt	a2, s8, a2
	xori	a2, a2, 1
	slt	s11, s8, s1
	addiw	a1, a1, -2
	ld	t5, 64(sp)
	andi	s7, t5, 1
	slt	t5, a1, s1
	sd	t5, 80(sp)
	and	t2, s7, t5
	bne	t2, zero, .main.bb38
.main.bb77:
.main.bb39:
	addiw	t2, a0, 3
	addiw	t2, t2, -2
	slt	s4, t2, s1
	and	a6, s7, s4
	bne	a6, zero, .main.bb40
.main.bb78:
.main.bb41:
	addiw	a6, a0, 4
	li	t4, 0
	and	a0, s10, s2
	addiw	s7, a6, -2
	subw	t4, a0, t4
	slt	a6, s7, s1
	sltu	t4, zero, t4
	and	t4, t4, s6
	bne	t4, zero, .main.bb42
.main.bb79:
.main.bb43:
	and	t4, s10, a2
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
	and	t4, t4, s11
	bne	t4, zero, .main.bb44
.main.bb80:
.main.bb45:
	andi	t4, s10, 1
	and	a0, t4, s4
	bne	a0, zero, .main.bb46
.main.bb81:
.main.bb47:
	and	t4, t4, a6
	bne	t4, zero, .main.bb48
.main.bb82:
.main.bb49:
	and	t4, a4, a2
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
	and	t4, t4, s11
	bne	t4, zero, .main.bb50
.main.bb83:
.main.bb51:
	andi	t4, a4, 1
	ld	t5, 80(sp)
	and	a0, t4, t5
	bne	a0, zero, .main.bb52
.main.bb84:
.main.bb53:
	and	t4, t4, a6
	bne	t4, zero, .main.bb54
.main.bb85:
.main.bb55:
	ld	t5, 96(sp)
	and	t4, t5, s2
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
	and	t4, t4, s6
	bne	t4, zero, .main.bb56
.main.bb86:
.main.bb57:
	ld	t5, 96(sp)
	andi	t4, t5, 1
	ld	t5, 80(sp)
	and	a0, t4, t5
	bne	a0, zero, .main.bb58
.main.bb87:
.main.bb59:
	and	t4, t4, s4
	bne	t4, zero, .main.bb60
.main.bb88:
.main.bb61:
	ld	t5, 104(sp)
	and	t4, t5, s2
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
	and	t4, t4, s6
	bne	t4, zero, .main.bb62
.main.bb89:
.main.bb63:
	ld	t5, 104(sp)
	and	t3, t5, a2
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t3, t3, s11
	bne	t3, zero, .main.bb64
.main.bb90:
.main.bb65:
	ld	t5, 104(sp)
	andi	s2, t5, 1
	and	t3, s2, s4
	bne	t3, zero, .main.bb66
.main.bb91:
.main.bb67:
	and	t2, s2, a6
	bne	t2, zero, .main.bb68
.main.bb92:
.main.bb69:
	ld	t5, 48(sp)
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
	ld	t5, 72(sp)
	addw	t0, t5, t0
	sd	t0, 72(sp)
	ld	a0, 40(sp)
	sd	t2, 48(sp)
	j	.main.bb7
.main.bb71:
	addw	t0, s5, t3
	slliw	t0, t0, 2
	add	t0, s3, t0
	lw	t0, 0(t0)
	subw	t0, zero, t0
	addw	t2, s5, a0
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addiw	s2, a0, 1
	addw	t2, s5, s2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	t0, t0, t2
	addiw	t2, t1, -1
	mulw	s4, t2, s1
	addw	t2, s4, t3
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addiw	s6, a0, -1
	addw	t2, s4, s6
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	t0, t0, t2
	addw	t2, s4, s2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s4, a1
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	t0, t0, t2
	ld	t5, 88(sp)
	addw	t2, t5, s6
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	ld	t5, 88(sp)
	addw	t2, t5, a0
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	t0, t0, t2
	ld	t5, 88(sp)
	addw	t2, t5, a1
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	ld	t5, 32(sp)
	mulw	s4, t5, s1
	addw	t2, s4, t3
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	t0, t0, t2
	addw	t2, s4, a0
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s4, s2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	t0, t0, t2
	ld	t5, 56(sp)
	mulw	s4, t5, s1
	addw	t2, s4, t3
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s4, s6
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	t0, t0, t2
	addw	t2, s4, s2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s4, a1
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	s4, t0, t2
	mulw	t0, s4, s4
	slliw	t2, s4, 1
	addw	t2, t2, s4
	addw	t0, t0, t2
	addiw	s4, t0, -7
	li	t0, 354224107
	mul	t0, s4, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 3	# const-div.shift
	srliw	t2, t0, 31	# const-div.sign
	addw	t0, t0, t2	# const-div.round
	li	t2, 97
	mulw	t0, t0, t2	# const-rem.mul
	subw	t0, s4, t0	# const-rem.sub
	ld	t5, 72(sp)
	addw	t0, t5, t0
	ld	t5, 48(sp)
	addi	t2, t5, 4
	sd	t0, 72(sp)
	mv	a0, s2	# phi-copy
	sd	t2, 48(sp)
	j	.main.bb7
.main.bb38:
	addw	t2, s5, a1
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	j	.main.bb39
.main.bb40:
	addw	a6, s5, t2
	slliw	a6, a6, 2
	add	a6, s3, a6
	lw	a6, 0(a6)
	subw	a6, zero, a6
	addw	t0, t0, a6
	j	.main.bb41
.main.bb42:
	addw	t4, s9, t3
	slliw	t4, t4, 2
	add	t4, s3, t4
	lw	t4, 0(t4)
	addw	t0, t0, t4
	j	.main.bb43
.main.bb44:
	addw	t4, s9, s8
	slliw	t4, t4, 2
	add	t4, s3, t4
	lw	t4, 0(t4)
	subw	t4, zero, t4
	addw	t0, t0, t4
	j	.main.bb45
.main.bb46:
	addw	a0, s9, t2
	slliw	a0, a0, 2
	add	a0, s3, a0
	lw	a0, 0(a0)
	addw	t0, t0, a0
	j	.main.bb47
.main.bb48:
	addw	t4, s9, s7
	slliw	t4, t4, 2
	add	t4, s3, t4
	lw	t4, 0(t4)
	subw	t4, zero, t4
	addw	t0, t0, t4
	j	.main.bb49
.main.bb50:
	addw	t4, a3, s8
	slliw	t4, t4, 2
	add	t4, s3, t4
	lw	t4, 0(t4)
	addw	t0, t0, t4
	j	.main.bb51
.main.bb52:
	addw	a0, a3, a1
	slliw	a0, a0, 2
	add	a0, s3, a0
	lw	a0, 0(a0)
	subw	a0, zero, a0
	addw	t0, t0, a0
	j	.main.bb53
.main.bb54:
	addw	t4, a3, s7
	slliw	t4, t4, 2
	add	t4, s3, t4
	lw	t4, 0(t4)
	addw	t0, t0, t4
	j	.main.bb55
.main.bb56:
	addw	t4, a5, t3
	slliw	t4, t4, 2
	add	t4, s3, t4
	lw	t4, 0(t4)
	subw	t4, zero, t4
	addw	t0, t0, t4
	j	.main.bb57
.main.bb58:
	addw	a0, a5, a1
	slliw	a0, a0, 2
	add	a0, s3, a0
	lw	a0, 0(a0)
	addw	t0, t0, a0
	j	.main.bb59
.main.bb60:
	addw	t4, a5, t2
	slliw	t4, t4, 2
	add	t4, s3, t4
	lw	t4, 0(t4)
	subw	t4, zero, t4
	addw	t0, t0, t4
	j	.main.bb61
.main.bb62:
	addw	t3, a7, t3
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	addw	t0, t0, t3
	j	.main.bb63
.main.bb64:
	addw	t3, a7, s8
	slliw	t3, t3, 2
	add	t3, s3, t3
	lw	t3, 0(t3)
	subw	t3, zero, t3
	addw	t0, t0, t3
	j	.main.bb65
.main.bb66:
	addw	t2, a7, t2
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	j	.main.bb67
.main.bb68:
	addw	t2, a7, s7
	slliw	t2, t2, 2
	add	t2, s3, t2
	lw	t2, 0(t2)
	subw	t2, zero, t2
	addw	t0, t0, t2
	j	.main.bb69
.main.bb76:
	li	t0, 0
	j	.main.bb37
.main.bb9:
	ld	t2, 72(sp)
	ld	t1, 32(sp)
	j	.main.bb4
.main.bb19:
	bge	t2, s6, .main.bb26
.main.bb34:
	addiw	t3, t2, 3
	blt	t3, s6, .main.bb35
.main.bb20:
	addw	t3, s2, t2
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	s7, s5, t3
	lw	t3, 0(s7)
	subw	t3, t3, t1
	sw	t3, 0(s7)
	j	.main.bb19
.main.bb35:
	addw	t3, s2, t2
	slliw	t3, t3, 2
	add	s7, s5, t3
	lw	t3, 0(s7)
	subw	t3, t3, t1
	sw	t3, 0(s7)
	addiw	s7, t2, 1
	addw	t2, s2, s7
	slliw	t2, t2, 2
	addiw	s7, s7, 1
	add	s8, s5, t2
	lw	t2, 0(s8)
	subw	t2, t2, t1
	sw	t2, 0(s8)
	addw	t2, s2, s7
	slliw	t2, t2, 2
	addiw	s7, s7, 1
	add	s8, s5, t2
	lw	t2, 0(s8)
	subw	t2, t2, t1
	sw	t2, 0(s8)
	addw	t2, s2, s7
	slliw	t2, t2, 2
	add	s8, s5, t2
	lw	t2, 0(s8)
	subw	t2, t2, t1
	sw	t2, 0(s8)
	addiw	t2, s7, 1
	j	.main.bb19
.main.bb31:
.main.bb26:
	addiw	s6, t2, 8
	bge	t2, s1, .main.bb21
.main.bb27:
	bge	s6, s1, .main.bb28
.main.bb29:
	j	.main.bb19
.main.bb30:
	j	.main.bb19
.main.bb28:
	mv	s6, s1	# phi-copy
	j	.main.bb19
.main.bb16:
	bge	t2, s1, .main.bb18
.main.bb32:
	addiw	t3, t2, 3
	blt	t3, s1, .main.bb33
.main.bb17:
	addw	t3, s2, t2
	la	a2, Out
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	t3, a2, t3
	lw	t3, 0(t3)
	addw	t1, t1, t3
	j	.main.bb16
.main.bb33:
	addw	t3, s2, t2
	addiw	s6, t2, 1
	slliw	t3, t3, 2
	la	s5, Out
	addw	t2, s2, s6
	add	t3, s5, t3
	lw	t3, 0(t3)
	slliw	t2, t2, 2
	add	t2, s5, t2
	addiw	s6, s6, 1
	lw	t2, 0(t2)
	addw	t1, t1, t3
	addw	t1, t1, t2
	addw	t2, s2, s6
	slliw	t2, t2, 2
	addiw	s6, s6, 1
	add	t2, s5, t2
	lw	t2, 0(t2)
	addw	t1, t1, t2
	addw	t2, s2, s6
	slliw	t2, t2, 2
	add	t2, s5, t2
	lw	t2, 0(t2)
	addw	t1, t1, t2
	addiw	t2, s6, 1
	j	.main.bb16
.main.bb18:
	li	t2, 0
	la	s5, Out	# loop.addr.hoisted
	j	.main.bb26
.main.bb6:
	ld	t5, 16(sp)
	addiw	t0, t5, 1
	mv	t1, t2	# phi-copy
	sd	t0, 16(sp)
	j	.main.bb1
.main.bb21:
	addiw	t0, t0, 1
	bge	t0, s1, .main.bb15	# rotated-loop-branch
.main.bb14:
	mulw	s2, t0, s1
	li	t1, 0
	li	t2, 0
	j	.main.bb16
.main.bb3:
	la	s3, Out
	mulw	s4, s1, s1
	li	t0, 0
	ld	t5, 8(sp)
	slt	t0, t0, t5
	li	t2, 0
	slt	t2, s1, t2
	xori	s2, t2, 1
	and	t0, t0, s2
	li	t2, 1
	subw	s5, t2, s1
	mulw	t1, s5, t1
	beqz	t0, .main.bb72
.main.bb73:
	mv	s2, t1	# phi-copy
.main.bb24:
	la	t0, N_eff
	sw	s1, 0(t0)
	li	a0, 145
	call	_sysy_stoptime
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s11, 200(sp)
	ld	s10, 192(sp)
	ld	s9, 184(sp)
	ld	s8, 176(sp)
	ld	s7, 168(sp)
	ld	s6, 160(sp)
	ld	s5, 152(sp)
	ld	s4, 144(sp)
	ld	s3, 136(sp)
	ld	s2, 128(sp)
	ld	s1, 120(sp)
	ld	ra, 112(sp)
	addi	sp, sp, 208
	ret
.main.bb15:
	la	t0, N_eff
	li	t1, 0
	sw	s1, 0(t0)
	li	t0, 0
.main.bb22:
	bge	t0, s4, .main.bb75
.main.bb23:
	lw	t2, 0(s3)
	addiw	t0, t0, 1
	addw	t1, t1, t2
	addi	s3, s3, 4
	blt	t0, s4, .main.bb23	# rotated-loop-branch
.main.bb75:
	mv	s2, t1	# phi-copy
	j	.main.bb24
.main.bb11:
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
	blt	t0, s4, .main.bb11	# rotated-loop-branch
.main.bb12:
	mulw	t0, s5, t1
	beqz	s2, .main.bb25
.main.bb74:
	mv	s2, t0	# phi-copy
	j	.main.bb24
.main.bb25:
	li	t0, 0
.main.bb13:
	bge	t0, s1, .main.bb15
	j	.main.bb14
.main.bb72:
	li	t0, 0
	li	t1, 0
	mv	s6, s3	# phi-copy
.main.bb10:
	bge	t0, s4, .main.bb12
	j	.main.bb11
	.size main, .-main
