
  .bss
  .globl interchange_values
  .align 2
  .type  interchange_values, @object
  .size  interchange_values, 1024
interchange_values:
  .zero 1024

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
	li	t2, 4
	bge	t1, t2, .main.bb3
.main.bb2:
	li	t2, 0
	li	t3, 64
.main.bb4:
	bge	t2, t3, .main.bb6
.main.bb5:
	addw	a2, t1, t2
	addw	t0, t0, a2
	addiw	t2, t2, 1
	blt	t2, t3, .main.bb5	# rotated-loop-branch
.main.bb6:
	addiw	t1, t1, 1
	j	.main.bb1
.main.bb3:
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
