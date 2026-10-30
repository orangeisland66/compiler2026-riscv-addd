
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	li	t0, 3
	addi	t1, sp, 8
	sw	t0, 0(t1)
	li	t0, 4
	sw	t0, 4(t1)
	li	t0, 5
	sw	t0, 8(t1)
	li	t0, 6
	sw	t0, 12(t1)
	call	getint
	mv	s1, a0
	call	getint
	mv	t0, a0
.main.bb1:
	beq	s1, zero, .main.bb2
.main.bb4:
	blt	s1, zero, .main.bb5
.main.bb6:
	addiw	s1, s1, -1
	addiw	t0, t0, 1
	bne	s1, zero, .main.bb4	# rotated-loop-branch
	j	.main.bb2	# rotated-loop-exit
.main.bb3:
.main.bb2:
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 48
	ret
.main.bb5:
	li	t0, 7
	j	.main.bb2
	.size main, .-main
