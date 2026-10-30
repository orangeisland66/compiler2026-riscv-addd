
  .bss
  .globl left
  .align 2
  .type  left, @object
  .size  left, 4096
left:
  .zero 4096

  .bss
  .globl right
  .align 2
  .type  right, @object
  .size  right, 4096
right:
  .zero 4096

  .bss
  .globl output
  .align 2
  .type  output, @object
  .size  output, 4096
output:
  .zero 4096

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
	li	t0, 0
.main.bb1:
	li	t1, 32
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s1, t0, 7
	li	t1, 0
	li	t2, 32
	la	s2, left	# loop.addr.hoisted
	la	s3, right	# loop.addr.hoisted
.main.bb17:
	addiw	s4, t1, 8
	bge	t1, t2, .main.bb6
.main.bb18:
	li	t3, 32
	bge	s4, t3, .main.bb19
.main.bb20:
	mv	t3, s4	# phi-copy
	j	.main.bb4
.main.bb21:
.main.bb4:
	bge	t1, t3, .main.bb17
.main.bb23:
	addiw	s4, t1, 3
	blt	s4, t3, .main.bb24
.main.bb5:
	slliw	a3, t1, 2
	addw	a2, t0, t1
	addw	s4, s1, a3
	addiw	a2, a2, 1
	add	a3, s2, s4
	sw	a2, 0(a3)
	subw	a2, t0, t1
	add	a3, s3, s4
	sw	a2, 0(a3)
	addiw	t1, t1, 1
	j	.main.bb4
.main.bb24:
	slliw	a3, t1, 2
	addw	a2, t0, t1
	addw	s5, s1, a3
	addiw	a2, a2, 1
	add	s6, s2, s5
	addiw	s7, t1, 1
	sw	a2, 0(s6)
	add	s5, s3, s5
	subw	a2, t0, t1
	sw	a2, 0(s5)
	addw	a2, t0, s7
	addiw	a2, a2, 1
	sw	a2, 4(s6)
	subw	a2, t0, s7
	sw	a2, 4(s5)
	addiw	s7, t1, 2
	addw	a2, t0, s7
	addiw	t1, t1, 4
	addiw	a2, a2, 1
	sw	a2, 8(s6)
	subw	a2, t0, s7
	sw	a2, 8(s5)
	addw	a2, t0, s4
	addiw	a2, a2, 1
	sw	a2, 12(s6)
	subw	a2, t0, s4
	sw	a2, 12(s5)
	j	.main.bb4
.main.bb22:
	j	.main.bb17
.main.bb19:
	li	t3, 32
	j	.main.bb4
.main.bb13:
	bge	s2, a3, .main.bb15
.main.bb14:
	lw	s5, 0(s4)
	li	a7, 0
	andi	a5, s5, 1
	sltiu	a5, a5, 1
	subw	a5, a7, a5
	lw	a6, 0(a4)
	mulw	a6, s5, a6
	addw	a6, a2, a6
	xor	a6, a2, a6
	and	a5, a6, a5
	xor	a2, a2, a5
.main.bb16:
	addiw	s2, s2, 1
	addi	s4, s4, 4
	addi	a4, a4, 128	# loop.addr.bump
	j	.main.bb13
.main.bb15:
	sw	a2, 0(t1)
	addiw	t3, t3, 1
	addi	t1, t1, 4
.main.bb10:
	li	a2, 32
	bge	t3, a2, .main.bb12
.main.bb11:
	li	s2, 0
	slliw	s1, t3, 2
	slliw	a4, s2, 7	# loop.addr.init
	la	s3, right	# loop.addr.hoisted
	addw	a4, a4, s1	# loop.addr.init
	li	a2, 0
	li	a3, 32
	add	a4, s3, a4	# loop.addr.init
	mv	s4, t2	# phi-copy
	j	.main.bb13
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb12:
	addiw	t0, t0, 1
.main.bb7:
	li	t1, 32
	bge	t0, t1, .main.bb9
.main.bb8:
	slliw	s1, t0, 7
	la	t1, output
	la	t2, left
	add	t1, t1, s1
	add	t2, t2, s1
	li	t3, 0
	j	.main.bb10
.main.bb3:
	li	t0, 0
	j	.main.bb7
.main.bb9:
	la	t0, output
	lw	a0, 404(t0)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
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
	.size main, .-main
