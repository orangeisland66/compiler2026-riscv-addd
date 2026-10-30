
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
	li	t1, 0
.main.bb1:
	li	t2, 9
	bge	t1, t2, .main.bb3
.main.bb2:
	li	t0, 0
	li	t2, 0
.main.bb4:
	bge	t0, t1, .main.bb6
.main.bb5:
	addw	t2, t2, t0
	addiw	t0, t0, 1
	blt	t0, t1, .main.bb5	# rotated-loop-branch
.main.bb6:
	addiw	t1, t1, 1
	mv	t0, t2	# phi-copy
	j	.main.bb1
.main.bb3:
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
