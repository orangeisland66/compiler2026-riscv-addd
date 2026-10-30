
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
	addi	sp, sp, -80
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
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
	li	t2, 1200
	slliw	t1, t0, 2
	mulw	t2, t0, t2
	li	t3, 0
.main.bb30:
	addiw	s1, t3, 32
	li	a2, 300
	bge	t3, a2, .main.bb12
.main.bb31:
	li	a2, 300
	bge	s1, a2, .main.bb32
.main.bb33:
	mv	a2, s1	# phi-copy
.main.bb34:
	li	a3, 1200
	la	a4, a
	mulw	a3, t3, a3
	la	a5, b
	addw	a3, t1, a3
	add	a3, a4, a3
	slliw	a4, t3, 2
	addw	a4, t2, a4
	add	a4, a5, a4
.main.bb10:
	bge	t3, a2, .main.bb30
.main.bb11:
	lw	a5, 0(a3)
	addiw	t3, t3, 1
	sw	a5, 0(a4)
	addi	a3, a3, 1200
	addi	a4, a4, 4
	blt	t3, a2, .main.bb11	# rotated-loop-branch
	j	.main.bb30	# rotated-loop-exit
.main.bb35:
	j	.main.bb30
.main.bb19:
	bge	a2, a3, .main.bb36
.main.bb20:
	lw	a1, 0(a4)
	lw	t4, 0(a5)
	lw	a0, 0(a6)
	and	t4, t4, a0	# parity.masked-accumulate
	andi	t4, t4, 1
	lw	a0, 0(a7)
	mulw	a0, a0, a1
	addiw	t4, t4, -1
	and	a0, a0, t4
	addw	t3, t3, a0
.main.bb22:
	addiw	a2, a2, 1
	addi	a4, a4, 1200
	addi	a5, a5, 4	# loop.addr.bump
	addi	a6, a6, 4	# loop.addr.bump
	addi	a7, a7, 4	# loop.addr.bump
	j	.main.bb19
.main.bb41:
.main.bb36:
	addiw	s6, a2, 16
	li	a3, 300
	bge	a2, a3, .main.bb21
.main.bb37:
	li	a3, 300
	bge	s6, a3, .main.bb38
.main.bb39:
	mv	a3, s6	# phi-copy
.main.bb40:
	li	a4, 1200
	la	a5, a
	mulw	a4, a2, a4
	slliw	a6, a2, 2	# loop.addr.init
	slliw	a7, a2, 2	# loop.addr.init
	la	s6, a	# loop.addr.hoisted
	la	s7, b	# loop.addr.hoisted
	addw	a6, a6, s5	# loop.addr.init
	addw	a4, t2, a4
	addw	a7, s3, a7	# loop.addr.init
	add	a4, a5, a4
	add	a6, s6, a6	# loop.addr.init
	slliw	a5, a2, 2	# loop.addr.init
	add	a7, s7, a7	# loop.addr.init
	addw	a5, s3, a5	# loop.addr.init
	add	a5, s6, a5	# loop.addr.init
	j	.main.bb19
.main.bb38:
	li	a3, 300
	j	.main.bb40
.main.bb21:
	addiw	t2, s4, 1
	lw	s4, 0(s2)
	slt	a2, t3, s4
	xor	a3, s4, t3
	subw	a2, zero, a2
	and	a2, a3, a2
	xor	a2, s4, a2
	sw	a2, 0(s2)
	lw	s4, 0(t1)
	slt	a2, t3, s4
	subw	a2, zero, a2
	xor	t3, s4, t3
	and	t3, t3, a2
	xor	t3, s4, t3
	sw	t3, 0(t1)
	addi	t1, t1, 4
	mv	s4, t2	# phi-copy
.main.bb16:
	li	t2, 300
	bge	s4, t2, .main.bb18
.main.bb17:
	li	t3, 1200
	slliw	t2, s4, 2
	mulw	s5, s4, t3
	li	a2, 0
	li	t3, 0
	j	.main.bb36
.main.bb32:
	li	a2, 300
	j	.main.bb34
.main.bb12:
	addiw	t0, t0, 1
	j	.main.bb7
.main.bb18:
	addiw	t0, t0, 1
	addi	s2, s2, 4
.main.bb13:
	li	t1, 300
	bge	t0, t1, .main.bb15
.main.bb14:
	li	t1, 1200
	la	t2, c
	mulw	s3, t0, t1
	slliw	t1, t0, 2
	add	t1, t2, t1
	mv	s4, t0	# phi-copy
	j	.main.bb16
.main.bb5:
	mv	t0, s3	# phi-copy
.main.bb1:
	mv	a0, t0
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
.main.bb9:
	la	s1, c
	li	t0, 0
	li	t1, 300
	mv	s2, s1	# phi-copy
.main.bb27:
	bge	t0, t1, .main.bb29
.main.bb28:
	li	t2, 2147483647
	addiw	t0, t0, 1
	sw	t2, 0(s2)
	addi	s2, s2, 4
	blt	t0, t1, .main.bb28	# rotated-loop-branch
.main.bb29:
	li	t0, 0
	mv	s2, s1	# phi-copy
	j	.main.bb13
.main.bb15:
	li	t0, 0
	li	t1, 0
	li	t2, 300
.main.bb23:
	bge	t0, t2, .main.bb25
.main.bb24:
	lw	t3, 0(s1)
	addiw	t0, t0, 1
	addw	t1, t1, t3
	addi	s1, s1, 4
	blt	t0, t2, .main.bb24	# rotated-loop-branch
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
	j	.main.bb1
	.size main, .-main
