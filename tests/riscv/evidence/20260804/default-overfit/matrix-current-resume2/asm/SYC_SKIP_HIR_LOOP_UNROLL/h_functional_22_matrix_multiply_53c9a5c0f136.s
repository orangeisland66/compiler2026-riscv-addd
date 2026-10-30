
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
	addi	sp, sp, -64
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	sd	s7, 48(sp)
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
	j	.matrix_multiply.bb16
.matrix_multiply.bb14:
.matrix_multiply.bb16:
	addiw	s7, t3, 2
	bge	t3, s6, .matrix_multiply.bb10
.matrix_multiply.bb17:
	bge	s7, s6, .matrix_multiply.bb18
.matrix_multiply.bb19:
.matrix_multiply.bb20:
	li	a4, 400	# loop.addr.init
	slliw	a2, t3, 2
	mulw	a4, t3, a4	# loop.addr.init
	addw	a2, s4, a2
	la	a3, a
	add	a2, a3, a2
	la	a3, b	# loop.addr.init
	addw	a4, a4, s5	# loop.addr.init
	add	a3, a3, a4	# loop.addr.init
.matrix_multiply.bb7:
	bge	t3, s7, .matrix_multiply.bb16
.matrix_multiply.bb8:
	lw	a5, 0(a2)
	lw	a4, 0(t1)
	addi	a2, a2, 4
	addiw	t3, t3, 1
	lw	a6, 0(a3)
	mulw	a5, a5, a6
	addi	a3, a3, 400	# loop.addr.bump
	addw	a4, a4, a5
	sw	a4, 0(t1)
	blt	t3, s7, .matrix_multiply.bb8	# rotated-loop-branch
	j	.matrix_multiply.bb16	# rotated-loop-exit
.matrix_multiply.bb21:
	j	.matrix_multiply.bb16
.matrix_multiply.bb18:
	mv	s7, s6	# phi-copy
	j	.matrix_multiply.bb20
.matrix_multiply.bb15:
	j	.matrix_multiply.bb10
.matrix_multiply.bb12:
	mv	s6, s3	# phi-copy
	j	.matrix_multiply.bb16
.matrix_multiply.bb9:
	addiw	t2, t2, 1
	addi	t1, t1, 4
	j	.matrix_multiply.bb4
.matrix_multiply.bb6:
	addiw	t0, t0, 1
	blt	t0, s1, .matrix_multiply.bb2	# rotated-loop-branch
.matrix_multiply.bb3:
	ld	s7, 48(sp)
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 64
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
