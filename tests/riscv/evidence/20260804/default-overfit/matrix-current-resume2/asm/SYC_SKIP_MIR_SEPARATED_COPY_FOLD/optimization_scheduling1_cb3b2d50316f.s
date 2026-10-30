
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
	li	t5, 40
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 0
	li	t1, 4
	li	t2, 3
	li	t3, 2
	li	a2, 1
.main.bb1:
	bge	t0, s1, .main.bb3
.main.bb2:
	addw	s2, a2, t3
	addiw	t0, t0, 1
	addw	t3, t3, t2
	addw	t2, t2, t1
	addw	t1, t1, s2
	mv	a2, s2	# phi-copy
	blt	t0, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	addw	t0, a2, t3
	addw	t0, t0, t2
	li	t3, 2
	addw	s2, t0, t1
	li	t2, 3
	li	t0, 0
	li	t1, 4
	li	a2, 1
.main.bb4:
	bge	t0, s1, .main.bb6
.main.bb5:
	addw	a3, a2, t3
	addiw	t0, t0, 1
	addw	t3, t3, t2
	addw	t2, t2, t1
	addw	t1, t1, a2
	mv	a2, a3	# phi-copy
	blt	t0, s1, .main.bb5	# rotated-loop-branch
.main.bb6:
	addw	t0, a2, t3
	addw	t0, t0, t2
	addw	s1, t0, t1
	li	a0, 43
	call	_sysy_stoptime
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	t5, s1
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
