
  .section .rodata
  .globl len
  .align 2
  .type  len, @object
  .size  len, 4
len:
  .word 20

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -576
	sd	ra, 536(sp)
	sd	s1, 544(sp)
	sd	s2, 552(sp)
	sd	s3, 560(sp)
	addi	t5, sp, 8
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	sd	zero, 64(t5)	# memset-zero
	sd	zero, 72(t5)	# memset-zero
	li	t0, 1
	addi	t1, sp, 8
	sw	t0, 0(t1)
	li	t0, 2
	sw	t0, 4(t1)
	li	t0, 3
	sw	t0, 8(t1)
	li	t0, 4
	sw	t0, 12(t1)
	li	t0, 5
	sw	t0, 16(t1)
	li	t0, 6
	sw	t0, 20(t1)
	li	t0, 7
	sw	t0, 24(t1)
	li	t0, 8
	sw	t0, 28(t1)
	li	t0, 9
	sw	t0, 32(t1)
	li	t0, 0
	sw	t0, 36(t1)
	li	t0, 1
	sw	t0, 40(t1)
	li	t0, 2
	sw	t0, 44(t1)
	li	t0, 3
	sw	t0, 48(t1)
	li	t0, 4
	sw	t0, 52(t1)
	li	t0, 5
	sw	t0, 56(t1)
	li	t0, 6
	sw	t0, 60(t1)
	li	t0, 7
	sw	t0, 64(t1)
	li	t0, 8
	sw	t0, 68(t1)
	li	t0, 9
	sw	t0, 72(t1)
	li	t0, 0
	sw	t0, 76(t1)
	addi	t5, sp, 88
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	sd	zero, 64(t5)	# memset-zero
	sd	zero, 72(t5)	# memset-zero
	li	t0, 2
	addi	t2, sp, 88
	sw	t0, 0(t2)
	li	t0, 3
	sw	t0, 4(t2)
	li	t0, 4
	sw	t0, 8(t2)
	li	t0, 2
	sw	t0, 12(t2)
	li	t0, 5
	sw	t0, 16(t2)
	li	t0, 7
	sw	t0, 20(t2)
	li	t0, 9
	sw	t0, 24(t2)
	sw	t0, 28(t2)
	li	t0, 0
	sw	t0, 32(t2)
	li	t0, 1
	sw	t0, 36(t2)
	li	t0, 9
	sw	t0, 40(t2)
	li	t0, 8
	sw	t0, 44(t2)
	li	t0, 7
	sw	t0, 48(t2)
	li	t0, 6
	sw	t0, 52(t2)
	li	t0, 4
	sw	t0, 56(t2)
	li	t0, 3
	sw	t0, 60(t2)
	li	t0, 2
	sw	t0, 64(t2)
	li	t0, 1
	sw	t0, 68(t2)
	li	t0, 2
	sw	t0, 72(t2)
	sw	t0, 76(t2)
	addi	t5, sp, 376
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	sd	zero, 64(t5)	# memset-zero
	sd	zero, 72(t5)	# memset-zero
	sd	zero, 80(t5)	# memset-zero
	sd	zero, 88(t5)	# memset-zero
	sd	zero, 96(t5)	# memset-zero
	sd	zero, 104(t5)	# memset-zero
	sd	zero, 112(t5)	# memset-zero
	sd	zero, 120(t5)	# memset-zero
	sd	zero, 128(t5)	# memset-zero
	sd	zero, 136(t5)	# memset-zero
	sd	zero, 144(t5)	# memset-zero
	sd	zero, 152(t5)	# memset-zero
	lw	t0, 0(t1)
	addi	t3, sp, 168
	sw	t0, 0(t3)
	lw	t0, 4(t1)
	sw	t0, 4(t3)
	lw	t0, 8(t1)
	sw	t0, 8(t3)
	lw	t0, 12(t1)
	sw	t0, 12(t3)
	lw	t0, 16(t1)
	sw	t0, 16(t3)
	lw	t0, 20(t1)
	sw	t0, 20(t3)
	lw	t0, 24(t1)
	sw	t0, 24(t3)
	lw	t0, 28(t1)
	sw	t0, 28(t3)
	lw	t0, 32(t1)
	sw	t0, 32(t3)
	lw	t0, 36(t1)
	sw	t0, 36(t3)
	lw	t0, 40(t1)
	sw	t0, 40(t3)
	lw	t0, 44(t1)
	sw	t0, 44(t3)
	lw	t0, 48(t1)
	sw	t0, 48(t3)
	lw	t0, 52(t1)
	sw	t0, 52(t3)
	lw	t0, 56(t1)
	sw	t0, 56(t3)
	lw	t0, 60(t1)
	sw	t0, 60(t3)
	lw	t0, 64(t1)
	sw	t0, 64(t3)
	lw	t0, 68(t1)
	sw	t0, 68(t3)
	lw	t0, 72(t1)
	sw	t0, 72(t3)
	li	t0, 0
	sw	t0, 76(t3)
	lw	t0, 0(t2)
	addi	t1, sp, 272
	sw	t0, 0(t1)
	lw	t0, 4(t2)
	sw	t0, 4(t1)
	lw	t0, 8(t2)
	sw	t0, 8(t1)
	lw	t0, 12(t2)
	sw	t0, 12(t1)
	lw	t0, 16(t2)
	sw	t0, 16(t1)
	lw	t0, 20(t2)
	sw	t0, 20(t1)
	lw	t0, 24(t2)
	sw	t0, 24(t1)
	lw	t0, 28(t2)
	sw	t0, 28(t1)
	lw	t0, 32(t2)
	sw	t0, 32(t1)
	lw	t0, 36(t2)
	sw	t0, 36(t1)
	lw	t0, 40(t2)
	sw	t0, 40(t1)
	lw	t0, 44(t2)
	sw	t0, 44(t1)
	lw	t0, 48(t2)
	sw	t0, 48(t1)
	lw	t0, 52(t2)
	sw	t0, 52(t1)
	lw	t0, 56(t2)
	sw	t0, 56(t1)
	lw	t0, 60(t2)
	sw	t0, 60(t1)
	lw	t0, 64(t2)
	sw	t0, 64(t1)
	lw	t0, 68(t2)
	sw	t0, 68(t1)
	lw	t0, 72(t2)
	sw	t0, 72(t1)
	li	t0, 2
	sw	t0, 76(t1)
	addi	t0, sp, 376
	li	t1, 0
	li	t2, 39
.main.bb1:
	blt	t2, t1, .main.bb3
.main.bb2:
	li	t3, 0
	addiw	t1, t1, 1
	sw	t3, 0(t0)
	addi	t0, t0, 4
	bge	t2, t1, .main.bb2	# rotated-loop-branch
.main.bb3:
	addi	t0, sp, 272
	addi	t1, sp, 168
	addi	t0, t0, 76
	addi	t1, t1, 76
	li	t2, 39
	li	t3, 19
.main.bb4:
	li	a2, -1
	bge	a2, t3, .main.bb6
.main.bb5:
	lw	a2, 0(t0)
	li	a3, 19
	li	a4, -1
	mv	s1, t1	# phi-copy
.main.bb7:
	bge	a4, a3, .main.bb9
.main.bb8:
	slliw	a5, t2, 2
	addi	a6, sp, 376
	add	s2, a6, a5
	lw	a6, 0(s1)
	lw	a5, 0(s2)
	mulw	a6, a2, a6
	addw	s3, a5, a6
	li	a5, 10
	blt	s3, a5, .main.bb11
.main.bb10:
	li	a6, 1717986919
	sw	s3, 0(s2)
	mul	a6, s3, a6	# const-div.fullmul
	lw	a5, -4(s2)
	srai	a6, a6, 32	# const-div.mulhi
	sraiw	a6, a6, 2	# const-div.shift
	srliw	a7, a6, 31	# const-div.sign
	addw	a6, a6, a7	# const-div.round
	addw	a5, a5, a6
	sw	a5, -4(s2)
.main.bb12:
	addiw	a3, a3, -1
	addiw	t2, t2, -1
	addi	s1, s1, -4
	j	.main.bb7
.main.bb11:
	sw	s3, 0(s2)
	j	.main.bb12
.main.bb9:
	addiw	t2, t2, 20
	addiw	t3, t3, -1
	addiw	t2, t2, -1
	addi	t0, t0, -4
	j	.main.bb4
.main.bb6:
	addi	t0, sp, 376
	lw	t0, 0(t0)
	bne	t0, zero, .main.bb13
.main.bb14:
	li	t0, 1
	li	s1, 39
	mv	s2, t0	# phi-copy
.main.bb15:
	blt	s1, s2, .main.bb17
.main.bb16:
	slliw	t0, s2, 2
	addi	t1, sp, 376
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	bge	s1, s2, .main.bb16	# rotated-loop-branch
.main.bb17:
	li	a0, 0
	ld	s3, 560(sp)
	ld	s2, 552(sp)
	ld	s1, 544(sp)
	ld	ra, 536(sp)
	addi	sp, sp, 576
	ret
.main.bb13:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	j	.main.bb14
	.size main, .-main
