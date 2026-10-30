
  .bss
  .globl state
  .align 2
  .type  state, @object
  .size  state, 4
state:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	la	t0, state
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
.main.bb1:
	bge	t1, zero, .main.bb3
.main.bb4:
	addiw	s1, t1, 3
	blt	s1, zero, .main.bb5
.main.bb2:
	addw	t2, t2, t1
	addiw	t0, t0, 1
	addiw	t1, t1, 1
	j	.main.bb1
.main.bb5:
	addw	t2, t2, t1
	addiw	t3, t1, 1
	addw	t2, t2, t3
	addiw	t0, t0, 4
	addiw	t3, t1, 2
	addw	t2, t2, t3
	addiw	t1, t1, 4
	addw	t2, t2, s1
	j	.main.bb1
.main.bb6:
	li	t3, 1
	bge	t1, t3, .main.bb8
.main.bb9:
	addiw	s1, t1, 3
	li	t3, 1
	blt	s1, t3, .main.bb10
.main.bb7:
	addw	t2, t2, t1
	addiw	t0, t0, 1
	addiw	t1, t1, 1
	j	.main.bb6
.main.bb10:
	addw	t2, t2, t1
	addiw	t3, t1, 1
	addw	t2, t2, t3
	addiw	t0, t0, 4
	addiw	t3, t1, 2
	addw	t2, t2, t3
	addiw	t1, t1, 4
	addw	t2, t2, s1
	j	.main.bb6
.main.bb11:
	li	t3, 2
	bge	t1, t3, .main.bb13
.main.bb14:
	addiw	s1, t1, 3
	li	t3, 2
	blt	s1, t3, .main.bb15
.main.bb12:
	addw	t2, t2, t1
	addiw	t0, t0, 1
	addiw	t1, t1, 1
	j	.main.bb11
.main.bb15:
	addw	t2, t2, t1
	addiw	t3, t1, 1
	addw	t2, t2, t3
	addiw	t0, t0, 4
	addiw	t3, t1, 2
	addw	t2, t2, t3
	addiw	t1, t1, 4
	addw	t2, t2, s1
	j	.main.bb11
.main.bb16:
	li	t0, 3
	bge	t1, t0, .main.bb18
.main.bb19:
	addiw	s2, t1, 3
	li	t0, 3
	blt	s2, t0, .main.bb20
.main.bb17:
	addw	t2, t2, t1
	addiw	s1, s1, 1
	addiw	t1, t1, 1
	j	.main.bb16
.main.bb20:
	addw	t0, t2, t1
	addiw	t2, t1, 1
	addw	t0, t0, t2
	addiw	t2, t1, 2
	addw	t0, t0, t2
	addiw	t1, t1, 4
	addiw	s1, s1, 4
	addw	t2, t0, s2
	j	.main.bb16
.main.bb3:
	li	t1, 0
	li	t2, 0
	j	.main.bb6
.main.bb8:
	li	t1, 0
	li	t2, 0
	j	.main.bb11
.main.bb13:
	li	t1, 0
	li	t2, 0
	mv	s1, t0	# phi-copy
	j	.main.bb16
.main.bb18:
	addw	a0, t2, s1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	la	t0, state
	sw	s1, 0(t0)
	li	a0, 0
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
