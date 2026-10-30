
  .bss
  .globl k
  .align 2
  .type  k, @object
  .size  k, 4
k:
  .zero 4

  .section .rodata
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .word 10

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	li	s1, 1
	li	t1, 0
	li	t2, 9
.main.bb1:
	blt	t2, t1, .main.bb3
.main.bb2:
	addiw	t1, t1, 1
	addw	s1, s1, s1
	bge	t2, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, s1
	call	putint
	la	t0, k
	sw	s1, 0(t0)
	mv	a0, s1
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
