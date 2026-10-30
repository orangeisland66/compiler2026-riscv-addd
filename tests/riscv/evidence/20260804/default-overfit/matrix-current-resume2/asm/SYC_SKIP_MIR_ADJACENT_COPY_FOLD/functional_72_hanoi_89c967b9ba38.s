
  .text

  .globl move
  .type  move, @function
move:
.move.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	mv	t0, a0
	mv	s1, a1
	mv	a0, t0
	call	putint
	li	a0, 32
	call	putch
	mv	a0, s1
	call	putint
	li	a0, 44
	call	putch
	li	a0, 32
	call	putch
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size move, .-move

  .globl hanoi
  .type  hanoi, @function
hanoi:
.hanoi.bb0:
	addi	sp, sp, -80
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	mv	t0, a0
	mv	s1, a1
	mv	s2, a2
	mv	s3, a3
	li	t1, 1
	beq	t0, t1, .hanoi.bb1
.hanoi.bb2:
	addiw	s4, t0, -1
	mv	a0, s4	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	mv	a3, s2	# call-arg-copy
	call	hanoi
	mv	a0, s1	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	move
	mv	a0, s4	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s1	# call-arg-copy
	mv	a3, s3	# call-arg-copy
	call	hanoi
.hanoi.bb3:
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 80
	ret
.hanoi.bb1:
	mv	a0, s1	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	move
	j	.hanoi.bb3
	.size hanoi, .-hanoi

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	call	getint
	mv	t0, a0
	mv	s1, t0	# phi-copy
.main.bb1:
	bge	zero, s1, .main.bb3
.main.bb2:
	call	getint
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	li	t5, 1
	sd	t5, 8(sp)
	li	t5, 2
	sd	t5, 16(sp)
	li	t5, 3
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	hanoi
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	t0, s1, -1
	mv	s1, t0	# phi-copy
	blt	zero, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 0
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
