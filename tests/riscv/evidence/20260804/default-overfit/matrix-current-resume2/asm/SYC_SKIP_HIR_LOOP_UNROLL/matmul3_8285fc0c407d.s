
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 250000
a:
  .zero 250000

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 250000
b:
  .zero 250000

  .bss
  .globl c
  .align 2
  .type  c, @object
  .size  c, 250000
c:
  .zero 250000

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
	sd	s8, 72(sp)
	li	s2, 0
	li	s1, 250
.main.bb2:
	bge	s2, s1, .main.bb4
.main.bb3:
	li	t0, 1000
	mulw	t0, s2, t0
	la	t1, a
	add	t5, t1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s3, a0
	li	t0, 250
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
	li	t1, 250
	bge	t0, t1, .main.bb9
.main.bb8:
	li	t1, 1000
	slliw	s1, t0, 2
	mulw	t1, t0, t1
	li	t2, 0
.main.bb30:
	addiw	s2, t2, 32
	li	t3, 250
	bge	t2, t3, .main.bb12
.main.bb31:
	li	t3, 250
	bge	s2, t3, .main.bb32
.main.bb33:
	mv	t3, s2	# phi-copy
.main.bb34:
	li	a4, 1000	# loop.addr.init
	slliw	a2, t2, 2
	mulw	a4, t2, a4	# loop.addr.init
	addw	a2, t1, a2
	la	a3, b
	add	a2, a3, a2
	la	a3, a	# loop.addr.init
	addw	a4, a4, s1	# loop.addr.init
	add	a3, a3, a4	# loop.addr.init
.main.bb10:
	bge	t2, t3, .main.bb30
.main.bb11:
	addiw	t2, t2, 1
	lw	a4, 0(a3)
	sw	a4, 0(a2)
	addi	a3, a3, 1000	# loop.addr.bump
	addi	a2, a2, 4
	blt	t2, t3, .main.bb11	# rotated-loop-branch
	j	.main.bb30	# rotated-loop-exit
.main.bb35:
	j	.main.bb30
.main.bb19:
	bge	t3, a2, .main.bb36
.main.bb20:
	li	t4, 1000
	lw	a7, 0(a4)
	mulw	t4, t3, t4
	lw	a0, 0(a5)
	andi	a7, a7, 1
	addw	t4, t4, s5
	andi	a0, a0, 1
	add	t4, s7, t4
	lw	t4, 0(a3)
	and	a7, a7, a0
	sltiu	a7, a7, 1
	lw	a0, 0(a6)
	mulw	t4, a0, t4
	addw	t4, t2, t4
	li	a0, 0
	subw	a7, a0, a7
	xor	t4, t2, t4
	and	a7, t4, a7
	xor	t2, t2, a7
.main.bb22:
	addiw	t3, t3, 1
	addi	a3, a3, 1000	# loop.addr.bump
	addi	a4, a4, 4	# loop.addr.bump
	addi	a5, a5, 4	# loop.addr.bump
	addi	a6, a6, 4	# loop.addr.bump
	j	.main.bb19
.main.bb41:
.main.bb36:
	addiw	s7, t3, 16
	li	a2, 250
	bge	t3, a2, .main.bb21
.main.bb37:
	li	a2, 250
	bge	s7, a2, .main.bb38
.main.bb39:
	mv	a2, s7	# phi-copy
.main.bb40:
	li	a3, 1000	# loop.addr.init
	slliw	a4, t3, 2	# loop.addr.init
	mulw	a3, t3, a3	# loop.addr.init
	slliw	a5, t3, 2	# loop.addr.init
	slliw	a6, t3, 2	# loop.addr.init
	la	s7, a	# loop.addr.hoisted
	la	s8, b	# loop.addr.hoisted
	addw	a4, s3, a4	# loop.addr.init
	addw	a3, a3, s5	# loop.addr.init
	addw	a5, a5, s6	# loop.addr.init
	addw	a6, s3, a6	# loop.addr.init
	add	a3, s7, a3	# loop.addr.init
	add	a4, s7, a4	# loop.addr.init
	add	a5, s7, a5	# loop.addr.init
	add	a6, s8, a6	# loop.addr.init
	j	.main.bb19
.main.bb38:
	li	a2, 250
	j	.main.bb40
.main.bb21:
	addiw	t3, s4, 1
	lw	s4, 0(s2)
	slt	a2, t2, s4
	xor	a3, s4, t2
	subw	a2, zero, a2
	and	a2, a3, a2
	xor	a2, s4, a2
	sw	a2, 0(s2)
	lw	s4, 0(t1)
	slt	a2, t2, s4
	subw	a2, zero, a2
	xor	t2, s4, t2
	and	t2, t2, a2
	xor	t2, s4, t2
	sw	t2, 0(t1)
	addi	t1, t1, 4
	mv	s4, t3	# phi-copy
.main.bb16:
	li	t2, 250
	bge	s4, t2, .main.bb18
.main.bb17:
	li	t2, 1000
	slliw	s5, s4, 2
	mulw	s6, s4, t2
	li	t3, 0
	li	t2, 0
	j	.main.bb36
.main.bb32:
	li	t3, 250
	j	.main.bb34
.main.bb12:
	addiw	t0, t0, 1
	j	.main.bb7
.main.bb18:
	addiw	t0, t0, 1
	addi	s2, s2, 4
.main.bb13:
	li	t1, 250
	bge	t0, t1, .main.bb15
.main.bb14:
	li	t1, 1000
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
.main.bb9:
	la	s1, c
	li	t0, 0
	li	t1, 250
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
	li	t2, 250
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
