
  .data
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .word 1

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 4
b:
  .zero 4

  .data
  .globl c
  .align 2
  .type  c, @object
  .size  c, 4
c:
  .word 1

  .data
  .globl d
  .align 2
  .type  d, @object
  .size  d, 4
d:
  .word 2

  .data
  .globl e
  .align 2
  .type  e, @object
  .size  e, 4
e:
  .word 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 1
	call	putint
	li	a0, 1
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
