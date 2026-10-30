
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

  .bss
  .globl fusion_anchor_a
  .align 2
  .type  fusion_anchor_a, @object
  .size  fusion_anchor_a, 512
fusion_anchor_a:
  .zero 512

  .bss
  .globl fusion_anchor_b
  .align 2
  .type  fusion_anchor_b, @object
  .size  fusion_anchor_b, 512
fusion_anchor_b:
  .zero 512

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	li	t0, 0
	li	t1, 128
.main.bb7:
	addiw	s1, t0, 8
	bge	t0, t1, .main.bb3
.main.bb8:
	li	t2, 128
	bge	s1, t2, .main.bb9
.main.bb10:
	mv	t2, s1	# phi-copy
	j	.main.bb1
.main.bb11:
.main.bb1:
	bge	t0, t2, .main.bb7
.main.bb13:
	addiw	t3, t0, 3
	blt	t3, t2, .main.bb14
.main.bb2:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb14:
	addiw	t0, t0, 4
	j	.main.bb1
.main.bb12:
	j	.main.bb7
.main.bb4:
	li	t1, 64
	bge	t0, t1, .main.bb6
.main.bb15:
	addiw	s1, t0, 3
	li	t1, 64
	blt	s1, t1, .main.bb16
.main.bb5:
	la	t1, fusion_first_a
	addiw	s1, t0, 1
	sw	t0, 0(t1)
	slliw	s2, t0, 1
	la	t1, fusion_first_b
	addw	t0, s2, t0
	sw	s1, 0(t1)
	la	t1, fusion_second_a
	sw	s2, 0(t1)
	la	t1, fusion_second_b
	sw	t0, 0(t1)
	mv	t0, s1	# phi-copy
	j	.main.bb4
.main.bb16:
	la	t1, fusion_first_a
	addiw	s2, t0, 4
	sw	s1, 0(t1)
	la	t0, fusion_first_b
	sw	s2, 0(t0)
	slliw	s3, s1, 1
	la	t0, fusion_second_a
	la	t1, fusion_second_b
	sw	s3, 0(t0)
	addw	t0, s3, s1
	sw	t0, 0(t1)
	mv	t0, s2	# phi-copy
	j	.main.bb4
.main.bb9:
	li	t2, 128
	j	.main.bb1
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
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
.main.bb3:
	li	t0, 0
	j	.main.bb4
	.size main, .-main
