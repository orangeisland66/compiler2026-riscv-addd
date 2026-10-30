
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
	li	t1, 1
	li	t2, 1
	li	t3, 131072
.main.bb1:
	bge	t0, t3, .main.bb3
.main.bb2:
	addw	t5, t2, t1
	addiw	t0, t0, 1
	mv	t1, t2	# phi-copy
	mv	t2, t5	# phi-copy
	blt	t0, t3, .main.bb2	# rotated-loop-branch
.main.bb3:
	addw	a0, t2, t1
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
