
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
  .globl vis
  .align 2
  .type  vis, @object
  .size  vis, 4020
vis:
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
	addi	sp, sp, -64
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	mv	t0, a0
	mv	s1, a1
	slliw	t1, t0, 2
	la	t2, vis
	add	t2, t2, t1
	li	t3, 1
	sw	t3, 0(t2)
	bne	t0, s1, .same.bb3
.same.bb2:
	li	t0, 1
.same.bb1:
	mv	a0, t0
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 64
	ret
.same.bb4:
	li	t1, -1
	beq	t0, t1, .same.bb6
.same.bb5:
	slliw	s2, t0, 2
	la	t1, vis
	la	t0, to
	add	t0, t0, s2
	lw	s3, 0(t0)
	slliw	t0, s3, 2
	add	t0, t1, t0
	lw	t0, 0(t0)
	li	t1, 0
	subw	t0, t0, t1
	sltiu	s4, t0, 1
	beqz	s4, .same.bb11
.same.bb8:
	mv	a0, s3	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	call	same
	mv	t0, a0
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
.same.bb7:
	bne	t0, zero, .same.bb9
.same.bb10:
	la	t0, next
	add	t0, t0, s2
	lw	t0, 0(t0)
	j	.same.bb4
.same.bb11:
	mv	t0, s4	# phi-copy
	j	.same.bb7
.same.bb6:
	li	t0, 0
	j	.same.bb1
.same.bb9:
	li	t0, 1
	j	.same.bb1
.same.bb3:
	la	t0, head
	add	t0, t0, t1
	lw	t0, 0(t0)
	j	.same.bb4
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
	sd	s9, 88(sp)
	call	quick_read
	mv	t0, a0
	la	t1, n
	sw	t0, 0(t1)
	call	quick_read
	mv	t0, a0
	la	t1, m
	sw	t0, 0(t1)
	la	t0, head
	li	t1, 0
.main.bb10:
	li	t2, 1005
	bge	t1, t2, .main.bb12
.main.bb16:
	addiw	t2, t1, 3
	li	t3, 1005
	blt	t2, t3, .main.bb17
.main.bb11:
	li	t2, -1
	addiw	t1, t1, 1
	sw	t2, 0(t0)
	addi	t0, t0, 4
	j	.main.bb10
.main.bb17:
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
	mv	s2, a0
	call	quick_read
	mv	s3, a0
	la	s4, cnt
	lw	s5, 0(s4)
	slliw	s6, s5, 2
	la	t0, to
	add	s7, t0, s6
	sw	s3, 0(s7)
	slliw	t0, s2, 2
	la	s8, head
	add	s9, s8, t0
	lw	t0, 0(s9)
	la	t1, next
	add	s6, t1, s6
	sw	t0, 0(s6)
	sw	s5, 0(s9)
	addiw	t0, s5, 1
	sw	s2, 4(s7)
	slliw	t1, s3, 2
	add	s2, s8, t1
	lw	t1, 0(s2)
	sw	t1, 4(s6)
	sw	t0, 0(s2)
	addiw	t0, s5, 2
	sw	t0, 0(s4)
.main.bb9:
	la	s2, m
	lw	t0, 0(s2)
	addiw	t0, t0, -1
	sw	t0, 0(s2)
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
	mv	s2, a0
	call	quick_read
	mv	t0, a0
	la	t1, n
	lw	s3, 0(t1)
	li	t1, 1
	mv	s4, s1	# phi-copy
.main.bb13:
	blt	s3, t1, .main.bb15
.main.bb14:
	li	t2, 0
	addiw	t1, t1, 1
	sw	t2, 0(s4)
	addi	s4, s4, 4
	bge	s3, t1, .main.bb14	# rotated-loop-branch
.main.bb15:
	mv	t5, s2
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
	la	t0, vis
	addi	s1, t0, 4
	j	.main.bb1
.main.bb3:
	li	a0, 0
	ld	s9, 88(sp)
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
