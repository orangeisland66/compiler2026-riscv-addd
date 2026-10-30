
  .bss
  .globl fusion_first_a
  .align 2
  .type  fusion_first_a, @object
  .size  fusion_first_a, 16
fusion_first_a:
  .zero 16

  .bss
  .globl fusion_first_b
  .align 2
  .type  fusion_first_b, @object
  .size  fusion_first_b, 16
fusion_first_b:
  .zero 16

  .bss
  .globl fusion_second_a
  .align 2
  .type  fusion_second_a, @object
  .size  fusion_second_a, 16
fusion_second_a:
  .zero 16

  .bss
  .globl fusion_second_b
  .align 2
  .type  fusion_second_b, @object
  .size  fusion_second_b, 16
fusion_second_b:
  .zero 16

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	li	t1, 64
	li	t0, 0
.main.bb1:
	bge	t0, t1, .main.bb3
.main.bb2:
	la	t2, fusion_first_a
	addiw	s1, t0, 1
	sw	t0, 0(t2)
	la	t0, fusion_first_b
	sw	s1, 0(t0)
	mv	t0, s1	# phi-copy
	blt	t0, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t1, 64
	li	t0, 0
.main.bb4:
	bge	t0, t1, .main.bb6
.main.bb5:
	slliw	s1, t0, 1
	la	t2, fusion_second_a
	sw	s1, 0(t2)
	la	t3, fusion_second_b
	addw	t2, s1, t0
	sw	t2, 0(t3)
	addiw	t0, t0, 1
	blt	t0, t1, .main.bb5	# rotated-loop-branch
.main.bb6:
	la	t0, fusion_first_a
	lw	t0, 0(t0)
	la	t1, fusion_first_b
	lw	t1, 0(t1)
	addw	t0, t0, t1
	la	t1, fusion_second_a
	lw	t1, 0(t1)
	addw	t0, t0, t1
	la	t1, fusion_second_b
	lw	t1, 0(t1)
	addw	a0, t0, t1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
