
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
	sd	s6, 56(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	call	getint
	mv	s3, a0
	mv	s4, s2	# phi-copy
	mv	s5, s1	# phi-copy
.main.bb2:
	beq	s5, zero, .main.bb3
.main.bb4:
	blt	s5, zero, .main.bb5
.main.bb6:
	addiw	s5, s5, -1
	addiw	s4, s4, 2
	bne	s5, zero, .main.bb4	# rotated-loop-branch
.main.bb3:
	mv	t0, s4	# phi-copy
.main.bb1:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 100
	mv	s4, s1	# phi-copy
	mv	s5, s2	# phi-copy
.main.bb8:
	blt	t0, s5, .main.bb7
.main.bb10:
	beq	s4, zero, .main.bb7
.main.bb12:
	blt	s4, zero, .main.bb13
.main.bb14:
	addiw	s4, s4, -1
	addiw	s5, s5, 1
	bge	t0, s5, .main.bb10	# rotated-loop-branch
	j	.main.bb7	# rotated-loop-exit
.main.bb9:
.main.bb7:
	mv	t5, s5
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	s4, s2	# phi-copy
	mv	s5, s1	# phi-copy
	mv	s6, s3	# phi-copy
.main.bb16:
	beq	s5, zero, .main.bb17
.main.bb18:
	blt	s5, zero, .main.bb19
.main.bb20:
	addiw	s5, s5, -1
	addw	s4, s4, s6
	bne	s5, zero, .main.bb18	# rotated-loop-branch
.main.bb17:
	mv	t0, s4	# phi-copy
.main.bb15:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	s4, s2	# phi-copy
	mv	s5, s1	# phi-copy
.main.bb22:
	beq	s5, zero, .main.bb23
.main.bb24:
	blt	s5, zero, .main.bb25
.main.bb26:
	addiw	s5, s5, -1
	addiw	s4, s4, 1
	bne	s5, zero, .main.bb24	# rotated-loop-branch
.main.bb23:
	mv	t0, s4	# phi-copy
.main.bb21:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
.main.bb28:
	beq	s1, zero, .main.bb29
.main.bb30:
	blt	s1, zero, .main.bb31
.main.bb32:
	addiw	s1, s1, -1
	addiw	s2, s2, 1
	bne	s1, zero, .main.bb30	# rotated-loop-branch
.main.bb29:
	mv	t0, s2	# phi-copy
.main.bb27:
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
.main.bb5:
	li	t0, 7
	j	.main.bb1
.main.bb11:
	j	.main.bb7
.main.bb19:
	li	t0, 7
	j	.main.bb15
.main.bb25:
	li	t0, 7
	j	.main.bb21
.main.bb31:
	li	t0, 7
	j	.main.bb27
.main.bb13:
	li	s5, 7
	j	.main.bb7
	.size main, .-main
