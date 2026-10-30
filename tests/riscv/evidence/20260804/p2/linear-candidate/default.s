
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	li	t0, 131072
	li	t1, 1
	li	t2, 0
	li	t3, 0
	li	a2, 1
.main.bb2:
	bge	zero, t0, .main.bb5
.main.bb3:
	mulw	a5, t1, a2
	mulw	a6, t2, t3
	andi	a3, t0, 1
	addw	a5, a5, a6
	mulw	a4, t1, t3
	mulw	a6, t2, a2
	addw	a4, a4, a6
	addw	a5, a5, a6
	li	a6, 0
	subw	s1, a6, a3
	xor	a3, t1, a4
	and	a3, a3, s1
	xor	t1, t1, a3
	xor	a3, t2, a5
	and	a3, a3, s1
	xor	t2, t2, a3
.main.bb4:
	mulw	a3, t3, t3
	sraiw	t0, t0, 1
	mulw	t3, t3, a2
	mulw	a2, a2, a2
	addw	t3, t3, t3
	addw	a3, a3, a2
	addw	a2, t3, a2
	mv	t3, a3	# phi-copy
	blt	zero, t0, .main.bb3	# rotated-loop-branch
.main.bb5:
	addw	t0, t1, t2
	slliw	t2, t2, 1
	addw	t1, t1, t2
.main.bb1:
	addw	a0, t1, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
