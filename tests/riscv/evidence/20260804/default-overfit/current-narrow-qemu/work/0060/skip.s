
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
	addi	sp, sp, -64
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
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
.main.bb38:
	addiw	s5, t1, 32
	bge	t1, t2, .main.bb12
.main.bb39:
	li	t3, 300
	bge	s5, t3, .main.bb40
.main.bb41:
	mv	t3, s5	# phi-copy
	j	.main.bb10
.main.bb42:
.main.bb10:
	bge	t1, t3, .main.bb38
.main.bb44:
	li	a3, 1200
	addiw	a2, t1, 3
	mulw	s5, t1, a3
	blt	a2, t3, .main.bb45
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
.main.bb45:
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
.main.bb43:
	j	.main.bb38
.main.bb19:
	bge	a3, a4, .main.bb32
.main.bb20:
	lw	a1, 0(a5)
	lw	a0, 0(a7)
	andi	a1, a1, 1
	andi	a0, a0, 1
	and	a0, a0, a1
	lw	a1, 0(a6)
	sltiu	s5, a0, 1
	lw	a0, 0(t4)
	mulw	a0, a0, a1
	addw	a0, a2, a0
	li	a1, 0
	subw	a1, a1, s5
	xor	a0, a2, a0
	and	a0, a0, a1
	xor	a2, a2, a0
.main.bb22:
	addiw	a3, a3, 1
	addi	a5, a5, 1200
	addi	a6, a6, 1200
	addi	a7, a7, 4	# loop.addr.bump
	addi	t4, t4, 4	# loop.addr.bump
	j	.main.bb19
.main.bb37:
.main.bb32:
	addiw	s2, a3, 16
	li	a4, 300
	bge	a3, a4, .main.bb21
.main.bb33:
	li	a4, 300
	bge	s2, a4, .main.bb34
.main.bb35:
	mv	a4, s2	# phi-copy
.main.bb36:
	li	a5, 1200
	la	a6, a
	mulw	a5, a3, a5
	slliw	a7, a3, 2	# loop.addr.init
	slliw	t4, a3, 2	# loop.addr.init
	la	s3, b	# loop.addr.hoisted
	addw	a7, s1, a7	# loop.addr.init
	addw	t4, s1, t4	# loop.addr.init
	addw	s2, t3, a5
	add	t4, s3, t4	# loop.addr.init
	la	a5, b
	add	a6, a6, s2
	add	a5, a5, s2
	la	s2, a	# loop.addr.hoisted
	add	a7, s2, a7	# loop.addr.init
	j	.main.bb19
.main.bb34:
	li	a4, 300
	j	.main.bb36
.main.bb21:
	sw	a2, 0(t1)
	addiw	t2, t2, 1
	addi	t1, t1, 4
.main.bb16:
	li	t3, 300
	bge	t2, t3, .main.bb18
.main.bb17:
	slliw	t3, t2, 2
	li	a2, 0
	li	a3, 0
	j	.main.bb32
.main.bb40:
	li	t3, 300
	j	.main.bb10
.main.bb12:
	addiw	t0, t0, 1
	j	.main.bb7
.main.bb18:
	addiw	t0, t0, 1
.main.bb13:
	li	t1, 300
	bge	t0, t1, .main.bb15
.main.bb14:
	li	t1, 1200
	li	t2, 0
	mulw	s1, t0, t1
	la	t1, c
	add	t1, t1, s1
	j	.main.bb16
.main.bb26:
	bge	a2, a3, .main.bb28
.main.bb27:
	lw	s1, 0(t2)
	blt	s1, t3, .main.bb30
	j	.main.bb46
.main.bb29:
.main.bb30:
	addiw	a2, a2, 1
	addi	t2, t2, 4
	mv	t3, s1	# phi-copy
	j	.main.bb26
.main.bb46:
	mv	s1, t3	# phi-copy
	j	.main.bb30
.main.bb28:
	addiw	t0, t0, 1
	addw	t1, t1, t3
.main.bb23:
	li	t2, 300
	bge	t0, t2, .main.bb25
.main.bb24:
	li	t2, 1200
	la	t3, c
	mulw	t2, t0, t2
	li	a2, 0
	li	a3, 300
	add	t2, t3, t2
	li	t3, 2147483647
	j	.main.bb26
.main.bb5:
	mv	t0, s3	# phi-copy
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
.main.bb9:
	li	t0, 0
	j	.main.bb13
.main.bb15:
	li	t0, 0
	li	t1, 0
	j	.main.bb23
.main.bb25:
	subw	s1, zero, t1
.main.bb31:
	li	a0, 93
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t0, 0
	j	.main.bb1
	.size main, .-main
