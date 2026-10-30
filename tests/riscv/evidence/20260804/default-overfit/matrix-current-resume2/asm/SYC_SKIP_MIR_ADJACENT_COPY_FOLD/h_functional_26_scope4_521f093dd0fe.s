
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
	addiw	t1, t0, 2
	addw	t2, s1, t1
	addiw	t3, t0, 3
	addw	t2, t2, t3
	addiw	a2, t0, 4
	addw	t2, t2, a2
	addw	t2, t2, t3
	addiw	a2, t0, 10
	addw	t2, t2, t3
	addw	t2, t2, s1
	addiw	t3, t0, 6
	addw	t2, t2, s1
	addw	t2, t2, t3
	addiw	t3, t0, 7
	addw	t2, t2, t3
	addw	t2, t2, t3
	addw	t2, t2, t1
	addiw	t3, t0, 9
	addw	t2, t2, t3
	addw	t2, t2, a2
	addw	t2, t2, t3
	addiw	a2, t0, 17
	addw	t2, t2, t3
	addw	t2, t2, s1
	addiw	t3, t0, 12
	addw	t2, t2, s1
	addw	t2, t2, t3
	addiw	t3, t0, 13
	addw	t2, t2, t3
	addw	t2, t2, t3
	addw	t2, t2, t1
	addiw	t3, t0, 16
	addw	t2, t2, t3
	addw	t2, t2, a2
	addw	t2, t2, t3
	addiw	a2, t0, 25
	addw	t2, t2, t3
	addw	t2, t2, s1
	addiw	t3, t0, 19
	addw	t2, t2, s1
	addw	t2, t2, t3
	addiw	t3, t0, 20
	addw	t2, t2, t3
	addw	t2, t2, t3
	addiw	t3, t0, 23
	addw	t2, t2, t3
	addiw	t3, t0, 24
	addw	t2, t2, t3
	addw	t2, t2, a2
	addw	t2, t2, t3
	addiw	a2, t0, 32
	addw	t2, t2, t3
	addw	t2, t2, s1
	addiw	t3, t0, 27
	addw	t2, t2, s1
	addw	t2, t2, t3
	addiw	t3, t0, 28
	addw	t2, t2, t3
	addw	t2, t2, t3
	addiw	t3, t0, 30
	addw	t2, t2, t3
	addiw	t3, t0, 31
	addw	t2, t2, t3
	addw	t2, t2, a2
	addw	t2, t2, t3
	addiw	a2, t0, 39
	addw	t2, t2, t3
	addw	t2, t2, s1
	addiw	t3, t0, 34
	addw	t2, t2, s1
	addw	t2, t2, t3
	addiw	t3, t0, 35
	addw	t2, t2, t3
	addw	t2, t2, t3
	addiw	t3, t0, 37
	addw	t2, t2, t3
	addiw	t3, t0, 38
	addw	t2, t2, t3
	addw	t2, t2, a2
	addw	t2, t2, t3
	addiw	a2, t0, 45
	addw	t2, t2, t3
	addw	t2, t2, s1
	addiw	t3, t0, 41
	addw	t2, t2, s1
	addw	t2, t2, t3
	addiw	t3, t0, 42
	addw	t2, t2, t3
	addw	t2, t2, t3
	addw	t2, t2, t1
	addiw	t3, t0, 44
	addw	t2, t2, t3
	addw	t2, t2, a2
	addw	t2, t2, t3
	addw	t2, t2, t3
	addw	t2, t2, s1
	addiw	t3, t0, 47
	addw	t2, t2, s1
	addw	t2, t2, t3
	addiw	t3, t0, 48
	addw	t2, t2, t3
	addiw	s2, t0, 49
	addw	s3, t2, t3
	li	t3, 0
.main.bb1:
	li	t0, 3
	bge	t3, t0, .main.bb3
.main.bb2:
	addw	t0, s3, t1
	addiw	t2, s2, 2
	addiw	s3, s2, 1
	addw	t0, t0, s3
	addw	t0, t0, t2
	addw	t0, t0, s3
	addiw	t2, s2, 4
	addw	t0, t0, s3
	addw	t0, t0, s1
	addw	t0, t0, s1
	addw	t0, t0, t2
	addiw	t2, s2, 5
	addw	t0, t0, t2
	addw	s3, t0, t2
	li	t0, 1
	bne	t3, t0, .main.bb5
.main.bb4:
	addiw	t0, s2, 7
	addiw	t2, s2, 9
	addw	t0, s3, t0
	li	t3, 2
	addiw	s3, s2, 8
	addw	t0, t0, s3
	addw	t0, t0, t2
	addw	t0, t0, s3
	addiw	t2, s2, 11
	addw	t0, t0, s3
	addw	t0, t0, s1
	addiw	s3, s2, 12
	addw	t0, t0, s1
	addw	t0, t0, t2
	addw	t0, t0, s3
	addiw	s2, s2, 13
	addw	t0, t0, s3
	mv	s3, t0	# phi-copy
	j	.main.bb1
.main.bb5:
	addw	t0, s3, t1
	addiw	t2, t3, 1
	addiw	s3, s2, 7
	addiw	t1, s2, 8
	addw	t0, t0, s3
	addw	t0, t0, t1
	addw	t0, t0, s3
	addiw	t1, s2, 10
	addw	t0, t0, s3
	addw	t0, t0, s1
	addw	t0, t0, s1
	addw	t0, t0, t1
	addiw	t1, s2, 11
	addw	t0, t0, t1
	addw	s3, t0, t1
	addiw	t1, s2, 13
	mv	t3, t2	# phi-copy
	mv	s2, t1	# phi-copy
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
