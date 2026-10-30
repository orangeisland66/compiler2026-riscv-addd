
  .bss
  .globl samples
  .align 2
  .type  samples, @object
  .size  samples, 512
samples:
  .zero 512

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	li	t0, 0
	la	t2, samples	# loop.addr.init
	slliw	t3, t0, 2	# loop.addr.init
	li	t1, 128
	add	t2, t2, t3	# loop.addr.init
.main.bb1:
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s1, t0, 2
	addw	t3, s1, t0
	addiw	t3, t3, 2
	addiw	t0, t0, 1
	sw	t3, 0(t2)
	addi	t2, t2, 4	# loop.addr.bump
	blt	t0, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
	li	t1, 0
	li	t2, 128
	la	t3, samples	# loop.addr.hoisted
.main.bb4:
	bge	t1, t2, .main.bb6
.main.bb5:
	slliw	a2, t1, 2
	add	a2, t3, a2
	addiw	t1, t1, 1
	lw	a2, 0(a2)
	addw	t0, t0, a2
	blt	t1, t2, .main.bb5	# rotated-loop-branch
.main.bb6:
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
