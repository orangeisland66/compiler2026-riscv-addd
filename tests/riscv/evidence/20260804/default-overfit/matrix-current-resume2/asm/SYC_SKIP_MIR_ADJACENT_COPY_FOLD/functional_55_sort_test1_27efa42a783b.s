
  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 48(sp)
	sd	s1, 56(sp)
	sd	s2, 64(sp)
	sd	s3, 72(sp)
	li	t0, 10
	la	t1, n
	sw	t0, 0(t1)
	addi	t1, sp, 8
	li	t0, 4
	sw	t0, 0(t1)
	li	t0, 3
	sw	t0, 4(t1)
	li	t0, 9
	sw	t0, 8(t1)
	li	t0, 2
	sw	t0, 12(t1)
	li	t0, 0
	sw	t0, 16(t1)
	li	t0, 1
	sw	t0, 20(t1)
	li	t0, 6
	sw	t0, 24(t1)
	li	t0, 5
	sw	t0, 28(t1)
	li	t0, 7
	sw	t0, 32(t1)
	li	t0, 8
	sw	t0, 36(t1)
	li	t0, 0
.main.bb4:
	li	t1, 9
	bge	t0, t1, .main.bb6
.main.bb5:
	li	t1, 0
.main.bb7:
	li	t2, 10
	subw	t2, t2, t0
	addiw	t2, t2, -1
	bge	t1, t2, .main.bb9
.main.bb8:
	slliw	t2, t1, 2
	addi	t3, sp, 8
	add	s1, t3, t2
	lw	s2, 0(s1)
	lw	s3, 4(s1)
	blt	s3, s2, .main.bb10
.main.bb11:
	addiw	t1, t1, 1
	j	.main.bb7
.main.bb10:
	sw	s2, 4(s1)
	sw	s3, 0(s1)
	j	.main.bb11
.main.bb9:
	addiw	t0, t0, 1
	j	.main.bb4
.main.bb6:
	li	t0, 10
	la	t1, n
	sw	t0, 0(t1)
	li	s1, 10
	li	t0, 0
	mv	s2, t0	# phi-copy
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	slliw	t0, s2, 2
	addi	t1, sp, 8
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	t0, s2, 1
	mv	s2, t0	# phi-copy
	blt	s2, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 0
	ld	s3, 72(sp)
	ld	s2, 64(sp)
	ld	s1, 56(sp)
	ld	ra, 48(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
