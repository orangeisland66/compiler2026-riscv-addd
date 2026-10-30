
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 2
	li	t1, 37
	li	t2, 5
.main.bb1:
	bge	zero, t1, .main.bb2
.main.bb3:
	addiw	t1, t1, -1
	addw	t2, t2, t0
	blt	zero, t1, .main.bb3	# rotated-loop-branch
.main.bb2:
	mv	a0, t2
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
