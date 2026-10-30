
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t0, 0
	li	t1, 0
.main.bb1:
	li	t2, 100
	bge	t0, t2, .main.bb3
.main.bb2:
	li	t2, 50
	beq	t0, t2, .main.bb3
.main.bb5:
	addw	t1, t1, t0
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	mv	a0, t1
	ret
.main.bb4:
	j	.main.bb3
	.size main, .-main
