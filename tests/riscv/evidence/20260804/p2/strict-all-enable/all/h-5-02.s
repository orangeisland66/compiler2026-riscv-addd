
  .data
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .word 20

  .bss
  .globl A
  .align 2
  .type  A, @object
  .size  A, 7840000
A:
  .zero 7840000

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 5600
b:
  .zero 5600

  .bss
  .globl x
  .align 2
  .type  x, @object
  .size  x, 5600
x:
  .zero 5600

  .bss
  .globl y
  .align 2
  .type  y, @object
  .size  y, 5600
y:
  .zero 5600

  .text

  .globl kernel_ludcmp
  .type  kernel_ludcmp, @function
kernel_ludcmp:
.kernel_ludcmp.bb0:
	addi	sp, sp, -80
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	sd	s7, 48(sp)
	sd	s8, 56(sp)
	sd	s9, 64(sp)
	mv	s1, a0
	mv	s2, a1
	mv	t0, a2
	mv	s3, a3
	mv	s4, a4
	mv	t1, s2
	li	t2, 0
.kernel_ludcmp.bb1:
	bge	t2, s1, .kernel_ludcmp.bb3
.kernel_ludcmp.bb2:
	li	t3, 5600
	li	a2, 0
	mulw	s5, t2, t3
	add	t3, s2, s5
	mv	s6, t1	# phi-copy
.kernel_ludcmp.bb4:
	bge	a2, t2, .kernel_ludcmp.bb6
.kernel_ludcmp.bb5:
	lw	a3, 0(t3)
	addiw	a4, a2, -1
	slliw	s7, a4, 2
	li	a4, 0
.kernel_ludcmp.bb7:
	bge	a4, a2, .kernel_ludcmp.bb9
.kernel_ludcmp.bb28:
	li	a6, 5600
	addiw	a5, a4, 3
	mulw	s8, a4, a6
	blt	a5, a2, .kernel_ludcmp.bb29
.kernel_ludcmp.bb8:
	slliw	a5, a4, 2
	addw	a6, s8, s7
	addw	a5, s5, a5
	add	a6, s2, a6
	lw	a6, 0(a6)
	add	a5, s2, a5
	lw	a5, 0(a5)
	addiw	a4, a4, 1
	mulw	a5, a5, a6
	subw	a3, a3, a5
	j	.kernel_ludcmp.bb7
.kernel_ludcmp.bb29:
	slliw	a5, a4, 2
	addw	a6, s8, s7
	addw	a5, s5, a5
	add	s8, s2, a6
	lw	a6, 0(s8)
	add	s9, s2, a5
	lw	a5, 0(s9)
	addiw	a4, a4, 4
	li	t4, 5600
	li	a7, 5600
	add	t4, s8, t4
	add	a7, s8, a7
	lw	t4, 0(t4)
	mulw	a5, a5, a6
	lw	a6, 4(s9)
	subw	a3, a3, a5
	mulw	a6, a6, t4
	lw	a5, 12(s9)
	li	t4, 5600
	subw	a3, a3, a6
	lw	a6, 8(s9)
	add	a7, a7, t4
	li	t4, 11200
	add	t4, s8, t4
	lw	t4, 0(t4)
	mulw	a6, a6, t4
	subw	a3, a3, a6
	li	a6, 16800
	add	a6, s8, a6
	lw	a6, 0(a6)
	mulw	a5, a5, a6
	subw	a3, a3, a5
	j	.kernel_ludcmp.bb7
.kernel_ludcmp.bb9:
	lw	a4, 0(s6)
	addiw	a2, a2, 1
	divw	a3, a3, a4
	sw	a3, 0(t3)
	li	a3, 5604
	addi	t3, t3, 4
	add	s6, s6, a3
	j	.kernel_ludcmp.bb4
.kernel_ludcmp.bb13:
	bge	a3, t2, .kernel_ludcmp.bb15
.kernel_ludcmp.bb30:
	li	a5, 5600
	addiw	a4, a3, 3
	mulw	s8, a3, a5
	blt	a4, t2, .kernel_ludcmp.bb31
.kernel_ludcmp.bb14:
	slliw	a4, a3, 2
	addw	a5, s8, s7
	addw	a4, s5, a4
	add	a5, s2, a5
	lw	a5, 0(a5)
	add	a4, s2, a4
	lw	a4, 0(a4)
	addiw	a3, a3, 1
	mulw	a4, a4, a5
	subw	a2, a2, a4
	j	.kernel_ludcmp.bb13
.kernel_ludcmp.bb31:
	slliw	a4, a3, 2
	addw	a5, s8, s7
	addw	a4, s5, a4
	add	s8, s2, a5
	lw	a5, 0(s8)
	add	s9, s2, a4
	lw	a4, 0(s9)
	addiw	a3, a3, 4
	li	a7, 5600
	add	a7, s8, a7
	lw	a7, 0(a7)
	mulw	a4, a4, a5
	lw	a5, 4(s9)
	subw	a2, a2, a4
	mulw	a5, a5, a7
	lw	a4, 12(s9)
	subw	a2, a2, a5
	lw	a5, 8(s9)
	li	a7, 11200
	add	a7, s8, a7
	lw	a7, 0(a7)
	mulw	a5, a5, a7
	subw	a2, a2, a5
	li	a5, 16800
	add	a5, s8, a5
	lw	a5, 0(a5)
	mulw	a4, a4, a5
	subw	a2, a2, a4
	j	.kernel_ludcmp.bb13
.kernel_ludcmp.bb15:
	sw	a2, 0(t3)
	addiw	s6, s6, 1
	addi	t3, t3, 4
.kernel_ludcmp.bb10:
	bge	s6, s1, .kernel_ludcmp.bb12
.kernel_ludcmp.bb11:
	lw	a2, 0(t3)
	slliw	s7, s6, 2
	li	a3, 0
	j	.kernel_ludcmp.bb13
.kernel_ludcmp.bb19:
	bge	a2, t2, .kernel_ludcmp.bb21
.kernel_ludcmp.bb32:
	addiw	a3, a2, 3
	blt	a3, t2, .kernel_ludcmp.bb33
.kernel_ludcmp.bb20:
	slliw	s6, a2, 2
	addw	a3, s5, s6
	add	a4, s4, s6
	lw	a4, 0(a4)
	add	a3, s2, a3
	lw	a3, 0(a3)
	addiw	a2, a2, 1
	mulw	a3, a3, a4
	subw	t3, t3, a3
	j	.kernel_ludcmp.bb19
.kernel_ludcmp.bb33:
	slliw	s6, a2, 2
	addw	a3, s5, s6
	addiw	a2, a2, 4
	add	s7, s2, a3
	add	s6, s4, s6
	lw	a3, 0(s7)
	lw	a4, 0(s6)
	lw	a6, 4(s6)
	mulw	a3, a3, a4
	lw	a4, 4(s7)
	subw	t3, t3, a3
	mulw	a4, a4, a6
	lw	a3, 12(s7)
	lw	a6, 8(s6)
	subw	t3, t3, a4
	lw	a4, 8(s7)
	mulw	a4, a4, a6
	subw	t3, t3, a4
	lw	a4, 12(s6)
	mulw	a3, a3, a4
	subw	t3, t3, a3
	j	.kernel_ludcmp.bb19
.kernel_ludcmp.bb21:
	sw	t3, 0(t1)
	addiw	t2, t2, 1
	addi	t0, t0, 4
	addi	t1, t1, 4
	bge	t2, s1, .kernel_ludcmp.bb18	# rotated-loop-branch
.kernel_ludcmp.bb17:
	lw	t3, 0(t0)
	li	a2, 5600
	mulw	s5, t2, a2
	li	a2, 0
	j	.kernel_ludcmp.bb19
.kernel_ludcmp.bb25:
	bge	a3, s1, .kernel_ludcmp.bb27
.kernel_ludcmp.bb34:
	addiw	a4, a3, 3
	blt	a4, s1, .kernel_ludcmp.bb35
.kernel_ludcmp.bb26:
	slliw	s5, a3, 2
	addw	a4, s4, s5
	add	a5, s3, s5
	lw	a5, 0(a5)
	add	a4, s2, a4
	lw	a4, 0(a4)
	addiw	a3, a3, 1
	mulw	a4, a4, a5
	subw	a2, a2, a4
	j	.kernel_ludcmp.bb25
.kernel_ludcmp.bb35:
	slliw	s5, a3, 2
	addw	a4, s4, s5
	addiw	a3, a3, 4
	add	s6, s2, a4
	add	s5, s3, s5
	lw	a4, 0(s6)
	lw	a5, 0(s5)
	lw	a7, 4(s5)
	mulw	a4, a4, a5
	lw	a5, 4(s6)
	subw	a2, a2, a4
	mulw	a5, a5, a7
	lw	a4, 12(s6)
	lw	a7, 8(s5)
	subw	a2, a2, a5
	lw	a5, 8(s6)
	mulw	a5, a5, a7
	subw	a2, a2, a5
	lw	a5, 12(s5)
	mulw	a4, a4, a5
	subw	a2, a2, a4
	j	.kernel_ludcmp.bb25
.kernel_ludcmp.bb27:
	lw	a3, 0(t1)
	add	t3, s3, t3
	addiw	t0, t0, -1
	addi	t2, t2, -4
	divw	a2, a2, a3
	sw	a2, 0(t3)
	li	t3, -5604
	add	t1, t1, t3
	blt	t0, zero, .kernel_ludcmp.bb24	# rotated-loop-branch
.kernel_ludcmp.bb23:
	lw	a2, 0(t2)
	slliw	t3, t0, 2
	li	a4, 5600
	addiw	a3, t0, 1
	mulw	s4, t0, a4
	j	.kernel_ludcmp.bb25
.kernel_ludcmp.bb6:
	slliw	t3, t2, 2
	addw	t3, s5, t3
	add	t3, s2, t3
	mv	s6, t2	# phi-copy
	j	.kernel_ludcmp.bb10
.kernel_ludcmp.bb12:
	addiw	t2, t2, 1
	blt	t2, s1, .kernel_ludcmp.bb2	# rotated-loop-branch
.kernel_ludcmp.bb3:
	mv	t1, s4
	li	t2, 0
.kernel_ludcmp.bb16:
	blt	t2, s1, .kernel_ludcmp.bb17
.kernel_ludcmp.bb18:
	addiw	t0, s1, -1
	li	t1, 5604
	mulw	t1, t0, t1
	slliw	t2, t0, 2
	add	t2, s4, t2
	add	t1, s2, t1
.kernel_ludcmp.bb22:
	bge	t0, zero, .kernel_ludcmp.bb23
.kernel_ludcmp.bb24:
	ld	s9, 64(sp)
	ld	s8, 56(sp)
	ld	s7, 48(sp)
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 80
	ret
	.size kernel_ludcmp, .-kernel_ludcmp

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	la	t5, A
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	la	t5, b
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	la	t5, x
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	la	t5, y
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	li	a0, 68
	call	_sysy_starttime
	la	t0, n
	lw	s1, 0(t0)
	mv	t5, s1
	sd	t5, 0(sp)
	la	t5, A
	sd	t5, 8(sp)
	la	t5, b
	sd	t5, 16(sp)
	la	t5, x
	sd	t5, 24(sp)
	la	t5, y
	sd	t5, 32(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	call	kernel_ludcmp
	li	t5, 70
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	la	t5, x
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	li	a0, 0
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main
