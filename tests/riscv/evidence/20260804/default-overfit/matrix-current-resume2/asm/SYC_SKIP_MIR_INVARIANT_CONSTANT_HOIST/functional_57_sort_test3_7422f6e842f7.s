
  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .text

  .globl QuickSort
  .type  QuickSort, @function
QuickSort:
.QuickSort.bb0:
	addi	sp, sp, -112
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	sd	s3, 48(sp)
	sd	s4, 56(sp)
	sd	s5, 64(sp)
	sd	s6, 72(sp)
	sd	s7, 80(sp)
	sd	s8, 88(sp)
	sd	s9, 96(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	blt	s2, s3, .QuickSort.bb1
.QuickSort.bb2:
	li	a0, 0
	ld	s9, 96(sp)
	ld	s8, 88(sp)
	ld	s7, 80(sp)
	ld	s6, 72(sp)
	ld	s5, 64(sp)
	ld	s4, 56(sp)
	ld	s3, 48(sp)
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 112
	ret
.QuickSort.bb6:
	slliw	t1, s6, 2
	slt	s7, s5, s6
	add	s8, s1, t1
	lw	s9, 0(s8)
	slt	t1, t0, s9
	and	t1, s7, t1
	beq	t1, zero, .QuickSort.bb8
.QuickSort.bb7:
	addiw	s6, s6, -1
	j	.QuickSort.bb6
.QuickSort.bb8:
	bnez	s7, .QuickSort.bb9
.QuickSort.bb16:
	mv	t1, s5	# phi-copy
.QuickSort.bb10:
	slliw	t2, t1, 2
	add	t2, s1, t2
.QuickSort.bb11:
	lw	s7, 0(t2)
	slt	s5, t1, s6
	slt	t3, s7, s4
	and	t3, s5, t3
	beq	t3, zero, .QuickSort.bb13
.QuickSort.bb12:
	addiw	t1, t1, 1
	addi	t2, t2, 4
	j	.QuickSort.bb11
.QuickSort.bb13:
	bnez	s5, .QuickSort.bb14
.QuickSort.bb17:
	mv	t2, s6	# phi-copy
.QuickSort.bb15:
	mv	s5, t1	# phi-copy
	mv	s6, t2	# phi-copy
	blt	s5, s6, .QuickSort.bb6	# rotated-loop-branch
	j	.QuickSort.bb5	# rotated-loop-exit
.QuickSort.bb4:
	j	.QuickSort.bb6
.QuickSort.bb9:
	slliw	t1, s5, 2
	add	t1, s1, t1
	sw	s9, 0(t1)
	addiw	t1, s5, 1
	j	.QuickSort.bb10
.QuickSort.bb14:
	sw	s7, 0(s8)
	addiw	t2, s6, -1
	j	.QuickSort.bb15
.QuickSort.bb5:
	slliw	t0, s5, 2
	add	t0, s1, t0
	sw	s4, 0(t0)
	addiw	a2, s5, -1
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	call	QuickSort
	addiw	a1, s5, 1
	mv	a0, s1	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	call	QuickSort
	j	.QuickSort.bb2
.QuickSort.bb1:
	slliw	t0, s2, 2
	add	t0, s1, t0
	lw	s4, 0(t0)
	addiw	t0, s4, -1
	mv	s5, s2	# phi-copy
	mv	s6, s3	# phi-copy
.QuickSort.bb3:
	bge	s5, s6, .QuickSort.bb5
	j	.QuickSort.bb6
	.size QuickSort, .-QuickSort

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 64(sp)
	sd	s1, 72(sp)
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
	li	t5, 0
	sd	t5, 8(sp)
	li	t5, 9
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	QuickSort
	mv	t0, a0
	mv	s1, t0	# phi-copy
.main.bb1:
	li	t0, 10
	bge	s1, t0, .main.bb3
.main.bb2:
	slliw	t0, s1, 2
	addi	t1, sp, 24
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
	ld	s1, 72(sp)
	ld	ra, 64(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
