
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
	li	t1, 4
	addi	t2, sp, 8
	sw	t1, 0(t2)
	addi	t0, sp, 8
	li	t1, 3
	sw	t1, 4(t2)
	li	t1, 9
	sw	t1, 8(t2)
	li	t1, 2
	sw	t1, 12(t2)
	li	t1, 0
	sw	t1, 16(t2)
	li	t1, 1
	sw	t1, 20(t2)
	li	t1, 6
	sw	t1, 24(t2)
	li	t1, 5
	sw	t1, 28(t2)
	li	t1, 7
	sw	t1, 32(t2)
	li	t1, 8
	sw	t1, 36(t2)
	li	t1, 0
.main.bb4:
	li	t2, 9
	bge	t1, t2, .main.bb6
.main.bb5:
	addiw	s1, t1, 1
	addi	t3, sp, 8
	slliw	t2, s1, 2
	add	t2, t3, t2
	mv	s2, s1	# phi-copy
	mv	s3, t1	# phi-copy
.main.bb7:
	li	t3, 10
	bge	s2, t3, .main.bb9
.main.bb8:
	slliw	t3, s3, 2
	addi	a2, sp, 8
	add	t3, a2, t3
	lw	t3, 0(t3)
	lw	a2, 0(t2)
	bge	a2, t3, .main.bb11
.main.bb10:
	mv	s3, s2	# phi-copy
.main.bb11:
	addiw	s2, s2, 1
	addi	t2, t2, 4
	j	.main.bb7
.main.bb14:
	j	.main.bb11
.main.bb9:
	bne	s3, t1, .main.bb12
.main.bb13:
	addi	t0, t0, 4
	mv	t1, s1	# phi-copy
	j	.main.bb4
.main.bb12:
	slliw	t1, s3, 2
	addi	t2, sp, 8
	add	s2, t2, t1
	lw	t1, 0(s2)
	lw	t2, 0(t0)
	sw	t2, 0(s2)
	sw	t1, 0(t0)
	j	.main.bb13
.main.bb6:
	li	s1, 0
.main.bb1:
	li	t0, 10
	bge	s1, t0, .main.bb3
.main.bb2:
	slliw	t0, s1, 2
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
	addiw	s1, s1, 1
	j	.main.bb1
.main.bb3:
	li	a0, 0
	ld	s3, 72(sp)
	ld	s2, 64(sp)
	ld	s1, 56(sp)
	ld	ra, 48(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
