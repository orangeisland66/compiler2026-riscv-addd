
  .section .rodata
  .globl maxn
  .align 2
  .type  maxn, @object
  .size  maxn, 4
maxn:
  .word 10005

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
  .globl f
  .align 2
  .type  f, @object
  .size  f, 800400
f:
  .zero 800400

  .bss
  .globl dep
  .align 2
  .type  dep, @object
  .size  dep, 40020
dep:
  .zero 40020

  .bss
  .globl to
  .align 2
  .type  to, @object
  .size  to, 40020
to:
  .zero 40020

  .bss
  .globl next
  .align 2
  .type  next, @object
  .size  next, 40020
next:
  .zero 40020

  .bss
  .globl head
  .align 2
  .type  head, @object
  .size  head, 40020
head:
  .zero 40020

  .bss
  .globl cnt
  .align 2
  .type  cnt, @object
  .size  cnt, 4
cnt:
  .zero 4

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

  .globl tree
  .type  tree, @function
tree:
.tree.bb0:
	addi	sp, sp, -64
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	mv	t0, a0
	mv	s1, a1
	slliw	s2, t0, 2
	la	t1, dep
	add	t1, t1, s2
	sw	s1, 0(t1)
	slliw	t1, t0, 6
	slliw	t0, t0, 4
	addw	t0, t1, t0
	li	t1, 0
.tree.bb1:
	slliw	s3, t1, 2
	la	t3, f
	addw	t2, t0, s3
	add	s4, t3, t2
	lw	s5, 0(s4)
	beq	s5, zero, .tree.bb3
.tree.bb2:
	slliw	t2, s5, 6
	slliw	t3, s5, 4
	addw	t2, t2, t3
	addiw	t1, t1, 1
	addw	t2, t2, s3
	la	t3, f
	add	t2, t3, t2
	lw	t2, 0(t2)
	sw	t2, 4(s4)
	j	.tree.bb1
.tree.bb3:
	la	t0, head
	addiw	s1, s1, 1
	add	t0, t0, s2
	lw	t0, 0(t0)
	li	s2, -1
.tree.bb4:
	beq	t0, s2, .tree.bb6
.tree.bb5:
	slliw	s3, t0, 2
	la	t0, to
	add	t0, t0, s3
	lw	a0, 0(t0)
	mv	a1, s1	# call-arg-copy
	call	tree
	la	t0, next
	add	t0, t0, s3
	lw	t0, 0(t0)
	bne	t0, s2, .tree.bb5	# rotated-loop-branch
.tree.bb6:
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 64
	ret
	.size tree, .-tree

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	call	quick_read
	mv	t0, a0
	la	s1, n
	sw	t0, 0(s1)
	call	quick_read
	mv	t0, a0
	la	t1, m
	sw	t0, 0(t1)
	lw	s1, 0(s1)
	li	t0, 1061109567
	la	t1, dep
	sw	t0, 0(t1)
	la	t0, head
	addi	t0, t0, 4
	li	t1, 1
.main.bb7:
	blt	s1, t1, .main.bb9
.main.bb8:
	li	t2, -1
	addiw	t1, t1, 1
	sw	t2, 0(t0)
	addi	t0, t0, 4
	bge	s1, t1, .main.bb8	# rotated-loop-branch
.main.bb9:
	li	s1, 1
.main.bb1:
	la	t0, n
	lw	t0, 0(t0)
	beq	s1, t0, .main.bb3
.main.bb2:
	call	quick_read
	mv	s2, a0
	call	quick_read
	mv	s3, a0
	la	s4, cnt
	lw	s5, 0(s4)
	slliw	s6, s5, 2
	la	t0, to
	add	t0, t0, s6
	sw	s3, 0(t0)
	slliw	t0, s2, 2
	la	t1, head
	add	s7, t1, t0
	lw	t0, 0(s7)
	la	t1, next
	add	t1, t1, s6
	sw	t0, 0(t1)
	sw	s5, 0(s7)
	addiw	t0, s5, 1
	slliw	t1, s3, 6
	slliw	t2, s3, 4
	addw	t1, t1, t2
	la	t2, f
	add	t1, t2, t1
	sw	s2, 0(t1)
	sw	t0, 0(s4)
	addiw	s1, s1, 1
	j	.main.bb1
.main.bb13:
	slliw	t1, s2, 2
	la	t2, dep
	add	t1, t2, t1
	lw	t1, 0(t1)
	bge	s3, t1, .main.bb15
.main.bb14:
	slliw	t1, s2, 6
	slliw	t2, s2, 4
	addw	t1, t1, t2
	la	t3, dep
	slliw	t2, t0, 2
	addw	t1, t1, t2
	la	t2, f
	add	t1, t2, t1
	lw	s4, 0(t1)
	slliw	t2, s4, 2
	li	t1, 0
	add	t2, t3, t2
	subw	t1, s4, t1
	lw	t2, 0(t2)
	sltu	t1, zero, t1
	slt	t2, t2, s3
	xori	t2, t2, 1
	and	t1, t1, t2
	bne	t1, zero, .main.bb17
	j	.main.bb26
.main.bb16:
.main.bb17:
	addiw	t0, t0, -1
	mv	s2, s4	# phi-copy
	j	.main.bb13
.main.bb26:
	mv	s4, s2	# phi-copy
	j	.main.bb17
.main.bb15:
	bne	s2, s1, .main.bb19
.main.bb18:
	mv	t0, s2	# phi-copy
.main.bb10:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	la	s1, m
	lw	t0, 0(s1)
	addiw	t0, t0, -1
	sw	t0, 0(s1)
.main.bb4:
	la	t0, m
	lw	t0, 0(t0)
	beq	t0, zero, .main.bb6
.main.bb5:
	call	quick_read
	mv	s1, a0
	call	quick_read
	mv	s2, a0
	slliw	t0, s1, 2
	la	s3, dep
	add	t0, s3, t0
	lw	t0, 0(t0)
	slliw	t1, s2, 2
	add	t1, s3, t1
	lw	t1, 0(t1)
	blt	t0, t1, .main.bb12
	j	.main.bb25
.main.bb11:
.main.bb12:
	slliw	t0, s1, 2
	la	t1, dep
	add	t0, t1, t0
	lw	s3, 0(t0)
	li	t0, 19
	j	.main.bb13
.main.bb19:
	li	t0, 19
.main.bb20:
	blt	t0, zero, .main.bb22
.main.bb21:
	slliw	t1, s2, 6
	slliw	t2, s2, 4
	addw	t1, t1, t2
	slliw	s3, t0, 2
	addw	t1, t1, s3
	la	s4, f
	add	t1, s4, t1
	slliw	t2, s1, 4
	lw	s5, 0(t1)
	slliw	t1, s1, 6
	addw	t1, t1, t2
	addw	t1, t1, s3
	add	t1, s4, t1
	lw	s3, 0(t1)
	bne	s5, s3, .main.bb24
	j	.main.bb27
.main.bb23:
.main.bb24:
	addiw	t0, t0, -1
	mv	s2, s5	# phi-copy
	mv	s1, s3	# phi-copy
	j	.main.bb20
.main.bb27:
	mv	s5, s2	# phi-copy
	mv	s3, s1	# phi-copy
	j	.main.bb24
.main.bb22:
	slliw	t0, s2, 6
	slliw	t1, s2, 4
	addw	t0, t0, t1
	la	t1, f
	add	t0, t1, t0
	lw	t0, 0(t0)
	j	.main.bb10
.main.bb25:
	mv	t5, s1	# phi-cycle-break
	mv	s1, s2	# phi-copy
	mv	s2, t5	# phi-copy
	j	.main.bb12
.main.bb3:
	li	t5, 1
	sd	t5, 0(sp)
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	tree
	j	.main.bb4
.main.bb6:
	li	a0, 0
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
