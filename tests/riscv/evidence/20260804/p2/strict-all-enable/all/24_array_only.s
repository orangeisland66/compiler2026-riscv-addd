
  .bss
  .globl i
  .align 2
  .type  i, @object
  .size  i, 4
i:
  .zero 4

  .bss
  .globl k
  .align 2
  .type  k, @object
  .size  k, 4
k:
  .zero 4

  .text

  .globl inc
  .type  inc, @function
inc:
.inc.bb0:
	mv	t0, a0
	la	t1, k
	lw	t1, 0(t1)
.inc.bb1:
	beq	t1, zero, .inc.bb2
.inc.bb3:
	addiw	t1, t1, -1
	lw	t2, 0(t0)
	slliw	t2, t2, 1
	sw	t2, 0(t0)
	bne	t1, zero, .inc.bb3	# rotated-loop-branch
.inc.bb2:
	lw	t1, 0(t0)
	addiw	t1, t1, 1
	sw	t1, 0(t0)
	ret
	.size inc, .-inc

  .globl add
  .type  add, @function
add:
.add.bb0:
	mv	t0, a0
	mv	t1, a1
	la	t2, k
	lw	t2, 0(t2)
.add.bb1:
	beq	t2, zero, .add.bb2
.add.bb3:
	addiw	t2, t2, -1
	lw	t3, 0(t0)
	slliw	t3, t3, 1
	sw	t3, 0(t0)
	bne	t2, zero, .add.bb3	# rotated-loop-branch
.add.bb2:
	lw	t2, 0(t0)
	lw	t1, 0(t1)
	addw	t1, t2, t1
	sw	t1, 0(t0)
	ret
	.size add, .-add

  .globl sub
  .type  sub, @function
sub:
.sub.bb0:
	mv	t0, a0
	mv	t1, a1
	la	t2, k
	lw	t2, 0(t2)
.sub.bb1:
	beq	t2, zero, .sub.bb2
.sub.bb3:
	addiw	t2, t2, -1
	lw	t3, 0(t0)
	slliw	t3, t3, 1
	sw	t3, 0(t0)
	bne	t2, zero, .sub.bb3	# rotated-loop-branch
.sub.bb2:
	lw	t2, 0(t0)
	lw	t1, 0(t1)
	subw	t1, t2, t1
	sw	t1, 0(t0)
	ret
	.size sub, .-sub

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	addi	t5, sp, 16
	sd	zero, 0(t5)	# memset-zero
	li	t0, -1
	addi	t1, sp, 16
	sw	t0, 0(t1)
	call	getint
	mv	t0, a0
	la	t1, k
	sw	t0, 0(t1)
	call	getint
	mv	t0, a0
	addi	t1, sp, 28
	sw	t0, 0(t1)
	addi	t5, sp, 16
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
.main.bb1:
	addi	t0, sp, 28
	lw	t0, 0(t0)
	beq	t0, zero, .main.bb3
.main.bb2:
	addi	t0, sp, 16
	addi	t1, sp, 24
	lw	t0, 0(t0)
	li	s2, 5
	sw	t0, 0(t1)
	la	t0, i
	lw	s1, 0(t0)
.main.bb4:
	addi	t0, sp, 24
	lw	s3, 0(t0)
	bge	s3, s2, .main.bb6
.main.bb5:
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	mv	t5, s3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addi	t0, sp, 28
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addi	t0, sp, 16
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addi	t5, sp, 16
	sd	t5, 0(sp)
	addi	t5, sp, 28
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	add
	addi	t5, sp, 24
	sd	t5, 0(sp)
	addi	t5, sp, 28
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	add
	addi	t5, sp, 16
	sd	t5, 0(sp)
	addi	t5, sp, 28
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	sub
	j	.main.bb4
.main.bb6:
	la	t5, i
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	inc
	la	t5, i
	sd	t5, 0(sp)
	addi	t5, sp, 16
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	add
	la	t0, i
	lw	t0, 0(t0)
	addi	t1, sp, 16
	lw	t1, 4(t1)
	beq	t0, t1, .main.bb3
.main.bb8:
	j	.main.bb1
.main.bb3:
	li	a0, 10
	call	putch
	li	a0, 0
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 64
	ret
.main.bb7:
	j	.main.bb3
	.size main, .-main
