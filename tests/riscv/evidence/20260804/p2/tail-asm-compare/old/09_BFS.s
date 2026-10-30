
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

  .section .rodata
  .globl maxn
  .align 2
  .type  maxn, @object
  .size  maxn, 4
maxn:
  .word 1005

  .section .rodata
  .globl maxm
  .align 2
  .type  maxm, @object
  .size  maxm, 4
maxm:
  .word 5005

  .bss
  .globl to
  .align 2
  .type  to, @object
  .size  to, 20020
to:
  .zero 20020

  .bss
  .globl next
  .align 2
  .type  next, @object
  .size  next, 20020
next:
  .zero 20020

  .bss
  .globl head
  .align 2
  .type  head, @object
  .size  head, 4020
head:
  .zero 4020

  .bss
  .globl cnt
  .align 2
  .type  cnt, @object
  .size  cnt, 4
cnt:
  .zero 4

  .bss
  .globl que
  .align 2
  .type  que, @object
  .size  que, 4020
que:
  .zero 4020

  .bss
  .globl h
  .align 2
  .type  h, @object
  .size  h, 4
h:
  .zero 4

  .bss
  .globl tail
  .align 2
  .type  tail, @object
  .size  tail, 4
tail:
  .zero 4

  .bss
  .globl inq
  .align 2
  .type  inq, @object
  .size  inq, 4020
inq:
  .zero 4020

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

  .globl same
  .type  same, @function
same:
.same.bb0:
	addi	sp, sp, -48
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	mv	t0, a0
	mv	t1, a1
	slliw	t2, t0, 2
	la	t3, inq
	add	t2, t3, t2
	li	t3, 1
	sw	t3, 0(t2)
	la	t2, que
	sw	t0, 4(t2)
	li	t0, 0
	li	t2, 1
	li	t3, 0
	la	s1, que	# loop.addr.hoisted
.same.bb1:
	bge	t0, t2, .same.bb3
.same.bb2:
	addiw	s2, t0, 1
	slliw	t0, s2, 2
	add	t0, s1, t0
	lw	s3, 0(t0)
	bne	s3, t1, .same.bb14
.same.bb4:
	li	t0, 1
.same.bb5:
	slliw	t3, s3, 2
	la	a2, head
	add	t3, a2, t3
	lw	t3, 0(t3)
	li	a2, -1
.same.bb6:
	beq	t3, a2, .same.bb8
.same.bb7:
	slliw	s3, t3, 2
	la	a3, inq
	la	t3, to
	add	t3, t3, s3
	lw	s4, 0(t3)
	slliw	t3, s4, 2
	add	s5, a3, t3
	lw	t3, 0(s5)
	bne	t3, zero, .same.bb15
.same.bb9:
	li	t3, 1
	sw	t3, 0(s5)
	addiw	s5, t2, 1
	slliw	t2, s5, 2
	add	t2, s1, t2
	sw	s4, 0(t2)
.same.bb10:
	la	t2, next
	add	t2, t2, s3
	lw	t3, 0(t2)
	mv	t2, s5	# phi-copy
	j	.same.bb6
.same.bb15:
	mv	s5, t2	# phi-copy
	j	.same.bb10
.same.bb8:
	mv	t3, t0	# phi-copy
	mv	t0, s2	# phi-copy
	blt	t0, t2, .same.bb2	# rotated-loop-branch
.same.bb3:
	la	t1, que
	li	a2, 0
.same.bb11:
	blt	t2, a2, .same.bb13
.same.bb12:
	lw	a3, 0(t1)
	addiw	a2, a2, 1
	la	a4, inq
	slliw	a3, a3, 2
	addi	t1, t1, 4
	add	a3, a4, a3
	li	a4, 0
	sw	a4, 0(a3)
	bge	t2, a2, .same.bb12	# rotated-loop-branch
.same.bb13:
	la	t1, tail
	sw	t2, 0(t1)
	la	t1, h
	sw	t0, 0(t1)
	mv	a0, t3
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 48
	ret
.same.bb14:
	mv	t0, t3	# phi-copy
	j	.same.bb5
	.size same, .-same

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -96
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	sd	s8, 80(sp)
	call	quick_read
	call	quick_read
	mv	t0, a0
	la	t1, m
	sw	t0, 0(t1)
	la	t0, head
	li	t1, 0
.main.bb10:
	li	t2, 1005
	bge	t1, t2, .main.bb1
.main.bb13:
	addiw	t2, t1, 3
	li	t3, 1005
	blt	t2, t3, .main.bb14
.main.bb11:
	li	t2, -1
	addiw	t1, t1, 1
	sw	t2, 0(t0)
	addi	t0, t0, 4
	j	.main.bb10
.main.bb14:
	li	t3, -1
	li	t2, -1
	sw	t3, 4(t0)
	sw	t2, 0(t0)
	sw	t3, 8(t0)
	addi	t2, t0, 4
	addi	t2, t2, 4
	sw	t3, 12(t0)
	addi	t2, t2, 4
	addiw	t1, t1, 4
	addi	t0, t2, 4
	j	.main.bb10
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
	mv	s1, a0
	call	quick_read
	mv	s2, a0
	la	s3, cnt
	lw	s4, 0(s3)
	slliw	s5, s4, 2
	la	t0, to
	add	s6, t0, s5
	sw	s2, 0(s6)
	slliw	t0, s1, 2
	la	s7, head
	add	s8, s7, t0
	lw	t0, 0(s8)
	la	t1, next
	add	s5, t1, s5
	sw	t0, 0(s5)
	sw	s4, 0(s8)
	addiw	t0, s4, 1
	sw	s1, 4(s6)
	slliw	t1, s2, 2
	add	s1, s7, t1
	lw	t1, 0(s1)
	sw	t1, 4(s5)
	sw	t0, 0(s1)
	addiw	t0, s4, 2
	sw	t0, 0(s3)
.main.bb9:
	la	s1, m
	lw	t0, 0(s1)
	addiw	t0, t0, -1
	sw	t0, 0(s1)
.main.bb1:
	la	t0, m
	lw	t0, 0(t0)
	beq	t0, zero, .main.bb3
.main.bb2:
	call	getch
	mv	t0, a0
	j	.main.bb4
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
.main.bb12:
	j	.main.bb1
.main.bb3:
	li	a0, 0
	ld	s8, 80(sp)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 96
	ret
	.size main, .-main
