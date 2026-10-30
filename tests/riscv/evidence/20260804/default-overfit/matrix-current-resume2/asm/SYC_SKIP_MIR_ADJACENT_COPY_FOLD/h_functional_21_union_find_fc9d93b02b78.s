
  .section .rodata
  .globl maxN
  .align 2
  .type  maxN, @object
  .size  maxN, 4
maxN:
  .word 1005

  .bss
  .globl parent
  .align 2
  .type  parent, @object
  .size  parent, 4020
parent:
  .zero 4020

  .text

  .globl find
  .type  find, @function
find:
.find.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	mv	t0, a0
	slliw	t1, t0, 2
	la	t2, parent
	add	s1, t2, t1
	lw	t1, 0(s1)
	beq	t1, t0, .find.bb1
	j	.find.bb3
.find.bb2:
.find.bb1:
	mv	a0, t0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.find.bb3:
	mv	a0, t1	# call-arg-copy
	call	find
	mv	t0, a0
	sw	t0, 0(s1)
	j	.find.bb1
	.size find, .-find

  .globl merge
  .type  merge, @function
merge:
.merge.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	mv	t0, a0
	mv	s1, a1
	mv	a0, t0	# call-arg-copy
	call	find
	mv	s2, a0
	mv	a0, s1	# call-arg-copy
	call	find
	mv	t0, a0
	bne	s2, t0, .merge.bb1
.merge.bb2:
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.merge.bb1:
	slliw	t0, t0, 2
	la	t1, parent
	add	t0, t1, t0
	sw	s2, 0(t0)
	j	.merge.bb2
	.size merge, .-merge

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	la	s3, parent
	li	t0, 0
	mv	s4, s3	# phi-copy
.main.bb1:
	bge	t0, s1, .main.bb3
.main.bb2:
	sw	t0, 0(s4)
	addi	s4, s4, 4
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
	mv	s4, t0	# phi-copy
.main.bb4:
	bge	s4, s2, .main.bb6
.main.bb5:
	call	getint
	mv	s5, a0
	call	getint
	mv	t0, a0
	mv	t5, s5
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	merge
	addiw	t0, s4, 1
	mv	s4, t0	# phi-copy
	blt	s4, s2, .main.bb5	# rotated-loop-branch
.main.bb6:
	li	t0, 0
	li	t1, 0
.main.bb7:
	bge	t1, s1, .main.bb9
.main.bb8:
	lw	t2, 0(s3)
	bne	t2, t1, .main.bb11
.main.bb10:
	addiw	t0, t0, 1
.main.bb11:
	addiw	t1, t1, 1
	addi	t2, s3, 4
	mv	s3, t2	# phi-copy
	blt	t1, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	mv	a0, t0
	call	putint
	li	a0, 0
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 64
	ret
.main.bb12:
	j	.main.bb11
	.size main, .-main
