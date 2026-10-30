
  .bss
  .globl counter
  .align 2
  .type  counter, @object
  .size  counter, 4
counter:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	la	t0, counter
	lw	t0, 0(t0)
	j	.main.bb2
.main.bb1:
.main.bb2:
	li	t1, 1
.main.bb3:
	bge	zero, t1, .main.bb5
.main.bb4:
	addiw	t0, t0, 1
	addiw	t1, t1, -1
	blt	zero, t1, .main.bb4	# rotated-loop-branch
.main.bb5:
	li	t1, 2
.main.bb6:
	bge	zero, t1, .main.bb8
.main.bb7:
	addiw	t0, t0, 1
	addiw	t1, t1, -1
	blt	zero, t1, .main.bb7	# rotated-loop-branch
.main.bb8:
	li	t1, 3
.main.bb9:
	bge	zero, t1, .main.bb11
.main.bb10:
	addiw	t0, t0, 1
	addiw	t1, t1, -1
	blt	zero, t1, .main.bb10	# rotated-loop-branch
.main.bb11:
	li	t1, 4
	mv	s1, t0	# phi-copy
.main.bb12:
	bge	zero, t1, .main.bb14
.main.bb13:
	addiw	s1, s1, 1
	addiw	t1, t1, -1
	blt	zero, t1, .main.bb13	# rotated-loop-branch
.main.bb14:
	mv	a0, s1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	la	t0, counter
	sw	s1, 0(t0)
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
