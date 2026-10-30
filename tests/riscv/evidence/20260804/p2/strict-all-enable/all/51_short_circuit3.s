
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
  .globl d
  .align 2
  .type  d, @object
  .size  d, 4
d:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
.main.bb1:
	bne	zero, zero, .main.bb3
.main.bb3:
	li	a0, 0
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
.main.bb4:
	bne	zero, zero, .main.bb6
.main.bb6:
	li	a0, 0
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 3
	la	t1, d
	sw	t0, 0(t1)
	li	t5, 3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 65
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 68
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 70
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 67
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 73
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 74
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 75
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 3
	la	t1, b
	sw	t0, 0(t1)
	li	t0, 0
	la	t1, a
	sw	t0, 0(t1)
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.main.bb2:
	j	.main.bb3
.main.bb5:
	j	.main.bb6
	.size main, .-main
