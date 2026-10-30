
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
	addi	sp, sp, -16
	sd	s1, 0(sp)
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
.main.bb9:
	li	t2, 128
	j	.main.bb1
.main.bb4:
	li	t1, 64
	blt	t0, t1, .main.bb5
.main.bb6:
	j	.main.bb4
.main.bb5:
	addiw	t0, t0, 1
	j	.main.bb4
.main.bb3:
	li	t0, 0
	j	.main.bb4
	.size main, .-main
