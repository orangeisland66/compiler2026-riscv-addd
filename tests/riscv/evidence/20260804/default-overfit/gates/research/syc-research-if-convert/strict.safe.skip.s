
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
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	li	t0, 0
.main.bb1:
	li	t1, 32
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s1, t0, 7
	li	t1, 0
	li	t2, 32
	la	s2, left	# loop.addr.hoisted
.main.bb17:
	addiw	s3, t1, 8
	bge	t1, t2, .main.bb6
.main.bb18:
	li	t3, 32
	bge	s3, t3, .main.bb19
.main.bb20:
	mv	t3, s3	# phi-copy
	j	.main.bb4
.main.bb21:
.main.bb4:
	bge	t1, t3, .main.bb17
.main.bb23:
	addiw	s3, t1, 3
	blt	s3, t3, .main.bb24
.main.bb5:
	slliw	a3, t1, 2
	addw	a2, t0, t1
	addw	a3, s1, a3
	addiw	a2, a2, 1
	add	a3, s2, a3
	addiw	t1, t1, 1
	sw	a2, 0(a3)
	j	.main.bb4
.main.bb24:
	slliw	a3, t1, 2
	addw	a2, t0, t1
	addw	a3, s1, a3
	addiw	a2, a2, 1
	add	s4, s2, a3
	sw	a2, 0(s4)
	addiw	a2, t1, 1
	addw	a2, t0, a2
	addiw	a2, a2, 1
	sw	a2, 4(s4)
	addiw	a2, t1, 2
	addw	a2, t0, a2
	addiw	t1, t1, 4
	addiw	a2, a2, 1
	sw	a2, 8(s4)
	addw	a2, t0, s3
	addiw	a2, a2, 1
	sw	a2, 12(s4)
	j	.main.bb4
.main.bb22:
	j	.main.bb17
.main.bb19:
	li	t3, 32
	j	.main.bb4
.main.bb13:
	bge	a3, a4, .main.bb15
.main.bb14:
	lw	a5, 0(s1)
	li	a7, 0
	andi	a5, a5, 1
	sltiu	a5, a5, 1
	addiw	a6, a2, 7
	subw	a5, a7, a5
	xor	a6, a2, a6
	and	a5, a6, a5
	xor	a2, a2, a5
.main.bb16:
	addiw	a3, a3, 1
	addi	s1, s1, 4
	j	.main.bb13
.main.bb15:
	sw	a2, 0(t1)
	addiw	t3, t3, 1
	addi	t1, t1, 4
.main.bb10:
	li	a2, 32
	bge	t3, a2, .main.bb12
.main.bb11:
	li	a2, 0
	li	a3, 0
	li	a4, 32
	mv	s1, t2	# phi-copy
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
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
