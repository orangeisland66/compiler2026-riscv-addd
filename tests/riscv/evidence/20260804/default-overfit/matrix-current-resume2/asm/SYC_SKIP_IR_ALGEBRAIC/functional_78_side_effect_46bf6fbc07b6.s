
  .data
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .word -1

  .data
  .globl b
  .align 2
  .type  b, @object
  .size  b, 4
b:
  .word 1

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	la	t0, a
	la	t1, b
	lw	s2, 0(t0)
	li	s1, 5
	lw	s3, 0(t1)
.main.bb1:
	blt	s1, zero, .main.bb3
.main.bb2:
	addiw	s2, s2, 1
	li	t0, 0
	subw	t0, s2, t0
	sltu	s4, zero, t0
	beqz	s4, .main.bb18
.main.bb6:
	addiw	s2, s2, 1
	li	t0, 0
	subw	t0, s2, t0
	sltu	t0, zero, t0
.main.bb5:
	li	t1, 0
	subw	t0, t0, t1
	sltu	s4, zero, t0
	beqz	s4, .main.bb17
.main.bb7:
	addiw	s2, s2, 1
	li	t0, 0
	subw	t0, s2, t0
	sltu	t0, zero, t0
.main.bb4:
	bne	t0, zero, .main.bb8
.main.bb9:
	addiw	s2, s2, 1
	li	t0, 14
	slt	s4, s2, t0
	beqz	s4, .main.bb11
.main.bb19:
	mv	t0, s4	# phi-copy
.main.bb10:
	beq	t0, zero, .main.bb15
.main.bb14:
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	slliw	s3, s3, 1
	mv	t0, s2	# phi-copy
.main.bb16:
	addiw	s1, s1, -1
	mv	s2, t0	# phi-copy
	bge	s1, zero, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, s2
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	t5, s3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	la	t0, b
	sw	s3, 0(t0)
	la	t0, a
	sw	s2, 0(t0)
	mv	a0, s2
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
.main.bb8:
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	t5, s3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb9
.main.bb11:
	addiw	t0, s2, 1
.main.bb13:
	addiw	t0, t0, 1
	li	t1, 0
	addiw	s2, t0, 1
	subw	t0, t0, s2
	addiw	t0, t0, 1
	subw	t0, t0, t1
	sltu	t0, zero, t0
.main.bb12:
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
	j	.main.bb10
.main.bb15:
	addiw	t0, s2, 1
	j	.main.bb16
.main.bb17:
	mv	t0, s4	# phi-copy
	j	.main.bb4
.main.bb18:
	mv	t0, s4	# phi-copy
	j	.main.bb5
	.size main, .-main
