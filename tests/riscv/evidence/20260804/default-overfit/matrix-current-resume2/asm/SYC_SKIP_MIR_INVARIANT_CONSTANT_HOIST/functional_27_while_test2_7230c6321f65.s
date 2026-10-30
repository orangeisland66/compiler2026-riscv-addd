
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t0, 5
	li	t1, 10
	li	t2, 7
	li	t3, 6
.main.bb1:
	li	a2, 20
	bge	t0, a2, .main.bb3
.main.bb2:
.main.bb4:
	li	a2, 10
	bge	t3, a2, .main.bb6
.main.bb5:
.main.bb7:
	li	a2, 7
	bne	t2, a2, .main.bb9
.main.bb8:
.main.bb10:
	li	t2, 20
	bge	t1, t2, .main.bb12
.main.bb11:
	addiw	t1, t1, 3
	j	.main.bb10
.main.bb12:
	addiw	t1, t1, -1
	li	t2, 6
	j	.main.bb7
.main.bb9:
	addiw	t2, t2, 1
	addiw	t3, t3, 1
	j	.main.bb4
.main.bb6:
	addiw	t3, t3, -2
	addiw	t0, t0, 3
	j	.main.bb1
.main.bb3:
	addw	t1, t3, t1
	addw	t0, t0, t1
	addw	a0, t0, t2
	ret
	.size main, .-main
