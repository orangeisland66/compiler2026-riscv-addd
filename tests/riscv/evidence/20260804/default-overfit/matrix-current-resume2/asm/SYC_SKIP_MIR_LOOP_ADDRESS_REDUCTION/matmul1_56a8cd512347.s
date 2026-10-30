
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 160000
a:
  .zero 160000

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 160000
b:
  .zero 160000

  .bss
  .globl c
  .align 2
  .type  c, @object
  .size  c, 160000
c:
  .zero 160000

  .section .rodata
  .globl MAX
  .align 2
  .type  MAX, @object
  .size  MAX, 4
MAX:
  .word 2147483647

  .text

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
	sd	s6, 56(sp)
	li	s2, 0
	li	s1, 200
.main.bb2:
	bge	s2, s1, .main.bb4
.main.bb3:
	li	t0, 800
	mulw	t0, s2, t0
	la	t1, a
	add	t5, t1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s3, a0
	li	t0, 200
	bne	s3, t0, .main.bb5
.main.bb6:
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb3	# rotated-loop-branch
.main.bb4:
	li	t5, 23
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 0
.main.bb7:
	li	t1, 200
	bge	t0, t1, .main.bb9
.main.bb8:
	li	t1, 800
	slliw	s1, t0, 2
	mulw	s2, t0, t1
	li	t2, 200
	la	s3, a	# loop.addr.hoisted
	la	s4, b	# loop.addr.hoisted
	li	t1, 0
.main.bb30:
	addiw	s5, t1, 32
	bge	t1, t2, .main.bb12
.main.bb31:
	li	t3, 200
	bge	s5, t3, .main.bb32
.main.bb33:
	mv	t3, s5	# phi-copy
	j	.main.bb10
.main.bb34:
.main.bb10:
	bge	t1, t3, .main.bb30
.main.bb42:
	li	a3, 800
	addiw	a2, t1, 3
	mulw	s5, t1, a3
	blt	a2, t3, .main.bb43
.main.bb11:
	addw	a2, s5, s1
	slliw	a3, t1, 2
	add	a2, s3, a2
	addw	a3, s2, a3
	lw	a2, 0(a2)
	add	a3, s4, a3
	addiw	t1, t1, 1
	sw	a2, 0(a3)
	j	.main.bb10
.main.bb43:
	addw	a2, s5, s1
	slliw	a3, t1, 2
	add	s5, s3, a2
	addw	a3, s2, a3
	lw	a2, 0(s5)
	add	s6, s4, a3
	addiw	t1, t1, 4
	sw	a2, 0(s6)
	lw	a3, 800(s5)
	sw	a3, 4(s6)
	addi	a2, s5, 353
	lw	a3, 1600(s5)
	addi	a2, a2, 2047
	sw	a3, 8(s6)
	lw	a2, 0(a2)
	sw	a2, 12(s6)
	j	.main.bb10
.main.bb35:
	j	.main.bb30
.main.bb19:
	bge	a5, a6, .main.bb36
.main.bb20:
	li	a0, 800
	slliw	s3, a5, 2
	mulw	a0, a5, a0
	addw	s4, t2, s3
	add	t4, s2, s4
	lw	t4, 0(t4)
	addw	s5, a0, a2
	addw	a0, s3, a3
	add	a1, s2, s5
	lw	a1, 0(a1)
	add	a0, s2, a0
	lw	a0, 0(a0)
	and	t4, t4, a0	# parity.masked-accumulate
	andi	t4, t4, 1
	add	a0, a7, s4
	lw	a0, 0(a0)
	mulw	a0, a0, a1
	addiw	t4, t4, -1
	and	a0, a0, t4
	addw	a4, a4, a0
.main.bb22:
	addiw	a5, a5, 1
	j	.main.bb19
.main.bb41:
.main.bb36:
	addiw	s2, a5, 16
	li	a6, 200
	bge	a5, a6, .main.bb21
.main.bb37:
	li	a6, 200
	bge	s2, a6, .main.bb38
.main.bb39:
	mv	a6, s2	# phi-copy
.main.bb40:
	la	s2, a	# loop.addr.hoisted
	la	a7, b	# loop.addr.hoisted
	j	.main.bb19
.main.bb38:
	li	a6, 200
	j	.main.bb40
.main.bb21:
	addiw	a2, s1, 1
	lw	s1, 0(t0)
	slt	a3, a4, s1
	xor	a5, s1, a4
	subw	a3, zero, a3
	and	a3, a5, a3
	xor	a3, s1, a3
	sw	a3, 0(t0)
	lw	s1, 0(t3)
	slt	a3, a4, s1
	subw	a3, zero, a3
	xor	a4, s1, a4
	and	a3, a4, a3
	xor	a3, s1, a3
	sw	a3, 0(t3)
	addi	t3, t3, 4
	mv	s1, a2	# phi-copy
.main.bb16:
	li	a2, 200
	bge	s1, a2, .main.bb18
.main.bb17:
	li	a3, 800
	slliw	a2, s1, 2
	mulw	a3, s1, a3
	li	a4, 0
	li	a5, 0
	j	.main.bb36
.main.bb32:
	li	t3, 200
	j	.main.bb10
.main.bb12:
	addiw	t0, t0, 1
	j	.main.bb7
.main.bb18:
	addiw	t1, t1, 1
	addi	t0, t0, 4
.main.bb13:
	li	t2, 200
	bge	t1, t2, .main.bb15
.main.bb14:
	li	t2, 800
	slliw	t3, t1, 2
	mulw	t2, t1, t2
	la	a2, c
	add	t3, a2, t3
	mv	s1, t1	# phi-copy
	j	.main.bb16
.main.bb23:
	li	t2, 200
	bge	t0, t2, .main.bb25
.main.bb44:
	addiw	t2, t0, 3
	li	t3, 200
	blt	t2, t3, .main.bb45
.main.bb24:
	slliw	t2, t0, 2
	la	t3, c
	add	t2, t3, t2
	addiw	t0, t0, 1
	lw	t2, 0(t2)
	addw	t1, t1, t2
	j	.main.bb23
.main.bb45:
	slliw	t2, t0, 2
	la	t3, c
	add	s1, t3, t2
	addiw	t0, t0, 4
	lw	t2, 0(s1)
	lw	t3, 4(s1)
	addw	t1, t1, t2
	addw	t1, t1, t3
	lw	t3, 8(s1)
	lw	t2, 12(s1)
	addw	t1, t1, t3
	addw	t1, t1, t2
	j	.main.bb23
.main.bb27:
	li	t1, 200
	bge	t0, t1, .main.bb29
.main.bb46:
	addiw	t1, t0, 3
	li	t2, 200
	blt	t1, t2, .main.bb47
.main.bb28:
	slliw	t1, t0, 2
	la	t2, c
	add	t1, t2, t1
	addiw	t0, t0, 1
	li	t2, 2147483647
	sw	t2, 0(t1)
	j	.main.bb27
.main.bb47:
	slliw	t1, t0, 2
	la	t2, c
	add	s1, t2, t1
	addiw	t0, t0, 4
	li	t1, 2147483647
	li	t2, 2147483647
	sw	t1, 0(s1)
	sw	t2, 4(s1)
	sw	t2, 8(s1)
	sw	t1, 12(s1)
	j	.main.bb27
.main.bb25:
	subw	s1, zero, t1
.main.bb26:
	li	a0, 93
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t0, 0
.main.bb1:
	mv	a0, t0
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
.main.bb29:
	la	t0, c
	li	t1, 0
	j	.main.bb13
.main.bb5:
	mv	t0, s3	# phi-copy
	j	.main.bb1
.main.bb9:
	li	t0, 0
	j	.main.bb27
.main.bb15:
	li	t0, 0
	li	t1, 0
	j	.main.bb23
	.size main, .-main
