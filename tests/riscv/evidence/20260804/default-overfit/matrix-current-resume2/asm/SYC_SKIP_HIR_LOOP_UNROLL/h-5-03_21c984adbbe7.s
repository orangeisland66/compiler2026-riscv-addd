
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
	mulw	s5, t2, t3
	add	s6, s2, s5
	li	t3, 0
	mv	s7, t1	# phi-copy
	mv	s8, s6	# phi-copy
.kernel_ludcmp.bb4:
	bge	t3, t2, .kernel_ludcmp.bb6
.kernel_ludcmp.bb5:
	lw	a2, 0(s8)
	slliw	a3, t3, 2
	addiw	a3, a3, -4
	li	a4, 0
	add	a3, s2, a3
	mv	s9, s6	# phi-copy
.kernel_ludcmp.bb7:
	bge	a4, t3, .kernel_ludcmp.bb9
.kernel_ludcmp.bb8:
	lw	a5, 0(s9)
	addiw	a4, a4, 1
	lw	a6, 0(a3)
	mulw	a5, a5, a6
	subw	a2, a2, a5
	li	a5, 5600
	add	a3, a3, a5
	addi	s9, s9, 4
	blt	a4, t3, .kernel_ludcmp.bb8	# rotated-loop-branch
.kernel_ludcmp.bb9:
	lw	a3, 0(s7)
	addiw	t3, t3, 1
	divw	a2, a2, a3
	sw	a2, 0(s8)
	li	a3, 5604
	addi	s8, s8, 4
	add	s7, s7, a3
	j	.kernel_ludcmp.bb4
.kernel_ludcmp.bb14:
	lw	a5, 0(s7)
	addiw	a4, a4, 1
	lw	a6, 0(a3)
	mulw	a5, a5, a6
	subw	a2, a2, a5
	li	a5, 5600
	add	a3, a3, a5
	addi	s7, s7, 4
	blt	a4, t2, .kernel_ludcmp.bb14	# rotated-loop-branch
.kernel_ludcmp.bb15:
	sw	a2, 0(t3)
	addiw	s5, s5, 1
	addi	t3, t3, 4
.kernel_ludcmp.bb10:
	bge	s5, s1, .kernel_ludcmp.bb12
.kernel_ludcmp.bb11:
	lw	a2, 0(t3)
	slliw	a3, s5, 2
	add	a3, s2, a3
	li	a4, 0
	mv	s7, s6	# phi-copy
.kernel_ludcmp.bb13:
	bge	a4, t2, .kernel_ludcmp.bb15
	j	.kernel_ludcmp.bb14
.kernel_ludcmp.bb6:
	slliw	t3, t2, 2
	addw	t3, s5, t3
	add	t3, s2, t3
	mv	s5, t2	# phi-copy
	j	.kernel_ludcmp.bb10
.kernel_ludcmp.bb12:
	addiw	t2, t2, 1
	blt	t2, s1, .kernel_ludcmp.bb2	# rotated-loop-branch
.kernel_ludcmp.bb3:
	mv	s5, s4
	li	t1, 0
	mv	s6, s5	# phi-copy
.kernel_ludcmp.bb16:
	bge	t1, s1, .kernel_ludcmp.bb18
.kernel_ludcmp.bb17:
	lw	t2, 0(t0)
	li	t3, 5600
	mulw	t3, t1, t3
	li	a2, 0
	add	t3, s2, t3
	mv	s7, s5	# phi-copy
.kernel_ludcmp.bb19:
	bge	a2, t1, .kernel_ludcmp.bb21
.kernel_ludcmp.bb20:
	lw	a3, 0(t3)
	addiw	a2, a2, 1
	lw	a4, 0(s7)
	addi	t3, t3, 4
	mulw	a3, a3, a4
	subw	t2, t2, a3
	addi	s7, s7, 4
	blt	a2, t1, .kernel_ludcmp.bb20	# rotated-loop-branch
.kernel_ludcmp.bb21:
	sw	t2, 0(s6)
	addiw	t1, t1, 1
	addi	t0, t0, 4
	addi	s6, s6, 4
	blt	t1, s1, .kernel_ludcmp.bb17	# rotated-loop-branch
.kernel_ludcmp.bb18:
	addiw	t0, s1, -1
	li	t1, 5604
	mulw	t1, t0, t1
	slliw	t2, t0, 2
	add	t2, s4, t2
	add	t1, s2, t1
.kernel_ludcmp.bb22:
	blt	t0, zero, .kernel_ludcmp.bb24
.kernel_ludcmp.bb23:
	lw	a2, 0(t2)
	addiw	s4, t0, 1
	li	a3, 5600
	slliw	s5, s4, 2
	mulw	a3, t0, a3
	slliw	t3, t0, 2
	add	a4, s3, s5
	addw	a3, a3, s5
	add	a3, s2, a3
.kernel_ludcmp.bb25:
	bge	s4, s1, .kernel_ludcmp.bb27
.kernel_ludcmp.bb26:
	lw	a5, 0(a3)
	lw	a6, 0(a4)
	addi	a3, a3, 4
	mulw	a5, a5, a6
	addi	a4, a4, 4
	subw	a2, a2, a5
	addiw	s4, s4, 1
	blt	s4, s1, .kernel_ludcmp.bb26	# rotated-loop-branch
.kernel_ludcmp.bb27:
	lw	a3, 0(t1)
	add	t3, s3, t3
	addiw	t0, t0, -1
	addi	t2, t2, -4
	divw	a2, a2, a3
	sw	a2, 0(t3)
	li	t3, -5604
	add	t1, t1, t3
	bge	t0, zero, .kernel_ludcmp.bb23	# rotated-loop-branch
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
