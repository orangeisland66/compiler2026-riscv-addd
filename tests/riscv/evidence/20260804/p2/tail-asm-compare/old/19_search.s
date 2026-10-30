
  .section .rodata
  .globl INF
  .align 2
  .type  INF, @object
  .size  INF, 4
INF:
  .word 1073741824

  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 3600
a:
  .zero 3600

  .data
  .globl step
  .align 2
  .type  step, @object
  .size  step, 32
step:
  .word 1
  .word 0
  .word -1
  .word 0
  .word 0
  .word 1
  .word 0
  .word -1

  .bss
  .globl w
  .align 2
  .type  w, @object
  .size  w, 4
w:
  .zero 4

  .bss
  .globl h
  .align 2
  .type  h, @object
  .size  h, 4
h:
  .zero 4

  .bss
  .globl x_0
  .align 2
  .type  x_0, @object
  .size  x_0, 4
x_0:
  .zero 4

  .bss
  .globl y_0
  .align 2
  .type  y_0, @object
  .size  y_0, 4
y_0:
  .zero 4

  .bss
  .globl x_1
  .align 2
  .type  x_1, @object
  .size  x_1, 4
x_1:
  .zero 4

  .bss
  .globl y_1
  .align 2
  .type  y_1, @object
  .size  y_1, 4
y_1:
  .zero 4

  .text

  .globl search
  .type  search, @function
search:
.search.bb0:
	addi	sp, sp, -112
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	sd	s3, 48(sp)
	sd	s4, 56(sp)
	sd	s5, 64(sp)
	sd	s6, 72(sp)
	sd	s7, 80(sp)
	sd	s8, 88(sp)
	sd	s9, 96(sp)
	mv	s1, a0
	mv	s2, a1
	mv	t0, a2
	li	t1, 10
	bge	t1, t0, .search.bb3
.search.bb2:
	li	s5, 1073741824
.search.bb1:
	mv	a0, s5
	ld	s9, 96(sp)
	ld	s8, 88(sp)
	ld	s7, 80(sp)
	ld	s6, 72(sp)
	ld	s5, 64(sp)
	ld	s4, 56(sp)
	ld	s3, 48(sp)
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 112
	ret
.search.bb7:
	slliw	a5, s7, 7
	slliw	a6, s7, 3
	subw	a5, a5, a6
	slliw	a6, s8, 2
	addw	a5, a5, a6
	add	s9, a4, a5
	lw	a5, 0(s9)
	beq	a5, a3, .search.bb9
.search.bb8:
	subw	a5, s7, t0
	subw	a6, s8, t1
	sltiu	a5, a5, 1
	sltiu	a6, a6, 1
	and	a5, a5, a6
	bne	a5, zero, .search.bb9
.search.bb11:
	addw	s7, s7, t2
	addw	s8, s8, t3
	addiw	a2, a2, 1
	j	.search.bb7
.search.bb9:
	la	t0, x_1
	la	t1, y_1
	lw	t0, 0(t0)
	lw	t1, 0(t1)
	subw	t0, s7, t0
	subw	t1, s8, t1
	sltiu	t0, t0, 1
	sltiu	t1, t1, 1
	and	t0, t0, t1
	bne	t0, zero, .search.bb12
.search.bb13:
	li	t0, 1
	bge	t0, a2, .search.bb14
.search.bb15:
	la	t1, h
	li	t0, 0
	lw	t1, 0(t1)
	subw	t0, s7, t0
	sltiu	t0, t0, 1
	addiw	t1, t1, 1
	subw	t1, s7, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	li	t1, 0
	subw	t1, s8, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	la	t1, w
	lw	t1, 0(t1)
	addiw	t1, t1, 1
	subw	t1, s8, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	bne	t0, zero, .search.bb16
.search.bb17:
	li	t0, 0
	sw	t0, 0(s9)
	lw	t0, 0(s6)
	subw	a0, s7, t0
	lw	t1, 4(s6)
	subw	a1, s8, t1
	mv	a2, s3	# call-arg-copy
	call	search
	mv	t0, a0
	addiw	s6, t0, 1
	blt	s6, s5, .search.bb19
.search.bb22:
	mv	s6, s5	# phi-copy
.search.bb19:
	li	t0, 1
	sw	t0, 0(s9)
	addiw	s4, s4, 1
	mv	s5, s6	# phi-copy
.search.bb4:
	li	t0, 4
	bge	s4, t0, .search.bb6
.search.bb5:
	slliw	t0, s4, 3
	la	t1, step
	add	s6, t1, t0
	li	a2, 0
	lw	t2, 0(s6)
	lw	t3, 4(s6)
	la	t0, x_1
	lw	t0, 0(t0)
	la	t1, y_1
	lw	t1, 0(t1)
	li	a3, 1
	la	a4, a	# loop.addr.hoisted
	mv	s7, s1	# phi-copy
	mv	s8, s2	# phi-copy
	j	.search.bb7
.search.bb10:
	j	.search.bb9
.search.bb14:
	addiw	s4, s4, 1
	j	.search.bb4
.search.bb6:
	li	t0, 10
	bge	t0, s5, .search.bb1
.search.bb20:
	li	s5, 1073741824
	j	.search.bb1
.search.bb16:
	addiw	s4, s4, 1
	j	.search.bb4
.search.bb12:
	li	s5, 1
	j	.search.bb1
.search.bb18:
	j	.search.bb19
.search.bb21:
	j	.search.bb1
.search.bb3:
	addiw	s3, t0, 1
	li	s4, 0
	li	s5, 1073741824
	j	.search.bb4
	.size search, .-search

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	sd	s3, 48(sp)
	sd	s4, 56(sp)
	sd	s5, 64(sp)
	sd	s6, 72(sp)
	call	getint
	mv	t0, a0
	la	t1, w
	sw	t0, 0(t1)
	call	getint
	mv	t0, a0
	la	t1, h
	sw	t0, 0(t1)
.main.bb1:
	la	t0, w
	lw	t0, 0(t0)
	beq	t0, zero, .main.bb3
.main.bb2:
	li	t0, 0
.main.bb4:
	li	t1, 30
	bge	t0, t1, .main.bb6
.main.bb5:
	slliw	t1, t0, 7
	slliw	t2, t0, 3
	subw	t1, t1, t2
	li	t3, 30
	la	t2, a
	add	t1, t2, t1
	li	t2, 0
.main.bb7:
	bge	t2, t3, .main.bb9
.main.bb8:
	li	a2, 1
	addiw	t2, t2, 1
	sw	a2, 0(t1)
	addi	t1, t1, 4
	blt	t2, t3, .main.bb8	# rotated-loop-branch
.main.bb9:
	addiw	t0, t0, 1
	j	.main.bb4
.main.bb13:
	blt	s1, s5, .main.bb15
.main.bb14:
	call	getint
	mv	s6, a0
	slliw	t0, s5, 2
	addw	t0, s4, t0
	la	t1, a
	add	t0, t1, t0
	sw	s6, 0(t0)
	li	t0, 2
	beq	s6, t0, .main.bb16
.main.bb17:
	li	t0, 3
	beq	s6, t0, .main.bb19
	j	.main.bb18
.main.bb20:
.main.bb18:
	addiw	s5, s5, 1
	j	.main.bb13
.main.bb16:
	la	t0, x_0
	sw	s3, 0(t0)
	la	t0, y_0
	sw	s5, 0(t0)
	j	.main.bb18
.main.bb19:
	la	t0, x_1
	sw	s3, 0(t0)
	la	t0, y_1
	sw	s5, 0(t0)
	j	.main.bb18
.main.bb15:
	addiw	s3, s3, 1
.main.bb10:
	blt	s2, s3, .main.bb12
.main.bb11:
	slliw	t0, s3, 7
	slliw	t1, s3, 3
	subw	s4, t0, t1
	li	s5, 1
	j	.main.bb13
.main.bb12:
	la	t0, x_0
	lw	t5, 0(t0)
	la	t1, y_0
	lw	t1, 0(t1)
	sd	t5, 0(sp)
	mv	t5, t1
	sd	t5, 8(sp)
	li	t5, 1
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	search
	mv	s1, a0
	li	t0, 10
	blt	t0, s1, .main.bb22
.main.bb21:
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
.main.bb23:
	call	getint
	mv	t0, a0
	la	t1, w
	sw	t0, 0(t1)
	call	getint
	mv	t0, a0
	la	t1, h
	sw	t0, 0(t1)
	j	.main.bb1
.main.bb6:
	la	t0, w
	lw	s1, 0(t0)
	la	t0, h
	lw	s2, 0(t0)
	li	s3, 1
	j	.main.bb10
.main.bb22:
	li	t5, -1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb23
.main.bb3:
	li	a0, 0
	ld	s6, 72(sp)
	ld	s5, 64(sp)
	ld	s4, 56(sp)
	ld	s3, 48(sp)
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
