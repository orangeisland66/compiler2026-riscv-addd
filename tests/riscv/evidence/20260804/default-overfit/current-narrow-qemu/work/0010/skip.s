
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
  .globl __memo.fun.epoch
  .align 2
  .type  __memo.fun.epoch, @object
  .size  __memo.fun.epoch, 4
__memo.fun.epoch:
  .zero 4

  .bss
  .globl __memo.fun.depth
  .align 2
  .type  __memo.fun.depth, @object
  .size  __memo.fun.depth, 4
__memo.fun.depth:
  .zero 4

  .bss
  .globl __memo.fun.state_hash
  .align 2
  .type  __memo.fun.state_hash, @object
  .size  __memo.fun.state_hash, 4
__memo.fun.state_hash:
  .zero 4

  .bss
  .globl __memo.fun.stamp
  .align 2
  .type  __memo.fun.stamp, @object
  .size  __memo.fun.stamp, 8388608
__memo.fun.stamp:
  .zero 8388608

  .bss
  .globl __memo.fun.value
  .align 2
  .type  __memo.fun.value, @object
  .size  __memo.fun.value, 8388608
__memo.fun.value:
  .zero 8388608

  .bss
  .globl __memo.fun.key0
  .align 2
  .type  __memo.fun.key0, @object
  .size  __memo.fun.key0, 8388608
__memo.fun.key0:
  .zero 8388608

  .bss
  .globl __memo.fun.key1
  .align 2
  .type  __memo.fun.key1, @object
  .size  __memo.fun.key1, 8388608
__memo.fun.key1:
  .zero 8388608

  .text

  .globl fun
  .type  fun, @function
fun:
.fun.bb0:
	addi	sp, sp, -80
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	mv	s1, a0
	mv	s2, a1
	li	t0, -189347645
	addw	t0, s1, t0
	li	t1, -1640531527
	xor	t0, t0, t1
	li	t1, 2135587861
	addw	t1, s2, t1
	slliw	t2, t0, 6
	sraiw	t3, t0, 2
	addw	t1, t1, t2
	addw	t1, t1, t3
	xor	t0, t0, t1
	li	t1, 2097151
	and	t0, t0, t1
	slliw	t0, t0, 2
	la	t1, __memo.fun.stamp
	add	s3, t1, t0
	lw	t1, 0(s3)
	la	t2, __memo.fun.depth
	lw	s4, 0(t2)
	la	t2, __memo.fun.epoch
	lw	t2, 0(t2)
	bne	s4, zero, .fun.bb12
	j	.fun.bb11
.fun.bb10:
.fun.bb12:
	subw	t1, t1, t2
	addiw	t3, s4, 1
	la	a2, __memo.fun.depth
	la	t2, __memo.fun.key0
	sw	t3, 0(a2)
	add	s5, t2, t0
	lw	t2, 0(s5)
	sltiu	t1, t1, 1
	subw	t2, t2, s1
	sltiu	t2, t2, 1
	and	t1, t1, t2
	la	t2, __memo.fun.key1
	add	s6, t2, t0
	lw	t2, 0(s6)
	subw	t2, t2, s2
	sltiu	t2, t2, 1
	and	t1, t1, t2
	la	t2, __memo.fun.value
	add	s7, t2, t0
	bnez	t1, .fun.bb14
.fun.bb13:
	li	t0, 1
	beq	s1, t0, .fun.bb2
.fun.bb3:
	andi	t0, s1, 1
	beq	t0, zero, .fun.bb4
.fun.bb5:
	la	t2, lim
	slliw	t0, s1, 1
	lw	t2, 0(t2)
	addw	t0, t0, s1
	addiw	t1, t0, 1
	bge	t2, t1, .fun.bb6
.fun.bb7:
	slliw	t0, s1, 2
	addiw	t1, t0, 1
	blt	t2, t1, .fun.bb9
.fun.bb8:
	li	t3, -189347644
	addiw	t2, s2, 1
	addw	t0, t0, t3
	li	t3, -1640531527
	xor	t0, t0, t3
	li	t3, 2135587862
	slliw	a2, t0, 6
	addw	t3, s2, t3
	sraiw	a3, t0, 2
	addw	t3, t3, a2
	addw	t3, t3, a3
	li	a2, 1
	xor	t0, t0, t3
	li	t3, 2097151
	and	t0, t0, t3
	slliw	t0, t0, 2
	la	t3, __memo.fun.stamp
	add	t3, t3, t0
	lw	t3, 0(t3)
	subw	t3, t3, a2
	la	a2, __memo.fun.key0
	sltiu	t3, t3, 1
	add	a2, a2, t0
	lw	a2, 0(a2)
	subw	a2, a2, t1
	sltiu	a2, a2, 1
	and	t3, t3, a2
	la	a2, __memo.fun.key1
	add	a2, a2, t0
	lw	a2, 0(a2)
	subw	a2, a2, t2
	sltiu	a2, a2, 1
	and	t3, t3, a2
	la	a2, __memo.fun.value
	add	t0, a2, t0
	beqz	t3, .fun.bb23
.fun.bb22:
	lw	t0, 0(t0)
	j	.fun.bb15
.fun.bb24:
	j	.fun.bb15
.fun.bb1:
.fun.bb15:
	sw	s1, 0(s5)
	sw	s2, 0(s6)
	sw	t0, 0(s7)
	la	t1, __memo.fun.epoch
	lw	t1, 0(t1)
	sw	t1, 0(s3)
	la	t1, __memo.fun.depth
	sw	s4, 0(t1)
	mv	a0, t0
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 80
	ret
.fun.bb4:
	li	t2, -189347645
	li	t3, -1640531527
	addiw	t1, s2, 1
	sraiw	t0, s1, 1	# even-path.exact-halving
	addw	t2, t0, t2
	xor	t2, t2, t3
	li	t3, 2135587862
	slliw	a2, t2, 6
	addw	t3, s2, t3
	sraiw	a3, t2, 2
	addw	t3, t3, a2
	addw	t3, t3, a3
	li	a2, 1
	xor	t2, t2, t3
	li	t3, 2097151
	and	t2, t2, t3
	slliw	t2, t2, 2
	la	t3, __memo.fun.stamp
	add	t3, t3, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	la	a2, __memo.fun.key0
	sltiu	t3, t3, 1
	add	a2, a2, t2
	lw	a2, 0(a2)
	subw	a2, a2, t0
	sltiu	a2, a2, 1
	and	t3, t3, a2
	la	a2, __memo.fun.key1
	add	a2, a2, t2
	lw	a2, 0(a2)
	subw	a2, a2, t1
	sltiu	a2, a2, 1
	and	t3, t3, a2
	la	a2, __memo.fun.value
	add	t2, a2, t2
	beqz	t3, .fun.bb17
.fun.bb16:
	lw	t0, 0(t2)
	j	.fun.bb15
.fun.bb18:
	j	.fun.bb15
.fun.bb6:
	li	t3, -189347644
	addiw	t2, s2, 1
	addw	t0, t0, t3
	li	t3, -1640531527
	xor	t0, t0, t3
	li	t3, 2135587862
	slliw	a2, t0, 6
	addw	t3, s2, t3
	sraiw	a3, t0, 2
	addw	t3, t3, a2
	addw	t3, t3, a3
	li	a2, 1
	xor	t0, t0, t3
	li	t3, 2097151
	and	t0, t0, t3
	slliw	t0, t0, 2
	la	t3, __memo.fun.stamp
	add	t3, t3, t0
	lw	t3, 0(t3)
	subw	t3, t3, a2
	la	a2, __memo.fun.key0
	sltiu	t3, t3, 1
	add	a2, a2, t0
	lw	a2, 0(a2)
	subw	a2, a2, t1
	sltiu	a2, a2, 1
	and	t3, t3, a2
	la	a2, __memo.fun.key1
	add	a2, a2, t0
	lw	a2, 0(a2)
	subw	a2, a2, t2
	sltiu	a2, a2, 1
	and	t3, t3, a2
	la	a2, __memo.fun.value
	add	t0, a2, t0
	beqz	t3, .fun.bb20
.fun.bb19:
	lw	t0, 0(t0)
	j	.fun.bb15
.fun.bb21:
	j	.fun.bb15
.fun.bb2:
	mv	t0, s2	# phi-copy
	j	.fun.bb15
.fun.bb9:
	li	t0, 7
	j	.fun.bb15
.fun.bb17:
	mv	a0, t0	# call-arg-copy
	mv	a1, t1	# call-arg-copy
	call	fun
	mv	t0, a0
	j	.fun.bb15
.fun.bb20:
	mv	a0, t1	# call-arg-copy
	mv	a1, t2	# call-arg-copy
	call	fun
	mv	t0, a0
	j	.fun.bb15
.fun.bb23:
	mv	a0, t1	# call-arg-copy
	mv	a1, t2	# call-arg-copy
	call	fun
	mv	t0, a0
	j	.fun.bb15
.fun.bb11:
	li	t3, 0
	subw	t3, t2, t3
	sltiu	t3, t3, 1
	addw	t2, t2, t3
	la	t3, __memo.fun.epoch
	sw	t2, 0(t3)
	j	.fun.bb12
.fun.bb14:
	lw	a0, 0(s7)
	la	t1, __memo.fun.depth
	sw	s4, 0(t1)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 80
	ret
	.size fun, .-fun

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	call	getint
	mv	s1, a0
	la	t0, lim
	sw	s1, 0(t0)
	li	t5, 23
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	s3, 1
	li	s2, 0
.main.bb1:
	blt	s1, s3, .main.bb3
.main.bb2:
	li	t0, -189347645
	li	t1, -1640531527
	addw	t0, s3, t0
	li	t2, 2135587861
	xor	s4, t0, t1
	slliw	t0, s4, 6
	sraiw	t1, s4, 2
	addw	t0, t0, t2
	addw	t0, t0, t1
	li	t2, 0
	xor	t0, s4, t0
	li	t1, 2097151
	and	t0, t0, t1
	slliw	s4, t0, 2
	li	t1, 1
	la	t0, __memo.fun.stamp
	add	t0, t0, s4
	lw	t0, 0(t0)
	subw	t0, t0, t1
	la	t1, __memo.fun.key0
	sltiu	t0, t0, 1
	add	t1, t1, s4
	lw	t1, 0(t1)
	subw	t1, t1, s3
	sltiu	t1, t1, 1
	and	t0, t0, t1
	la	t1, __memo.fun.key1
	add	t1, t1, s4
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltiu	t1, t1, 1
	and	t0, t0, t1
	la	t1, __memo.fun.value
	add	t1, t1, s4
	beqz	t0, .main.bb7
.main.bb4:
	lw	t0, 0(t1)
.main.bb6:
	addw	s2, s2, t0
	li	t0, 1152921497
	mul	t0, s2, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 28	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 1000000007
	mulw	t0, t0, t1	# const-rem.mul
	subw	s2, s2, t0	# const-rem.sub
	addiw	s3, s3, 1
	bge	s1, s3, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 28
	call	_sysy_stoptime
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	a0, 0
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 64
	ret
.main.bb5:
.main.bb7:
	mv	t5, s3
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	fun
	mv	t0, a0
	j	.main.bb6
.main.bb8:
	j	.main.bb6
	.size main, .-main
