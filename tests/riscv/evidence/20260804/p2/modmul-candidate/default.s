
  .section .rodata
  .globl MODULUS
  .align 2
  .type  MODULUS, @object
  .size  MODULUS, 4
MODULUS:
  .word 1009

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 777
	li	t1, 321
	mul	t0, t0, t1	# modmul.wide.mul
	li	t2, 1009
	rem	t0, t0, t2	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
.main.bb1:
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
