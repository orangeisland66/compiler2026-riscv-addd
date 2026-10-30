
  .bss
  .globl lim
  .align 2
  .type  lim, @object
  .size  lim, 4
lim:
  .zero 4

  .section .rodata
  .globl mod
  .align 2
  .type  mod, @object
  .size  mod, 4
mod:
  .word 1000000007

  .bss
  .globl __memo.__norm_fun.epoch
  .align 2
  .type  __memo.__norm_fun.epoch, @object
  .size  __memo.__norm_fun.epoch, 4
__memo.__norm_fun.epoch:
  .zero 4

  .bss
  .globl __memo.__norm_fun.depth
  .align 2
  .type  __memo.__norm_fun.depth, @object
  .size  __memo.__norm_fun.depth, 4
__memo.__norm_fun.depth:
  .zero 4

  .bss
  .globl __memo.__norm_fun.state_hash
  .align 2
  .type  __memo.__norm_fun.state_hash, @object
  .size  __memo.__norm_fun.state_hash, 4
__memo.__norm_fun.state_hash:
  .zero 4

  .bss
  .globl __memo.__norm_fun.stamp
  .align 2
  .type  __memo.__norm_fun.stamp, @object
  .size  __memo.__norm_fun.stamp, 8388608
__memo.__norm_fun.stamp:
  .zero 8388608

  .bss
  .globl __memo.__norm_fun.value
  .align 2
  .type  __memo.__norm_fun.value, @object
  .size  __memo.__norm_fun.value, 8388608
__memo.__norm_fun.value:
  .zero 8388608

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	call	getint
	mv	s1, a0
	la	t0, lim
	sw	s1, 0(t0)
	li	t5, 23
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 1
	li	s2, 0
	mv	s3, t0	# phi-copy
.main.bb1:
	blt	s1, s3, .main.bb3
.main.bb2:
	li	t0, 0
	li	t1, 2097152
	slt	t0, s3, t0
	slt	t1, s3, t1
	xori	t0, t0, 1
	and	s4, t0, t1
	li	t1, 1
	mulw	t0, s3, s4
	slliw	s5, t0, 2
	la	t0, __memo.__norm_fun.stamp
	add	t0, t0, s5
	lw	t0, 0(t0)
	subw	t0, t0, t1
	sltiu	t0, t0, 1
	la	t1, __memo.__norm_fun.value
	add	t1, t1, s5
	and	t0, t0, s4
	beqz	t0, .main.bb6
.main.bb5:
	lw	t0, 0(t1)
.main.bb7:
	li	t1, 7
	li	t3, 1
	subw	t1, t0, t1
	subw	t2, zero, t0
	sltiu	t1, t1, 1
	addiw	t2, t2, -1
	subw	t1, t3, t1
	xor	t2, t0, t2
	li	t3, 0
	subw	t1, t3, t1
	and	t1, t2, t1
	xor	t0, t0, t1
.main.bb4:
	addw	s2, s2, t0
	li	t0, 1152921497
	mul	t0, s2, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 28	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 1000000007
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s2, t0	# const-rem.sub
	addiw	t1, s3, 1
	mv	s2, t0	# phi-copy
	mv	s3, t1	# phi-copy
	bge	s1, s3, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 28
	call	_sysy_stoptime
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	a0, 0
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
.main.bb6:
	mv	t5, s3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	__norm_fun
	mv	t0, a0
	j	.main.bb7
	.size main, .-main

  .globl __norm_fun
  .type  __norm_fun, @function
__norm_fun:
.__norm_fun.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	mv	t0, a0
	li	t1, 0
	slt	t1, t0, t1
	xori	t1, t1, 1
	li	t2, 2097152
	slt	t2, t0, t2
	and	s1, t1, t2
	mulw	t1, t0, s1
	slliw	t1, t1, 2
	la	t2, __memo.__norm_fun.stamp
	add	s2, t2, t1
	lw	t2, 0(s2)
	la	t3, __memo.__norm_fun.depth
	lw	s3, 0(t3)
	la	t3, __memo.__norm_fun.epoch
	lw	t3, 0(t3)
	bne	s3, zero, .__norm_fun.bb18
	j	.__norm_fun.bb17
.__norm_fun.bb16:
.__norm_fun.bb18:
	subw	t2, t2, t3
	addiw	a2, s3, 1
	la	a3, __memo.__norm_fun.depth
	sltiu	t2, t2, 1
	la	t3, __memo.__norm_fun.value
	sw	a2, 0(a3)
	and	t2, t2, s1
	add	s4, t3, t1
	bnez	t2, .__norm_fun.bb20
.__norm_fun.bb19:
	li	t1, 1
	beq	t0, t1, .__norm_fun.bb2
.__norm_fun.bb3:
	andi	t1, t0, 1
	beq	t1, zero, .__norm_fun.bb4
.__norm_fun.bb5:
	la	t2, lim
	slliw	t1, t0, 1
	lw	t2, 0(t2)
	addw	t1, t1, t0
	addiw	t1, t1, 1
	blt	t2, t1, .__norm_fun.bb9
.__norm_fun.bb8:
	li	t0, 0
	li	t2, 2097152
	slt	t0, t1, t0
	slt	t2, t1, t2
	xori	t0, t0, 1
	la	t3, __memo.__norm_fun.stamp
	and	t0, t0, t2
	li	a2, 1
	mulw	t2, t1, t0
	slliw	t2, t2, 2
	add	t3, t3, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltiu	t3, t3, 1
	and	t0, t3, t0
	la	t3, __memo.__norm_fun.value
	add	t2, t3, t2
	beqz	t0, .__norm_fun.bb28
.__norm_fun.bb27:
	lw	t0, 0(t2)
.__norm_fun.bb29:
	li	t1, 7
	beq	t0, t1, .__norm_fun.bb21
	j	.__norm_fun.bb11
.__norm_fun.bb10:
	j	.__norm_fun.bb21
.__norm_fun.bb1:
.__norm_fun.bb21:
	bnez	s1, .__norm_fun.bb22
.__norm_fun.bb23:
	la	t1, __memo.__norm_fun.depth
	sw	s3, 0(t1)
	mv	a0, t0
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
.__norm_fun.bb4:
	li	t2, 2097152
	la	t3, __memo.__norm_fun.stamp
	li	a2, 1
	sraiw	t0, t0, 1	# even-path.exact-halving
	li	t1, 0
	slt	t1, t0, t1
	slt	t2, t0, t2
	xori	t1, t1, 1
	and	t1, t1, t2
	mulw	t2, t0, t1
	slliw	t2, t2, 2
	add	t3, t3, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltiu	t3, t3, 1
	and	t1, t3, t1
	la	t3, __memo.__norm_fun.value
	add	t2, t3, t2
	beqz	t1, .__norm_fun.bb25
.__norm_fun.bb24:
	lw	t0, 0(t2)
.__norm_fun.bb26:
	li	t1, 7
	beq	t0, t1, .__norm_fun.bb21
	j	.__norm_fun.bb7
.__norm_fun.bb6:
	j	.__norm_fun.bb21
.__norm_fun.bb9:
	slliw	t0, t0, 2
	addiw	t0, t0, 1
	blt	t2, t0, .__norm_fun.bb13
.__norm_fun.bb12:
	li	t1, 0
	li	t2, 2097152
	slt	t1, t0, t1
	slt	t2, t0, t2
	xori	t1, t1, 1
	la	t3, __memo.__norm_fun.stamp
	and	t1, t1, t2
	li	a2, 1
	mulw	t2, t0, t1
	slliw	t2, t2, 2
	add	t3, t3, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltiu	t3, t3, 1
	and	t1, t3, t1
	la	t3, __memo.__norm_fun.value
	add	t2, t3, t2
	beqz	t1, .__norm_fun.bb31
.__norm_fun.bb30:
	lw	t0, 0(t2)
.__norm_fun.bb32:
	li	t1, 7
	beq	t0, t1, .__norm_fun.bb21
	j	.__norm_fun.bb15
.__norm_fun.bb14:
	j	.__norm_fun.bb21
.__norm_fun.bb7:
	addiw	t0, t0, -1
	j	.__norm_fun.bb21
.__norm_fun.bb11:
	addiw	t0, t0, -1
	j	.__norm_fun.bb21
.__norm_fun.bb15:
	addiw	t0, t0, -1
	j	.__norm_fun.bb21
.__norm_fun.bb2:
	li	t0, -1
	j	.__norm_fun.bb21
.__norm_fun.bb13:
	li	t0, 7
	j	.__norm_fun.bb21
.__norm_fun.bb22:
	sw	t0, 0(s4)
	la	t1, __memo.__norm_fun.epoch
	lw	t1, 0(t1)
	sw	t1, 0(s2)
	j	.__norm_fun.bb23
.__norm_fun.bb25:
	mv	a0, t0	# call-arg-copy
	call	__norm_fun
	mv	t0, a0
	j	.__norm_fun.bb26
.__norm_fun.bb28:
	mv	a0, t1	# call-arg-copy
	call	__norm_fun
	mv	t0, a0
	j	.__norm_fun.bb29
.__norm_fun.bb31:
	mv	a0, t0	# call-arg-copy
	call	__norm_fun
	mv	t0, a0
	j	.__norm_fun.bb32
.__norm_fun.bb17:
	li	a2, 0
	subw	a2, t3, a2
	sltiu	a2, a2, 1
	addw	t3, t3, a2
	la	a2, __memo.__norm_fun.epoch
	sw	t3, 0(a2)
	j	.__norm_fun.bb18
.__norm_fun.bb20:
	lw	t0, 0(s4)
	la	t1, __memo.__norm_fun.depth
	sw	s3, 0(t1)
	mv	a0, t0
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size __norm_fun, .-__norm_fun
