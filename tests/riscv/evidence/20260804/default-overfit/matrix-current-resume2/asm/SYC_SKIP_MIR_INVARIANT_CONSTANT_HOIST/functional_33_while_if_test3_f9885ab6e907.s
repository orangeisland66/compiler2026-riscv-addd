
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	s1, 0(sp)
	li	t0, 4
.main.bb1:
	li	t1, 75
	bge	t0, t1, .main.bb3
.main.bb2:
.main.bb4:
	addiw	s1, t0, 42
	li	t0, 99
	bge	t0, s1, .main.bb8
.main.bb6:
	li	t0, 168
	j	.main.bb1
.main.bb7:
	j	.main.bb1
.main.bb5:
	j	.main.bb1
.main.bb8:
	mv	t0, s1	# phi-copy
	j	.main.bb1
.main.bb3:
	mv	a0, t0
	ld	s1, 0(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
