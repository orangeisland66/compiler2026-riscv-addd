
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
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
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
	li	t0, 0
	li	s2, 0
	li	s1, 64
.main.bb4:
	bge	s2, s1, .main.bb6
.main.bb5:
	slliw	t1, s2, 1
	addiw	t1, t1, 1
	slliw	t1, t1, 2
	la	t2, samples
	add	t1, t2, t1
	lw	t1, 0(t1)
	addw	s3, t0, t1
	li	t5, 0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s2, s2, 1
	mv	t0, s3	# phi-copy
	blt	s2, s1, .main.bb5	# rotated-loop-branch
.main.bb6:
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
