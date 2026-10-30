
  .bss
  .globl k
  .align 2
  .type  k, @object
  .size  k, 4
k:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	li	t0, 3390
	la	t1, k
	sw	t0, 0(t1)
	li	t0, 112
.main.bb1:
	li	t1, 10
	bge	t1, t0, .main.bb3
.main.bb2:
	addiw	s1, t0, -88
	li	t2, 0
	li	t0, 1000
	addiw	t1, s1, -10
	slt	t0, s1, t0
	addiw	t1, t1, 22
	subw	t0, t2, t0
	xor	t1, s1, t1
	and	t0, t1, t0
	xor	t0, s1, t0
	j	.main.bb1
.main.bb4:
	j	.main.bb1
.main.bb3:
	mv	a0, t0
	call	putint
	la	t0, k
	lw	a0, 0(t0)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
