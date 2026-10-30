
  .section .rodata
  .globl maxm
  .align 2
  .type  maxm, @object
  .size  maxm, 4
maxm:
  .word 1005

  .section .rodata
  .globl maxn
  .align 2
  .type  maxn, @object
  .size  maxn, 4
maxn:
  .word 105

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
  .globl u
  .align 2
  .type  u, @object
  .size  u, 4020
u:
  .zero 4020

  .bss
  .globl v
  .align 2
  .type  v, @object
  .size  v, 4020
v:
  .zero 4020

  .bss
  .globl c
  .align 2
  .type  c, @object
  .size  c, 4020
c:
  .zero 4020

  .bss
  .globl fa
  .align 2
  .type  fa, @object
  .size  fa, 4020
fa:
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
	li	t1, 0
	mv	s1, t1	# phi-copy
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
	li	t0, 1
	mv	s1, t0	# phi-copy
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
	subw	t0, zero, t1
	mv	t1, t0	# phi-copy
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
	beq	t0, t1, .find.bb1
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

  .globl prim
  .type  prim, @function
prim:
.prim.bb0:
	addi	sp, sp, -64
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	la	t0, m
	la	t1, u
	lw	s1, 0(t0)
	la	t2, v
	li	t3, 0
	la	t0, c
.prim.bb1:
	bge	t3, s1, .prim.bb3
.prim.bb2:
	addiw	s2, t3, 1
	la	a2, u
	slliw	s3, s2, 2
	la	t3, c
	la	a3, v
	add	t3, t3, s3
	add	a2, a2, s3
	add	a3, a3, s3
	mv	s3, s2	# phi-copy
.prim.bb4:
	bge	s3, s1, .prim.bb6
.prim.bb5:
	lw	s4, 0(t0)
	lw	s5, 0(t3)
	blt	s5, s4, .prim.bb7
.prim.bb8:
	addiw	s3, s3, 1
	addi	t3, t3, 4
	addi	a2, a2, 4
	addi	a3, a3, 4
	j	.prim.bb4
.prim.bb7:
	lw	a4, 0(t1)
	lw	a5, 0(a2)
	sw	a5, 0(t1)
	sw	a4, 0(a2)
	lw	a4, 0(t2)
	lw	a5, 0(a3)
	sw	a5, 0(t2)
	sw	a4, 0(a3)
	sw	s5, 0(t0)
	sw	s4, 0(t3)
	j	.prim.bb8
.prim.bb6:
	addi	t0, t0, 4
	addi	t1, t1, 4
	addi	t2, t2, 4
	mv	t3, s2	# phi-copy
	blt	t3, s1, .prim.bb2	# rotated-loop-branch
.prim.bb3:
	la	t0, n
	la	t1, fa
	lw	t0, 0(t0)
	addi	t1, t1, 4
	li	t2, 1
.prim.bb9:
	blt	t0, t2, .prim.bb11
.prim.bb10:
	sw	t2, 0(t1)
	addiw	t2, t2, 1
	addi	t1, t1, 4
	bge	t0, t2, .prim.bb10	# rotated-loop-branch
.prim.bb11:
	li	s2, 0
	li	t1, 0
	mv	s1, t1	# phi-copy
.prim.bb12:
	la	t0, m
	lw	t0, 0(t0)
	bge	s1, t0, .prim.bb14
.prim.bb13:
	slliw	s3, s1, 2
	la	t0, u
	add	s4, t0, s3
	lw	a0, 0(s4)
	la	t1, v
	add	s5, t1, s3
	lw	t1, 0(s5)
	mv	a1, t1	# call-arg-copy
	call	same
	mv	t0, a0
	bne	t0, zero, .prim.bb12
	j	.prim.bb16
.prim.bb15:
	j	.prim.bb12
.prim.bb16:
	la	t0, c
	add	t0, t0, s3
	lw	t0, 0(t0)
	addw	s2, s2, t0
	lw	s3, 0(s5)
	lw	t0, 0(s4)
	mv	a0, t0	# call-arg-copy
	call	find
	mv	t0, a0
	slliw	t0, t0, 2
	la	t1, fa
	add	t0, t1, t0
	sw	s3, 0(t0)
	addiw	t0, s1, 1
	mv	s1, t0	# phi-copy
	j	.prim.bb12
.prim.bb14:
	mv	a0, s2
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 64
	ret
	.size prim, .-prim

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	sd	s3, 24(sp)
	call	quick_read
	call	quick_read
	mv	s1, a0
	li	t0, 0
	mv	s2, t0	# phi-copy
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	call	quick_read
	mv	t0, a0
	slliw	s3, s2, 2
	la	t1, u
	add	t1, t1, s3
	sw	t0, 0(t1)
	call	quick_read
	mv	t0, a0
	la	t1, v
	add	t1, t1, s3
	sw	t0, 0(t1)
	call	quick_read
	mv	t0, a0
	la	t1, c
	add	t1, t1, s3
	sw	t0, 0(t1)
	addiw	t0, s2, 1
	mv	s2, t0	# phi-copy
	blt	s2, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	call	prim
	mv	t0, a0
	mv	a0, t0
	ld	s3, 24(sp)
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
