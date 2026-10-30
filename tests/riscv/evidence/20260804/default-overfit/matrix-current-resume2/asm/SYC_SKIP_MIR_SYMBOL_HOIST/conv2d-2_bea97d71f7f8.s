
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
	addiw	s4, t3, 3
	blt	s4, s2, .init_matrix.bb14
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
	li	a3, -1
	slliw	a2, a2, 2
	add	a2, s1, a2
	sw	a3, 0(a2)
	addiw	a2, t3, 1
	addw	a2, s3, a2
	slliw	a2, a2, 2
	add	a2, s1, a2
	sw	a3, 0(a2)
	addiw	a2, t3, 2
	addw	a2, s3, a2
	addiw	t3, t3, 4
	slliw	a2, a2, 2
	add	a2, s1, a2
	sw	a3, 0(a2)
	addw	a2, s3, s4
	slliw	a2, a2, 2
	add	a2, s1, a2
	sw	a3, 0(a2)
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
.main.bb4:
	bge	t1, s1, .main.bb6
.main.bb5:
	mulw	s3, t1, s1
	slliw	t3, s3, 2
	la	a2, Out
	add	t3, a2, t3
	addiw	s4, t1, -2
	li	a2, 0
	slt	a2, s4, a2
	xori	t5, a2, 1
	sd	t5, 24(sp)
	slt	a2, s4, s1
	and	t5, t5, a2
	sd	t5, 32(sp)
	mulw	s4, s4, s1
	addiw	t5, t1, 1
	sd	t5, 48(sp)
	addiw	s8, t5, -2
	li	a2, 0
	slt	a2, s8, a2
	xori	a2, a2, 1
	slt	a3, s8, s1
	and	t5, a2, a3
	sd	t5, 72(sp)
	mulw	s8, s8, s1
	addiw	a2, t1, 3
	addiw	s10, a2, -2
	slt	s11, s10, s1
	mulw	s10, s10, s1
	addiw	a2, t1, 4
	addiw	a2, a2, -2
	slt	a3, a2, s1
	li	a4, 0
	mulw	a2, a2, s1
	sd	t2, 64(sp)
	sd	t3, 56(sp)
.main.bb7:
	bge	a4, s1, .main.bb9
.main.bb69:
	addiw	a5, t1, 2
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
	bnez	a6, .main.bb70
.main.bb8:
	ld	t5, 32(sp)
	and	a5, t5, t4
	li	a6, 0
	subw	a5, a5, a6
	sltu	a5, zero, a5
	slt	a6, a7, s1
	and	a5, a5, a6
	beq	a5, zero, .main.bb75
.main.bb36:
	addw	a5, s4, a7
	la	a0, In
	slliw	a5, a5, 2
	add	a5, a0, a5
	lw	a5, 0(a5)
	subw	a5, zero, a5
.main.bb37:
	addiw	t5, a4, 1
	sd	t5, 40(sp)
	addiw	a1, t5, -2
	li	s2, 0
	slt	t0, a1, s2
	xori	s2, t0, 1
	slt	t0, a1, s1
	ld	t5, 32(sp)
	andi	s5, t5, 1
	mv	a0, s5
	bne	a0, zero, .main.bb38
.main.bb76:
.main.bb39:
	addiw	a0, a4, 3
	addiw	s6, a0, -2
	slt	s7, s6, s1
	and	a0, s5, s7
	bne	a0, zero, .main.bb40
.main.bb77:
	mv	t3, a5	# phi-copy
.main.bb41:
	addiw	a5, a4, 4
	addiw	s5, a5, -2
	slt	a5, s5, s1
	ld	t5, 72(sp)
	and	a0, t5, t4
	li	t2, 0
	subw	t2, a0, t2
	sltu	t2, zero, t2
	and	t2, t2, a6
	bne	t2, zero, .main.bb42
.main.bb78:
	mv	t2, t3	# phi-copy
.main.bb43:
	ld	t5, 72(sp)
	and	t3, t5, s2
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
	and	t3, t3, t0
	bne	t3, zero, .main.bb44
.main.bb79:
.main.bb45:
	ld	t5, 72(sp)
	andi	t3, t5, 1
	and	a0, t3, s7
	bne	a0, zero, .main.bb46
.main.bb80:
.main.bb47:
	and	t3, t3, a5
	bne	t3, zero, .main.bb48
.main.bb81:
.main.bb49:
	andi	t3, s2, 1
	and	t3, t3, t0
	bne	t3, zero, .main.bb50
.main.bb82:
.main.bb51:
	addw	t3, s3, a4
	la	a0, In
	slliw	t3, t3, 2
	add	t3, a0, t3
	lw	t3, 0(t3)
	subw	s9, t2, t3
.main.bb52:
	andi	t2, a5, 1
	bne	t2, zero, .main.bb53
.main.bb83:
	mv	t2, s9	# phi-copy
.main.bb54:
	and	t3, s11, t4
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
	and	t3, t3, a6
	bne	t3, zero, .main.bb55
.main.bb84:
.main.bb56:
	andi	s9, s11, 1
	mv	t3, s9
	bne	t3, zero, .main.bb57
.main.bb85:
.main.bb58:
	and	t3, s9, s7
	bne	t3, zero, .main.bb59
.main.bb86:
.main.bb60:
	and	t3, a3, t4
	li	a4, 0
	subw	t3, t3, a4
	sltu	t3, zero, t3
	and	t3, t3, a6
	bne	t3, zero, .main.bb61
.main.bb87:
.main.bb62:
	and	t3, a3, s2
	li	a4, 0
	subw	t3, t3, a4
	sltu	t3, zero, t3
	and	t0, t3, t0
	bne	t0, zero, .main.bb63
.main.bb88:
	mv	t0, t2	# phi-copy
.main.bb64:
	andi	s2, a3, 1
	and	t2, s2, s7
	bne	t2, zero, .main.bb65
.main.bb89:
.main.bb66:
	and	t2, s2, a5
	bne	t2, zero, .main.bb67
.main.bb90:
.main.bb68:
	ld	t5, 56(sp)
	addi	t2, t5, 4
	mulw	t3, t0, t0
	slliw	a4, t0, 1
	addw	t0, a4, t0
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
	ld	a4, 40(sp)
	sd	t2, 56(sp)
	j	.main.bb7
.main.bb70:
	addw	t0, s4, a7
	slliw	t0, t0, 2
	la	s2, In
	add	t0, s2, t0
	lw	t0, 0(t0)
	addw	t2, s4, a4
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	t0, t2, t0
	addiw	s5, a4, 1
	addw	t2, s4, s5
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addiw	t2, t1, -1
	mulw	s6, t2, s1
	addw	t2, s6, a7
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addiw	s7, a4, -1
	addw	t2, s6, s7
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s6, s5
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s6, a0
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s3, s7
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s3, a4
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s3, a0
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	ld	t5, 48(sp)
	mulw	s6, t5, s1
	addw	t2, s6, a7
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s6, a4
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s6, s5
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	mulw	s6, a5, s1
	addw	t2, s6, a7
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s6, s7
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	addw	t2, s6, s5
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	addw	t2, s6, a0
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	subw	s2, t0, t2
	mulw	t0, s2, s2
	slliw	t2, s2, 1
	addw	t2, t2, s2
	addw	t0, t0, t2
	addiw	s2, t0, -7
	li	t0, 354224107
	mul	t0, s2, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 3	# const-div.shift
	srliw	t2, t0, 31	# const-div.sign
	addw	t0, t0, t2	# const-div.round
	li	t2, 97
	mulw	t0, t0, t2	# const-rem.mul
	subw	t0, s2, t0	# const-rem.sub
	ld	t5, 64(sp)
	addw	t0, t5, t0
	ld	t5, 56(sp)
	addi	t2, t5, 4
	sd	t0, 64(sp)
	mv	a4, s5	# phi-copy
	sd	t2, 56(sp)
	j	.main.bb7
.main.bb38:
	addw	a0, s4, a4
	la	s7, In
	slliw	a0, a0, 2
	add	a0, s7, a0
	lw	a0, 0(a0)
	addw	a5, a5, a0
	j	.main.bb39
.main.bb40:
	addw	a0, s4, s6
	la	t3, In
	slliw	a0, a0, 2
	add	t3, t3, a0
	lw	t3, 0(t3)
	subw	t3, a5, t3
	j	.main.bb41
.main.bb42:
	addw	t2, s8, a7
	la	a0, In
	slliw	t2, t2, 2
	add	t2, a0, t2
	lw	t2, 0(t2)
	addw	t2, t3, t2
	j	.main.bb43
.main.bb44:
	addw	t3, s8, a1
	la	a0, In
	slliw	t3, t3, 2
	add	t3, a0, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
	j	.main.bb45
.main.bb46:
	addw	a0, s8, s6
	la	s9, In
	slliw	a0, a0, 2
	add	a0, s9, a0
	lw	a0, 0(a0)
	addw	t2, t2, a0
	j	.main.bb47
.main.bb48:
	addw	t3, s8, s5
	la	a0, In
	slliw	t3, t3, 2
	add	t3, a0, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
	j	.main.bb49
.main.bb50:
	addw	t3, s3, a1
	la	a0, In
	slliw	t3, t3, 2
	add	t3, a0, t3
	lw	t3, 0(t3)
	addw	t2, t2, t3
	j	.main.bb51
.main.bb55:
	addw	t3, s10, a7
	la	a0, In
	slliw	t3, t3, 2
	add	t3, a0, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
	j	.main.bb56
.main.bb57:
	addw	t3, s10, a4
	slliw	t3, t3, 2
	la	a4, In
	add	t3, a4, t3
	lw	t3, 0(t3)
	addw	t2, t2, t3
	j	.main.bb58
.main.bb59:
	addw	t3, s10, s6
	la	a4, In
	slliw	t3, t3, 2
	add	t3, a4, t3
	lw	t3, 0(t3)
	subw	t2, t2, t3
	j	.main.bb60
.main.bb61:
	addw	t3, a2, a7
	la	a4, In
	slliw	t3, t3, 2
	add	t3, a4, t3
	lw	t3, 0(t3)
	addw	t2, t2, t3
	j	.main.bb62
.main.bb63:
	addw	t0, a2, a1
	la	t3, In
	slliw	t0, t0, 2
	add	t0, t3, t0
	lw	t0, 0(t0)
	subw	t0, t2, t0
	j	.main.bb64
.main.bb65:
	addw	t2, a2, s6
	la	t3, In
	slliw	t2, t2, 2
	add	t2, t3, t2
	lw	t2, 0(t2)
	addw	t0, t0, t2
	j	.main.bb66
.main.bb67:
	addw	t2, a2, s5
	la	t3, In
	slliw	t2, t2, 2
	add	t2, t3, t2
	lw	t2, 0(t2)
	subw	t0, t0, t2
	j	.main.bb68
.main.bb53:
	addw	t2, s3, s5
	la	t3, In
	slliw	t2, t2, 2
	add	t2, t3, t2
	lw	t2, 0(t2)
	addw	t2, s9, t2
	j	.main.bb54
.main.bb75:
	li	a5, 0
	j	.main.bb37
.main.bb9:
	ld	t2, 64(sp)
	ld	t1, 48(sp)
	j	.main.bb4
.main.bb19:
	bge	t2, s5, .main.bb26
.main.bb34:
	addiw	s6, t2, 3
	blt	s6, s5, .main.bb35
.main.bb20:
	addw	t3, s2, t2
	la	a2, Out
	slliw	t3, t3, 2
	addiw	t2, t2, 1
	add	s6, a2, t3
	lw	t3, 0(s6)
	subw	t3, t3, t1
	sw	t3, 0(s6)
	j	.main.bb19
.main.bb35:
	addw	t3, s2, t2
	la	s7, Out
	slliw	t3, t3, 2
	add	s8, s7, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addiw	t3, t2, 1
	addw	t3, s2, t3
	slliw	t3, t3, 2
	add	s8, s7, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addiw	t3, t2, 2
	addw	t3, s2, t3
	addiw	t2, t2, 4
	slliw	t3, t3, 2
	add	s8, s7, t3
	lw	t3, 0(s8)
	subw	t3, t3, t1
	sw	t3, 0(s8)
	addw	t3, s2, s6
	slliw	t3, t3, 2
	add	s6, s7, t3
	lw	t3, 0(s6)
	subw	t3, t3, t1
	sw	t3, 0(s6)
	j	.main.bb19
.main.bb31:
.main.bb26:
	addiw	s5, t2, 8
	bge	t2, s1, .main.bb21
.main.bb27:
	bge	s5, s1, .main.bb28
.main.bb29:
	j	.main.bb19
.main.bb30:
	j	.main.bb19
.main.bb28:
	mv	s5, s1	# phi-copy
	j	.main.bb19
.main.bb16:
	bge	t2, s1, .main.bb18
.main.bb32:
	addiw	s5, t2, 3
	blt	s5, s1, .main.bb33
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
	la	s6, Out
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
	j	.main.bb16
.main.bb18:
	li	t2, 0
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
	beqz	t0, .main.bb71
.main.bb72:
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
.main.bb15:
	la	t0, N_eff
	li	t1, 0
	sw	s1, 0(t0)
	li	t0, 0
.main.bb22:
	bge	t0, s4, .main.bb74
.main.bb23:
	lw	t2, 0(s3)
	addiw	t0, t0, 1
	addw	t1, t1, t2
	addi	s3, s3, 4
	blt	t0, s4, .main.bb23	# rotated-loop-branch
.main.bb74:
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
.main.bb73:
	mv	s2, t0	# phi-copy
	j	.main.bb24
.main.bb25:
	li	t0, 0
.main.bb13:
	bge	t0, s1, .main.bb15
	j	.main.bb14
.main.bb71:
	li	t0, 0
	li	t1, 0
	mv	s6, s3	# phi-copy
.main.bb10:
	bge	t0, s4, .main.bb12
	j	.main.bb11
	.size main, .-main
