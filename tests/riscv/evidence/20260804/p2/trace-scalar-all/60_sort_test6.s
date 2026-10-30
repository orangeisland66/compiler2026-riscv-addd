
  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .text

  .globl counting_sort
  .type  counting_sort, @function
counting_sort:
.counting_sort.bb0:
	addi	sp, sp, -80
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	mv	s1, a0
	mv	t0, a1
	mv	s2, a2
	li	t1, 0
	addi	t2, sp, 0
	sw	t1, 0(t2)
	sw	t1, 4(t2)
	sw	t1, 8(t2)
	sw	t1, 12(t2)
	sw	t1, 16(t2)
	sw	t1, 20(t2)
	sw	t1, 24(t2)
	sw	t1, 28(t2)
	sw	t1, 32(t2)
	sw	t1, 36(t2)
.counting_sort.bb1:
	bge	t1, s2, .counting_sort.bb3
.counting_sort.bb16:
	addiw	t2, t1, 15
	blt	t2, s2, .counting_sort.bb17
.counting_sort.bb2:
	slliw	t2, t1, 2
	addi	t3, sp, 0
	add	t2, s1, t2
	addiw	t1, t1, 1
	lw	t2, 0(t2)
	slliw	t2, t2, 2
	add	s3, t3, t2
	lw	t2, 0(s3)
	addiw	t2, t2, 1
	sw	t2, 0(s3)
	j	.counting_sort.bb1
.counting_sort.bb4:
	bge	t1, t3, .counting_sort.bb10
.counting_sort.bb18:
	addiw	a2, t1, 15
	blt	a2, t3, .counting_sort.bb19
.counting_sort.bb5:
	slliw	a2, t1, 2
	addi	a3, sp, 0
	add	s3, a3, a2
	addiw	t1, t1, 1
	lw	a2, 0(s3)
	lw	a3, -4(s3)
	addw	a2, a2, a3
	sw	a2, 0(s3)
	j	.counting_sort.bb4
.counting_sort.bb19:
	slliw	a2, t1, 2
	addi	a3, sp, 0
	add	s3, a3, a2
	addiw	t1, t1, 16
	lw	a2, 0(s3)
	lw	a4, -4(s3)
	addw	a2, a2, a4
	sw	a2, 0(s3)
	lw	a4, 0(s3)
	lw	a3, 4(s3)
	addw	a3, a3, a4
	sw	a3, 4(s3)
	lw	a3, 8(s3)
	lw	a4, 4(s3)
	addw	a3, a3, a4
	sw	a3, 8(s3)
	lw	a3, 12(s3)
	lw	a4, 8(s3)
	addw	a3, a3, a4
	sw	a3, 12(s3)
	lw	a3, 16(s3)
	lw	a4, 12(s3)
	addw	a3, a3, a4
	sw	a3, 16(s3)
	lw	a3, 20(s3)
	lw	a4, 16(s3)
	addw	a3, a3, a4
	sw	a3, 20(s3)
	lw	a3, 24(s3)
	lw	a4, 20(s3)
	addw	a3, a3, a4
	sw	a3, 24(s3)
	lw	a3, 28(s3)
	lw	a4, 24(s3)
	addw	a3, a3, a4
	sw	a3, 28(s3)
	lw	a3, 32(s3)
	lw	a4, 28(s3)
	addw	a3, a3, a4
	sw	a3, 32(s3)
	lw	a3, 36(s3)
	lw	a4, 32(s3)
	addw	a3, a3, a4
	sw	a3, 36(s3)
	lw	a3, 40(s3)
	lw	a4, 36(s3)
	addw	a3, a3, a4
	sw	a3, 40(s3)
	lw	a3, 44(s3)
	lw	a4, 40(s3)
	addw	a3, a3, a4
	sw	a3, 44(s3)
	lw	a3, 48(s3)
	lw	a4, 44(s3)
	addw	a3, a3, a4
	sw	a3, 48(s3)
	lw	a3, 52(s3)
	lw	a4, 48(s3)
	addw	a3, a3, a4
	sw	a3, 52(s3)
	lw	a3, 56(s3)
	lw	a4, 52(s3)
	addw	a3, a3, a4
	sw	a3, 56(s3)
	lw	a3, 60(s3)
	lw	a2, 56(s3)
	addw	a2, a3, a2
	sw	a2, 60(s3)
	j	.counting_sort.bb4
.counting_sort.bb15:
.counting_sort.bb10:
	addiw	s3, t1, 8
	bge	t1, t2, .counting_sort.bb6
.counting_sort.bb11:
	li	t3, 10
	bge	s3, t3, .counting_sort.bb12
.counting_sort.bb13:
	mv	t3, s3	# phi-copy
	j	.counting_sort.bb4
.counting_sort.bb14:
	j	.counting_sort.bb4
.counting_sort.bb17:
	slliw	t2, t1, 2
	addi	s4, sp, 0
	add	s3, s1, t2
	addiw	t1, t1, 16
	lw	t2, 0(s3)
	slliw	t2, t2, 2
	add	s5, s4, t2
	lw	t2, 0(s5)
	addiw	t2, t2, 1
	sw	t2, 0(s5)
	lw	t3, 4(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 8(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 12(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 16(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 20(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 24(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 28(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 32(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 36(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 40(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 44(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 48(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 52(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t3, 56(s3)
	slliw	t3, t3, 2
	add	s5, s4, t3
	lw	t3, 0(s5)
	addiw	t3, t3, 1
	sw	t3, 0(s5)
	lw	t2, 60(s3)
	slliw	t2, t2, 2
	add	s3, s4, t2
	lw	t2, 0(s3)
	addiw	t2, t2, 1
	sw	t2, 0(s3)
	j	.counting_sort.bb1
.counting_sort.bb12:
	li	t3, 10
	j	.counting_sort.bb4
.counting_sort.bb3:
	li	t1, 1
	li	t2, 10
	j	.counting_sort.bb10
.counting_sort.bb6:
	slliw	t1, s2, 2
	addiw	t1, t1, -4
	add	t1, s1, t1
.counting_sort.bb7:
	bge	zero, s2, .counting_sort.bb9
.counting_sort.bb8:
	lw	s1, 0(t1)
	addiw	t2, s2, -1
	addi	a2, sp, 0
	slliw	t3, s1, 2
	addi	t1, t1, -4
	add	s2, a2, t3
	lw	t3, 0(s2)
	addiw	t3, t3, -1
	sw	t3, 0(s2)
	slliw	t3, t3, 2
	add	t3, t0, t3
	sw	s1, 0(t3)
	mv	s2, t2	# phi-copy
	blt	zero, s2, .counting_sort.bb8	# rotated-loop-branch
.counting_sort.bb9:
	li	a0, 0
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	addi	sp, sp, 80
	ret
	.size counting_sort, .-counting_sort

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -128
	sd	ra, 104(sp)
	sd	s1, 112(sp)
	sd	s2, 120(sp)
	li	t0, 4
	addi	t1, sp, 24
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
	addi	t5, sp, 24
	sd	t5, 0(sp)
	addi	t5, sp, 64
	sd	t5, 8(sp)
	li	t5, 10
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	counting_sort
	mv	t0, a0
	li	s1, 10
	mv	s2, t0	# phi-copy
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	slliw	t0, s2, 2
	addi	t1, sp, 64
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
	ld	s2, 120(sp)
	ld	s1, 112(sp)
	ld	ra, 104(sp)
	addi	sp, sp, 128
	ret
	.size main, .-main
