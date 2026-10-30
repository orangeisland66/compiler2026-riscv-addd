
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t0, 5
	li	t1, 7
.main.bb1:
	li	t2, 100
	bge	t0, t2, .main.bb3
.main.bb2:
.main.bb4:
	li	t2, 100
	bge	t1, t2, .main.bb6
.main.bb5:
	addiw	t1, t1, 6
	j	.main.bb4
.main.bb6:
	addiw	t1, t1, -100
	addiw	t0, t0, 30
	j	.main.bb1
.main.bb3:
	mv	a0, t1
	ret
	.size main, .-main
