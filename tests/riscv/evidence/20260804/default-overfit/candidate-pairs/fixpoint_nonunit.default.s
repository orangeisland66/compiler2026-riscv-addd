
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
	li	t1, 0
	lw	t0, 0(t0)
.main.bb1:
	bge	zero, t1, .main.bb3
.main.bb2:
	addiw	t0, t0, 1
	addiw	t1, t1, -2
	blt	zero, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t1, 1
.main.bb4:
	bge	zero, t1, .main.bb6
.main.bb5:
	addiw	t0, t0, 1
	addiw	t1, t1, -2
	blt	zero, t1, .main.bb5	# rotated-loop-branch
.main.bb6:
	li	t1, 2
.main.bb7:
	bge	zero, t1, .main.bb9
.main.bb8:
	addiw	t0, t0, 1
	addiw	t1, t1, -2
	blt	zero, t1, .main.bb8	# rotated-loop-branch
.main.bb9:
	li	t1, 3
.main.bb10:
	bge	zero, t1, .main.bb12
.main.bb11:
	addiw	t0, t0, 1
	addiw	t1, t1, -2
	blt	zero, t1, .main.bb11	# rotated-loop-branch
.main.bb12:
	li	t1, 4
	mv	s1, t0	# phi-copy
.main.bb13:
	bge	zero, t1, .main.bb15
.main.bb14:
	addiw	s1, s1, 1
	addiw	t1, t1, -2
	blt	zero, t1, .main.bb14	# rotated-loop-branch
.main.bb15:
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
