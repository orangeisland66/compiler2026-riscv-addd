
  .text

  .globl max_sum_nonadjacent
  .type  max_sum_nonadjacent, @function
max_sum_nonadjacent:
.max_sum_nonadjacent.bb0:
	addi	sp, sp, -96
	sd	s1, 64(sp)
	sd	s2, 72(sp)
	sd	s3, 80(sp)
	sd	s4, 88(sp)
	mv	t0, a0
	mv	s1, a1
	addi	t5, sp, 0
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	lw	t2, 0(t0)
	addi	t3, sp, 0
	sw	t2, 0(t3)
	lw	t1, 4(t0)
	beq	t2, t1, .max_sum_nonadjacent.bb4
.max_sum_nonadjacent.bb6:
	blt	t1, t2, .max_sum_nonadjacent.bb4
.max_sum_nonadjacent.bb8:
	mv	t2, t1	# phi-copy
.max_sum_nonadjacent.bb4:
	addi	t0, t0, 8
	addi	t1, sp, 0
	sw	t2, 4(t1)
	li	t1, 2
.max_sum_nonadjacent.bb1:
	bge	t1, s1, .max_sum_nonadjacent.bb3
.max_sum_nonadjacent.bb2:
	addiw	t2, t1, -2
	addi	t3, sp, 0
	slliw	t2, t2, 2
	add	s2, t3, t2
	lw	t2, 0(s2)
	lw	t3, 0(t0)
	lw	s4, 4(s2)
	addw	s3, t2, t3
	beq	s3, s4, .max_sum_nonadjacent.bb9
.max_sum_nonadjacent.bb11:
	blt	s4, s3, .max_sum_nonadjacent.bb9
.max_sum_nonadjacent.bb13:
	mv	s3, s4	# phi-copy
.max_sum_nonadjacent.bb9:
	sw	s3, 8(s2)
	addiw	t1, t1, 1
	addi	t0, t0, 4
	blt	t1, s1, .max_sum_nonadjacent.bb2	# rotated-loop-branch
.max_sum_nonadjacent.bb3:
	addiw	t0, s1, -1
	slliw	t0, t0, 2
	addi	t1, sp, 0
	add	t0, t1, t0
	lw	a0, 0(t0)
	ld	s4, 88(sp)
	ld	s3, 80(sp)
	ld	s2, 72(sp)
	ld	s1, 64(sp)
	addi	sp, sp, 96
	ret
.max_sum_nonadjacent.bb10:
	j	.max_sum_nonadjacent.bb9
.max_sum_nonadjacent.bb12:
	j	.max_sum_nonadjacent.bb9
.max_sum_nonadjacent.bb7:
	j	.max_sum_nonadjacent.bb4
.max_sum_nonadjacent.bb5:
	j	.max_sum_nonadjacent.bb4
	.size max_sum_nonadjacent, .-max_sum_nonadjacent

  .globl longest_common_subseq
  .type  longest_common_subseq, @function
longest_common_subseq:
.longest_common_subseq.bb0:
	addi	sp, sp, -1136
	sd	ra, 1048(sp)
	sd	s1, 1056(sp)
	sd	s2, 1064(sp)
	sd	s3, 1072(sp)
	sd	s4, 1080(sp)
	sd	s5, 1088(sp)
	sd	s6, 1096(sp)
	sd	s7, 1104(sp)
	sd	s8, 1112(sp)
	sd	s9, 1120(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	addi	a0, sp, 24
	li	a1, 0
	li	a2, 1024
	call	memset
	addi	s5, sp, 24
	li	t0, 1
.longest_common_subseq.bb1:
	blt	s2, t0, .longest_common_subseq.bb3
.longest_common_subseq.bb2:
	addiw	s6, t0, -1
	li	t2, 1
	slliw	t1, s6, 2
	add	t1, s1, t1
	slliw	s6, s6, 6
	lw	t1, 0(t1)
.longest_common_subseq.bb4:
	blt	s4, t2, .longest_common_subseq.bb6
.longest_common_subseq.bb5:
	addiw	t3, t2, -1
	slliw	s7, t3, 2
	add	t3, s3, s7
	lw	t3, 0(t3)
	beq	t1, t3, .longest_common_subseq.bb7
.longest_common_subseq.bb8:
	slliw	t3, t2, 2
	addw	t3, s6, t3
	add	s7, s5, t3
	lw	s8, 0(s7)
	lw	s9, 60(s7)
	beq	s8, s9, .longest_common_subseq.bb10
.longest_common_subseq.bb12:
	blt	s9, s8, .longest_common_subseq.bb10
.longest_common_subseq.bb14:
	mv	s8, s9	# phi-copy
.longest_common_subseq.bb10:
	sw	s8, 64(s7)
.longest_common_subseq.bb9:
	addiw	t2, t2, 1
	j	.longest_common_subseq.bb4
.longest_common_subseq.bb7:
	addw	t3, s6, s7
	add	s7, s5, t3
	lw	t3, 0(s7)
	addiw	t3, t3, 1
	sw	t3, 68(s7)
	j	.longest_common_subseq.bb9
.longest_common_subseq.bb11:
	j	.longest_common_subseq.bb10
.longest_common_subseq.bb13:
	j	.longest_common_subseq.bb10
.longest_common_subseq.bb6:
	addiw	t0, t0, 1
	bge	s2, t0, .longest_common_subseq.bb2	# rotated-loop-branch
.longest_common_subseq.bb3:
	slliw	t0, s2, 6
	slliw	t1, s4, 2
	addw	t0, t0, t1
	add	t0, s5, t0
	lw	a0, 0(t0)
	ld	s9, 1120(sp)
	ld	s8, 1112(sp)
	ld	s7, 1104(sp)
	ld	s6, 1096(sp)
	ld	s5, 1088(sp)
	ld	s4, 1080(sp)
	ld	s3, 1072(sp)
	ld	s2, 1064(sp)
	ld	s1, 1056(sp)
	ld	ra, 1048(sp)
	addi	sp, sp, 1136
	ret
	.size longest_common_subseq, .-longest_common_subseq

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -160
	sd	ra, 152(sp)
	addi	t5, sp, 88
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sw	zero, 56(t5)	# memset-zero
	li	t0, 8
	addi	t1, sp, 88
	sw	t0, 0(t1)
	li	t0, 7
	sw	t0, 4(t1)
	li	t0, 4
	sw	t0, 8(t1)
	li	t0, 1
	sw	t0, 12(t1)
	li	t0, 2
	sw	t0, 16(t1)
	li	t0, 7
	sw	t0, 20(t1)
	li	t0, 0
	sw	t0, 24(t1)
	li	t0, 1
	sw	t0, 28(t1)
	li	t0, 9
	sw	t0, 32(t1)
	li	t0, 3
	sw	t0, 36(t1)
	li	t0, 4
	sw	t0, 40(t1)
	li	t0, 8
	sw	t0, 44(t1)
	li	t0, 3
	sw	t0, 48(t1)
	li	t0, 7
	sw	t0, 52(t1)
	li	t0, 0
	sw	t0, 56(t1)
	addi	t5, sp, 32
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sw	zero, 48(t5)	# memset-zero
	li	t0, 3
	addi	t1, sp, 32
	sw	t0, 0(t1)
	li	t0, 9
	sw	t0, 4(t1)
	li	t0, 7
	sw	t0, 8(t1)
	li	t0, 1
	sw	t0, 12(t1)
	li	t0, 4
	sw	t0, 16(t1)
	li	t0, 2
	sw	t0, 20(t1)
	li	t0, 4
	sw	t0, 24(t1)
	li	t0, 3
	sw	t0, 28(t1)
	li	t0, 6
	sw	t0, 32(t1)
	li	t0, 8
	sw	t0, 36(t1)
	li	t0, 0
	sw	t0, 40(t1)
	li	t0, 1
	sw	t0, 44(t1)
	li	t0, 5
	sw	t0, 48(t1)
	addi	t5, sp, 88
	sd	t5, 0(sp)
	li	t5, 15
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	max_sum_nonadjacent
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addi	t5, sp, 88
	sd	t5, 0(sp)
	li	t5, 15
	sd	t5, 8(sp)
	addi	t5, sp, 32
	sd	t5, 16(sp)
	li	t5, 13
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	longest_common_subseq
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 152(sp)
	addi	sp, sp, 160
	ret
	.size main, .-main
