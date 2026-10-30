
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 2
	call	putint
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
