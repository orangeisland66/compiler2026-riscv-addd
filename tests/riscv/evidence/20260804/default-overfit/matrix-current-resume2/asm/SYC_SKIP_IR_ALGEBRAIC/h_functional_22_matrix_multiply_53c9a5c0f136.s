
  .section .rodata
  .globl MAX_SIZE
  .align 2
  .type  MAX_SIZE, @object
  .size  MAX_SIZE, 4
MAX_SIZE:
  .word 100

  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 40000
a:
  .zero 40000

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 40000
b:
  .zero 40000

  .bss
  .globl res
  .align 2
  .type  res, @object
  .size  res, 40000
res:
  .zero 40000

  .bss
  .globl n1
  .align 2
  .type  n1, @object
  .size  n1, 4
n1:
  .zero 4

  .bss
  .globl m1
  .align 2
  .type  m1, @object
  .size  m1, 4
m1:
  .zero 4

  .bss
  .globl n2
  .align 2
  .type  n2, @object
  .size  n2, 4
n2:
  .zero 4

  .bss
  .globl m2
  .align 2
  .type  m2, @object
  .size  m2, 4
m2:
  .zero 4

  .text

  .globl matrix_multiply
  .type  matrix_multiply, @function
matrix_multiply:
.matrix_multiply.bb0:
	addi	sp, sp, -96
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	sd	s7, 48(sp)
	sd	s8, 56(sp)
	sd	s9, 64(sp)
	sd	s10, 72(sp)
	sd	s11, 80(sp)
	la	t0, m1
	lw	s1, 0(t0)
	la	t0, n2
	lw	s2, 0(t0)
	la	t0, n1
	lw	s3, 0(t0)
	li	t0, 0
.matrix_multiply.bb1:
	bge	t0, s1, .matrix_multiply.bb3
.matrix_multiply.bb2:
	li	t1, 400
	li	t2, 0
	mulw	s4, t0, t1
	la	t1, res
	add	t1, t1, s4
.matrix_multiply.bb4:
	bge	t2, s2, .matrix_multiply.bb6
.matrix_multiply.bb5:
	slliw	s5, t2, 2
	li	t3, 0
.matrix_multiply.bb10:
	addiw	s6, t3, 16
	bge	t3, s3, .matrix_multiply.bb9
.matrix_multiply.bb11:
	bge	s6, s3, .matrix_multiply.bb12
.matrix_multiply.bb13:
.matrix_multiply.bb14:
	la	s7, a	# loop.addr.hoisted
	la	s8, b	# loop.addr.hoisted
.matrix_multiply.bb16:
	addiw	s9, t3, 2
	bge	t3, s6, .matrix_multiply.bb10
.matrix_multiply.bb17:
	bge	s9, s6, .matrix_multiply.bb18
.matrix_multiply.bb19:
	j	.matrix_multiply.bb7
.matrix_multiply.bb20:
.matrix_multiply.bb7:
	bge	t3, s9, .matrix_multiply.bb16
.matrix_multiply.bb22:
	li	a3, 400
	addiw	a2, t3, 3
	mulw	s10, t3, a3
	blt	a2, s9, .matrix_multiply.bb23
.matrix_multiply.bb8:
	lw	a2, 0(t1)
	slliw	a3, t3, 2
	addw	a3, s4, a3
	addw	a4, s10, s5
	add	a3, s7, a3
	add	a4, s8, a4
	lw	a3, 0(a3)
	addiw	t3, t3, 1
	lw	a4, 0(a4)
	mulw	a3, a3, a4
	addw	a2, a2, a3
	sw	a2, 0(t1)
	j	.matrix_multiply.bb7
.matrix_multiply.bb23:
	lw	a2, 0(t1)
	slliw	a3, t3, 2
	addw	a3, s4, a3
	addw	a4, s10, s5
	add	s11, s7, a3
	add	s10, s8, a4
	lw	a3, 0(s11)
	lw	a4, 0(s10)
	lw	a6, 400(s10)
	addiw	t3, t3, 1
	addiw	t3, t3, 1
	mulw	a3, a3, a4
	addiw	t3, t3, 1
	addiw	t3, t3, 1
	lw	a4, 4(s11)
	addw	a2, a2, a3
	mulw	a4, a4, a6
	lw	a3, 12(s11)
	lw	a6, 800(s10)
	addw	a2, a2, a4
	lw	a4, 8(s11)
	mulw	a4, a4, a6
	addw	a2, a2, a4
	lw	a4, 1200(s10)
	mulw	a3, a3, a4
	addw	a2, a2, a3
	sw	a2, 0(t1)
	j	.matrix_multiply.bb7
.matrix_multiply.bb21:
	j	.matrix_multiply.bb16
.matrix_multiply.bb18:
	mv	s9, s6	# phi-copy
	j	.matrix_multiply.bb7
.matrix_multiply.bb15:
	j	.matrix_multiply.bb10
.matrix_multiply.bb12:
	mv	s6, s3	# phi-copy
	j	.matrix_multiply.bb14
.matrix_multiply.bb9:
	addiw	t2, t2, 1
	addi	t1, t1, 4
	j	.matrix_multiply.bb4
.matrix_multiply.bb6:
	addiw	t0, t0, 1
	blt	t0, s1, .matrix_multiply.bb2	# rotated-loop-branch
.matrix_multiply.bb3:
	ld	s11, 80(sp)
	ld	s10, 72(sp)
	ld	s9, 64(sp)
	ld	s8, 56(sp)
	ld	s7, 48(sp)
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 96
	ret
	.size matrix_multiply, .-matrix_multiply

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	call	getint
	mv	s1, a0
	la	t0, m1
	sw	s1, 0(t0)
	call	getint
	mv	s2, a0
	la	t0, n1
	sw	s2, 0(t0)
	li	s3, 0
.main.bb1:
	bge	s3, s1, .main.bb3
.main.bb2:
	li	t0, 400
	mulw	s4, s3, t0
	li	s5, 0
.main.bb4:
	bge	s5, s2, .main.bb6
.main.bb5:
	call	getint
	mv	t0, a0
	slliw	t1, s5, 2
	addw	t1, s4, t1
	la	t2, a
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s5, s5, 1
	blt	s5, s2, .main.bb5	# rotated-loop-branch
.main.bb6:
	addiw	s3, s3, 1
	blt	s3, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	la	t0, n2
	sw	s2, 0(t0)
	li	s3, 0
.main.bb7:
	bge	s3, s1, .main.bb9
.main.bb8:
	li	t0, 400
	mulw	s4, s3, t0
	li	s5, 0
.main.bb10:
	bge	s5, s2, .main.bb12
.main.bb11:
	call	getint
	mv	t0, a0
	slliw	t1, s5, 2
	addw	t1, s4, t1
	la	t2, b
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s5, s5, 1
	blt	s5, s2, .main.bb11	# rotated-loop-branch
.main.bb12:
	addiw	s3, s3, 1
	blt	s3, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	call	matrix_multiply
	la	t0, n2
	lw	s1, 0(t0)
	la	t0, m1
	lw	s2, 0(t0)
	li	s3, 0
.main.bb13:
	bge	s3, s2, .main.bb15
.main.bb14:
	li	t0, 400
	mulw	s4, s3, t0
	li	s5, 0
.main.bb16:
	bge	s5, s1, .main.bb18
.main.bb17:
	slliw	t0, s5, 2
	addw	t0, s4, t0
	la	t1, res
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s5, s5, 1
	blt	s5, s1, .main.bb17	# rotated-loop-branch
.main.bb18:
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s3, s3, 1
	blt	s3, s2, .main.bb14	# rotated-loop-branch
.main.bb15:
	li	a0, 0
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main
