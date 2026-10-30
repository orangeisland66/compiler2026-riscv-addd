
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t0, 0
	li	t1, 3
	li	t2, 5
.main.bb1:
	bge	t0, t2, .main.bb3
.main.bb2:
	slliw	t1, t1, 1
	addiw	t0, t0, 1
	blt	t0, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, t1
	ret
	.size main, .-main
