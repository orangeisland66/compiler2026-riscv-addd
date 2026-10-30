
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	s1, 0(sp)
	li	t0, 0
	li	t1, 0
.main.bb1:
	li	t2, 20
	bge	t0, t2, .main.bb3
.main.bb2:
	li	t2, 0
.main.bb4:
	li	t3, 10
	bge	t2, t3, .main.bb6
.main.bb5:
	li	t3, 0
.main.bb7:
	li	a2, 5
	bge	t3, a2, .main.bb9
.main.bb8:
	li	a2, 0
.main.bb10:
	li	a3, 3
	bge	a2, a3, .main.bb12
.main.bb11:
	addiw	s1, a2, 1
	li	a3, 3
	bge	s1, a3, .main.bb13
.main.bb14:
	li	a2, 0
.main.bb21:
	li	a3, 2
	bge	a2, a3, .main.bb23
.main.bb22:
	addiw	a2, a2, 1
	j	.main.bb21
.main.bb23:
	addiw	t1, t1, 1
	mv	a2, s1	# phi-copy
	j	.main.bb10
.main.bb13:
	bne	a2, zero, .main.bb17
	j	.main.bb14
.main.bb16:
	j	.main.bb14
.main.bb15:
.main.bb17:
	addiw	a2, a2, 1
	li	a3, 3
	bge	a2, a3, .main.bb12
	j	.main.bb14
.main.bb20:
	j	.main.bb14
.main.bb18:
	j	.main.bb14
.main.bb12:
	addiw	t3, t3, 1
	j	.main.bb7
.main.bb19:
	j	.main.bb12
.main.bb9:
	addiw	t2, t2, 1
	j	.main.bb4
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	mv	a0, t1
	ld	s1, 0(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
