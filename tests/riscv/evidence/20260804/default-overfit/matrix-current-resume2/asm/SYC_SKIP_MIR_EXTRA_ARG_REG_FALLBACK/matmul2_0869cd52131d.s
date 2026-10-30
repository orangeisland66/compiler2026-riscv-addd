
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 360000
a:
  .zero 360000

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 360000
b:
  .zero 360000

  .bss
  .globl c
  .align 2
  .type  c, @object
  .size  c, 360000
c:
  .zero 360000

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
	addi	sp, sp, -112
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	sd	s2, 56(sp)
	sd	s3, 64(sp)
	sd	s4, 72(sp)
	sd	s5, 80(sp)
	sd	s6, 88(sp)
	sd	s7, 96(sp)
	li	s2, 0
	li	s1, 300
.main.bb2:
	bge	s2, s1, .main.bb4
.main.bb3:
	li	t0, 1200
	mulw	t0, s2, t0
	la	t1, a
	add	t5, t1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s3, a0
	li	t0, 300
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
	li	t1, 300
	bge	t0, t1, .main.bb9
.main.bb8:
	li	t1, 1200
	slliw	s1, t0, 2
	mulw	s2, t0, t1
	li	t2, 300
	la	s3, a	# loop.addr.hoisted
	la	s4, b	# loop.addr.hoisted
	li	t1, 0
	li	t4, 2400
.main.bb30:
	addiw	s5, t1, 32
	bge	t1, t2, .main.bb12
.main.bb31:
	li	t3, 300
	bge	s5, t3, .main.bb32
.main.bb33:
	mv	t3, s5	# phi-copy
	j	.main.bb10
.main.bb34:
.main.bb10:
	bge	t1, t3, .main.bb30
.main.bb42:
	li	a3, 1200
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
	lw	a3, 1200(s5)
	addi	a2, s5, 1200
	add	a2, a2, t4
	sw	a3, 4(s6)
	addi	a2, s5, 1553
	addi	a3, s5, 353
	addi	a2, a2, 2047
	addi	a3, a3, 2047
	lw	a3, 0(a3)
	sw	a3, 8(s6)
	lw	a2, 0(a2)
	sw	a2, 12(s6)
	j	.main.bb10
.main.bb35:
	j	.main.bb30
.main.bb19:
	ld	t5, 32(sp)
	bge	a3, t5, .main.bb36
.main.bb20:
	slliw	s6, a3, 2
	addw	s7, s1, s6
	add	t5, s4, s7
	sd	t5, 8(sp)
	lw	t3, 0(a6)
	lw	t0, 0(a7)
	andi	t3, t3, 1
	andi	t0, t0, 1
	and	t0, t3, t0
	sltiu	t0, t0, 1
	la	t5, b	# remat
	add	t3, t5, s7
	lw	t3, 0(t4)
	lw	s5, 0(a5)
	mulw	t3, t3, s5
	addw	t3, a2, t3
	li	a4, 0
	subw	t0, a4, t0
	xor	t3, a2, t3
	and	t0, t3, t0
	xor	t0, a2, t0
.main.bb22:
	addiw	a3, a3, 1
	addi	a2, a5, 1200
	addi	a6, a6, 4	# loop.addr.bump
	addi	a7, a7, 4	# loop.addr.bump
	addi	a5, t4, 4	# loop.addr.bump
	mv	t4, a5	# phi-copy
	mv	a5, a2	# phi-copy
	mv	a2, t0	# phi-copy
	j	.main.bb19
.main.bb41:
.main.bb36:
	addiw	s4, a3, 16
	li	a4, 300
	bge	a3, a4, .main.bb21
.main.bb37:
	li	a4, 300
	bge	s4, a4, .main.bb38
.main.bb39:
	sd	s4, 32(sp)
.main.bb40:
	li	a5, 1200
	mulw	a5, a3, a5
	ld	t5, 16(sp)
	addw	a5, t5, a5
	la	a6, a
	add	a5, a6, a5
	la	s4, a	# loop.addr.hoisted
	la	t5, b	# loop.addr.hoisted
	slliw	a6, a3, 2	# loop.addr.init
	addw	a6, s1, a6	# loop.addr.init
	add	a6, s4, a6	# loop.addr.init
	slliw	a7, a3, 2	# loop.addr.init
	addw	a7, a7, s3	# loop.addr.init
	add	a7, s4, a7	# loop.addr.init
	slliw	t4, a3, 2	# loop.addr.init
	addw	t4, s1, t4	# loop.addr.init
	add	t4, t5, t4	# loop.addr.init
	j	.main.bb19
.main.bb38:
	li	a4, 300
	sd	a4, 32(sp)
	j	.main.bb40
.main.bb21:
	addiw	t3, s2, 1
	ld	t5, 24(sp)
	lw	s2, 0(t5)
	slt	a3, a2, s2
	subw	a3, zero, a3
	xor	a4, s2, a2
	and	a3, a4, a3
	xor	a3, s2, a3
	ld	t5, 24(sp)
	sw	a3, 0(t5)
	lw	s2, 0(t2)
	slt	a3, a2, s2
	subw	a3, zero, a3
	xor	a2, s2, a2
	and	a2, a2, a3
	xor	a2, s2, a2
	sw	a2, 0(t2)
	addi	t2, t2, 4
	mv	s2, t3	# phi-copy
.main.bb16:
	li	t3, 300
	bge	s2, t3, .main.bb18
.main.bb17:
	slliw	t5, s2, 2
	sd	t5, 16(sp)
	li	a2, 1200
	mulw	s3, s2, a2
	li	a2, 0
	li	a3, 0
	j	.main.bb36
.main.bb32:
	li	t3, 300
	j	.main.bb10
.main.bb12:
	addiw	t0, t0, 1
	j	.main.bb7
.main.bb18:
	addiw	t1, t1, 1
	ld	t5, 24(sp)
	addi	t0, t5, 4
	sd	t0, 24(sp)
.main.bb13:
	li	t2, 300
	bge	t1, t2, .main.bb15
.main.bb14:
	li	t2, 1200
	la	t3, c
	mulw	s1, t1, t2
	slliw	t2, t1, 2
	add	t2, t3, t2
	mv	s2, t1	# phi-copy
	j	.main.bb16
.main.bb23:
	li	t2, 300
	bge	t0, t2, .main.bb25
.main.bb44:
	addiw	t2, t0, 3
	li	t3, 300
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
	li	t1, 300
	bge	t0, t1, .main.bb29
.main.bb46:
	addiw	t1, t0, 3
	li	t2, 300
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
	ld	s7, 96(sp)
	ld	s6, 88(sp)
	ld	s5, 80(sp)
	ld	s4, 72(sp)
	ld	s3, 64(sp)
	ld	s2, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 112
	ret
.main.bb29:
	la	t0, c
	li	t1, 0
	sd	t0, 24(sp)
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
