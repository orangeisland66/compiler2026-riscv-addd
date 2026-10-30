
  .bss
  .globl __memo.walk_edges.epoch
  .align 2
  .type  __memo.walk_edges.epoch, @object
  .size  __memo.walk_edges.epoch, 4
__memo.walk_edges.epoch:
  .zero 4

  .bss
  .globl __memo.walk_edges.depth
  .align 2
  .type  __memo.walk_edges.depth, @object
  .size  __memo.walk_edges.depth, 4
__memo.walk_edges.depth:
  .zero 4

  .bss
  .globl __memo.walk_edges.state_hash
  .align 2
  .type  __memo.walk_edges.state_hash, @object
  .size  __memo.walk_edges.state_hash, 4
__memo.walk_edges.state_hash:
  .zero 4

  .bss
  .globl __memo.walk_edges.stamp
  .align 2
  .type  __memo.walk_edges.stamp, @object
  .size  __memo.walk_edges.stamp, 80
__memo.walk_edges.stamp:
  .zero 80

  .bss
  .globl __memo.walk_edges.value
  .align 2
  .type  __memo.walk_edges.value, @object
  .size  __memo.walk_edges.value, 80
__memo.walk_edges.value:
  .zero 80

  .text

  .globl walk_edges
  .type  walk_edges, @function
walk_edges:
.walk_edges.bb0:
	mv	t0, a0
	mv	t1, a1
	li	t2, 0
	slt	t2, t0, t2
	xori	t2, t2, 1
	li	t3, 20
	slt	t3, t0, t3
	and	t2, t2, t3
	addiw	t3, t1, -3
	li	a2, 0
	slt	a2, t3, a2
	xori	a2, a2, 1
	li	a3, 1
	slt	a3, t3, a3
	and	a2, a2, a3
	and	t2, t2, a2
	addw	t3, t0, t3
	mulw	t3, t3, t2
	slliw	t3, t3, 2
	la	a2, __memo.walk_edges.stamp
	add	a2, a2, t3
	lw	a3, 0(a2)
	li	a4, 1
	subw	a3, a3, a4
	sltiu	a3, a3, 1
	and	a3, a3, t2
	la	a4, __memo.walk_edges.value
	add	t3, a4, t3
	bnez	a3, .walk_edges.bb7
.walk_edges.bb6:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	mv	s1, t3	# early-return.slow-copy
	mv	s2, a2	# early-return.slow-copy
	mv	s3, t2	# early-return.slow-copy
	bge	zero, t0, .walk_edges.bb2
.walk_edges.bb3:
	andi	t2, t0, 1
	beq	t2, zero, .walk_edges.bb4
.walk_edges.bb5:
	addiw	a0, t0, -2
	addiw	a1, t1, 2
	call	walk_edges
	mv	t0, a0
	j	.walk_edges.bb8
.walk_edges.bb1:
.walk_edges.bb8:
	bnez	s3, .walk_edges.bb9
.walk_edges.bb10:
	mv	a0, t0
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.walk_edges.bb2:
	mv	t0, t1	# phi-copy
	j	.walk_edges.bb8
.walk_edges.bb4:
	addiw	a0, t0, -1
	addiw	a1, t1, 1
	call	walk_edges
	mv	t0, a0
	j	.walk_edges.bb8
.walk_edges.bb9:
	sw	t0, 0(s1)
	li	t1, 1
	sw	t1, 0(s2)
	j	.walk_edges.bb10
.walk_edges.bb7:
	lw	a0, 0(t3)
	ret
	.size walk_edges, .-walk_edges

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	li	t5, 19
	sd	t5, 0(sp)
	li	t5, 3
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	walk_edges
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
