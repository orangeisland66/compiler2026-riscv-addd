
  .bss
  .globl side_state
  .align 2
  .type  side_state, @object
  .size  side_state, 4
side_state:
  .zero 4

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
	addi	sp, sp, -64
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	li	t0, 0
.main.bb1:
	li	t1, 32
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	t1, t0, 7
	la	t2, left
	add	t1, t2, t1
	li	t2, 0
.main.bb4:
	li	t3, 32
	bge	t2, t3, .main.bb6
.main.bb5:
	li	s2, 0
	slliw	s1, t2, 2
	slliw	a4, s2, 7	# loop.addr.init
	la	a3, right	# loop.addr.init
	addw	a4, a4, s1	# loop.addr.init
	li	t3, 0
	li	a2, 32
	add	a3, a3, a4	# loop.addr.init
	mv	s3, t1	# phi-copy
.main.bb7:
	bge	s2, a2, .main.bb9
.main.bb8:
	lw	s4, 0(s3)
	andi	a4, s4, 1
	bne	a4, zero, .main.bb11
.main.bb10:
	la	s5, side_state
	lw	a4, 0(s5)
	addw	a4, a4, s2
	sw	a4, 0(s5)
	lw	a4, 0(a3)
	mulw	a4, s4, a4
	addw	t3, t3, a4
.main.bb11:
	addiw	s2, s2, 1
	addi	s3, s3, 4
	addi	a3, a3, 128	# loop.addr.bump
	j	.main.bb7
.main.bb12:
	j	.main.bb11
.main.bb9:
	addiw	t2, t2, 1
	j	.main.bb4
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	la	t0, side_state
	lw	a0, 0(t0)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
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
