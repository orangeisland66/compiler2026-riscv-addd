
  .bss
  .globl samples
  .align 2
  .type  samples, @object
  .size  samples, 16384
samples:
  .zero 16384

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
	li	t1, 64
.main.bb1:
	bge	t0, t1, .main.bb3
.main.bb2:
	mulw	t2, t0, t0
	la	a2, samples
	addiw	t3, t0, 3
	slliw	t2, t2, 2
	addiw	t0, t0, 1
	add	t2, a2, t2
	sw	t3, 0(t2)
	blt	t0, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
	li	t1, 0
	li	t2, 64
	la	t3, samples	# loop.addr.hoisted
.main.bb4:
	bge	t1, t2, .main.bb6
.main.bb5:
	mulw	a2, t1, t1
	slliw	a2, a2, 2
	addiw	t1, t1, 1
	add	a2, t3, a2
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
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
