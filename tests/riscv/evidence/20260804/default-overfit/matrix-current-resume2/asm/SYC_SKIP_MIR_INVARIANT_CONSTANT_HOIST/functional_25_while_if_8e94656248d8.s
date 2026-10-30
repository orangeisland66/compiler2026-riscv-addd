
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	li	t0, 4
.main.bb1:
	li	t1, 75
	bge	t0, t1, .main.bb3
.main.bb2:
.main.bb4:
	addiw	s1, t0, 42
	li	t0, 99
	blt	t0, s1, .main.bb8
	j	.main.bb9
.main.bb6:
.main.bb8:
	li	t0, 168
	j	.main.bb1
.main.bb7:
	j	.main.bb1
.main.bb5:
	j	.main.bb1
.main.bb9:
	mv	t0, s1	# phi-copy
	j	.main.bb1
.main.bb3:
	mv	a0, t0
	call	putint
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
