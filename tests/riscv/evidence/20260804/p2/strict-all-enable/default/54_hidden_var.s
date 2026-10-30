
  .data
  .globl b
  .align 2
  .type  b, @object
  .size  b, 4
b:
  .word 5

  .data
  .globl c
  .align 2
  .type  c, @object
  .size  c, 16
c:
  .word 6
  .word 7
  .word 8
  .word 9

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -176
	sd	ra, 152(sp)
	sd	s1, 160(sp)
	sd	s2, 168(sp)
	li	a0, 3
	call	putint
	li	t5, 3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 1
	la	s1, c
	sw	t0, 8(s1)
	addi	t5, sp, 8
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	sd	zero, 64(t5)	# memset-zero
	sd	zero, 72(t5)	# memset-zero
	sd	zero, 80(t5)	# memset-zero
	sd	zero, 88(t5)	# memset-zero
	sd	zero, 96(t5)	# memset-zero
	sd	zero, 104(t5)	# memset-zero
	sd	zero, 112(t5)	# memset-zero
	sd	zero, 120(t5)	# memset-zero
	sd	zero, 128(t5)	# memset-zero
	sw	zero, 136(t5)	# memset-zero
	li	t0, 2
	addi	s2, sp, 8
	sw	t0, 40(s2)
	li	t0, 1
	sw	t0, 44(s2)
	li	t0, 8
	sw	t0, 48(s2)
	lw	t5, 40(s2)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	lw	t5, 44(s2)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 8
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 5
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	lw	t5, 0(s1)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	lw	t5, 4(s1)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	lw	t5, 12(s1)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s2, 168(sp)
	ld	s1, 160(sp)
	ld	ra, 152(sp)
	addi	sp, sp, 176
	ret
	.size main, .-main
