
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
	beq	t1, zero, .main.bb6
.main.bb5:
	addiw	t5, t1, 1
	mv	t0, t1	# phi-copy
	mv	t1, t5	# phi-copy
	blt	t1, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, t0
	call	putint
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.main.bb4:
.main.bb6:
	j	.main.bb6
	.size main, .-main
