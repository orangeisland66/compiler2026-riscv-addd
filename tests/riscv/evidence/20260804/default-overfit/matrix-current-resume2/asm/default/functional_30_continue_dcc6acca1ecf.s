
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t0, 0
	li	t1, 0
.main.bb1:
	li	t2, 100
	bge	t1, t2, .main.bb3
.main.bb2:
	li	t2, 50
	bne	t1, t2, .main.bb5
.main.bb4:
	li	t1, 51
	j	.main.bb1
.main.bb5:
	addw	t0, t0, t1
	addiw	t1, t1, 1
	j	.main.bb1
.main.bb3:
	mv	a0, t0
	ret
	.size main, .-main
