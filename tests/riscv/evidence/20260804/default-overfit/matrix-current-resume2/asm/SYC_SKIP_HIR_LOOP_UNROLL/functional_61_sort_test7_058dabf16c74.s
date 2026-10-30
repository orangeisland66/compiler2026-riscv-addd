
  .bss
  .globl buf
  .align 2
  .type  buf, @object
  .size  buf, 800
buf:
  .zero 800

  .text

  .globl merge_sort
  .type  merge_sort, @function
merge_sort:
.merge_sort.bb0:
	addi	sp, sp, -96
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	sd	s8, 80(sp)
	mv	s1, a0
	mv	s2, a1
	addiw	t0, s1, 1
	bge	t0, s2, .merge_sort.bb1
.merge_sort.bb3:
	addw	t1, s1, s2
	srliw	t2, t1, 31	# const-div.pow2.bias-fold
	addw	t1, t1, t2
	sraiw	s3, t1, 1
	blt	t0, s3, .merge_sort.bb19
.merge_sort.bb20:
	mv	a0, s3	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	call	merge_sort
	slliw	t0, s1, 2
	addiw	t0, t0, 400
	la	t1, buf
	add	t0, t1, t0
	mv	s4, s1	# phi-copy
	mv	s5, s1	# phi-copy
	mv	s6, s3	# phi-copy
.merge_sort.bb4:
	slt	t1, s5, s3
	slt	t2, s6, s2
	and	t1, t1, t2
	beq	t1, zero, .merge_sort.bb6
.merge_sort.bb5:
	slliw	t1, s5, 2
	la	s7, buf
	add	t1, s7, t1
	lw	s8, 0(t1)
	slliw	t1, s6, 2
	add	t1, s7, t1
	lw	s7, 0(t1)
	bge	s8, s7, .merge_sort.bb8
.merge_sort.bb7:
	sw	s8, 0(t0)
	addiw	s5, s5, 1
	mv	t1, s6	# phi-copy
.merge_sort.bb9:
	addiw	s4, s4, 1
	addi	t0, t0, 4
	mv	s6, t1	# phi-copy
	j	.merge_sort.bb4
.merge_sort.bb8:
	sw	s7, 0(t0)
	addiw	t1, s6, 1
	j	.merge_sort.bb9
.merge_sort.bb6:
	slliw	t0, s5, 2
	la	t1, buf
	slliw	t2, s4, 2
	add	t0, t1, t0
	addiw	t2, t2, 400
	add	t1, t1, t2
.merge_sort.bb10:
	bge	s5, s3, .merge_sort.bb12
.merge_sort.bb11:
	lw	t2, 0(t0)
	addiw	s4, s4, 1
	sw	t2, 0(t1)
	addi	t0, t0, 4
	addiw	s5, s5, 1
	addi	t1, t1, 4
	blt	s5, s3, .merge_sort.bb11	# rotated-loop-branch
.merge_sort.bb12:
	slliw	t0, s6, 2
	la	t1, buf
	slliw	t2, s4, 2
	add	t0, t1, t0
	addiw	t2, t2, 400
	add	t1, t1, t2
.merge_sort.bb13:
	bge	s6, s2, .merge_sort.bb16
.merge_sort.bb14:
	lw	t2, 0(t0)
	addiw	s4, s4, 1
	sw	t2, 0(t1)
	addi	t0, t0, 4
	addiw	s6, s6, 1
	addi	t1, t1, 4
	blt	s6, s2, .merge_sort.bb14	# rotated-loop-branch
	j	.merge_sort.bb16	# rotated-loop-exit
.merge_sort.bb15:
.merge_sort.bb16:
	bge	s1, s2, .merge_sort.bb1
.merge_sort.bb17:
	slliw	t0, s1, 2
	la	t1, buf
	addiw	t0, t0, 400
	add	s3, t1, t0
	lw	t0, 0(s3)
	sw	t0, -400(s3)
	addiw	s1, s1, 1
	blt	s1, s2, .merge_sort.bb17	# rotated-loop-branch
	j	.merge_sort.bb1	# rotated-loop-exit
.merge_sort.bb18:
.merge_sort.bb1:
	ld	s8, 80(sp)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 96
	ret
.merge_sort.bb19:
	mv	a0, s1	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	merge_sort
	j	.merge_sort.bb20
.merge_sort.bb2:
	j	.merge_sort.bb1
	.size merge_sort, .-merge_sort

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	la	t5, buf
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s1, a0
	li	t0, 1
	blt	t0, s1, .main.bb1
.main.bb2:
	mv	t5, s1
	sd	t5, 0(sp)
	la	t5, buf
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	li	a0, 0
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
.main.bb1:
	li	t5, 0
	sd	t5, 0(sp)
	mv	t5, s1
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	merge_sort
	j	.main.bb2
	.size main, .-main
