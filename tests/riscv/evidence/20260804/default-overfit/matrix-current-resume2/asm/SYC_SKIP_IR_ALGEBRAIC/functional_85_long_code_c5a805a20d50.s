
  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .text

  .globl bubblesort
  .type  bubblesort, @function
bubblesort:
.bubblesort.bb0:
	addi	sp, sp, -48
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	mv	t0, a0
	la	t1, n
	lw	s1, 0(t1)
	addiw	t1, s1, -1
	addi	t2, t0, 4
	li	t3, 0
.bubblesort.bb1:
	bge	t3, t1, .bubblesort.bb3
.bubblesort.bb2:
	subw	a2, s1, t3
	li	a3, 0
	addiw	a2, a2, -1
	mv	s2, t2	# phi-copy
	mv	s3, t0	# phi-copy
.bubblesort.bb4:
	bge	a3, a2, .bubblesort.bb6
.bubblesort.bb5:
	lw	s4, 0(s3)
	lw	s5, 0(s2)
	blt	s5, s4, .bubblesort.bb7
.bubblesort.bb8:
	addiw	a3, a3, 1
	addi	s3, s3, 4
	addi	s2, s2, 4
	j	.bubblesort.bb4
.bubblesort.bb7:
	sw	s4, 0(s2)
	sw	s5, 0(s3)
	j	.bubblesort.bb8
.bubblesort.bb6:
	addiw	t3, t3, 1
	blt	t3, t1, .bubblesort.bb2	# rotated-loop-branch
.bubblesort.bb3:
	li	a0, 0
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 48
	ret
	.size bubblesort, .-bubblesort

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

  .globl getMost
  .type  getMost, @function
getMost:
.getMost.bb0:
	li	t6, -4048
	add	sp, sp, t6
	li	t6, 4008
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 4016
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 4024
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 4032
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 4040
	add	t6, sp, t6
	sd	s5, 0(t6)
	mv	t0, a0
	la	t1, n
	lw	s1, 0(t1)
	addi	s2, sp, 8
	li	t1, 0
.getMost.bb1:
	li	t2, 1000
	bge	t1, t2, .getMost.bb3
.getMost.bb9:
	addiw	t2, t1, 3
	li	t3, 1000
	blt	t2, t3, .getMost.bb10
.getMost.bb2:
	slliw	t2, t1, 2
	li	t3, 0
	add	t2, s2, t2
	addiw	t1, t1, 1
	sw	t3, 0(t2)
	j	.getMost.bb1
.getMost.bb10:
	slliw	t2, t1, 2
	li	t3, 0
	add	s3, s2, t2
	addiw	t1, t1, 1
	li	t2, 0
	sw	t3, 4(s3)
	sw	t2, 0(s3)
	addiw	t1, t1, 1
	sw	t3, 8(s3)
	addiw	t1, t1, 1
	addiw	t1, t1, 1
	sw	t2, 12(s3)
	j	.getMost.bb1
.getMost.bb3:
	li	t1, 0
	li	t2, 0
.getMost.bb4:
	bge	t2, s1, .getMost.bb6
.getMost.bb5:
	lw	s3, 0(t0)
	slliw	t3, s3, 2
	add	s4, s2, t3
	lw	t3, 0(s4)
	addiw	s5, t3, 1
	sw	s5, 0(s4)
	bge	t1, s5, .getMost.bb11
.getMost.bb7:
	addi	t1, sp, 0
	sw	s3, 0(t1)
.getMost.bb8:
	addiw	t2, t2, 1
	addi	t0, t0, 4
	mv	t1, s5	# phi-copy
	blt	t2, s1, .getMost.bb5	# rotated-loop-branch
.getMost.bb6:
	addi	t0, sp, 0
	lw	a0, 0(t0)
	li	t6, 4040
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 4032
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 4024
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 4016
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 4008
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 4048
	add	sp, sp, t6
	ret
.getMost.bb11:
	mv	s5, t1	# phi-copy
	j	.getMost.bb8
	.size getMost, .-getMost

  .globl arrCopy
  .type  arrCopy, @function
arrCopy:
.arrCopy.bb0:
	addi	sp, sp, -48
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	mv	s1, a0
	mv	s2, a1
	la	t0, n
	lw	s3, 0(t0)
	li	t0, 0
.arrCopy.bb4:
	addiw	s4, t0, 8
	bge	t0, s3, .arrCopy.bb3
.arrCopy.bb5:
	bge	s4, s3, .arrCopy.bb6
.arrCopy.bb7:
	j	.arrCopy.bb1
.arrCopy.bb8:
.arrCopy.bb1:
	bge	t0, s4, .arrCopy.bb4
.arrCopy.bb10:
	addiw	t1, t0, 3
	blt	t1, s4, .arrCopy.bb11
.arrCopy.bb2:
	slliw	s5, t0, 2
	add	t1, s1, s5
	add	t2, s2, s5
	lw	t1, 0(t1)
	addiw	t0, t0, 1
	sw	t1, 0(t2)
	j	.arrCopy.bb1
.arrCopy.bb11:
	slliw	s5, t0, 2
	add	s6, s1, s5
	addiw	t0, t0, 1
	lw	t1, 0(s6)
	add	s5, s2, s5
	addiw	t0, t0, 1
	addiw	t0, t0, 1
	sw	t1, 0(s5)
	addiw	t0, t0, 1
	lw	t2, 4(s6)
	sw	t2, 4(s5)
	lw	t2, 8(s6)
	sw	t2, 8(s5)
	lw	t1, 12(s6)
	sw	t1, 12(s5)
	j	.arrCopy.bb1
.arrCopy.bb9:
	j	.arrCopy.bb4
.arrCopy.bb6:
	mv	s4, s3	# phi-copy
	j	.arrCopy.bb1
.arrCopy.bb3:
	li	a0, 0
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 48
	ret
	.size arrCopy, .-arrCopy

  .globl avgPooling
  .type  avgPooling, @function
avgPooling:
.avgPooling.bb0:
	addi	sp, sp, -64
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	sd	s3, 24(sp)
	sd	s4, 32(sp)
	sd	s5, 40(sp)
	sd	s6, 48(sp)
	mv	s1, a0
	mv	s2, a1
	la	t0, n
	lw	s3, 0(t0)
	addiw	s4, s2, -1
	mv	t0, s1
	slliw	t1, s2, 2
	li	t2, 3
	bne	s2, t2, .avgPooling.bb22
.avgPooling.bb21:
	addi	t1, s1, -8
	li	t2, 0
	li	t3, 0
.avgPooling.bb13:
	bge	t3, s3, .avgPooling.bb3
.avgPooling.bb14:
	li	a2, 2
	blt	t3, a2, .avgPooling.bb15
.avgPooling.bb16:
	li	a2, 2
	bne	t3, a2, .avgPooling.bb19
.avgPooling.bb18:
	lw	a2, 0(s1)
	addi	a3, sp, 0
	sw	a2, 0(a3)
	li	a2, 1431655766
	mul	a2, t2, a2	# const-div.fullmul
	srai	a2, a2, 32	# const-div.mulhi
	srliw	a3, a2, 31	# const-div.sign
	addw	a2, a2, a3	# const-div.round
	sw	a2, 0(s1)
	mv	s5, t2	# phi-copy
	j	.avgPooling.bb17
.avgPooling.bb20:
.avgPooling.bb17:
	addiw	t3, t3, 1
	addi	t0, t0, 4
	addi	t1, t1, 4
	mv	t2, s5	# phi-copy
	blt	t3, s3, .avgPooling.bb14	# rotated-loop-branch
.avgPooling.bb3:
	subw	t0, s3, s2
	addiw	t0, t0, 1
.avgPooling.bb10:
	bge	t0, s3, .avgPooling.bb12
.avgPooling.bb23:
	addiw	t1, t0, 3
	blt	t1, s3, .avgPooling.bb24
.avgPooling.bb11:
	slliw	t1, t0, 2
	li	t2, 0
	add	t1, s1, t1
	addiw	t0, t0, 1
	sw	t2, 0(t1)
	j	.avgPooling.bb10
.avgPooling.bb24:
	slliw	t1, t0, 2
	li	t2, 0
	add	s2, s1, t1
	addiw	t0, t0, 1
	li	t1, 0
	sw	t2, 4(s2)
	sw	t1, 0(s2)
	addiw	t0, t0, 1
	sw	t2, 8(s2)
	addiw	t0, t0, 1
	addiw	t0, t0, 1
	sw	t1, 12(s2)
	j	.avgPooling.bb10
.avgPooling.bb15:
	lw	a2, 0(t0)
	addw	s5, t2, a2
	j	.avgPooling.bb17
.avgPooling.bb19:
	lw	a2, 0(t0)
	addi	s4, sp, 0
	addw	t2, t2, a2
	lw	a2, 0(s4)
	subw	s5, t2, a2
	lw	t2, 0(t1)
	sw	t2, 0(s4)
	li	t2, 1431655766
	mul	t2, s5, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	srliw	a2, t2, 31	# const-div.sign
	addw	t2, t2, a2	# const-div.round
	sw	t2, 0(t1)
	j	.avgPooling.bb17
.avgPooling.bb12:
	li	a0, 0
	ld	s6, 48(sp)
	ld	s5, 40(sp)
	ld	s4, 32(sp)
	ld	s3, 24(sp)
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	addi	sp, sp, 64
	ret
.avgPooling.bb2:
	blt	t3, s4, .avgPooling.bb4
.avgPooling.bb5:
	bne	t3, s4, .avgPooling.bb8
.avgPooling.bb7:
	lw	a2, 0(s1)
	addi	a3, sp, 0
	sw	a2, 0(a3)
	divw	a2, t2, s2
	sw	a2, 0(s1)
	mv	s6, t2	# phi-copy
	j	.avgPooling.bb6
.avgPooling.bb9:
.avgPooling.bb6:
	addiw	t3, t3, 1
	addi	t0, t0, 4
	addi	t1, t1, 4
	mv	t2, s6	# phi-copy
	blt	t3, s3, .avgPooling.bb2	# rotated-loop-branch
	j	.avgPooling.bb3	# rotated-loop-exit
.avgPooling.bb4:
	lw	a2, 0(t0)
	addw	s6, t2, a2
	j	.avgPooling.bb6
.avgPooling.bb8:
	lw	a2, 0(t0)
	addi	s5, sp, 0
	addw	t2, t2, a2
	lw	a2, 0(s5)
	subw	s6, t2, a2
	lw	t2, 0(t1)
	sw	t2, 0(s5)
	divw	t2, s6, s2
	sw	t2, 0(t1)
	j	.avgPooling.bb6
.avgPooling.bb22:
	li	t2, 4
	li	t3, 0
	subw	t1, t2, t1
	add	t1, s1, t1
	li	t2, 0
.avgPooling.bb1:
	bge	t3, s3, .avgPooling.bb3
	j	.avgPooling.bb2
	.size avgPooling, .-avgPooling

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -320
	sd	ra, 280(sp)
	sd	s1, 288(sp)
	sd	s2, 296(sp)
	sd	s3, 304(sp)
	sd	s4, 312(sp)
	li	t0, 32
	la	t1, n
	sw	t0, 0(t1)
	li	t0, 7
	addi	t1, sp, 24
	sw	t0, 0(t1)
	li	t0, 23
	sw	t0, 4(t1)
	li	t0, 89
	sw	t0, 8(t1)
	li	t0, 26
	sw	t0, 12(t1)
	li	t0, 282
	sw	t0, 16(t1)
	li	t0, 254
	sw	t0, 20(t1)
	li	t0, 27
	sw	t0, 24(t1)
	li	t0, 5
	sw	t0, 28(t1)
	li	t0, 83
	sw	t0, 32(t1)
	li	t0, 273
	sw	t0, 36(t1)
	li	t0, 574
	sw	t0, 40(t1)
	li	t0, 905
	sw	t0, 44(t1)
	li	t0, 354
	sw	t0, 48(t1)
	li	t0, 657
	sw	t0, 52(t1)
	li	t0, 935
	sw	t0, 56(t1)
	li	t0, 264
	sw	t0, 60(t1)
	li	t0, 639
	sw	t0, 64(t1)
	li	t0, 459
	sw	t0, 68(t1)
	li	t0, 29
	sw	t0, 72(t1)
	li	t0, 68
	sw	t0, 76(t1)
	li	t0, 929
	sw	t0, 80(t1)
	li	t0, 756
	sw	t0, 84(t1)
	li	t0, 452
	sw	t0, 88(t1)
	li	t0, 279
	sw	t0, 92(t1)
	li	t0, 58
	sw	t0, 96(t1)
	li	t0, 87
	sw	t0, 100(t1)
	li	t0, 96
	sw	t0, 104(t1)
	li	t0, 36
	sw	t0, 108(t1)
	li	t0, 39
	sw	t0, 112(t1)
	li	t0, 28
	sw	t0, 116(t1)
	li	t0, 1
	sw	t0, 120(t1)
	li	t0, 290
	sw	t0, 124(t1)
	addi	t5, sp, 24
	sd	t5, 0(sp)
	addi	t5, sp, 152
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	arrCopy
	li	t0, 0
	li	t1, 0
.main.bb22:
	bge	t1, t0, .main.bb24
.main.bb37:
	addiw	t2, t1, 3
	blt	t2, t0, .main.bb38
.main.bb23:
	slliw	t2, t1, 2
	addi	s1, sp, 152
	add	s2, s1, t2
	slliw	t3, t0, 2
	lw	t2, 0(s2)
	add	s1, s1, t3
	lw	t3, 0(s1)
	addiw	t1, t1, 1
	addiw	t0, t0, -1
	sw	t3, 0(s2)
	sw	t2, 0(s1)
	j	.main.bb22
.main.bb38:
	slliw	t2, t1, 2
	addi	s1, sp, 152
	add	s2, s1, t2
	slliw	t3, t0, 2
	lw	t2, 0(s2)
	add	s1, s1, t3
	lw	t3, 0(s1)
	addiw	t1, t1, 1
	addiw	t0, t0, -1
	addiw	t1, t1, 1
	sw	t3, 0(s2)
	addiw	t0, t0, -1
	lw	a3, -4(s1)
	addiw	t1, t1, 1
	sw	t2, 0(s1)
	addiw	t0, t0, -1
	lw	t3, 4(s2)
	addiw	t1, t1, 1
	addiw	t0, t0, -1
	sw	a3, 4(s2)
	lw	a3, -8(s1)
	sw	t3, -4(s1)
	lw	t3, 8(s2)
	sw	a3, 8(s2)
	sw	t3, -8(s1)
	lw	t2, 12(s2)
	lw	t3, -12(s1)
	sw	t3, 12(s2)
	sw	t2, -12(s1)
	j	.main.bb22
.main.bb28:
	slliw	a2, t2, 2
	addi	a3, sp, 152
	add	s3, a3, a2
	li	t3, -1
	lw	s4, 0(s3)
	slt	t3, t3, t2
	slt	a2, s2, s4
	and	t3, t3, a2
	beq	t3, zero, .main.bb30
.main.bb29:
	sw	s4, 4(s3)
	addiw	t2, t2, -1
	j	.main.bb28
.main.bb30:
	addiw	t2, t2, 1
	addi	t3, sp, 152
	slliw	t2, t2, 2
	addiw	t1, t1, 1
	add	t2, t3, t2
	addi	t0, t0, 4
	sw	s2, 0(t2)
	bge	t1, s1, .main.bb27	# rotated-loop-branch
.main.bb26:
	lw	s2, 0(t0)
	addiw	t2, t1, -1
	j	.main.bb28
.main.bb24:
	li	s2, 0
	li	s1, 32
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	slliw	t0, s2, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	addi	t5, sp, 152
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	bubblesort
	li	s2, 0
	li	s1, 32
.main.bb4:
	bge	s2, s1, .main.bb6
.main.bb5:
	slliw	t0, s2, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb5	# rotated-loop-branch
.main.bb6:
	la	t0, n
	lw	t0, 0(t0)
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t1, t0, t1
	andi	t0, t0, 1
	bne	t0, zero, .main.bb41
.main.bb40:
	sraiw	t0, t1, 1
	slliw	t0, t0, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t1, 0(t0)
	lw	t0, -4(t0)
	addw	t0, t1, t0
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t1
	sraiw	t0, t0, 1
.main.bb39:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addi	t5, sp, 152
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getMost
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addi	t5, sp, 24
	sd	t5, 0(sp)
	addi	t5, sp, 152
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	arrCopy
	addi	t5, sp, 152
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	bubblesort
	li	s2, 0
	li	s1, 32
.main.bb7:
	bge	s2, s1, .main.bb9
.main.bb8:
	slliw	t0, s2, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	addi	t5, sp, 24
	sd	t5, 0(sp)
	addi	t5, sp, 152
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	arrCopy
	la	t0, n
	lw	s1, 0(t0)
	addi	t0, sp, 152
	addi	t0, t0, 4
	li	t1, 1
.main.bb25:
	blt	t1, s1, .main.bb26
.main.bb27:
	li	s2, 0
	li	s1, 32
.main.bb10:
	bge	s2, s1, .main.bb12
.main.bb11:
	slliw	t0, s2, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb11	# rotated-loop-branch
.main.bb12:
	addi	t5, sp, 24
	sd	t5, 0(sp)
	addi	t5, sp, 152
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	arrCopy
	addi	t5, sp, 152
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	li	t5, 31
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	QuickSort
	li	s2, 0
	li	s1, 32
.main.bb13:
	bge	s2, s1, .main.bb15
.main.bb14:
	slliw	t0, s2, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb14	# rotated-loop-branch
.main.bb15:
	addi	t5, sp, 24
	sd	t5, 0(sp)
	addi	t5, sp, 152
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	arrCopy
	la	t0, n
	lw	s1, 0(t0)
	addi	t0, sp, 152
	li	t1, 0
	li	t2, 0
.main.bb31:
	bge	t1, s1, .main.bb33
.main.bb32:
	lw	t3, 0(t0)
	addw	s2, t2, t3
	sraiw	t2, t1, 31
	li	t3, 3
	andi	s3, t2, 3
	addw	t2, t1, s3
	andi	t2, t2, 3
	subw	t2, t2, s3
	beq	t2, t3, .main.bb35
.main.bb34:
	li	t2, 0
	sw	t2, 0(t0)
	mv	t2, s2	# phi-copy
.main.bb36:
	addiw	t1, t1, 1
	addi	t0, t0, 4
	blt	t1, s1, .main.bb32	# rotated-loop-branch
.main.bb33:
	li	s2, 0
	li	s1, 32
.main.bb16:
	bge	s2, s1, .main.bb18
.main.bb17:
	slliw	t0, s2, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb17	# rotated-loop-branch
.main.bb18:
	addi	t5, sp, 24
	sd	t5, 0(sp)
	addi	t5, sp, 152
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	arrCopy
	addi	t5, sp, 152
	sd	t5, 0(sp)
	li	t5, 3
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	avgPooling
	li	s2, 0
	li	s1, 32
.main.bb19:
	bge	s2, s1, .main.bb21
.main.bb20:
	slliw	t0, s2, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb20	# rotated-loop-branch
.main.bb21:
	li	a0, 0
	ld	s4, 312(sp)
	ld	s3, 304(sp)
	ld	s2, 296(sp)
	ld	s1, 288(sp)
	ld	ra, 280(sp)
	addi	sp, sp, 320
	ret
.main.bb35:
	sw	s2, 0(t0)
	li	t2, 0
	j	.main.bb36
.main.bb41:
	sraiw	t0, t1, 1
	slliw	t0, t0, 2
	addi	t1, sp, 152
	add	t0, t1, t0
	lw	t0, 0(t0)
	j	.main.bb39
	.size main, .-main
