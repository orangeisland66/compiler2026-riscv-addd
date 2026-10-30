
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
	beqz	a3, .init_matrix.bb17
.init_matrix.bb16:
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
.init_matrix.bb15:
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
.init_matrix.bb17:
	li	a3, 0
.init_matrix.bb13:
	bge	a3, s3, .init_matrix.bb15
.init_matrix.bb14:
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
	blt	a3, s3, .init_matrix.bb14	# rotated-loop-branch
	j	.init_matrix.bb15	# rotated-loop-exit
.init_matrix.bb18:
	j	.init_matrix.bb15
.init_matrix.bb9:
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
.init_matrix.bb5:
	slliw	t3, s3, 2
	li	a2, 0
	add	t3, s1, t3
.init_matrix.bb10:
	bge	a2, s2, .init_matrix.bb6
.init_matrix.bb11:
	li	a3, -1
	addiw	a2, a2, 1
	sw	a3, 0(t3)
	addi	t3, t3, 4
	blt	a2, s2, .init_matrix.bb11	# rotated-loop-branch
	j	.init_matrix.bb6	# rotated-loop-exit
.init_matrix.bb12:
	j	.init_matrix.bb6
	.size init_matrix, .-init_matrix

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	call	getint
	mv	s1, a0
	la	t0, state
	sw	s1, 0(t0)
	call	getint
	mv	t0, a0
	la	t1, repeat_factor
	sw	t0, 0(t1)
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
	la	t1, N_eff
	sw	t0, 0(t1)
	li	t5, 134
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	la	t5, In
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	init_matrix
	la	t0, K
	li	t1, 0
	li	t2, 25
.main.bb1:
	bge	t1, t2, .main.bb3
.main.bb2:
	li	t3, 1431655766
	mul	t3, t1, t3	# const-div.fullmul
	srai	t3, t3, 32	# const-div.mulhi
	srliw	a2, t3, 31	# const-div.sign
	addw	t3, t3, a2	# const-div.round
	li	a2, 3
	mulw	t3, t3, a2	# const-rem.mul
	subw	t3, t1, t3	# const-rem.sub
	addiw	t3, t3, -1
	addiw	t1, t1, 1
	sw	t3, 0(t0)
	addi	t0, t0, 4
	blt	t1, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	la	t0, repeat_factor
	lw	t5, 0(t0)
	sd	t5, 8(sp)
	la	t0, N_eff
	lw	s2, 0(t0)
	li	t0, 0
.main.bb4:
	ld	t5, 8(sp)
	bge	t0, t5, .main.bb6
.main.bb5:
	li	t1, 0
.main.bb7:
	bge	t1, s2, .main.bb9
.main.bb8:
	mulw	t2, t1, s2
	la	t3, Out
	li	a2, 0
	slliw	t2, t2, 2
	add	t2, t3, t2
	li	t3, 2
	subw	s3, t3, t1
	addw	t3, s3, s2
.main.bb10:
	bge	a2, s2, .main.bb12
.main.bb11:
	li	a3, 2
	li	a4, 0
	subw	s4, a3, a2
	li	a5, 0
	addw	s5, s4, s2
	slliw	a3, a2, 2
.main.bb13:
	li	a6, 5
	bge	a5, a6, .main.bb15
.main.bb14:
	addw	a6, t1, a5
	li	a7, 5
	addiw	a6, a6, -2
	bge	s5, a7, .main.bb43
.main.bb42:
	mv	a7, s5
.main.bb44:
	blt	zero, s4, .main.bb45
.main.bb46:
	li	t4, 0
.main.bb47:
	slt	a0, a5, s3
	beq	a0, zero, .main.bb50
.main.bb49:
	mv	t4, a7
.main.bb50:
	slt	a0, a5, t3
	xori	a0, a0, 1
	beq	a0, zero, .main.bb53
.main.bb52:
	mv	t4, a7
.main.bb53:
	mulw	a6, a6, s2
	slliw	s6, t4, 2
	la	a0, In
	slliw	a1, a5, 2
	slliw	a6, a6, 2
	addiw	a6, a6, -8
	addw	a6, a6, a3
	addw	a6, a6, s6
	add	a6, a0, a6
	slliw	a0, a5, 4
	addw	a0, a0, a1
	addw	a0, a0, s6
	la	a1, K
	add	a0, a1, a0
.main.bb16:
	bge	t4, a7, .main.bb18
.main.bb17:
	lw	a1, 0(a6)
	lw	s1, 0(a0)
	mulw	a1, a1, s1
	addw	a4, a4, a1
.main.bb19:
	addiw	t4, t4, 1
	addi	a6, a6, 4
	addi	a0, a0, 4
	j	.main.bb16
.main.bb18:
	addiw	a5, a5, 1
	j	.main.bb13
.main.bb45:
	mv	t4, s4
	j	.main.bb47
.main.bb48:
	j	.main.bb50
.main.bb51:
	j	.main.bb53
.main.bb43:
	li	a7, 5
	j	.main.bb44
.main.bb15:
	sw	a4, 0(t2)
	addiw	a2, a2, 1
	addi	t2, t2, 4
	j	.main.bb10
.main.bb12:
	addiw	t1, t1, 1
	j	.main.bb7
.main.bb30:
	lw	a2, 0(t3)
	addiw	t1, t1, 1
	subw	a2, a2, t2
	sw	a2, 0(t3)
	addi	t3, t3, 4
	blt	t1, s5, .main.bb30	# rotated-loop-branch
	j	.main.bb36	# rotated-loop-exit
.main.bb41:
.main.bb36:
	addiw	s5, t1, 8
	bge	t1, s2, .main.bb31
.main.bb37:
	bge	s5, s2, .main.bb38
.main.bb39:
.main.bb40:
	slliw	t3, t1, 2
	la	a2, Out
	addw	t3, s4, t3
	add	t3, a2, t3
.main.bb29:
	bge	t1, s5, .main.bb36
	j	.main.bb30
.main.bb38:
	mv	s5, s2	# phi-copy
	j	.main.bb40
.main.bb9:
	addiw	t0, t0, 1
	j	.main.bb4
.main.bb31:
	addiw	t0, t0, 1
	bge	t0, s2, .main.bb25	# rotated-loop-branch
.main.bb24:
	mulw	t1, t0, s2
	li	t2, 0
	li	t3, 0
	slliw	s4, t1, 2
	la	t1, Out
	add	t1, t1, s4
.main.bb26:
	bge	t3, s2, .main.bb28
.main.bb27:
	lw	a2, 0(t1)
	addiw	t3, t3, 1
	addw	t2, t2, a2
	addi	t1, t1, 4
	blt	t3, s2, .main.bb27	# rotated-loop-branch
.main.bb28:
	li	t1, 0
	j	.main.bb36
.main.bb25:
	la	t0, N_eff
	li	t1, 0
	sw	s2, 0(t0)
	li	t0, 0
.main.bb32:
	bge	t0, s3, .main.bb55
.main.bb33:
	lw	t2, 0(s1)
	addiw	t0, t0, 1
	addw	t1, t1, t2
	addi	s1, s1, 4
	blt	t0, s3, .main.bb33	# rotated-loop-branch
.main.bb55:
	mv	s1, t1	# phi-copy
.main.bb34:
	la	t0, N_eff
	sw	s2, 0(t0)
	li	a0, 145
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
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 80
	ret
.main.bb6:
	la	t0, repeat_factor
	ld	t5, 8(sp)
	sw	t5, 0(t0)
	la	s1, Out
	mulw	s3, s2, s2
	li	t0, 0
	li	t1, 0
	mv	s4, s1	# phi-copy
.main.bb20:
	bge	t0, s3, .main.bb22
.main.bb21:
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
	blt	t0, s3, .main.bb21	# rotated-loop-branch
.main.bb22:
	la	t0, N_eff
	sw	s2, 0(t0)
	li	t0, 1
	subw	t0, t0, s2
	mulw	t0, t0, t1
	blt	s2, zero, .main.bb35
.main.bb54:
	mv	s1, t0	# phi-copy
	j	.main.bb34
.main.bb35:
	li	t0, 0
.main.bb23:
	bge	t0, s2, .main.bb25
	j	.main.bb24
	.size main, .-main
