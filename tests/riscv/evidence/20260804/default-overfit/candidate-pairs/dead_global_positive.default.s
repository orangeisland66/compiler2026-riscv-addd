
  .bss
  .globl scratchpad
  .align 2
  .type  scratchpad, @object
  .size  scratchpad, 128
scratchpad:
  .zero 128

  .data
  .globl answer
  .align 2
  .type  answer, @object
  .size  answer, 4
answer:
  .word 7

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
	li	t1, 32
.main.bb1:
	bge	t0, t1, .main.bb3
.main.bb2:
	addiw	t0, t0, 1
	blt	t0, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 7
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
