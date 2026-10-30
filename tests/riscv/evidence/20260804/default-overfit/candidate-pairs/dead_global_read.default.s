
  .bss
  .globl scratchpad
  .align 2
  .type  scratchpad, @object
  .size  scratchpad, 128
scratchpad:
  .zero 128

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	la	t0, scratchpad
	li	t1, 0
	li	t2, 32
.main.bb1:
	bge	t1, t2, .main.bb3
.main.bb2:
	slliw	t3, t1, 1
	addw	t3, t3, t1
	addiw	t3, t3, 1
	addiw	t1, t1, 1
	sw	t3, 0(t0)
	addi	t0, t0, 4
	blt	t1, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	la	t0, scratchpad
	lw	a0, 12(t0)
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
