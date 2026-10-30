
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
	mulw	t1, t0, t1
	li	t2, 0
.main.bb52:
	addiw	s2, t2, 32
	li	t3, 200
	bge	t2, t3, .main.bb12
.main.bb53:
	li	t3, 200
	bge	s2, t3, .main.bb54
.main.bb55:
	mv	t3, s2	# phi-copy
.main.bb56:
	li	a4, 800	# loop.addr.init
	slliw	a2, t2, 2
	mulw	a4, t2, a4	# loop.addr.init
	addw	a2, t1, a2
	la	a3, b
	add	a2, a3, a2
	la	a3, a	# loop.addr.init
	addw	a4, a4, s1	# loop.addr.init
	add	a3, a3, a4	# loop.addr.init
.main.bb10:
	bge	t2, t3, .main.bb52
.main.bb11:
	addiw	t2, t2, 1
	lw	a4, 0(a3)
	sw	a4, 0(a2)
	addi	a3, a3, 800	# loop.addr.bump
	addi	a2, a2, 4
	blt	t2, t3, .main.bb11	# rotated-loop-branch
	j	.main.bb52	# rotated-loop-exit
.main.bb57:
	j	.main.bb52
.main.bb19:
	bge	a2, a3, .main.bb58
.main.bb20:
	lw	t4, 0(a5)
	lw	a1, 0(a4)
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
	addi	a4, a4, 800	# loop.addr.bump
	addi	a5, a5, 4	# loop.addr.bump
	addi	a6, a6, 4	# loop.addr.bump
	addi	a7, a7, 4	# loop.addr.bump
	j	.main.bb19
.main.bb63:
.main.bb58:
	addiw	s4, a2, 2
	li	a3, 200
	bge	a2, a3, .main.bb21
.main.bb59:
	li	a3, 200
	bge	s4, a3, .main.bb60
.main.bb61:
	mv	a3, s4	# phi-copy
.main.bb62:
	li	a4, 800	# loop.addr.init
	slliw	a5, a2, 2	# loop.addr.init
	mulw	a4, a2, a4	# loop.addr.init
	slliw	a6, a2, 2	# loop.addr.init
	slliw	a7, a2, 2	# loop.addr.init
	la	s4, a	# loop.addr.hoisted
	la	s5, b	# loop.addr.hoisted
	addw	a5, s1, a5	# loop.addr.init
	addw	a4, a4, s2	# loop.addr.init
	addw	a6, a6, s3	# loop.addr.init
	addw	a7, s1, a7	# loop.addr.init
	add	a4, s4, a4	# loop.addr.init
	add	a5, s4, a5	# loop.addr.init
	add	a6, s4, a6	# loop.addr.init
	add	a7, s5, a7	# loop.addr.init
	j	.main.bb19
.main.bb60:
	li	a3, 200
	j	.main.bb62
.main.bb21:
	sw	t3, 0(t1)
	addiw	t2, t2, 1
	addi	t1, t1, 4
.main.bb16:
	li	t3, 200
	bge	t2, t3, .main.bb18
.main.bb17:
	li	t3, 800
	slliw	s2, t2, 2
	mulw	s3, t2, t3
	li	a2, 0
	li	t3, 0
	j	.main.bb58
.main.bb38:
	addiw	t2, t2, 1
	lw	a4, 0(a3)
	subw	a4, zero, a4
	addi	a3, a3, 800	# loop.addr.bump
	sw	a4, 0(a2)
	addi	a2, a2, 4
	blt	t2, s3, .main.bb38	# rotated-loop-branch
	j	.main.bb64	# rotated-loop-exit
.main.bb69:
.main.bb64:
	addiw	s3, t2, 8
	bge	t2, t3, .main.bb46
.main.bb65:
	bge	s3, t3, .main.bb66
.main.bb67:
.main.bb68:
	li	a3, 800	# loop.addr.init
	slliw	a2, t2, 2
	mulw	a3, t2, a3	# loop.addr.init
	addw	a2, t1, a2
	add	a2, s2, a2
	addw	a3, a3, s1	# loop.addr.init
	add	a3, s2, a3	# loop.addr.init
.main.bb37:
	bge	t2, s3, .main.bb64
	j	.main.bb38
.main.bb66:
	mv	s3, t3	# phi-copy
	j	.main.bb68
.main.bb51:
.main.bb46:
	addiw	s2, t2, 32
	li	t3, 200
	bge	t2, t3, .main.bb39
.main.bb47:
	li	t3, 200
	bge	s2, t3, .main.bb48
.main.bb49:
	mv	t3, s2	# phi-copy
.main.bb50:
	la	s2, c	# loop.addr.hoisted
	j	.main.bb64
.main.bb48:
	li	t3, 200
	j	.main.bb50
.main.bb54:
	li	t3, 200
	j	.main.bb56
.main.bb12:
	addiw	t0, t0, 1
	j	.main.bb7
.main.bb18:
	addiw	t0, t0, 1
.main.bb13:
	li	t1, 200
	bge	t0, t1, .main.bb15
.main.bb14:
	li	t1, 800
	li	t2, 0
	mulw	s1, t0, t1
	la	t1, c
	add	t1, t1, s1
	j	.main.bb16
.main.bb39:
	addiw	t0, t0, 1
.main.bb34:
	li	t1, 200
	bge	t0, t1, .main.bb36
.main.bb35:
	li	t1, 800
	slliw	s1, t0, 2
	mulw	t1, t0, t1
	li	t2, 0
	j	.main.bb46
.main.bb26:
	bge	t2, t3, .main.bb28
.main.bb27:
	lw	s3, 0(s2)
	blt	s3, t1, .main.bb30
	j	.main.bb70
.main.bb29:
.main.bb30:
	addiw	t2, t2, 1
	addi	s2, s2, 4
	mv	t1, s3	# phi-copy
	j	.main.bb26
.main.bb70:
	mv	s3, t1	# phi-copy
	j	.main.bb30
.main.bb28:
	li	t2, 0
	li	t3, 200
.main.bb31:
	bge	t2, t3, .main.bb33
.main.bb32:
	sw	t1, 0(s1)
	addiw	t2, t2, 1
	addi	s1, s1, 4
	blt	t2, t3, .main.bb32	# rotated-loop-branch
.main.bb33:
	addiw	t0, t0, 1
.main.bb23:
	li	t1, 200
	bge	t0, t1, .main.bb25
.main.bb24:
	li	t1, 800
	la	t2, c
	mulw	t1, t0, t1
	li	t3, 200
	add	s1, t2, t1
	li	t1, 2147483647
	li	t2, 0
	mv	s2, s1	# phi-copy
	j	.main.bb26
.main.bb44:
	lw	a2, 0(t0)
	addiw	t2, t2, 1
	addw	s1, s1, a2
	addi	t0, t0, 4
	blt	t2, t3, .main.bb44	# rotated-loop-branch
.main.bb45:
	addiw	t1, t1, 1
.main.bb40:
	li	t0, 200
	bge	t1, t0, .main.bb42
.main.bb41:
	li	t0, 800
	la	t2, c
	mulw	t0, t1, t0
	li	t3, 200
	add	t0, t2, t0
	li	t2, 0
.main.bb43:
	bge	t2, t3, .main.bb45
	j	.main.bb44
.main.bb5:
	mv	t0, s3	# phi-copy
.main.bb1:
	mv	a0, t0
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
	j	.main.bb23
.main.bb25:
	li	t0, 0
	j	.main.bb34
.main.bb36:
	li	s1, 0
	li	t1, 0
	j	.main.bb40
.main.bb42:
	li	a0, 93
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t0, 0
	j	.main.bb1
	.size main, .-main
