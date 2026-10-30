
  .bss
  .globl samples
  .align 2
  .type  samples, @object
  .size  samples, 1024
samples:
  .zero 1024

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	la	t0, samples
	li	t1, 0
	li	t2, 256
.main.bb1:
	bge	t1, t2, .main.bb3
.main.bb2:
	addiw	t3, t1, 3
	sw	t3, 0(t0)
	addiw	t1, t1, 1
	addi	t0, t0, 4
	blt	t1, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t1, 0
	la	s1, samples	# loop.addr.hoisted
	slliw	t3, t1, 1	# loop.addr.init
	li	t0, 0
	addiw	t3, t3, 1	# loop.addr.init
	li	t2, 64
	slliw	t3, t3, 2	# loop.addr.init
	add	t3, s1, t3	# loop.addr.init
.main.bb4:
	bge	t1, t2, .main.bb6
.main.bb5:
	addiw	t1, t1, 1
	lw	a2, 0(t3)
	addw	t0, t0, a2
	addi	t3, t3, 8	# loop.addr.bump
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
