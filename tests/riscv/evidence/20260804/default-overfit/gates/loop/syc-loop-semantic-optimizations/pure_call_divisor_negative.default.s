
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
	li	t1, 0
	li	t2, 4
.main.bb1:
	bge	t1, t2, .main.bb3
.main.bb2:
	addiw	t0, t1, -1
	divw	t0, t1, t0
	addiw	t1, t1, 1
	blt	t1, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, t0
	call	putint
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
