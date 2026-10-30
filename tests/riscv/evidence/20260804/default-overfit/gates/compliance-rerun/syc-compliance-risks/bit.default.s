
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	li	t0, 0
	slt	t0, t0, s1
	li	t1, 0
	slt	t1, t1, s2
	and	t0, t0, t1
	li	t1, 1
	slt	t1, t1, s1
	li	t2, 1
	slt	t2, t2, s2
	and	t1, t1, t2
	addw	t0, t0, t1
	li	t1, 2
	slt	t1, t1, s1
	li	t2, 2
	slt	t2, t2, s2
	and	t1, t1, t2
	addw	t0, t0, t1
	li	t1, 3
	slt	t1, t1, s1
	li	t2, 3
	slt	t2, t2, s2
	and	t1, t1, t2
	addw	t0, t0, t1
	li	t1, 4
	slt	t1, t1, s1
	li	t2, 4
	slt	t2, t2, s2
	and	t1, t1, t2
	addw	t0, t0, t1
	li	t1, 5
	slt	t1, t1, s1
	li	t2, 5
	slt	t2, t2, s2
	and	t1, t1, t2
	addw	t0, t0, t1
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
	li	t2, 0
	li	t3, 8
.main.bb1:
	bge	t1, t3, .main.bb3
.main.bb2:
	li	a2, 0
	addiw	t1, t1, 1
	subw	t2, t2, a2
	sltu	t2, zero, t2
	or	t2, t2, t0
	blt	t1, t3, .main.bb2	# rotated-loop-branch
.main.bb3:
	addw	a0, t2, s2
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
