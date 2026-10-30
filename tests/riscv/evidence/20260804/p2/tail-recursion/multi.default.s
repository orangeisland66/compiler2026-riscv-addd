
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 19
	li	t1, 3
.main.bb1:
	bge	zero, t0, .main.bb2
.main.bb3:
	andi	t2, t0, 1
	beq	t2, zero, .main.bb4
.main.bb5:
	addiw	t0, t0, -2
	addiw	t1, t1, 2
	j	.main.bb1
.main.bb4:
	addiw	t0, t0, -1
	addiw	t1, t1, 1
	j	.main.bb1
.main.bb2:
	mv	a0, t1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
