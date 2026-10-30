
  .data
  .globl arr
  .align 2
  .type  arr, @object
  .size  arr, 24
arr:
  .word 1
  .word 2
  .word 33
  .word 4
  .word 5
  .word 6

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	la	t0, arr
	li	t1, 0
	li	t2, 0
	li	t3, 6
.main.bb1:
	bge	t1, t3, .main.bb3
.main.bb2:
	lw	a2, 0(t0)
	addiw	t1, t1, 1
	addw	t2, t2, a2
	addi	t0, t0, 4
	blt	t1, t3, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, t2
	ret
	.size main, .-main
