
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
	addi	t0, sp, 8
	li	t1, 1
	addi	t2, sp, 8
	sw	t1, 0(t2)
	li	t1, 2
	sw	t1, 4(t2)
	li	t1, 3
	sw	t1, 8(t2)
	li	t1, 4
	sw	t1, 12(t2)
	li	t1, 5
	sw	t1, 16(t2)
	li	t1, 6
	sw	t1, 20(t2)
	li	t1, 7
	sw	t1, 24(t2)
	li	t1, 8
	sw	t1, 28(t2)
	li	t1, 9
	sw	t1, 32(t2)
	li	t1, 0
	sw	t1, 36(t2)
	li	t1, 1
	sw	t1, 40(t2)
	li	t1, 2
	sw	t1, 44(t2)
	li	t1, 3
	sw	t1, 48(t2)
	li	t1, 4
	sw	t1, 52(t2)
	li	t1, 5
	sw	t1, 56(t2)
	li	t1, 6
	sw	t1, 60(t2)
	li	t1, 7
	sw	t1, 64(t2)
	li	t1, 8
	sw	t1, 68(t2)
	li	t1, 9
	sw	t1, 72(t2)
	li	t1, 0
	sw	t1, 76(t2)
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
	addi	t1, sp, 88
	li	t2, 2
	addi	t3, sp, 88
	sw	t2, 0(t3)
	li	t2, 3
	sw	t2, 4(t3)
	li	t2, 4
	sw	t2, 8(t3)
	li	t2, 2
	sw	t2, 12(t3)
	li	t2, 5
	sw	t2, 16(t3)
	li	t2, 7
	sw	t2, 20(t3)
	li	t2, 9
	sw	t2, 24(t3)
	sw	t2, 28(t3)
	li	t2, 0
	sw	t2, 32(t3)
	li	t2, 1
	sw	t2, 36(t3)
	li	t2, 9
	sw	t2, 40(t3)
	li	t2, 8
	sw	t2, 44(t3)
	li	t2, 7
	sw	t2, 48(t3)
	li	t2, 6
	sw	t2, 52(t3)
	li	t2, 4
	sw	t2, 56(t3)
	li	t2, 3
	sw	t2, 60(t3)
	li	t2, 2
	sw	t2, 64(t3)
	li	t2, 1
	sw	t2, 68(t3)
	li	t2, 2
	sw	t2, 72(t3)
	sw	t2, 76(t3)
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
	addi	t2, sp, 168
	li	t3, 0
	li	a2, 20
.main.bb1:
	bge	t3, a2, .main.bb3
.main.bb2:
	lw	a3, 0(t0)
	addiw	t3, t3, 1
	sw	a3, 0(t2)
	addi	t0, t0, 4
	addi	t2, t2, 4
	blt	t3, a2, .main.bb2	# rotated-loop-branch
.main.bb3:
	addi	t0, sp, 272
	li	t2, 0
	li	t3, 20
.main.bb4:
	bge	t2, t3, .main.bb6
.main.bb5:
	lw	a2, 0(t1)
	addiw	t2, t2, 1
	sw	a2, 0(t0)
	addi	t1, t1, 4
	addi	t0, t0, 4
	blt	t2, t3, .main.bb5	# rotated-loop-branch
.main.bb6:
	addi	t0, sp, 376
	li	t1, 0
	li	t2, 39
.main.bb7:
	blt	t2, t1, .main.bb9
.main.bb8:
	li	t3, 0
	addiw	t1, t1, 1
	sw	t3, 0(t0)
	addi	t0, t0, 4
	bge	t2, t1, .main.bb8	# rotated-loop-branch
.main.bb9:
	addi	t0, sp, 272
	addi	t1, sp, 168
	addi	t0, t0, 76
	addi	t1, t1, 76
	li	t2, 39
	li	t3, 19
.main.bb10:
	li	a2, -1
	bge	a2, t3, .main.bb12
.main.bb11:
	lw	a2, 0(t0)
	li	a3, 19
	li	a4, -1
	mv	s1, t1	# phi-copy
.main.bb13:
	bge	a4, a3, .main.bb15
.main.bb14:
	slliw	a5, t2, 2
	addi	a6, sp, 376
	add	s2, a6, a5
	lw	a6, 0(s1)
	lw	a5, 0(s2)
	mulw	a6, a2, a6
	addw	s3, a5, a6
	li	a5, 10
	blt	s3, a5, .main.bb17
.main.bb16:
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
.main.bb18:
	addiw	a3, a3, -1
	addiw	t2, t2, -1
	addi	s1, s1, -4
	j	.main.bb13
.main.bb17:
	sw	s3, 0(s2)
	j	.main.bb18
.main.bb15:
	addiw	t2, t2, 20
	addiw	t3, t3, -1
	addiw	t2, t2, -1
	addi	t0, t0, -4
	j	.main.bb10
.main.bb12:
	addi	t0, sp, 376
	lw	t0, 0(t0)
	bne	t0, zero, .main.bb19
.main.bb20:
	li	s2, 1
	li	s1, 39
.main.bb21:
	blt	s1, s2, .main.bb23
.main.bb22:
	slliw	t0, s2, 2
	addi	t1, sp, 376
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	bge	s1, s2, .main.bb22	# rotated-loop-branch
.main.bb23:
	li	a0, 0
	ld	s3, 560(sp)
	ld	s2, 552(sp)
	ld	s1, 544(sp)
	ld	ra, 536(sp)
	addi	sp, sp, 576
	ret
.main.bb19:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	j	.main.bb20
	.size main, .-main
