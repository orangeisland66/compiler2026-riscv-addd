
  .bss
  .globl arr1
  .align 2
  .type  arr1, @object
  .size  arr1, 57600
arr1:
  .zero 57600

  .bss
  .globl arr2
  .align 2
  .type  arr2, @object
  .size  arr2, 107520
arr2:
  .zero 107520

  .text

  .globl loop1
  .type  loop1, @function
loop1:
.loop1.bb0:
	addi	sp, sp, -96
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	sd	s7, 48(sp)
	sd	s8, 56(sp)
	sd	s9, 64(sp)
	sd	s10, 72(sp)
	sd	s11, 80(sp)
	mv	s5, a0
	mv	s6, a1
	li	t1, 0
.loop1.bb1:
	slt	t2, t1, s5
	slt	t3, t1, s6
	and	t2, t2, t3
	beq	t2, zero, .loop1.bb3
.loop1.bb2:
	li	t2, 5760
	li	t3, 0
	mulw	t2, t1, t2
.loop1.bb4:
	li	a2, 2
	bge	t3, a2, .loop1.bb6
.loop1.bb5:
	li	a2, 2880
	addw	s7, t1, t3
	mulw	a2, t3, a2
	li	a3, 5
	addw	s8, t2, a2
	li	a2, 0
.loop1.bb7:
	bge	a2, a3, .loop1.bb9
.loop1.bb8:
	slliw	a4, a2, 5
	slliw	a5, a2, 4
	addw	a4, a4, a5
	addw	s9, s7, a2
	addw	a4, s8, a4
	la	a6, arr1
	addw	a5, s9, s5
	add	s10, a6, a4
	addw	a5, a5, s6
	addiw	a4, s9, 1
	sw	a5, 0(s10)
	addw	a4, a4, s5
	addw	s11, a4, s6
	addiw	a5, s9, 2
	sw	s11, 4(s10)
	addw	a5, a5, s5
	sw	s11, 8(s10)
	addw	s11, a5, s6
	addiw	a5, s9, 3
	sw	s11, 12(s10)
	sw	s11, 16(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a6, s9, 6
	addiw	a5, s9, 4
	sw	s11, 20(s10)
	sw	s11, 24(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a5, s9, 5
	sw	s11, 28(s10)
	addw	s9, s4, a2
	sw	s11, 32(s10)
	addw	a7, s9, s5
	addw	a7, a7, s6
	sw	a7, 240(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a7, s9, 1
	sw	s11, 36(s10)
	sw	s11, 40(s10)
	addw	a7, a7, s5
	addw	s11, a7, s6
	sw	s11, 244(s10)
	addiw	a7, s9, 2
	sw	s11, 248(s10)
	addw	a7, a7, s5
	addw	s11, a7, s6
	sw	s11, 252(s10)
	addiw	a7, s9, 3
	sw	s11, 256(s10)
	addw	a7, a7, s5
	addw	s11, a7, s6
	sw	s11, 260(s10)
	addiw	a7, s9, 4
	sw	s11, 264(s10)
	addw	a7, a7, s5
	addw	s11, a7, s6
	sw	s11, 268(s10)
	addiw	a7, s9, 5
	sw	s11, 272(s10)
	addw	a7, a7, s5
	addw	s11, a7, s6
	sw	s11, 276(s10)
	addiw	a7, s9, 6
	sw	s11, 280(s10)
	addw	s9, s3, a2
	addw	a5, s9, s5
	addw	a5, a5, s6
	sw	a5, 480(s10)
	addiw	a5, s9, 1
	addw	a6, a6, s5
	addw	a5, a5, s5
	addw	s11, a5, s6
	addw	a6, a6, s6
	addiw	a5, s9, 2
	sw	s11, 484(s10)
	sw	s11, 488(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	sw	a6, 44(s10)
	addiw	a5, s9, 3
	sw	s11, 492(s10)
	sw	s11, 496(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a5, s9, 4
	sw	s11, 500(s10)
	sw	s11, 504(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a5, s9, 5
	sw	s11, 508(s10)
	sw	s11, 512(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a6, a2, 1
	addiw	a5, s9, 6
	sw	s11, 516(s10)
	sw	s11, 520(s10)
	addw	s9, s1, a2
	addw	a2, s9, s5
	addw	a2, a2, s6
	sw	a2, 720(s10)
	addiw	a2, s9, 1
	addw	a2, a2, s5
	addw	s11, a2, s6
	addw	a7, a7, s5
	sw	s11, 724(s10)
	addiw	a4, s9, 2
	sw	s11, 728(s10)
	addw	a4, a4, s5
	addw	s11, a4, s6
	addiw	a4, s9, 3
	sw	s11, 732(s10)
	sw	s11, 736(s10)
	addw	a4, a4, s5
	addw	s11, a4, s6
	addiw	a4, s9, 4
	sw	s11, 740(s10)
	sw	s11, 744(s10)
	addw	a4, a4, s5
	addw	s11, a4, s6
	addiw	a4, s9, 5
	sw	s11, 748(s10)
	sw	s11, 752(s10)
	addw	a4, a4, s5
	addw	s11, a4, s6
	addiw	a4, s9, 6
	sw	s11, 756(s10)
	addw	a5, a5, s5
	sw	s11, 760(s10)
	addw	a4, a4, s5
	addw	a7, a7, s6
	addw	a5, a5, s6
	sw	a7, 284(s10)
	addw	a4, a4, s6
	sw	a5, 524(s10)
	sw	a4, 764(s10)
	mv	a2, a6	# phi-copy
	blt	a2, a3, .loop1.bb8	# rotated-loop-branch
.loop1.bb9:
	addiw	s4, s7, 1
	addiw	s3, s7, 2
	addiw	s1, s7, 3
	addiw	a2, s8, 960
	li	a3, 0
	li	a4, 5
.loop1.bb10:
	bge	a3, a4, .loop1.bb12
.loop1.bb11:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	a5, a5, a6
	addw	s9, s4, a3
	addw	a5, a2, a5
	la	a7, arr1
	addw	a6, s9, s5
	add	s10, a7, a5
	addw	a6, a6, s6
	addiw	a5, s9, 1
	sw	a6, 0(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a6, s9, 2
	sw	s11, 4(s10)
	addw	a6, a6, s5
	sw	s11, 8(s10)
	addw	s11, a6, s6
	addiw	a6, s9, 3
	sw	s11, 12(s10)
	sw	s11, 16(s10)
	addw	a6, a6, s5
	addw	s11, a6, s6
	addiw	a7, s9, 6
	addiw	a6, s9, 4
	sw	s11, 20(s10)
	sw	s11, 24(s10)
	addw	a6, a6, s5
	addw	s11, a6, s6
	addiw	a6, s9, 5
	sw	s11, 28(s10)
	addw	s9, s3, a3
	sw	s11, 32(s10)
	addw	t4, s9, s5
	addw	t4, t4, s6
	sw	t4, 240(s10)
	addw	a6, a6, s5
	addw	s11, a6, s6
	addiw	t4, s9, 1
	sw	s11, 36(s10)
	sw	s11, 40(s10)
	addw	t4, t4, s5
	addw	s11, t4, s6
	sw	s11, 244(s10)
	addiw	t4, s9, 2
	sw	s11, 248(s10)
	addw	t4, t4, s5
	addw	s11, t4, s6
	sw	s11, 252(s10)
	addiw	t4, s9, 3
	sw	s11, 256(s10)
	addw	t4, t4, s5
	addw	s11, t4, s6
	sw	s11, 260(s10)
	addiw	t4, s9, 4
	sw	s11, 264(s10)
	addw	t4, t4, s5
	addw	s11, t4, s6
	sw	s11, 268(s10)
	addiw	t4, s9, 5
	sw	s11, 272(s10)
	addw	t4, t4, s5
	addw	s11, t4, s6
	sw	s11, 276(s10)
	addiw	t4, s9, 6
	sw	s11, 280(s10)
	addw	s9, s1, a3
	addw	a6, s9, s5
	addw	a6, a6, s6
	sw	a6, 480(s10)
	addiw	a6, s9, 1
	addw	a7, a7, s5
	addw	a6, a6, s5
	addw	s11, a6, s6
	addw	a7, a7, s6
	addiw	a6, s9, 2
	sw	s11, 484(s10)
	sw	s11, 488(s10)
	addw	a6, a6, s5
	addw	s11, a6, s6
	sw	a7, 44(s10)
	addiw	a6, s9, 3
	sw	s11, 492(s10)
	sw	s11, 496(s10)
	addw	a6, a6, s5
	addw	s11, a6, s6
	addiw	a6, s9, 4
	sw	s11, 500(s10)
	sw	s11, 504(s10)
	addw	a6, a6, s5
	addw	s11, a6, s6
	addiw	a6, s9, 5
	sw	s11, 508(s10)
	sw	s11, 512(s10)
	addw	a6, a6, s5
	addw	s11, a6, s6
	addiw	a7, a3, 1
	addiw	a6, s9, 6
	sw	s11, 516(s10)
	sw	s11, 520(s10)
	addw	s9, s2, a3
	addw	a3, s9, s5
	addw	a3, a3, s6
	sw	a3, 720(s10)
	addiw	a3, s9, 1
	addw	a3, a3, s5
	addw	s11, a3, s6
	addw	t4, t4, s5
	sw	s11, 724(s10)
	addiw	a5, s9, 2
	sw	s11, 728(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a5, s9, 3
	sw	s11, 732(s10)
	sw	s11, 736(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a5, s9, 4
	sw	s11, 740(s10)
	sw	s11, 744(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a5, s9, 5
	sw	s11, 748(s10)
	sw	s11, 752(s10)
	addw	a5, a5, s5
	addw	s11, a5, s6
	addiw	a5, s9, 6
	sw	s11, 756(s10)
	addw	a6, a6, s5
	sw	s11, 760(s10)
	addw	a5, a5, s5
	addw	t4, t4, s6
	addw	a6, a6, s6
	sw	t4, 284(s10)
	addw	a5, a5, s6
	sw	a6, 524(s10)
	sw	a5, 764(s10)
	mv	a3, a7	# phi-copy
	blt	a3, a4, .loop1.bb11	# rotated-loop-branch
.loop1.bb12:
	addiw	s2, s7, 4
	addiw	a2, s8, 1920
	li	a3, 0
	li	a4, 5
.loop1.bb13:
	bge	a3, a4, .loop1.bb15
.loop1.bb14:
	slliw	a5, a3, 5
	slliw	a6, a3, 4
	addw	a5, a5, a6
	addw	s8, s3, a3
	addw	a5, a2, a5
	la	a7, arr1
	addw	a6, s8, s5
	add	s9, a7, a5
	addw	a6, a6, s6
	addiw	a5, s8, 1
	sw	a6, 0(s9)
	addw	a5, a5, s5
	addw	s10, a5, s6
	addiw	a6, s8, 2
	sw	s10, 4(s9)
	addw	a6, a6, s5
	sw	s10, 8(s9)
	addw	s10, a6, s6
	addiw	a6, s8, 3
	sw	s10, 12(s9)
	sw	s10, 16(s9)
	addw	a6, a6, s5
	addw	s10, a6, s6
	addiw	a6, s8, 4
	sw	s10, 20(s9)
	sw	s10, 24(s9)
	addw	a6, a6, s5
	addw	s10, a6, s6
	addiw	a6, s8, 5
	sw	s10, 28(s9)
	sw	s10, 32(s9)
	addw	a6, a6, s5
	addw	s10, a6, s6
	addiw	a6, s8, 6
	sw	s10, 36(s9)
	addw	s8, s1, a3
	sw	s10, 40(s9)
	addw	a7, s8, s5
	addw	a7, a7, s6
	sw	a7, 240(s9)
	addiw	a7, s8, 1
	addw	a7, a7, s5
	addw	s10, a7, s6
	addiw	a7, s8, 2
	sw	s10, 244(s9)
	sw	s10, 248(s9)
	addw	a7, a7, s5
	addw	s10, a7, s6
	sw	s10, 252(s9)
	addiw	a7, s8, 3
	sw	s10, 256(s9)
	addw	a7, a7, s5
	addw	s10, a7, s6
	sw	s10, 260(s9)
	addiw	a7, s8, 4
	sw	s10, 264(s9)
	addw	a7, a7, s5
	addw	s10, a7, s6
	sw	s10, 268(s9)
	addiw	a7, s8, 5
	sw	s10, 272(s9)
	addw	a7, a7, s5
	addw	s10, a7, s6
	sw	s10, 276(s9)
	addiw	a7, s8, 6
	sw	s10, 280(s9)
	addw	s8, s2, a3
	addw	a5, s8, s5
	addw	a7, a7, s5
	addw	a5, a5, s6
	addw	a7, a7, s6
	sw	a5, 480(s9)
	addw	a6, a6, s5
	addiw	a5, s8, 1
	sw	a7, 284(s9)
	addw	a5, a5, s5
	addw	s10, a5, s6
	addw	a6, a6, s6
	sw	s10, 484(s9)
	addiw	a7, s8, 2
	sw	s10, 488(s9)
	addw	a7, a7, s5
	sw	a6, 44(s9)
	addw	s10, a7, s6
	addiw	a7, s8, 3
	sw	s10, 492(s9)
	sw	s10, 496(s9)
	addw	a7, a7, s5
	addw	s10, a7, s6
	addiw	a7, s8, 4
	sw	s10, 500(s9)
	sw	s10, 504(s9)
	addw	a7, a7, s5
	addw	s10, a7, s6
	addiw	a7, s8, 5
	sw	s10, 508(s9)
	sw	s10, 512(s9)
	addw	a7, a7, s5
	addw	s10, a7, s6
	addiw	a7, s8, 6
	sw	s10, 516(s9)
	addiw	a6, a3, 1
	sw	s10, 520(s9)
	addw	s8, t0, a3
	addw	a3, s8, s5
	addw	a3, a3, s6
	sw	a3, 720(s9)
	addiw	a3, s8, 1
	addw	a3, a3, s5
	addw	s10, a3, s6
	sw	s10, 724(s9)
	addiw	a5, s8, 2
	sw	s10, 728(s9)
	addw	a5, a5, s5
	addw	s10, a5, s6
	addiw	a5, s8, 3
	sw	s10, 732(s9)
	sw	s10, 736(s9)
	addw	a5, a5, s5
	addw	s10, a5, s6
	addiw	a5, s8, 4
	sw	s10, 740(s9)
	sw	s10, 744(s9)
	addw	a5, a5, s5
	addw	s10, a5, s6
	addiw	a5, s8, 5
	sw	s10, 748(s9)
	sw	s10, 752(s9)
	addw	a5, a5, s5
	addw	s10, a5, s6
	addiw	a5, s8, 6
	sw	s10, 756(s9)
	addw	a7, a7, s5
	sw	s10, 760(s9)
	addw	a5, a5, s5
	addw	a7, a7, s6
	addw	a5, a5, s6
	sw	a7, 524(s9)
	sw	a5, 764(s9)
	mv	a3, a6	# phi-copy
	blt	a3, a4, .loop1.bb14	# rotated-loop-branch
.loop1.bb15:
	addiw	t0, s7, 5
	addiw	t3, t3, 1
	j	.loop1.bb4
.loop1.bb6:
	addiw	t1, t1, 1
	j	.loop1.bb1
.loop1.bb3:
	ld	s11, 80(sp)
	ld	s10, 72(sp)
	ld	s9, 64(sp)
	ld	s8, 56(sp)
	ld	s7, 48(sp)
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 96
	ret
	.size loop1, .-loop1

  .globl loop2
  .type  loop2, @function
loop2:
.loop2.bb0:
	addi	sp, sp, -80
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	sd	s7, 48(sp)
	sd	s8, 56(sp)
	sd	s9, 64(sp)
	sd	s10, 72(sp)
	li	t0, 0
.loop2.bb1:
	li	t1, 10
	bge	t0, t1, .loop2.bb3
.loop2.bb2:
	li	t1, 10752
	li	t2, 0
	mulw	t1, t0, t1
.loop2.bb4:
	li	t3, 2
	bge	t2, t3, .loop2.bb6
.loop2.bb5:
	li	t3, 5376
	addw	s1, t0, t2
	mulw	t3, t2, t3
	addiw	s3, s1, 1
	addiw	s4, s1, 2
	addiw	s5, s1, 3
	addiw	s6, s1, 4
	addiw	s7, s1, 5
	addw	s2, t1, t3
	addiw	s8, s1, 6
	li	t3, 0
	li	a2, 8
.loop2.bb7:
	bge	t3, a2, .loop2.bb9
.loop2.bb8:
	slliw	a3, t3, 5
	slliw	a4, t3, 2
	subw	a3, a3, a4
	addiw	t3, t3, 1
	addw	a3, s2, a3
	la	a4, arr2
	add	s9, a4, a3
	sw	s1, 0(s9)
	sw	s3, 4(s9)
	sw	s4, 8(s9)
	sw	s5, 12(s9)
	sw	s6, 16(s9)
	sw	s7, 20(s9)
	sw	s8, 24(s9)
	sw	s1, 224(s9)
	sw	s3, 228(s9)
	sw	s4, 232(s9)
	sw	s5, 236(s9)
	sw	s6, 240(s9)
	sw	s7, 244(s9)
	sw	s8, 248(s9)
	sw	s1, 448(s9)
	sw	s3, 452(s9)
	sw	s4, 456(s9)
	sw	s5, 460(s9)
	sw	s6, 464(s9)
	sw	s7, 468(s9)
	sw	s8, 472(s9)
	sw	s1, 672(s9)
	sw	s3, 676(s9)
	sw	s4, 680(s9)
	sw	s5, 684(s9)
	sw	s6, 688(s9)
	sw	s7, 692(s9)
	sw	s8, 696(s9)
	blt	t3, a2, .loop2.bb8	# rotated-loop-branch
.loop2.bb9:
	addiw	t3, s2, 896
	addiw	s9, s1, 7
	li	a2, 0
	li	a3, 8
.loop2.bb10:
	bge	a2, a3, .loop2.bb12
.loop2.bb11:
	slliw	a4, a2, 5
	slliw	a5, a2, 2
	subw	a4, a4, a5
	addiw	a2, a2, 1
	addw	a4, t3, a4
	la	a5, arr2
	add	s10, a5, a4
	sw	s3, 0(s10)
	sw	s4, 4(s10)
	sw	s5, 8(s10)
	sw	s6, 12(s10)
	sw	s7, 16(s10)
	sw	s8, 20(s10)
	sw	s9, 24(s10)
	sw	s3, 224(s10)
	sw	s4, 228(s10)
	sw	s5, 232(s10)
	sw	s6, 236(s10)
	sw	s7, 240(s10)
	sw	s8, 244(s10)
	sw	s9, 248(s10)
	sw	s3, 448(s10)
	sw	s4, 452(s10)
	sw	s5, 456(s10)
	sw	s6, 460(s10)
	sw	s7, 464(s10)
	sw	s8, 468(s10)
	sw	s9, 472(s10)
	sw	s3, 672(s10)
	sw	s4, 676(s10)
	sw	s5, 680(s10)
	sw	s6, 684(s10)
	sw	s7, 688(s10)
	sw	s8, 692(s10)
	sw	s9, 696(s10)
	blt	a2, a3, .loop2.bb11	# rotated-loop-branch
.loop2.bb12:
	addiw	t3, s2, 1792
	li	a2, 0
	li	a3, 8
.loop2.bb13:
	bge	a2, a3, .loop2.bb15
.loop2.bb14:
	slliw	a4, a2, 5
	slliw	a5, a2, 2
	subw	a4, a4, a5
	addiw	a2, a2, 1
	addw	a4, t3, a4
	la	a5, arr2
	add	s10, a5, a4
	sw	s1, 0(s10)
	sw	s3, 4(s10)
	sw	s4, 8(s10)
	sw	s5, 12(s10)
	sw	s6, 16(s10)
	sw	s7, 20(s10)
	sw	s8, 24(s10)
	sw	s1, 224(s10)
	sw	s3, 228(s10)
	sw	s4, 232(s10)
	sw	s5, 236(s10)
	sw	s6, 240(s10)
	sw	s7, 244(s10)
	sw	s8, 248(s10)
	sw	s1, 448(s10)
	sw	s3, 452(s10)
	sw	s4, 456(s10)
	sw	s5, 460(s10)
	sw	s6, 464(s10)
	sw	s7, 468(s10)
	sw	s8, 472(s10)
	sw	s1, 672(s10)
	sw	s3, 676(s10)
	sw	s4, 680(s10)
	sw	s5, 684(s10)
	sw	s6, 688(s10)
	sw	s7, 692(s10)
	sw	s8, 696(s10)
	blt	a2, a3, .loop2.bb14	# rotated-loop-branch
.loop2.bb15:
	li	t3, 2688
	li	a2, 0
	addw	t3, s2, t3
	li	a3, 8
.loop2.bb16:
	bge	a2, a3, .loop2.bb18
.loop2.bb17:
	slliw	a4, a2, 5
	slliw	a5, a2, 2
	subw	a4, a4, a5
	addiw	a2, a2, 1
	addw	a4, t3, a4
	la	a5, arr2
	add	s10, a5, a4
	sw	s3, 0(s10)
	sw	s4, 4(s10)
	sw	s5, 8(s10)
	sw	s6, 12(s10)
	sw	s7, 16(s10)
	sw	s8, 20(s10)
	sw	s9, 24(s10)
	sw	s3, 224(s10)
	sw	s4, 228(s10)
	sw	s5, 232(s10)
	sw	s6, 236(s10)
	sw	s7, 240(s10)
	sw	s8, 244(s10)
	sw	s9, 248(s10)
	sw	s3, 448(s10)
	sw	s4, 452(s10)
	sw	s5, 456(s10)
	sw	s6, 460(s10)
	sw	s7, 464(s10)
	sw	s8, 468(s10)
	sw	s9, 472(s10)
	sw	s3, 672(s10)
	sw	s4, 676(s10)
	sw	s5, 680(s10)
	sw	s6, 684(s10)
	sw	s7, 688(s10)
	sw	s8, 692(s10)
	sw	s9, 696(s10)
	blt	a2, a3, .loop2.bb17	# rotated-loop-branch
.loop2.bb18:
	li	t3, 3584
	li	a2, 0
	addw	t3, s2, t3
	li	a3, 8
.loop2.bb19:
	bge	a2, a3, .loop2.bb21
.loop2.bb20:
	slliw	a4, a2, 5
	slliw	a5, a2, 2
	subw	a4, a4, a5
	addiw	a2, a2, 1
	addw	a4, t3, a4
	la	a5, arr2
	add	s10, a5, a4
	sw	s1, 0(s10)
	sw	s3, 4(s10)
	sw	s4, 8(s10)
	sw	s5, 12(s10)
	sw	s6, 16(s10)
	sw	s7, 20(s10)
	sw	s8, 24(s10)
	sw	s1, 224(s10)
	sw	s3, 228(s10)
	sw	s4, 232(s10)
	sw	s5, 236(s10)
	sw	s6, 240(s10)
	sw	s7, 244(s10)
	sw	s8, 248(s10)
	sw	s1, 448(s10)
	sw	s3, 452(s10)
	sw	s4, 456(s10)
	sw	s5, 460(s10)
	sw	s6, 464(s10)
	sw	s7, 468(s10)
	sw	s8, 472(s10)
	sw	s1, 672(s10)
	sw	s3, 676(s10)
	sw	s4, 680(s10)
	sw	s5, 684(s10)
	sw	s6, 688(s10)
	sw	s7, 692(s10)
	sw	s8, 696(s10)
	blt	a2, a3, .loop2.bb20	# rotated-loop-branch
.loop2.bb21:
	li	t3, 4480
	li	a2, 0
	addw	t3, s2, t3
	li	a3, 8
.loop2.bb22:
	bge	a2, a3, .loop2.bb24
.loop2.bb23:
	slliw	a4, a2, 5
	slliw	a5, a2, 2
	subw	a4, a4, a5
	addiw	a2, a2, 1
	addw	a4, t3, a4
	la	a5, arr2
	add	s1, a5, a4
	sw	s3, 0(s1)
	sw	s4, 4(s1)
	sw	s5, 8(s1)
	sw	s6, 12(s1)
	sw	s7, 16(s1)
	sw	s8, 20(s1)
	sw	s9, 24(s1)
	sw	s3, 224(s1)
	sw	s4, 228(s1)
	sw	s5, 232(s1)
	sw	s6, 236(s1)
	sw	s7, 240(s1)
	sw	s8, 244(s1)
	sw	s9, 248(s1)
	sw	s3, 448(s1)
	sw	s4, 452(s1)
	sw	s5, 456(s1)
	sw	s6, 460(s1)
	sw	s7, 464(s1)
	sw	s8, 468(s1)
	sw	s9, 472(s1)
	sw	s3, 672(s1)
	sw	s4, 676(s1)
	sw	s5, 680(s1)
	sw	s6, 684(s1)
	sw	s7, 688(s1)
	sw	s8, 692(s1)
	sw	s9, 696(s1)
	blt	a2, a3, .loop2.bb23	# rotated-loop-branch
.loop2.bb24:
	addiw	t2, t2, 1
	j	.loop2.bb4
.loop2.bb6:
	addiw	t0, t0, 1
	j	.loop2.bb1
.loop2.bb3:
	ld	s10, 72(sp)
	ld	s9, 64(sp)
	ld	s8, 56(sp)
	ld	s7, 48(sp)
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 80
	ret
	.size loop2, .-loop2

  .globl loop3
  .type  loop3, @function
loop3:
.loop3.bb0:
	addi	sp, sp, -160
	sd	s1, 136(sp)
	sd	s2, 144(sp)
	mv	t5, a0
	sd	t5, 16(sp)
	mv	t5, a1
	sd	t5, 40(sp)
	mv	t5, a2
	sd	t5, 64(sp)
	mv	t5, a3
	sd	t5, 88(sp)
	mv	t5, a4
	sd	t5, 128(sp)
	mv	a3, a5
	mv	a4, a6
	li	a5, 0
	li	a6, 0
	sd	a6, 112(sp)
.loop3.bb1:
	li	a7, 10
	ld	t5, 112(sp)
	bge	t5, a7, .loop3.bb36
.loop3.bb2:
	li	a7, 5760
	ld	t5, 112(sp)
	mulw	t5, t5, a7
	sd	t5, 0(sp)
	li	t4, 10752
	ld	t5, 112(sp)
	mulw	t5, t5, t4
	sd	t5, 8(sp)
	li	a0, 0
.loop3.bb4:
	li	a1, 100
	bge	a0, a1, .loop3.bb37
.loop3.bb5:
	li	a1, 2880
	mulw	a1, a0, a1
	ld	t5, 0(sp)
	addw	t5, t5, a1
	sd	t5, 24(sp)
	li	a7, 5376
	mulw	a7, a0, a7
	ld	t5, 8(sp)
	addw	t5, t5, a7
	sd	t5, 32(sp)
	li	t4, 0
.loop3.bb7:
	li	t0, 1000
	bge	t4, t0, .loop3.bb38
.loop3.bb8:
	slliw	t0, t4, 10
	ld	t5, 24(sp)
	addw	t0, t5, t0
	slliw	a1, t4, 6
	subw	t5, t0, a1
	sd	t5, 48(sp)
	slliw	a1, t4, 11
	ld	t5, 32(sp)
	addw	a1, t5, a1
	slliw	a7, t4, 8
	subw	t5, a1, a7
	sd	t5, 56(sp)
	li	a1, 0
.loop3.bb10:
	li	t1, 10000
	bge	a1, t1, .loop3.bb39
.loop3.bb11:
	slliw	t1, a1, 8
	ld	t5, 48(sp)
	addw	t1, t5, t1
	slliw	t0, a1, 4
	subw	t5, t1, t0
	sd	t5, 72(sp)
	slliw	t1, a1, 10
	ld	t5, 56(sp)
	addw	t1, t5, t1
	slliw	a7, a1, 7
	subw	t5, t1, a7
	sd	t5, 80(sp)
	li	a7, 0
.loop3.bb13:
	li	t2, 100000
	bge	a7, t2, .loop3.bb40
.loop3.bb14:
	slliw	s1, a7, 5
	ld	t5, 72(sp)
	addw	t2, t5, s1
	slliw	t0, a7, 4
	addw	t5, t2, t0
	sd	t5, 96(sp)
	slliw	t2, a7, 8
	ld	t5, 80(sp)
	addw	t2, t5, t2
	subw	t5, t2, s1
	sd	t5, 104(sp)
	li	t1, 0
.loop3.bb16:
	li	t3, 1000000
	bge	t1, t3, .loop3.bb41
.loop3.bb17:
	slliw	t3, t1, 3
	ld	t5, 96(sp)
	addw	t3, t5, t3
	la	t0, arr1
	add	t0, t0, t3
	slliw	t3, t1, 5
	ld	t5, 104(sp)
	addw	t3, t5, t3
	slliw	t2, t1, 2
	subw	t2, t3, t2
	la	t3, arr2
	add	t2, t3, t2
	li	t3, 0
	li	t5, 10000000
	sd	t5, 120(sp)
.loop3.bb19:
	ld	t5, 120(sp)
	bge	t3, t5, .loop3.bb42
.loop3.bb20:
	li	a6, -1603384363
	li	a2, 817
	mul	a6, a5, a6	# const-div.fullmul
	addiw	s2, t3, 1
	srai	a6, a6, 32	# const-div.mulhi
	addw	a6, a6, a5	# const-div.fixup
	sraiw	a6, a6, 9	# const-div.shift
	srliw	s1, a6, 31	# const-div.sign
	addw	a6, a6, s1	# const-div.round
	mulw	a2, a6, a2	# const-rem.mul
	subw	a2, a5, a2	# const-rem.sub
	lw	a5, 0(t0)
	addw	a2, a2, a5
	lw	a5, 0(t2)
	addw	s1, a2, a5
	bge	s2, a4, .loop3.bb21
.loop3.bb23:
	addi	t0, t0, 4
	addi	t2, t2, 4
	mv	a5, s1	# phi-copy
	mv	t3, s2	# phi-copy
	j	.loop3.bb19
.loop3.bb42:
	mv	s1, a5	# phi-copy
.loop3.bb21:
	addiw	s2, t1, 1
	bge	s2, a3, .loop3.bb18
.loop3.bb25:
	mv	t1, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb16
.loop3.bb22:
	j	.loop3.bb21
.loop3.bb24:
.loop3.bb18:
	addiw	s2, a7, 1
	ld	t5, 128(sp)
	bge	s2, t5, .loop3.bb15
.loop3.bb27:
	mv	a7, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb13
.loop3.bb41:
	mv	s1, a5	# phi-copy
	j	.loop3.bb18
.loop3.bb26:
.loop3.bb15:
	addiw	s2, a1, 1
	ld	t5, 88(sp)
	bge	s2, t5, .loop3.bb12
.loop3.bb29:
	mv	a1, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb10
.loop3.bb40:
	mv	s1, a5	# phi-copy
	j	.loop3.bb15
.loop3.bb28:
.loop3.bb12:
	addiw	s2, t4, 1
	ld	t5, 64(sp)
	bge	s2, t5, .loop3.bb9
.loop3.bb31:
	mv	t4, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb7
.loop3.bb39:
	mv	s1, a5	# phi-copy
	j	.loop3.bb12
.loop3.bb30:
.loop3.bb9:
	addiw	s2, a0, 1
	ld	t5, 40(sp)
	bge	s2, t5, .loop3.bb6
.loop3.bb33:
	mv	a0, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb4
.loop3.bb38:
	mv	s1, a5	# phi-copy
	j	.loop3.bb9
.loop3.bb32:
.loop3.bb6:
	ld	t5, 112(sp)
	addiw	s2, t5, 1
	ld	t5, 16(sp)
	bge	s2, t5, .loop3.bb3
.loop3.bb35:
	sd	s2, 112(sp)
	mv	a5, s1	# phi-copy
	j	.loop3.bb1
.loop3.bb37:
	mv	s1, a5	# phi-copy
	j	.loop3.bb6
.loop3.bb34:
.loop3.bb3:
	mv	a0, s1
	ld	s2, 144(sp)
	ld	s1, 136(sp)
	addi	sp, sp, 160
	ret
.loop3.bb36:
	mv	s1, a5	# phi-copy
	j	.loop3.bb3
	.size loop3, .-loop3

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -144
	sd	ra, 56(sp)
	sd	s1, 64(sp)
	sd	s2, 72(sp)
	sd	s3, 80(sp)
	sd	s4, 88(sp)
	sd	s5, 96(sp)
	sd	s6, 104(sp)
	sd	s7, 112(sp)
	sd	s8, 120(sp)
	sd	s9, 128(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	call	getint
	mv	s3, a0
	call	getint
	mv	s4, a0
	call	getint
	mv	s5, a0
	call	getint
	mv	s6, a0
	call	getint
	mv	s7, a0
	call	getint
	mv	s8, a0
	call	getint
	mv	s9, a0
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, s2
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	loop1
	call	loop2
	mv	t5, s3
	sd	t5, 0(sp)
	mv	t5, s4
	sd	t5, 8(sp)
	mv	t5, s5
	sd	t5, 16(sp)
	mv	t5, s6
	sd	t5, 24(sp)
	mv	t5, s7
	sd	t5, 32(sp)
	mv	t5, s8
	sd	t5, 40(sp)
	mv	t5, s9
	sd	t5, 48(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	ld	a5, 40(sp)
	ld	a6, 48(sp)
	call	loop3
	mv	t0, a0
	mv	a0, t0
	ld	s9, 128(sp)
	ld	s8, 120(sp)
	ld	s7, 112(sp)
	ld	s6, 104(sp)
	ld	s5, 96(sp)
	ld	s4, 88(sp)
	ld	s3, 80(sp)
	ld	s2, 72(sp)
	ld	s1, 64(sp)
	ld	ra, 56(sp)
	addi	sp, sp, 144
	ret
	.size main, .-main
