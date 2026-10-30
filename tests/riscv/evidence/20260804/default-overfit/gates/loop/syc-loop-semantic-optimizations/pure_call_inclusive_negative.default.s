
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	call	getint
	mv	t0, a0
	li	t1, 0
	li	t2, 0
.main.bb1:
	blt	t0, t2, .main.bb3
.main.bb2:
	addiw	t5, t2, 1
	mv	t1, t2	# phi-copy
	mv	t2, t5	# phi-copy
	bge	t0, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, t1
	call	putint
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
