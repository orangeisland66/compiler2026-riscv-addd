
  .section .rodata
  .globl maxN
  .align 2
  .type  maxN, @object
  .size  maxN, 4
maxN:
  .word 1000

  .section .rodata
  .globl space
  .align 2
  .type  space, @object
  .size  space, 4
space:
  .word 32

  .bss
  .globl array
  .align 2
  .type  array, @object
  .size  array, 4000
array:
  .zero 4000

  .text

  .globl findPivot
  .type  findPivot, @function
findPivot:
.findPivot.bb0:
	addi	sp, sp, -48
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	mv	t0, a0
	mv	s1, a1
	slliw	t1, s1, 2
	la	t2, array
	add	s2, t2, t1
	lw	t1, 0(s2)
	slliw	t3, t0, 2
	add	t2, t2, t3
	mv	s3, t0	# phi-copy
.findPivot.bb1:
	bge	s3, s1, .findPivot.bb3
.findPivot.bb2:
	lw	s4, 0(t2)
	blt	t1, s4, .findPivot.bb5
.findPivot.bb4:
	slliw	t3, t0, 2
	la	a2, array
	add	s5, a2, t3
	addiw	t0, t0, 1
	lw	t3, 0(s5)
	sw	t3, 0(t2)
	sw	s4, 0(s5)
.findPivot.bb5:
	addiw	s3, s3, 1
	addi	t2, t2, 4
	blt	s3, s1, .findPivot.bb2	# rotated-loop-branch
.findPivot.bb3:
	slliw	t1, t0, 2
	la	t2, array
	add	t1, t2, t1
	lw	t2, 0(t1)
	lw	t3, 0(s2)
	sw	t3, 0(t1)
	sw	t2, 0(s2)
	mv	a0, t0
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 48
	ret
.findPivot.bb6:
	j	.findPivot.bb5
	.size findPivot, .-findPivot

  .globl findSmallest
  .type  findSmallest, @function
findSmallest:
.findSmallest.bb0:
	addi	sp, sp, -80
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	beq	s1, s2, .findSmallest.bb1
.findSmallest.bb3:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	call	findPivot
	mv	s5, a0
	beq	s3, s5, .findSmallest.bb4
.findSmallest.bb5:
	bge	s3, s5, .findSmallest.bb11
.findSmallest.bb10:
	addiw	t0, s5, -1
	beq	s1, t0, .findSmallest.bb1
	j	.findSmallest.bb13
.findSmallest.bb14:
	j	.findSmallest.bb1
.findSmallest.bb12:
	j	.findSmallest.bb1
.findSmallest.bb6:
.findSmallest.bb1:
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 80
	ret
.findSmallest.bb8:
	slliw	t0, s1, 2
	la	t1, array
	add	t0, t1, t0
	lw	a0, 0(t0)
	call	putint
	li	a0, 32
	call	putch
	addiw	s1, s1, 1
	blt	s1, s5, .findSmallest.bb8	# rotated-loop-branch
	j	.findSmallest.bb1	# rotated-loop-exit
.findSmallest.bb9:
	j	.findSmallest.bb1
.findSmallest.bb4:
	li	s1, 0
.findSmallest.bb7:
	bge	s1, s5, .findSmallest.bb1
	j	.findSmallest.bb8
.findSmallest.bb11:
	addiw	a0, s5, 1
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	mv	a3, s4	# call-arg-copy
	call	findSmallest
	j	.findSmallest.bb1
.findSmallest.bb13:
	mv	a0, s1	# call-arg-copy
	mv	a1, t0	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	mv	a3, s4	# call-arg-copy
	call	findSmallest
	j	.findSmallest.bb1
.findSmallest.bb2:
	j	.findSmallest.bb1
	.size findSmallest, .-findSmallest

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	li	s3, 0
.main.bb1:
	bge	s3, s1, .main.bb3
.main.bb2:
	call	getint
	mv	t0, a0
	slliw	t1, s3, 2
	la	t2, array
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s3, s3, 1
	blt	s3, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	addiw	t0, s1, -1
	bne	zero, t0, .main.bb4
.main.bb5:
	li	a0, 0
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 64
	ret
.main.bb4:
	li	t5, 0
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	mv	t5, s2
	sd	t5, 16(sp)
	mv	t5, s1
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	findSmallest
	j	.main.bb5
	.size main, .-main
