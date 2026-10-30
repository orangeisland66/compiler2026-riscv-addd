
  .data
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .word 7

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
	li	t1, 0
.main.bb1:
	li	t2, 100
	bge	t0, t2, .main.bb3
.main.bb2:
.main.bb4:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	li	t0, 100
	bge	t1, t0, .main.bb6
.main.bb5:
	li	a0, 1
	call	putint
.main.bb7:
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.main.bb6:
	li	a0, 0
	call	putint
	j	.main.bb7
	.size main, .-main
