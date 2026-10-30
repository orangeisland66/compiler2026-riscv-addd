
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
	li	t0, 1
	la	t1, scratchpad
	sw	t0, 0(t1)
	li	t0, 4
	sw	t0, 4(t1)
	li	t0, 7
	sw	t0, 8(t1)
	li	t0, 10
	sw	t0, 12(t1)
	li	t0, 13
	sw	t0, 16(t1)
	li	t0, 16
	sw	t0, 20(t1)
	li	t0, 19
	sw	t0, 24(t1)
	li	t0, 22
	sw	t0, 28(t1)
	li	t0, 25
	sw	t0, 32(t1)
	li	t0, 28
	sw	t0, 36(t1)
	li	t0, 31
	sw	t0, 40(t1)
	li	t0, 34
	sw	t0, 44(t1)
	li	t0, 37
	sw	t0, 48(t1)
	li	t0, 40
	sw	t0, 52(t1)
	li	t0, 43
	sw	t0, 56(t1)
	li	t0, 46
	sw	t0, 60(t1)
	li	t0, 49
	sw	t0, 64(t1)
	li	t0, 52
	sw	t0, 68(t1)
	li	t0, 55
	sw	t0, 72(t1)
	li	t0, 58
	sw	t0, 76(t1)
	li	t0, 61
	sw	t0, 80(t1)
	li	t0, 64
	sw	t0, 84(t1)
	li	t0, 67
	sw	t0, 88(t1)
	li	t0, 70
	sw	t0, 92(t1)
	li	t0, 73
	sw	t0, 96(t1)
	li	t0, 76
	sw	t0, 100(t1)
	li	t0, 79
	sw	t0, 104(t1)
	li	t0, 82
	sw	t0, 108(t1)
	li	t0, 85
	sw	t0, 112(t1)
	li	t0, 88
	sw	t0, 116(t1)
	li	t0, 91
	sw	t0, 120(t1)
	li	t0, 94
	sw	t0, 124(t1)
	lw	a0, 0(t1)
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
