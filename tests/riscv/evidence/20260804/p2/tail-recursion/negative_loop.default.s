
  .text

  .globl recursive_loop
  .type  recursive_loop, @function
recursive_loop:
.recursive_loop.bb0:
	addi	sp, sp, -96
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	sd	s9, 80(sp)
	sd	s10, 88(sp)
	mv	t0, a0
	li	s1, 0
.recursive_loop.bb1:
	bge	zero, t0, .recursive_loop.bb3
.recursive_loop.bb2:
	addiw	s2, t0, -1
	li	s3, 0
	mv	s4, s2	# phi-copy
.recursive_loop.bb4:
	bge	zero, s4, .recursive_loop.bb6
.recursive_loop.bb5:
	addiw	s4, s4, -1
	li	s5, 0
	mv	s6, s4	# phi-copy
.recursive_loop.bb7:
	bge	zero, s6, .recursive_loop.bb9
.recursive_loop.bb8:
	addiw	s6, s6, -1
	li	s7, 0
	mv	s8, s6	# phi-copy
.recursive_loop.bb10:
	bge	zero, s8, .recursive_loop.bb12
.recursive_loop.bb11:
	addiw	s8, s8, -1
	li	s9, 0
	mv	s10, s8	# phi-copy
.recursive_loop.bb13:
	bge	zero, s10, .recursive_loop.bb15
.recursive_loop.bb14:
	addiw	s10, s10, -1
	mv	a0, s10	# call-arg-copy
	call	recursive_loop
	mv	t0, a0
	addw	s9, s9, t0
	blt	zero, s10, .recursive_loop.bb14	# rotated-loop-branch
.recursive_loop.bb15:
	addw	s7, s7, s9
	j	.recursive_loop.bb10
.recursive_loop.bb12:
	addw	s5, s5, s7
	j	.recursive_loop.bb7
.recursive_loop.bb9:
	addw	s3, s3, s5
	j	.recursive_loop.bb4
.recursive_loop.bb6:
	addw	s1, s1, s3
	mv	t0, s2	# phi-copy
	blt	zero, t0, .recursive_loop.bb2	# rotated-loop-branch
.recursive_loop.bb3:
	mv	a0, s1
	ld	s10, 88(sp)
	ld	s9, 80(sp)
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 96
	ret
	.size recursive_loop, .-recursive_loop

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t5, 4
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	recursive_loop
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
