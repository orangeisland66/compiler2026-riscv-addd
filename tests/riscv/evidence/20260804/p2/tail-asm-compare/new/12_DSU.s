
  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .bss
  .globl m
  .align 2
  .type  m, @object
  .size  m, 4
m:
  .zero 4

  .bss
  .globl fa
  .align 2
  .type  fa, @object
  .size  fa, 400020
fa:
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

  .globl find
  .type  find, @function
find:
.find.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	mv	t0, a0
	slliw	t1, t0, 2
	la	t2, fa
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

  .globl same
  .type  same, @function
same:
.same.bb0:
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
	subw	t0, s2, t0
	sltiu	t0, t0, 1
.same.bb1:
	mv	a0, t0
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size same, .-same

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	call	quick_read
	mv	t0, a0
	la	s1, n
	sw	t0, 0(s1)
	call	quick_read
	mv	t0, a0
	la	t1, m
	sw	t0, 0(t1)
	lw	s1, 0(s1)
	la	t0, fa
	addi	t0, t0, 4
	li	t1, 1
.main.bb10:
	blt	s1, t1, .main.bb12
.main.bb11:
	sw	t1, 0(t0)
	addiw	t1, t1, 1
	addi	t0, t0, 4
	bge	s1, t1, .main.bb11	# rotated-loop-branch
.main.bb12:
.main.bb1:
	la	t0, m
	lw	t0, 0(t0)
	beq	t0, zero, .main.bb3
.main.bb2:
	call	getch
	mv	t0, a0
.main.bb4:
	li	t1, 81
	li	t2, 85
	subw	t1, t0, t1
	subw	t2, t0, t2
	sltu	t1, zero, t1
	sltu	t2, zero, t2
	and	t1, t1, t2
	beq	t1, zero, .main.bb6
.main.bb5:
	call	getch
	mv	t0, a0
	j	.main.bb4
.main.bb6:
	li	t1, 81
	beq	t0, t1, .main.bb7
.main.bb8:
	call	quick_read
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	find
	mv	s1, a0
	call	quick_read
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	find
	mv	t0, a0
	slliw	t1, s1, 2
	la	t2, fa
	add	t1, t2, t1
	sw	t0, 0(t1)
.main.bb9:
	la	s1, m
	lw	t0, 0(s1)
	addiw	t0, t0, -1
	sw	t0, 0(s1)
	j	.main.bb1
.main.bb7:
	call	quick_read
	mv	s1, a0
	call	quick_read
	mv	t0, a0
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	same
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb9
.main.bb3:
	li	a0, 0
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
