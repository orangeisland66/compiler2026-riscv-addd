
  .text

  .globl walk_depth
  .type  walk_depth, @function
walk_depth:
.walk_depth.bb0:
	addi	sp, sp, -32
	sd	ra, 24(sp)
	mv	t0, a0
	mv	t1, a1
	mv	t2, a2
	bge	zero, t0, .walk_depth.bb2
.walk_depth.bb3:
	addiw	t0, t0, -1
	addw	t1, t1, t2
	bge	zero, t0, .walk_depth.bb5
.walk_depth.bb6:
	addiw	t0, t0, -1
	addw	t1, t1, t2
	bge	zero, t0, .walk_depth.bb8
.walk_depth.bb9:
	addiw	t0, t0, -1
	addw	t1, t1, t2
	bge	zero, t0, .walk_depth.bb11
.walk_depth.bb12:
	addiw	t0, t0, -1
	addw	t1, t1, t2
	blt	zero, t0, .walk_depth.bb15
.walk_depth.bb14:
	mv	t0, t1	# phi-copy
	j	.walk_depth.bb1
.walk_depth.bb13:
	j	.walk_depth.bb1
.walk_depth.bb10:
	j	.walk_depth.bb1
.walk_depth.bb7:
	j	.walk_depth.bb1
.walk_depth.bb4:
.walk_depth.bb1:
	mv	a0, t0
	ld	ra, 24(sp)
	addi	sp, sp, 32
	ret
.walk_depth.bb5:
	mv	t0, t1	# phi-copy
	j	.walk_depth.bb1
.walk_depth.bb8:
	mv	t0, t1	# phi-copy
	j	.walk_depth.bb1
.walk_depth.bb11:
	mv	t0, t1	# phi-copy
	j	.walk_depth.bb1
.walk_depth.bb15:
	addiw	a0, t0, -1
	addw	a1, t1, t2
	mv	a2, t2	# call-arg-copy
	call	walk_depth
	mv	t0, a0
	j	.walk_depth.bb1
.walk_depth.bb2:
	mv	t0, t1	# phi-copy
	j	.walk_depth.bb1
	.size walk_depth, .-walk_depth

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 24(sp)
	li	t5, 37
	sd	t5, 0(sp)
	li	t5, 5
	sd	t5, 8(sp)
	li	t5, 2
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	walk_depth
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 24(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
