
  .text

  .globl add_after_recursion
  .type  add_after_recursion, @function
add_after_recursion:
.add_after_recursion.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	mv	t0, a0
	bge	zero, t0, .add_after_recursion.bb2
.add_after_recursion.bb3:
	addiw	t0, t0, -1
	bge	zero, t0, .add_after_recursion.bb5
.add_after_recursion.bb6:
	addiw	t0, t0, -1
	bge	zero, t0, .add_after_recursion.bb8
.add_after_recursion.bb9:
	addiw	t0, t0, -1
	bge	zero, t0, .add_after_recursion.bb11
.add_after_recursion.bb12:
	addiw	t0, t0, -1
	blt	zero, t0, .add_after_recursion.bb15
.add_after_recursion.bb14:
	li	t0, 0
.add_after_recursion.bb13:
	addiw	t0, t0, 1
.add_after_recursion.bb10:
	addiw	t0, t0, 1
.add_after_recursion.bb7:
	addiw	t0, t0, 1
.add_after_recursion.bb4:
	addiw	t0, t0, 1
.add_after_recursion.bb1:
	mv	a0, t0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.add_after_recursion.bb5:
	li	t0, 0
	j	.add_after_recursion.bb4
.add_after_recursion.bb8:
	li	t0, 0
	j	.add_after_recursion.bb7
.add_after_recursion.bb11:
	li	t0, 0
	j	.add_after_recursion.bb10
.add_after_recursion.bb15:
	addiw	a0, t0, -1
	call	add_after_recursion
	mv	t0, a0
	addiw	t0, t0, 1
	j	.add_after_recursion.bb13
.add_after_recursion.bb2:
	li	t0, 0
	j	.add_after_recursion.bb1
	.size add_after_recursion, .-add_after_recursion

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t5, 7
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	add_after_recursion
	mv	t0, a0
	mv	a0, t0
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
