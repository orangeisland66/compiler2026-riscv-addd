
  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	call	getint
	mv	t0, a0
	mv	s1, t0	# phi-copy
.main.bb1:
	li	t0, 5
	bne	s1, t0, .main.bb3
.main.bb2:
	li	s1, 6
	j	.main.bb1
.main.bb3:
	li	a0, 0
	call	putint
	li	t5, 1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 4
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 5
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 6
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 7
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 8
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 9
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 11
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 12
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 13
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 14
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 15
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 16
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 17
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 18
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 19
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 20
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 21
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 22
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 23
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 24
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 25
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 26
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 27
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 28
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 29
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 25
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
