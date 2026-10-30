
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
	sd	s2, 24(sp)
	la	s1, samples
	li	t0, 0
	li	t1, 128
	mv	s2, s1	# phi-copy
.main.bb1:
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	t2, t0, 2
	addw	t2, t2, t0
	addiw	t2, t2, 2
	addiw	t0, t0, 1
	sw	t2, 0(s2)
	addi	s2, s2, 4
	blt	t0, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
	li	t1, 0
	li	t2, 128
.main.bb4:
	bge	t1, t2, .main.bb6
.main.bb5:
	lw	t3, 0(s1)
	addiw	t1, t1, 1
	addw	t0, t0, t3
	addi	s1, s1, 4
	blt	t1, t2, .main.bb5	# rotated-loop-branch
.main.bb6:
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
