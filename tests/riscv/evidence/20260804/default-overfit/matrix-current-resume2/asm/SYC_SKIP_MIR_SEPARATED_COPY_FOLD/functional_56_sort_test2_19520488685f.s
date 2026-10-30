
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
	addi	t1, sp, 8
	li	t2, 3
	sw	t2, 4(t1)
	li	t0, 4
	sw	t0, 0(t1)
	li	t2, 9
	addi	t0, sp, 8
	sw	t2, 8(t1)
	addi	t0, t0, 4
	li	t2, 2
	sw	t2, 12(t1)
	li	t2, 0
	sw	t2, 16(t1)
	li	t2, 1
	sw	t2, 20(t1)
	li	t2, 6
	sw	t2, 24(t1)
	li	t2, 5
	sw	t2, 28(t1)
	li	t2, 7
	sw	t2, 32(t1)
	li	t2, 8
	sw	t2, 36(t1)
	li	t1, 1
.main.bb4:
	li	t2, 10
	bge	t1, t2, .main.bb6
.main.bb5:
	lw	s1, 0(t0)
	addiw	t2, t1, -1
.main.bb7:
	slliw	a2, t2, 2
	addi	a3, sp, 8
	add	s2, a3, a2
	li	t3, -1
	lw	s3, 0(s2)
	slt	t3, t3, t2
	slt	a2, s1, s3
	and	t3, t3, a2
	beq	t3, zero, .main.bb9
.main.bb8:
	sw	s3, 4(s2)
	addiw	t2, t2, -1
	j	.main.bb7
.main.bb9:
	addiw	t2, t2, 1
	addi	t3, sp, 8
	slliw	t2, t2, 2
	addiw	t1, t1, 1
	add	t2, t3, t2
	addi	t0, t0, 4
	sw	s1, 0(t2)
	j	.main.bb4
.main.bb6:
	li	t0, 0
	li	s1, 10
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
	addiw	s2, s2, 1
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
