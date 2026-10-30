
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 37
	li	t1, 5
	li	t2, 2
.main.bb1:
	bge	zero, t0, .main.bb2
.main.bb4:
	addiw	t0, t0, -1
	addw	t1, t1, t2
	blt	zero, t0, .main.bb4	# rotated-loop-branch
	j	.main.bb2	# rotated-loop-exit
.main.bb3:
.main.bb2:
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
	.size main, .-main
