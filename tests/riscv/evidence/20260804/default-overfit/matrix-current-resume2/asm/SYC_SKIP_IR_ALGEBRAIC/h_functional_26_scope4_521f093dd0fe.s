
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .zero 4

  .bss
  .globl sum
  .align 2
  .type  sum, @object
  .size  sum, 4
sum:
  .zero 4

  .bss
  .globl count
  .align 2
  .type  count, @object
  .size  count, 4
count:
  .zero 4

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
	la	t0, count
	lw	t0, 0(t0)
	addiw	s1, t0, 1
	addiw	t0, s1, 1
	addw	t1, s1, t0
	addiw	t2, t0, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t3
	addw	t1, t1, t2
	addw	t1, t1, t2
	addw	t1, t1, s1
	addiw	t2, t3, 1
	addw	t1, t1, s1
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t2
	addw	t1, t1, t0
	addiw	t2, t3, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t3
	addw	t1, t1, t2
	addw	t1, t1, t2
	addw	t1, t1, s1
	addiw	t2, t3, 1
	addw	t1, t1, s1
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t2
	addiw	t2, t3, 1
	addw	t1, t1, t0
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t3
	addw	t1, t1, t2
	addw	t1, t1, t2
	addw	t1, t1, s1
	addiw	t2, t3, 1
	addw	t1, t1, s1
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t2
	addiw	t2, t3, 1
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t3
	addw	t1, t1, t2
	addw	t1, t1, t2
	addw	t1, t1, s1
	addiw	t2, t3, 1
	addw	t1, t1, s1
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t2
	addiw	t2, t3, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t3
	addw	t1, t1, t2
	addw	t1, t1, t2
	addw	t1, t1, s1
	addiw	t2, t3, 1
	addw	t1, t1, s1
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t2
	addiw	t2, t3, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t3
	addw	t1, t1, t2
	addw	t1, t1, t2
	addw	t1, t1, s1
	addiw	t2, t3, 1
	addw	t1, t1, s1
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t2
	addw	t1, t1, t0
	addiw	t2, t3, 1
	addw	t1, t1, t2
	addiw	t3, t2, 1
	addw	t1, t1, t3
	addw	t1, t1, t2
	addw	t1, t1, t2
	addw	t1, t1, s1
	addiw	t2, t3, 1
	addw	t1, t1, s1
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	t2, t2, 1
	addw	t1, t1, t2
	addiw	s2, t2, 1
	addw	s3, t1, t2
	li	t2, 0
.main.bb1:
	li	t1, 3
	bge	t2, t1, .main.bb3
.main.bb2:
	addw	t1, s3, t0
	addiw	s2, s2, 1
	addw	t1, t1, s2
	addiw	s3, s2, 1
	addw	t1, t1, s3
	addiw	t3, s3, 1
	addw	t1, t1, s2
	addiw	t3, t3, 1
	addw	t1, t1, s2
	addw	t1, t1, s1
	addiw	s2, t3, 1
	addw	t1, t1, s1
	addiw	s3, s2, 1
	addw	t1, t1, t3
	addw	t1, t1, s2
	addw	s2, t1, s2
	li	t1, 1
	bne	t2, t1, .main.bb5
.main.bb4:
	addiw	t1, s3, 1
	li	t3, 2
	addw	t2, s2, t1
	addiw	s2, t1, 1
	addw	t1, t2, s2
	addiw	s3, s2, 1
	addw	t1, t1, s3
	addiw	t2, s3, 1
	addw	t1, t1, s2
	addiw	t2, t2, 1
	addw	t1, t1, s2
	addw	t1, t1, s1
	addiw	s2, t2, 1
	addw	t1, t1, s1
	addw	t1, t1, t2
	addw	t1, t1, s2
	addiw	t2, s2, 1
	addw	s3, t1, s2
	mv	s2, t2	# phi-copy
	mv	t2, t3	# phi-copy
	j	.main.bb1
.main.bb5:
	addw	t0, s2, t0
	addiw	t2, t2, 1
	addiw	s2, s3, 1
	addw	t0, t0, s2
	addiw	s3, s2, 1
	addw	t0, t0, s3
	addiw	t1, s3, 1
	addw	t0, t0, s2
	addiw	t1, t1, 1
	addw	t0, t0, s2
	addw	t0, t0, s1
	addiw	s2, t1, 1
	addw	t0, t0, s1
	addw	t0, t0, t1
	addw	t0, t0, s2
	addiw	t1, s2, 1
	addw	s3, t0, s2
	addiw	t0, t1, 1
	mv	s2, t0	# phi-copy
	j	.main.bb1
.main.bb3:
	mv	a0, s3
	call	putint
	la	t0, count
	sw	s2, 0(t0)
	la	t0, a
	sw	s1, 0(t0)
	la	t0, sum
	sw	s3, 0(t0)
	li	a0, 0
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
