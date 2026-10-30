
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
	li	t1, 0
	li	t2, 7
.main.bb2:
	bge	t0, t2, .main.bb1
.main.bb3:
	addw	t1, t1, t0
	addiw	t0, t0, 1
	blt	t0, t2, .main.bb3	# rotated-loop-branch
	j	.main.bb1	# rotated-loop-exit
.main.bb4:
.main.bb1:
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
