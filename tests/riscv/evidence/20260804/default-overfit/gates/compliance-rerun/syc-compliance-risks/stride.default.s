
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	li	t0, 1
	li	t1, 0
.main.bb1:
	li	t2, 8
	bge	t0, t2, .main.bb3
.main.bb12:
	addiw	s1, t0, 3
	addiw	s2, t0, 6
	slt	t2, t0, s1
	slt	t3, s1, s2
	li	a2, 8
	and	t2, t2, t3
	slt	a2, s1, a2
	and	t2, t2, a2
	bnez	t2, .main.bb13
.main.bb2:
	slliw	t2, t0, 1
	addw	t0, t2, t0
	addiw	t0, t0, 1
	addw	t1, t1, t0
	mv	t0, s1	# phi-copy
	j	.main.bb1
.main.bb13:
	slliw	t2, t0, 1
	addw	t0, t2, t0
	addiw	t0, t0, 1
	addw	t0, t1, t0
	slliw	t1, s1, 1
	addw	t1, t1, s1
	addiw	t1, t1, 1
	addw	t1, t0, t1
	mv	t0, s2	# phi-copy
	j	.main.bb1
.main.bb4:
	li	t2, 10
	bge	t0, t2, .main.bb6
.main.bb14:
	addiw	s1, t0, 4
	addiw	s2, t0, 8
	slt	t2, t0, s1
	slt	t3, s1, s2
	li	a2, 10
	and	t2, t2, t3
	slt	a2, s1, a2
	and	t2, t2, a2
	bnez	t2, .main.bb15
.main.bb5:
	slliw	t2, t0, 1
	addw	t0, t2, t0
	addiw	t0, t0, 1
	addw	t1, t1, t0
	mv	t0, s1	# phi-copy
	j	.main.bb4
.main.bb15:
	slliw	t2, t0, 1
	addw	t0, t2, t0
	addiw	t0, t0, 1
	addw	t0, t1, t0
	slliw	t1, s1, 1
	addw	t1, t1, s1
	addiw	t1, t1, 1
	addw	t1, t0, t1
	mv	t0, s2	# phi-copy
	j	.main.bb4
.main.bb9:
	li	t2, 1
	bge	t0, t2, .main.bb11
.main.bb17:
	addiw	s1, t0, -1
	addiw	s2, t0, -2
	slt	t2, t0, s1
	slt	t3, s1, s2
	and	t2, t2, t3
	bnez	t2, .main.bb18
.main.bb10:
	slliw	t2, t0, 1
	addw	t0, t2, t0
	addiw	t0, t0, 1
	addw	t1, t1, t0
	mv	t0, s1	# phi-copy
	j	.main.bb9
.main.bb18:
	slliw	t2, t0, 1
	addw	t0, t2, t0
	addiw	t0, t0, 1
	addw	t0, t1, t0
	slliw	t1, s1, 1
	addw	t1, t1, s1
	addiw	t1, t1, 1
	addw	t1, t0, t1
	mv	t0, s2	# phi-copy
	j	.main.bb9
.main.bb3:
	mv	t5, t1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 2
	li	t1, 0
	j	.main.bb4
.main.bb6:
	mv	t5, t1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 10
	li	t1, 0
	li	t2, 1
.main.bb7:
	bge	t0, t2, .main.bb8
.main.bb16:
	slliw	t3, t0, 1
	addw	t3, t3, t0
	addiw	t3, t3, 1
	addw	t1, t1, t3
	blt	t0, t2, .main.bb16	# rotated-loop-branch
.main.bb8:
	mv	t5, t1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 10
	li	t1, 0
	j	.main.bb9
.main.bb11:
	mv	a0, t1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 2147483643
	sd	t5, 0(sp)
	ld	a0, 0(sp)
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
