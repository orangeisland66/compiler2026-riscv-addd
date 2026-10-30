
  .bss
  .globl g
  .align 2
  .type  g, @object
  .size  g, 4
g:
  .zero 4

  .bss
  .globl h
  .align 2
  .type  h, @object
  .size  h, 4
h:
  .zero 4

  .bss
  .globl f
  .align 2
  .type  f, @object
  .size  f, 4
f:
  .zero 4

  .bss
  .globl e
  .align 2
  .type  e, @object
  .size  e, 4
e:
  .zero 4

  .text

  .globl EightWhile
  .type  EightWhile, @function
EightWhile:
.EightWhile.bb0:
	la	t0, e
	la	t1, g
	lw	t0, 0(t0)
	la	t2, h
	lw	t1, 0(t1)
	la	t3, f
	lw	t2, 0(t2)
	li	a2, 5
	lw	t3, 0(t3)
	li	a3, 10
	li	a4, 7
	li	a5, 6
.EightWhile.bb1:
	li	a6, 20
	bge	a2, a6, .EightWhile.bb3
.EightWhile.bb2:
.EightWhile.bb4:
	li	a6, 10
	bge	a5, a6, .EightWhile.bb6
.EightWhile.bb5:
.EightWhile.bb7:
	li	a6, 7
	bne	a4, a6, .EightWhile.bb9
.EightWhile.bb8:
.EightWhile.bb10:
	li	a4, 20
	bge	a3, a4, .EightWhile.bb12
.EightWhile.bb11:
.EightWhile.bb13:
	li	a4, 1
	bge	a4, t0, .EightWhile.bb15
.EightWhile.bb14:
	addiw	t0, t0, -1
.EightWhile.bb16:
	li	a4, 2
	bge	a4, t3, .EightWhile.bb18
.EightWhile.bb17:
	addiw	t3, t3, -2
.EightWhile.bb19:
	li	a4, 3
	bge	t1, a4, .EightWhile.bb21
.EightWhile.bb20:
	addiw	t1, t1, 10
	li	a4, 10
.EightWhile.bb22:
	bge	t2, a4, .EightWhile.bb24
.EightWhile.bb23:
	addiw	t2, t2, 8
	blt	t2, a4, .EightWhile.bb23	# rotated-loop-branch
.EightWhile.bb24:
	addiw	t2, t2, -1
	j	.EightWhile.bb19
.EightWhile.bb21:
	addiw	t1, t1, -8
	j	.EightWhile.bb16
.EightWhile.bb18:
	addiw	t3, t3, 1
	j	.EightWhile.bb13
.EightWhile.bb15:
	addiw	t0, t0, 1
	addiw	a3, a3, 3
	j	.EightWhile.bb10
.EightWhile.bb12:
	addiw	a3, a3, -1
	li	a4, 6
	j	.EightWhile.bb7
.EightWhile.bb9:
	addiw	a4, a4, 1
	addiw	a5, a5, 1
	j	.EightWhile.bb4
.EightWhile.bb6:
	addiw	a5, a5, -2
	addiw	a2, a2, 3
	j	.EightWhile.bb1
.EightWhile.bb3:
	addw	a5, a5, a3
	addw	a2, a2, a5
	addw	a2, a2, a4
	addw	a3, t0, a3
	subw	a3, a3, t1
	addw	a3, a3, t2
	subw	a2, a2, a3
	la	a3, f
	sw	t3, 0(a3)
	la	t3, h
	sw	t2, 0(t3)
	la	t2, g
	sw	t1, 0(t2)
	la	t1, e
	sw	t0, 0(t1)
	mv	a0, a2
	ret
	.size EightWhile, .-EightWhile

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 0(sp)
	li	t0, 1
	la	t1, g
	sw	t0, 0(t1)
	li	t0, 2
	la	t1, h
	sw	t0, 0(t1)
	li	t0, 4
	la	t1, e
	sw	t0, 0(t1)
	li	t0, 6
	la	t1, f
	sw	t0, 0(t1)
	call	EightWhile
	mv	t0, a0
	mv	a0, t0
	ld	ra, 0(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
