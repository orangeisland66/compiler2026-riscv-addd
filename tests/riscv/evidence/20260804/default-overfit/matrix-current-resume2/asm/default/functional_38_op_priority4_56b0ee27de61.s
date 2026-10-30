
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .zero 4

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 4
b:
  .zero 4

  .bss
  .globl c
  .align 2
  .type  c, @object
  .size  c, 4
c:
  .zero 4

  .bss
  .globl d
  .align 2
  .type  d, @object
  .size  d, 4
d:
  .zero 4

  .bss
  .globl e
  .align 2
  .type  e, @object
  .size  e, 4
e:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	sd	s3, 24(sp)
	sd	s4, 32(sp)
	call	getint
	mv	s1, a0
	la	t0, a
	sw	s1, 0(t0)
	call	getint
	mv	s2, a0
	la	t0, b
	sw	s2, 0(t0)
	call	getint
	mv	s3, a0
	la	t0, c
	sw	s3, 0(t0)
	call	getint
	mv	s4, a0
	la	t0, d
	sw	s4, 0(t0)
	call	getint
	mv	t0, a0
	la	t1, e
	sw	t0, 0(t1)
	mulw	t1, s2, s3
	subw	t1, s1, t1
	divw	t2, s1, s3
	subw	t2, s4, t2
	subw	t1, t1, t2
	sltu	t1, zero, t1
	mulw	t2, s1, s2
	divw	t2, t2, s3
	addw	t0, t0, s4
	subw	t2, t2, t0
	sltiu	t2, t2, 1
	or	t1, t1, t2
	addw	t2, s1, s2
	addw	t2, t2, s3
	subw	t0, t2, t0
	sltiu	t0, t0, 1
	or	t0, t1, t0
.main.bb1:
	mv	a0, t0
	ld	s4, 32(sp)
	ld	s3, 24(sp)
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
