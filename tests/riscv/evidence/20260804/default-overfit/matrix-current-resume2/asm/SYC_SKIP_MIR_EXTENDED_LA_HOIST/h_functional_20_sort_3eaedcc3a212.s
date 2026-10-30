
  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .section .rodata
  .globl maxn
  .align 2
  .type  maxn, @object
  .size  maxn, 4
maxn:
  .word 100005

  .bss
  .globl cnt
  .align 2
  .type  cnt, @object
  .size  cnt, 1600080
cnt:
  .zero 1600080

  .bss
  .globl x
  .align 2
  .type  x, @object
  .size  x, 400020
x:
  .zero 400020

  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 400020
a:
  .zero 400020

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 400020
b:
  .zero 400020

  .bss
  .globl c
  .align 2
  .type  c, @object
  .size  c, 400020
c:
  .zero 400020

  .text

  .globl quick_read
  .type  quick_read, @function
quick_read:
.quick_read.bb0:
	addi	sp, sp, -32
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	call	getch
	mv	t0, a0
	li	s1, 0
.quick_read.bb2:
	li	t1, 48
	li	t2, 57
	slt	t1, t0, t1
	slt	t2, t2, t0
	or	t1, t1, t2
	beq	t1, zero, .quick_read.bb4
.quick_read.bb3:
	li	t1, 45
	bne	t0, t1, .quick_read.bb6
.quick_read.bb5:
	li	s1, 1
.quick_read.bb6:
	call	getch
	mv	t0, a0
	j	.quick_read.bb2
.quick_read.bb12:
	j	.quick_read.bb6
.quick_read.bb4:
	li	t1, 0
.quick_read.bb7:
	li	t2, 48
	li	t3, 57
	slt	t2, t0, t2
	slt	t3, t3, t0
	xori	t2, t2, 1
	xori	t3, t3, 1
	and	t2, t2, t3
	beq	t2, zero, .quick_read.bb9
.quick_read.bb8:
	slliw	t2, t1, 3
	slliw	t1, t1, 1
	addw	t1, t2, t1
	addw	t0, t1, t0
	addiw	s2, t0, -48
	call	getch
	mv	t0, a0
	mv	t1, s2	# phi-copy
	j	.quick_read.bb7
.quick_read.bb9:
	bne	s1, zero, .quick_read.bb10
.quick_read.bb11:
.quick_read.bb1:
	mv	a0, t1
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 32
	ret
.quick_read.bb10:
	subw	t1, zero, t1
	j	.quick_read.bb1
	.size quick_read, .-quick_read

  .globl sortB
  .type  sortB, @function
sortB:
.sortB.bb0:
	addi	sp, sp, -32
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	mv	s1, a0
	la	t0, n
	lw	s2, 0(t0)
	mv	t0, s1
	li	t1, -100
	li	t2, 0
.sortB.bb12:
	addiw	s3, t2, 8
	bge	t2, s2, .sortB.bb3
.sortB.bb13:
	bge	s3, s2, .sortB.bb14
.sortB.bb15:
	j	.sortB.bb1
.sortB.bb16:
.sortB.bb1:
	bge	t2, s3, .sortB.bb12
.sortB.bb2:
	lw	t3, 0(t0)
	la	a2, cnt
	slliw	t3, t3, 2
	add	s4, a2, t3
	lw	t3, 0(s4)
	addiw	t3, t3, 1
	sw	t3, 0(s4)
	lw	s4, 0(t0)
	blt	t1, s4, .sortB.bb5
	j	.sortB.bb18
.sortB.bb4:
.sortB.bb5:
	addiw	t2, t2, 1
	addi	t0, t0, 4
	mv	t1, s4	# phi-copy
	j	.sortB.bb1
.sortB.bb18:
	mv	s4, t1	# phi-copy
	j	.sortB.bb5
.sortB.bb17:
	j	.sortB.bb12
.sortB.bb14:
	mv	s3, s2	# phi-copy
	j	.sortB.bb1
.sortB.bb10:
	sw	t3, 0(a3)
	addiw	t2, t2, 1
	addiw	a2, a2, -1
	addi	a3, a3, 4
	bne	a2, zero, .sortB.bb10	# rotated-loop-branch
.sortB.bb11:
	addiw	t3, t3, 1
	addi	t0, t0, 4
	blt	t1, t3, .sortB.bb8	# rotated-loop-branch
.sortB.bb7:
	lw	a2, 0(t0)
	slliw	a3, t2, 2
	add	a3, s1, a3
.sortB.bb9:
	beq	a2, zero, .sortB.bb11
	j	.sortB.bb10
.sortB.bb3:
	la	t0, cnt
	li	t2, 0
	li	t3, 0
.sortB.bb6:
	bge	t1, t3, .sortB.bb7
.sortB.bb8:
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 32
	ret
	.size sortB, .-sortB

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	call	quick_read
	mv	t0, a0
	la	t1, n
	sw	t0, 0(t1)
	li	s1, 0
.main.bb2:
	la	t0, n
	lw	s2, 0(t0)
	beq	s1, s2, .main.bb4
.main.bb3:
	call	quick_read
	mv	s2, a0
	slliw	s3, s1, 2
	la	t0, a
	add	t0, t0, s3
	sw	s2, 0(t0)
	la	t0, b
	add	t0, t0, s3
	sw	s2, 0(t0)
	la	t0, c
	add	t0, t0, s3
	sw	s2, 0(t0)
	addiw	s1, s1, 1
	j	.main.bb2
.main.bb18:
	bge	s5, s2, .main.bb20
.main.bb19:
	lw	s6, 0(s3)
	lw	s7, 0(t0)
	blt	s7, s6, .main.bb21
.main.bb22:
	addiw	s5, s5, 1
	addi	t0, t0, 4
	j	.main.bb18
.main.bb21:
	sw	s7, 0(s3)
	sw	s6, 0(t0)
	j	.main.bb22
.main.bb20:
	addi	s3, s3, 4
	mv	t0, s4	# phi-copy
	bge	t0, s2, .main.bb17	# rotated-loop-branch
.main.bb16:
	addiw	s4, t0, 1
	la	t1, a
	slliw	t0, s4, 2
	add	t0, t1, t0
	mv	s5, s4	# phi-copy
	j	.main.bb18
.main.bb26:
	bge	s6, s2, .main.bb28
.main.bb27:
	lw	t3, 0(t1)
	slliw	a2, t0, 2
	add	a2, t2, a2
	lw	a2, 0(a2)
	bge	t3, a2, .main.bb30
.main.bb29:
	mv	t0, s6	# phi-copy
.main.bb30:
	addiw	s6, s6, 1
	addi	t1, t1, 4
	j	.main.bb26
.main.bb31:
	j	.main.bb30
.main.bb28:
	lw	t1, 0(s4)
	slliw	t0, t0, 2
	la	t2, c
	add	s6, t2, t0
	lw	t0, 0(s6)
	sw	t0, 0(s4)
	sw	t1, 0(s6)
	addi	s4, s4, 4
	mv	t0, s5	# phi-copy
	bge	t0, s2, .main.bb25	# rotated-loop-branch
.main.bb24:
	addiw	s5, t0, 1
	la	t2, c
	slliw	t1, s5, 2
	add	t1, t2, t1
	mv	s6, s5	# phi-copy
	j	.main.bb26
.main.bb17:
	la	s3, n
	sw	s2, 0(s3)
	la	t5, b
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	sortB
	lw	s2, 0(s3)
	la	s3, c
	li	t0, 0
	mv	s4, s3	# phi-copy
.main.bb23:
	blt	t0, s2, .main.bb24
.main.bb25:
	la	s4, b
	li	t0, 0
	mv	s5, s1	# phi-copy
	mv	s1, s3	# phi-copy
	mv	s6, s4	# phi-copy
.main.bb5:
	subw	t1, s2, t0
	beq	t1, zero, .main.bb7
.main.bb6:
	lw	t1, 0(s6)
	addiw	t0, t0, 1
	lw	s7, 0(s5)
	addi	t2, s5, 4
	addi	t3, s1, 4
	subw	s8, t1, s7
	sw	s8, 0(s6)
	lw	t1, 0(s1)
	subw	t1, t1, s8
	subw	t1, t1, s7
	sw	t1, 0(s1)
	addi	s6, s6, 4
	mv	s1, t3	# phi-copy
	mv	s5, t2	# phi-copy
	j	.main.bb5
.main.bb4:
	la	s1, a
	li	t0, 0
	mv	s3, s1	# phi-copy
.main.bb15:
	bge	t0, s2, .main.bb17
	j	.main.bb16
.main.bb7:
	la	t0, n
	li	t1, 0
	lw	t0, 0(t0)
.main.bb8:
	subw	t2, t1, t0
	beq	t2, zero, .main.bb10
.main.bb9:
	lw	t2, 0(s4)
	bne	t2, zero, .main.bb11
.main.bb12:
	lw	t2, 0(s3)
	bne	t2, zero, .main.bb13
.main.bb14:
	addiw	t1, t1, 1
	addi	s4, s4, 4
	addi	s3, s3, 4
	j	.main.bb8
.main.bb10:
	li	t0, -123
.main.bb1:
	mv	a0, t0
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 80
	ret
.main.bb11:
	li	t0, 1
	j	.main.bb1
.main.bb13:
	li	t0, 2
	j	.main.bb1
	.size main, .-main
