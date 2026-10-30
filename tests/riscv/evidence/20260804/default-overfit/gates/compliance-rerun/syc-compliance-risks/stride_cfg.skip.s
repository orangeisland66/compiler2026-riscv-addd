
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 1
	li	t1, 0
	li	t2, 8
.main.bb1:
	bge	t0, t2, .main.bb3
.main.bb2:
	li	t3, 5
	bge	t0, t3, .main.bb6
.main.bb5:
	addiw	t3, t0, 1
.main.bb4:
	addw	t1, t1, t3
	addiw	t0, t0, 3
	blt	t0, t2, .main.bb2	# rotated-loop-branch
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
	li	t2, 10
.main.bb7:
	bge	t0, t2, .main.bb9
.main.bb8:
	li	t3, 5
	bge	t0, t3, .main.bb12
.main.bb11:
	addiw	t3, t0, 1
.main.bb10:
	addw	t1, t1, t3
	addiw	t0, t0, 4
	blt	t0, t2, .main.bb8	# rotated-loop-branch
.main.bb9:
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
.main.bb13:
	bge	t0, t2, .main.bb15
.main.bb14:
.main.bb17:
	addiw	t3, t0, 1
.main.bb16:
	addw	t1, t1, t3
	blt	t0, t2, .main.bb17	# rotated-loop-branch
.main.bb15:
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
.main.bb18:
	bge	t0, t2, .main.bb20
.main.bb19:
.main.bb22:
	addiw	t3, t0, 1
.main.bb21:
	addw	t1, t1, t3
	addiw	t0, t0, -1
	blt	t0, t2, .main.bb22	# rotated-loop-branch
.main.bb20:
	mv	t5, t1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 2147483646
	li	t1, 0
	li	t2, 2147483647
.main.bb23:
	bge	t0, t2, .main.bb25
.main.bb24:
.main.bb27:
	li	t3, 10
	subw	t3, t3, t0
.main.bb26:
	addw	t1, t1, t3
	addiw	t0, t0, 1
	blt	t0, t2, .main.bb27	# rotated-loop-branch
.main.bb25:
	mv	a0, t1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.main.bb6:
	li	t3, 10
	subw	t3, t3, t0
	j	.main.bb4
.main.bb12:
	li	t3, 10
	subw	t3, t3, t0
	j	.main.bb10
	.size main, .-main
