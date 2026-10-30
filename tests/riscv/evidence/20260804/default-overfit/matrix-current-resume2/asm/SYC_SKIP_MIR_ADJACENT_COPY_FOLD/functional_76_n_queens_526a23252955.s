
  .bss
  .globl ans
  .align 2
  .type  ans, @object
  .size  ans, 200
ans:
  .zero 200

  .bss
  .globl sum
  .align 2
  .type  sum, @object
  .size  sum, 4
sum:
  .zero 4

  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .bss
  .globl row
  .align 2
  .type  row, @object
  .size  row, 200
row:
  .zero 200

  .bss
  .globl line1
  .align 2
  .type  line1, @object
  .size  line1, 200
line1:
  .zero 200

  .bss
  .globl line2
  .align 2
  .type  line2, @object
  .size  line2, 400
line2:
  .zero 400

  .text

  .globl printans
  .type  printans, @function
printans:
.printans.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	la	t0, sum
	lw	t1, 0(t0)
	addiw	t1, t1, 1
	sw	t1, 0(t0)
	la	t0, n
	lw	s1, 0(t0)
	li	t0, 1
	mv	s2, t0	# phi-copy
.printans.bb2:
	blt	s1, s2, .printans.bb1
.printans.bb3:
	slliw	t0, s2, 2
	la	t1, ans
	add	t0, t1, t0
	lw	a0, 0(t0)
	call	putint
	beq	s2, s1, .printans.bb5
.printans.bb6:
	li	a0, 32
	call	putch
	addiw	t0, s2, 1
	mv	s2, t0	# phi-copy
	bge	s1, s2, .printans.bb3	# rotated-loop-branch
	j	.printans.bb1	# rotated-loop-exit
.printans.bb4:
.printans.bb1:
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.printans.bb5:
	li	a0, 10
	call	putch
	j	.printans.bb1
	.size printans, .-printans

  .globl f
  .type  f, @function
f:
.f.bb0:
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
	mv	s1, a0
	slliw	t0, s1, 2
	la	t1, ans
	add	s2, t1, t0
	addiw	s3, s1, 1
	li	t0, 1
	mv	s4, t0	# phi-copy
.f.bb1:
	la	t0, n
	lw	s5, 0(t0)
	blt	s5, s4, .f.bb3
.f.bb2:
	slliw	t0, s4, 2
	la	t1, row
	add	s6, t1, t0
	la	t2, line1
	lw	t0, 0(s6)
	li	t1, 1
	subw	t0, t0, t1
	addw	t1, s1, s4
	sltu	t0, zero, t0
	slliw	t1, t1, 2
	add	s7, t2, t1
	lw	t1, 0(s7)
	li	t2, 0
	subw	t1, t1, t2
	sltiu	t1, t1, 1
	la	t2, line2
	and	t0, t0, t1
	addw	t1, s5, s1
	subw	t1, t1, s4
	slliw	t1, t1, 2
	add	t1, t2, t1
	lw	t1, 0(t1)
	li	t2, 0
	subw	t1, t1, t2
	sltiu	t1, t1, 1
	and	t0, t0, t1
	bne	t0, zero, .f.bb4
.f.bb5:
	addiw	t0, s4, 1
	mv	s4, t0	# phi-copy
	j	.f.bb1
.f.bb4:
	sw	s4, 0(s2)
	beq	s1, s5, .f.bb6
.f.bb7:
	li	t0, 1
	sw	t0, 0(s6)
	sw	t0, 0(s7)
	la	s5, n
	lw	t0, 0(s5)
	addw	t0, t0, s1
	subw	t0, t0, s4
	slliw	t0, t0, 2
	la	s8, line2
	add	t0, s8, t0
	li	t1, 1
	sw	t1, 0(t0)
	mv	a0, s3
	call	f
	li	t0, 0
	sw	t0, 0(s6)
	sw	t0, 0(s7)
	lw	t0, 0(s5)
	addw	t0, t0, s1
	subw	t0, t0, s4
	slliw	t0, t0, 2
	add	t0, s8, t0
	li	t1, 0
	sw	t1, 0(t0)
	j	.f.bb5
.f.bb6:
	call	printans
	j	.f.bb7
.f.bb3:
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
	.size f, .-f

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	call	getint
	mv	t0, a0
	mv	s1, t0	# phi-copy
.main.bb1:
	bge	zero, s1, .main.bb3
.main.bb2:
	call	getint
	mv	t0, a0
	la	t1, n
	sw	t0, 0(t1)
	li	t5, 1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	f
	addiw	t0, s1, -1
	mv	s1, t0	# phi-copy
	blt	zero, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	la	t0, sum
	lw	a0, 0(t0)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
