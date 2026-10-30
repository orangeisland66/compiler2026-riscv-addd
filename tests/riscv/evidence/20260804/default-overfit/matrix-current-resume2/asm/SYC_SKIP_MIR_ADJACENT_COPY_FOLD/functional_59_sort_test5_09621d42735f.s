
  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .text

  .globl heap_sort
  .type  heap_sort, @function
heap_sort:
.heap_sort.bb0:
	addi	sp, sp, -48
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	mv	s1, a0
	mv	s2, a1
	srliw	t0, s2, 31	# const-div.pow2.bias-fold
	addw	t0, s2, t0
	sraiw	t0, t0, 1
	addiw	t0, t0, -1
	addiw	s3, s2, -1
.heap_sort.bb1:
	li	t1, -1
	bge	t1, t0, .heap_sort.bb3
.heap_sort.bb2:
	slliw	t1, t0, 1
	addiw	t1, t1, 1
	mv	t2, t0	# phi-copy
.heap_sort.bb8:
	bge	t1, s2, .heap_sort.bb7
.heap_sort.bb9:
	slliw	a2, t1, 2
	slt	t3, t1, s3
	add	s4, s1, a2
	lw	a2, 0(s4)
	lw	a3, 4(s4)
	slt	a2, a2, a3
	and	t3, t3, a2
	beq	t3, zero, .heap_sort.bb12
.heap_sort.bb11:
	addiw	t1, t1, 1
.heap_sort.bb12:
	slliw	t2, t2, 2
	add	s4, s1, t2
	lw	s5, 0(s4)
	slliw	t2, t1, 2
	add	s6, s1, t2
	lw	t2, 0(s6)
	blt	t2, s5, .heap_sort.bb7
.heap_sort.bb14:
	sw	t2, 0(s4)
	slliw	t2, t1, 1
	sw	s5, 0(s6)
	addiw	t2, t2, 1
	mv	t5, t2	# phi-cycle-break
	mv	t2, t1	# phi-copy
	mv	t1, t5	# phi-copy
	blt	t1, s2, .heap_sort.bb9	# rotated-loop-branch
	j	.heap_sort.bb7	# rotated-loop-exit
.heap_sort.bb10:
.heap_sort.bb7:
	addiw	t0, t0, -1
	j	.heap_sort.bb1
.heap_sort.bb23:
	j	.heap_sort.bb12
.heap_sort.bb13:
	j	.heap_sort.bb7
.heap_sort.bb17:
	slliw	a2, t2, 2
	slt	t3, t2, s2
	add	s4, s1, a2
	lw	a2, 0(s4)
	lw	a3, 4(s4)
	slt	a2, a2, a3
	and	t3, t3, a2
	beq	t3, zero, .heap_sort.bb20
.heap_sort.bb19:
	addiw	t2, t2, 1
.heap_sort.bb20:
	slliw	t1, t1, 2
	add	s4, s1, t1
	lw	s5, 0(s4)
	slliw	t1, t2, 2
	add	s6, s1, t1
	lw	t1, 0(s6)
	blt	t1, s5, .heap_sort.bb15
.heap_sort.bb22:
	sw	t1, 0(s4)
	slliw	t1, t2, 1
	sw	s5, 0(s6)
	addiw	t1, t1, 1
	mv	t5, t1	# phi-cycle-break
	mv	t1, t2	# phi-copy
	mv	t2, t5	# phi-copy
	blt	t2, s3, .heap_sort.bb17	# rotated-loop-branch
	j	.heap_sort.bb15	# rotated-loop-exit
.heap_sort.bb18:
.heap_sort.bb15:
	addi	t0, t0, -4
	mv	s3, s2	# phi-copy
	bge	zero, s3, .heap_sort.bb6	# rotated-loop-branch
.heap_sort.bb5:
	lw	t1, 0(s1)
	addiw	s2, s3, -1
	lw	t2, 0(t0)
	sw	t2, 0(s1)
	sw	t1, 0(t0)
	li	t2, 1
	li	t1, 0
.heap_sort.bb16:
	bge	t2, s3, .heap_sort.bb15
	j	.heap_sort.bb17
.heap_sort.bb24:
	j	.heap_sort.bb20
.heap_sort.bb21:
	j	.heap_sort.bb15
.heap_sort.bb3:
	slliw	t0, s3, 2
	add	t0, s1, t0
.heap_sort.bb4:
	blt	zero, s3, .heap_sort.bb5
.heap_sort.bb6:
	li	a0, 0
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 48
	ret
	.size heap_sort, .-heap_sort

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 56(sp)
	sd	s1, 64(sp)
	sd	s2, 72(sp)
	li	t0, 4
	addi	t1, sp, 16
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
	addi	t5, sp, 16
	sd	t5, 0(sp)
	li	t5, 10
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	heap_sort
	mv	t0, a0
	li	s1, 10
	mv	s2, t0	# phi-copy
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	slliw	t0, s2, 2
	addi	t1, sp, 16
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
	ld	s2, 72(sp)
	ld	s1, 64(sp)
	ld	ra, 56(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
