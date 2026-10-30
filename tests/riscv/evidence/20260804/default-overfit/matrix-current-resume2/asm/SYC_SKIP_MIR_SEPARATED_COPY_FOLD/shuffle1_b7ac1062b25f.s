
  .bss
  .globl hashmod
  .align 2
  .type  hashmod, @object
  .size  hashmod, 4
hashmod:
  .zero 4

  .section .rodata
  .globl maxn
  .align 2
  .type  maxn, @object
  .size  maxn, 4
maxn:
  .word 10000000

  .section .rodata
  .globl maxm
  .align 2
  .type  maxm, @object
  .size  maxm, 4
maxm:
  .word 10000000

  .bss
  .globl bucket
  .align 2
  .type  bucket, @object
  .size  bucket, 40000000
bucket:
  .zero 40000000

  .bss
  .globl head
  .align 2
  .type  head, @object
  .size  head, 40000000
head:
  .zero 40000000

  .bss
  .globl next
  .align 2
  .type  next, @object
  .size  next, 40000000
next:
  .zero 40000000

  .bss
  .globl nextvalue
  .align 2
  .type  nextvalue, @object
  .size  nextvalue, 40000000
nextvalue:
  .zero 40000000

  .bss
  .globl key
  .align 2
  .type  key, @object
  .size  key, 40000000
key:
  .zero 40000000

  .bss
  .globl value
  .align 2
  .type  value, @object
  .size  value, 40000000
value:
  .zero 40000000

  .bss
  .globl cnt
  .align 2
  .type  cnt, @object
  .size  cnt, 4
cnt:
  .zero 4

  .bss
  .globl keys
  .align 2
  .type  keys, @object
  .size  keys, 40000000
keys:
  .zero 40000000

  .bss
  .globl values
  .align 2
  .type  values, @object
  .size  values, 40000000
values:
  .zero 40000000

  .bss
  .globl requests
  .align 2
  .type  requests, @object
  .size  requests, 40000000
requests:
  .zero 40000000

  .bss
  .globl ans
  .align 2
  .type  ans, @object
  .size  ans, 40000000
ans:
  .zero 40000000

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -112
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	sd	s8, 80(sp)
	sd	s9, 88(sp)
	sd	s10, 96(sp)
	sd	s11, 104(sp)
	call	getint
	mv	s1, a0
	la	t5, keys
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s2, a0
	la	t5, values
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	la	t5, requests
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s3, a0
	li	t5, 81
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	la	t0, keys
	la	t1, values
	li	t2, 0
	la	s4, nextvalue	# loop.addr.hoisted
	la	s5, value	# loop.addr.hoisted
	la	s6, cnt	# loop.addr.hoisted
	la	s7, key	# loop.addr.hoisted
.main.bb1:
	bge	t2, s2, .main.bb3
.main.bb2:
	lw	s8, 0(t0)
	la	a2, head
	lw	s9, 0(t1)
	lw	s10, 0(s6)
	remw	t3, s8, s1
	slliw	t3, t3, 2
	add	s11, a2, t3
	lw	t3, 0(s11)
	bne	t3, zero, .main.bb9
.main.bb8:
	addiw	s10, s10, 1
	li	a2, 0
	sw	s10, 0(s11)
	slliw	s11, s10, 2
	add	t3, s7, s11
	sw	s8, 0(t3)
	add	t3, s5, s11
	sw	s9, 0(t3)
	la	t3, next
	add	t3, t3, s11
	sw	a2, 0(t3)
	add	t3, s4, s11
	sw	a2, 0(t3)
.main.bb7:
	sw	s10, 0(s6)
	addiw	t2, t2, 1
	addi	t0, t0, 4
	addi	t1, t1, 4
	blt	t2, s2, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
.main.bb26:
	addiw	s2, t0, 8
	bge	t0, s3, .main.bb6
.main.bb27:
	bge	s2, s3, .main.bb28
.main.bb29:
.main.bb30:
	slliw	s4, t0, 2
	la	t1, requests
	la	t2, ans
	add	t1, t1, s4
	add	t2, t2, s4
.main.bb4:
	bge	t0, s2, .main.bb26
.main.bb5:
	lw	s4, 0(t1)
	la	a2, head
	la	a3, next	# loop.addr.hoisted
	remw	t3, s4, s1
	slliw	t3, t3, 2
	add	t3, a2, t3
	lw	t3, 0(t3)
	la	a2, key	# loop.addr.hoisted
.main.bb16:
	beq	t3, zero, .main.bb18
.main.bb17:
	slliw	s5, t3, 2
	add	a4, a2, s5
	lw	a4, 0(a4)
	beq	a4, s4, .main.bb19
.main.bb20:
	add	t3, a3, s5
	lw	t3, 0(t3)
	bne	t3, zero, .main.bb17	# rotated-loop-branch
.main.bb18:
	li	t3, 0
.main.bb15:
	sw	t3, 0(t2)
	addiw	t0, t0, 1
	addi	t1, t1, 4
	addi	t2, t2, 4
	j	.main.bb4
.main.bb19:
	li	a2, 0
	la	a3, nextvalue	# loop.addr.hoisted
	la	a4, value	# loop.addr.hoisted
.main.bb21:
	beq	t3, zero, .main.bb23
.main.bb22:
	slliw	s5, t3, 2
	add	t3, a4, s5
	lw	t3, 0(t3)
	addw	t3, a2, t3
	add	a2, a3, s5
	lw	a2, 0(a2)
	mv	t5, t3	# phi-cycle-break
	mv	t3, a2	# phi-copy
	mv	a2, t5	# phi-copy
	bne	t3, zero, .main.bb22	# rotated-loop-branch
.main.bb23:
	li	t3, 100
	blt	t3, s4, .main.bb24
.main.bb25:
	slliw	t3, a2, 1
	addw	t3, t3, a2
	j	.main.bb15
.main.bb24:
	slliw	t3, a2, 1
	j	.main.bb15
.main.bb31:
	j	.main.bb26
.main.bb9:
	la	a2, key	# loop.addr.hoisted
	la	a3, next	# loop.addr.hoisted
.main.bb10:
	beq	t3, zero, .main.bb12
.main.bb11:
	slliw	t3, t3, 2
	add	a4, a2, t3
	lw	a4, 0(a4)
	beq	a4, s8, .main.bb13
.main.bb14:
	add	t3, a3, t3
	lw	t3, 0(t3)
	bne	t3, zero, .main.bb11	# rotated-loop-branch
.main.bb12:
	lw	t3, 0(s11)
	addiw	s10, s10, 1
	slliw	a2, s10, 2
	la	a3, next
	add	a3, a3, a2
	sw	t3, 0(a3)
	sw	s10, 0(s11)
	add	t3, s7, a2
	sw	s8, 0(t3)
	add	t3, s5, a2
	sw	s9, 0(t3)
	add	t3, s4, a2
	li	a2, 0
	sw	a2, 0(t3)
	j	.main.bb7
.main.bb13:
	addiw	s8, s10, 1
	add	s10, s4, t3
	slliw	s11, s8, 2
	lw	t3, 0(s10)
	add	a2, s4, s11
	sw	t3, 0(a2)
	sw	s8, 0(s10)
	add	t3, s5, s11
	sw	s9, 0(t3)
	mv	s10, s8	# phi-copy
	j	.main.bb7
.main.bb28:
	mv	s2, s3	# phi-copy
	j	.main.bb30
.main.bb6:
	li	a0, 93
	call	_sysy_stoptime
	mv	t5, s3
	sd	t5, 0(sp)
	la	t5, ans
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	la	t0, hashmod
	sw	s1, 0(t0)
	li	a0, 0
	ld	s11, 104(sp)
	ld	s10, 96(sp)
	ld	s9, 88(sp)
	ld	s8, 80(sp)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 112
	ret
	.size main, .-main
