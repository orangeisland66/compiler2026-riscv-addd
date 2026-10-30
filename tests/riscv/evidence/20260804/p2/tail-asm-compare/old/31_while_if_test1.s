
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t0, 0
	li	t1, 0
	li	t2, 100
.main.bb1:
	bge	t0, t2, .main.bb3
.main.bb2:
	li	t1, 5
	beq	t0, t1, .main.bb4
.main.bb5:
	li	t1, 10
	bne	t0, t1, .main.bb8
.main.bb7:
	li	t1, 42
	j	.main.bb6
.main.bb9:
.main.bb6:
	addiw	t0, t0, 1
	blt	t0, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, t1
	ret
.main.bb4:
	li	t1, 25
	j	.main.bb6
.main.bb8:
	slliw	t1, t0, 1
	j	.main.bb6
	.size main, .-main
