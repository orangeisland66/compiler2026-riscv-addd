
  .bss
  .globl t
  .align 2
  .type  t, @object
  .size  t, 8040
t:
  .zero 8040

  .bss
  .globl dp
  .align 2
  .type  dp, @object
  .size  dp, 140700
dp:
  .zero 140700

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	sd	s3, 24(sp)
	sd	s4, 32(sp)
	sd	s5, 40(sp)
	sd	s6, 48(sp)
	sd	s7, 56(sp)
	sd	s8, 64(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	la	t0, dp
	addi	s5, t0, 140
	la	t1, t
	addi	s4, t1, 12
	la	s3, dp
	li	t3, 1
	mv	s6, t3	# phi-copy
.main.bb1:
	blt	s1, s6, .main.bb3
.main.bb2:
	call	getint
	mv	s7, a0
	slliw	t0, s6, 3
	srliw	s8, s7, 31	# const-div.pow2.bias-fold
	addw	t1, s7, s8
	andi	t1, t1, 1
	subw	t1, t1, s8
	slliw	t1, t1, 2
	addw	t0, t0, t1
	la	t1, t
	add	t0, t1, t0
	li	t1, 1
	sw	t1, 0(t0)
	lw	t0, 0(s3)
	lw	t1, 0(s4)
	addw	t0, t0, t1
	sw	t0, 0(s5)
	addiw	s6, s6, 1
	addi	s5, s5, 140
	addi	s4, s4, 8
	addi	s3, s3, 140
	bge	s1, s6, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 1
.main.bb4:
	blt	s1, t0, .main.bb6
.main.bb5:
	li	t3, 140
	addiw	t1, t0, -1
	mulw	t3, t0, t3
	li	t2, 140
	mulw	t1, t1, t2
	la	a2, dp
	addiw	t3, t3, 4
	add	t3, a2, t3
	slliw	t2, t0, 3
	li	a2, 1
.main.bb7:
	blt	s2, a2, .main.bb9
.main.bb8:
	slliw	a3, a2, 2
	addiw	s4, a2, 1
	addw	a3, t1, a3
	srliw	s5, s4, 31	# const-div.pow2.bias-fold
	la	a4, dp
	addw	a2, s4, s5
	add	s3, a4, a3
	lw	a3, 0(s3)
	andi	a2, a2, 1
	subw	a2, a2, s5
	la	a4, t
	slliw	a2, a2, 2
	addw	a2, t2, a2
	add	a2, a4, a2
	lw	s5, 0(a2)
	addw	s6, a3, s5
	lw	a2, -4(s3)
	addw	s3, a2, s5
	bge	s3, s6, .main.bb11
.main.bb10:
	sw	s6, 0(t3)
.main.bb12:
	addi	t3, t3, 4
	mv	a2, s4	# phi-copy
	j	.main.bb7
.main.bb11:
	sw	s3, 0(t3)
	j	.main.bb12
.main.bb9:
	addiw	t0, t0, 1
	bge	s1, t0, .main.bb5	# rotated-loop-branch
.main.bb6:
	li	t0, 140
	la	t1, dp
	mulw	t0, s1, t0
	li	t2, 0
	add	t0, t1, t0
	li	t1, 0
.main.bb13:
	blt	s2, t2, .main.bb15
.main.bb14:
	lw	s1, 0(t0)
	blt	t1, s1, .main.bb17
	j	.main.bb18
.main.bb16:
.main.bb17:
	addiw	t2, t2, 1
	addi	t0, t0, 4
	mv	t1, s1	# phi-copy
	bge	s2, t2, .main.bb14	# rotated-loop-branch
.main.bb15:
	mv	a0, t1
	ld	s8, 64(sp)
	ld	s7, 56(sp)
	ld	s6, 48(sp)
	ld	s5, 40(sp)
	ld	s4, 32(sp)
	ld	s3, 24(sp)
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 80
	ret
.main.bb18:
	mv	s1, t1	# phi-copy
	j	.main.bb17
	.size main, .-main
