
  .bss
  .globl x
  .align 2
  .type  x, @object
  .size  x, 62500000
x:
  .zero 62500000

  .bss
  .globl y
  .align 2
  .type  y, @object
  .size  y, 62500000
y:
  .zero 62500000

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -288
	sd	ra, 192(sp)
	sd	s1, 200(sp)
	sd	s2, 208(sp)
	sd	s3, 216(sp)
	sd	s4, 224(sp)
	sd	s5, 232(sp)
	sd	s6, 240(sp)
	sd	s7, 248(sp)
	sd	s8, 256(sp)
	sd	s9, 264(sp)
	sd	s10, 272(sp)
	sd	s11, 280(sp)
	call	getint
	mv	t5, a0
	sd	t5, 72(sp)
	call	getint
	mv	s2, a0
	li	t5, 13
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 0
.main.bb1:
	ld	t5, 72(sp)
	bge	t0, t5, .main.bb3
.main.bb2:
	li	t1, 250000
	li	t2, 0
	mulw	t1, t0, t1
.main.bb4:
	ld	t5, 72(sp)
	bge	t2, t5, .main.bb6
.main.bb5:
	li	t3, 1000
	la	s3, x	# loop.addr.hoisted
	mulw	t3, t2, t3
	addw	s1, t1, t3
	li	t3, 0
.main.bb23:
	addiw	s4, t3, 8
	ld	t5, 72(sp)
	bge	t3, t5, .main.bb9
.main.bb24:
	ld	t5, 72(sp)
	bge	s4, t5, .main.bb25
.main.bb26:
	j	.main.bb7
.main.bb27:
.main.bb7:
	bge	t3, s4, .main.bb23
.main.bb29:
	addiw	a2, t3, 3
	blt	a2, s4, .main.bb30
.main.bb8:
	slliw	a2, t3, 2
	li	a3, 1
	addw	a2, s1, a2
	addiw	t3, t3, 1
	add	a2, s3, a2
	sw	a3, 0(a2)
	j	.main.bb7
.main.bb30:
	slliw	a2, t3, 2
	li	a3, 1
	addw	a2, s1, a2
	addiw	t3, t3, 4
	add	s5, s3, a2
	li	a2, 1
	sw	a3, 4(s5)
	sw	a2, 0(s5)
	sw	a3, 8(s5)
	sw	a2, 12(s5)
	j	.main.bb7
.main.bb28:
	j	.main.bb23
.main.bb25:
	ld	s4, 72(sp)
	j	.main.bb7
.main.bb16:
	ld	t5, 64(sp)
	ld	t6, 56(sp)
	bge	t5, t6, .main.bb18
.main.bb31:
	ld	t5, 64(sp)
	addiw	a6, t5, 15
	ld	t5, 56(sp)
	blt	a6, t5, .main.bb32
.main.bb17:
	lw	a6, 0(s11)
	lw	a7, 0(a3)
	addw	a6, a6, a7
	lw	a7, 0(s10)
	addw	a6, a6, a7
	lw	a7, 0(a2)
	addw	a6, a6, a7
	lw	a7, 0(t3)
	addw	a6, a6, a7
	ld	t5, 64(sp)
	addiw	a5, t5, 1
	lw	a7, 0(a4)
	addw	a6, a6, a7
	lw	a7, 0(t2)
	addw	a6, a6, a7
	divw	a6, a6, s2
	sw	a6, 0(s9)
	addi	s9, s9, 4
	addi	s10, s10, 4
	addi	s11, s11, 4
	addi	t2, t2, 4
	addi	t3, t3, 4
	addi	a2, a2, 4
	addi	a3, a3, 4
	addi	a4, a4, 4
	sd	a5, 64(sp)
	j	.main.bb16
.main.bb32:
	lw	a6, 0(s11)
	lw	a7, 0(a3)
	addw	a6, a6, a7
	lw	a7, 0(s10)
	addw	a6, a6, a7
	lw	a7, 0(a2)
	addw	a6, a6, a7
	lw	a7, 0(t3)
	addw	a6, a6, a7
	lw	a7, 0(a4)
	addw	a6, a6, a7
	lw	a7, 0(t2)
	addw	a6, a6, a7
	divw	s8, a6, s2
	sw	s8, 0(s9)
	addi	a6, s9, 4
	addi	a7, s10, 4
	addi	t4, s11, 4
	addi	a0, t2, 4
	addi	a1, a2, 4
	addi	s7, a3, 4
	addi	t0, a4, 4
	lw	s6, 4(s11)
	lw	s5, 4(a3)
	addw	s4, s6, s5
	lw	t1, 4(s10)
	addw	t1, s4, t1
	lw	s3, 4(a2)
	addw	t1, t1, s3
	addw	t1, t1, s8
	lw	a5, 4(a4)
	addw	t1, t1, a5
	lw	a5, 4(t2)
	addw	t1, t1, a5
	divw	s3, t1, s2
	sw	s3, 4(s9)
	addi	t1, a6, 4
	addi	a5, a7, 4
	addi	a6, t4, 4
	addi	a7, a0, 4
	addi	t4, a1, 4
	addi	a0, s7, 4
	addi	t5, t0, 4
	lw	a1, 8(s11)
	lw	s1, 8(a3)
	addw	a1, a1, s1
	lw	t0, 8(s10)
	addw	t0, a1, t0
	lw	a1, 8(a2)
	addw	t0, t0, a1
	addw	t0, t0, s3
	lw	a1, 8(a4)
	addw	t0, t0, a1
	lw	a1, 8(t2)
	addw	t0, t0, a1
	divw	s1, t0, s2
	sw	s1, 8(s9)
	addi	t0, t1, 4
	addi	t1, a5, 4
	addi	a5, a6, 4
	addi	a6, a7, 4
	addi	a7, t4, 4
	addi	t4, a0, 4
	addi	t5, t5, 4
	lw	a1, 12(s11)
	lw	a0, 12(a3)
	addw	a0, a1, a0
	lw	a1, 12(s10)
	addw	a0, a0, a1
	lw	a1, 12(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 12(a4)
	addw	a0, a0, a1
	lw	a1, 12(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 12(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 16(s11)
	lw	a0, 16(a3)
	addw	a0, a1, a0
	lw	a1, 16(s10)
	addw	a0, a0, a1
	lw	a1, 16(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 16(a4)
	addw	a0, a0, a1
	lw	a1, 16(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 16(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 20(s11)
	lw	a0, 20(a3)
	addw	a0, a1, a0
	lw	a1, 20(s10)
	addw	a0, a0, a1
	lw	a1, 20(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 20(a4)
	addw	a0, a0, a1
	lw	a1, 20(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 20(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 24(s11)
	lw	a0, 24(a3)
	addw	a0, a1, a0
	lw	a1, 24(s10)
	addw	a0, a0, a1
	lw	a1, 24(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 24(a4)
	addw	a0, a0, a1
	lw	a1, 24(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 24(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 28(s11)
	lw	a0, 28(a3)
	addw	a0, a1, a0
	lw	a1, 28(s10)
	addw	a0, a0, a1
	lw	a1, 28(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 28(a4)
	addw	a0, a0, a1
	lw	a1, 28(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 28(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 32(s11)
	lw	a0, 32(a3)
	addw	a0, a1, a0
	lw	a1, 32(s10)
	addw	a0, a0, a1
	lw	a1, 32(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 32(a4)
	addw	a0, a0, a1
	lw	a1, 32(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 32(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 36(s11)
	lw	a0, 36(a3)
	addw	a0, a1, a0
	lw	a1, 36(s10)
	addw	a0, a0, a1
	lw	a1, 36(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 36(a4)
	addw	a0, a0, a1
	lw	a1, 36(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 36(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 40(s11)
	lw	a0, 40(a3)
	addw	a0, a1, a0
	lw	a1, 40(s10)
	addw	a0, a0, a1
	lw	a1, 40(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 40(a4)
	addw	a0, a0, a1
	lw	a1, 40(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 40(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 44(s11)
	lw	a0, 44(a3)
	addw	a0, a1, a0
	lw	a1, 44(s10)
	addw	a0, a0, a1
	lw	a1, 44(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 44(a4)
	addw	a0, a0, a1
	lw	a1, 44(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 44(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 48(s11)
	lw	a0, 48(a3)
	addw	a0, a1, a0
	lw	a1, 48(s10)
	addw	a0, a0, a1
	lw	a1, 48(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 48(a4)
	addw	a0, a0, a1
	lw	a1, 48(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 48(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 52(s11)
	lw	a0, 52(a3)
	addw	a0, a1, a0
	lw	a1, 52(s10)
	addw	a0, a0, a1
	lw	a1, 52(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 52(a4)
	addw	a0, a0, a1
	lw	a1, 52(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 52(s9)
	addi	a6, a6, 4
	addi	t5, t5, 4
	lw	a1, 56(s11)
	lw	a0, 56(a3)
	addw	a0, a1, a0
	lw	a1, 56(s10)
	addw	a0, a0, a1
	lw	a1, 56(a2)
	addw	a0, a0, a1
	addw	a0, a0, s1
	lw	a1, 56(a4)
	addw	a0, a0, a1
	lw	a1, 56(t2)
	addw	a0, a0, a1
	divw	s1, a0, s2
	sw	s1, 56(s9)
	addi	a5, a5, 48
	addi	a6, a6, 4
	addi	a7, a7, 48
	addi	t4, t4, 48
	addi	a0, t5, 4
	lw	a1, 60(s11)
	lw	a3, 60(a3)
	addw	a3, a1, a3
	lw	a1, 60(s10)
	addw	a3, a3, a1
	lw	a2, 60(a2)
	addw	a2, a3, a2
	addw	a2, a2, s1
	ld	t5, 64(sp)
	addiw	a3, t5, 16
	lw	a4, 60(a4)
	addw	a2, a2, a4
	lw	t2, 60(t2)
	addw	t2, a2, t2
	divw	t2, t2, s2
	sw	t2, 60(s9)
	addi	t0, t0, 52
	addi	t1, t1, 52
	addi	s11, a5, 4
	addi	t2, a6, 4
	addi	t3, t3, 64
	addi	a2, a7, 4
	addi	a5, t4, 4
	addi	a6, a0, 4
	mv	a4, a6	# phi-copy
	mv	s10, t1	# phi-copy
	mv	s9, t0	# phi-copy
	sd	a3, 64(sp)
	mv	a3, a5	# phi-copy
	j	.main.bb16
.main.bb18:
	ld	t5, 48(sp)
	addiw	t0, t5, 1
	sd	t0, 48(sp)
.main.bb13:
	ld	t5, 48(sp)
	ld	t6, 56(sp)
	bge	t5, t6, .main.bb15
.main.bb14:
	li	t2, 1000
	ld	t5, 48(sp)
	mulw	s8, t5, t2
	li	t2, 3
	bne	s2, t2, .main.bb22
.main.bb21:
	ld	t5, 40(sp)
	addw	t0, t5, s8
	addiw	t0, t0, 4
	la	t5, x	# remat
	add	s1, t5, t0
	addi	s3, s1, -1000
	ld	t5, 32(sp)
	addw	t0, t5, s8
	addiw	t0, t0, 4
	la	t5, x	# remat
	add	s4, t5, t0
	addi	t0, s4, -1004
	addi	s5, s3, 996
	addi	s6, s5, 1004
	ld	t5, 24(sp)
	addw	t1, t5, s8
	addiw	t1, t1, 4
	la	t5, x	# remat
	add	t1, t5, t1
	addi	t2, s6, -996
	li	t3, 1
	sd	t3, 184(sp)
.main.bb19:
	ld	t5, 184(sp)
	ld	t6, 56(sp)
	bge	t5, t6, .main.bb18
.main.bb33:
	ld	t5, 184(sp)
	addiw	a2, t5, 15
	ld	t5, 56(sp)
	blt	a2, t5, .main.bb34
.main.bb20:
	lw	a2, 0(s4)
	lw	a3, 0(t1)
	addw	a2, a2, a3
	lw	a3, 0(s3)
	addw	a2, a2, a3
	lw	a3, 0(s6)
	addw	a2, a2, a3
	lw	a3, 0(s5)
	addw	a2, a2, a3
	ld	t5, 184(sp)
	addiw	t3, t5, 1
	lw	a3, 0(t2)
	addw	a2, a2, a3
	lw	a3, 0(t0)
	addw	a2, a2, a3
	li	a3, 1431655766
	mul	a2, a2, a3	# const-div.fullmul
	srai	a2, a2, 32	# const-div.mulhi
	srliw	a3, a2, 31	# const-div.sign
	addw	a2, a2, a3	# const-div.round
	sw	a2, 0(s1)
	addi	a2, s1, 4
	addi	s3, s3, 4
	addi	s4, s4, 4
	addi	t0, t0, 4
	addi	s5, s5, 4
	addi	a6, s6, 4
	addi	t1, t1, 4
	addi	t2, t2, 4
	mv	s6, a6	# phi-copy
	mv	s1, a2	# phi-copy
	sd	t3, 184(sp)
	j	.main.bb19
.main.bb34:
	lw	a2, 0(s4)
	lw	a3, 0(t1)
	addw	a2, a2, a3
	lw	a3, 0(s3)
	addw	a2, a2, a3
	lw	a3, 0(s6)
	addw	a2, a2, a3
	lw	a3, 0(s5)
	addw	a2, a2, a3
	lw	a3, 0(t2)
	addw	a2, a2, a3
	lw	a3, 0(t0)
	addw	a2, a2, a3
	li	a3, 1431655766
	mul	a2, a2, a3	# const-div.fullmul
	srai	a2, a2, 32	# const-div.mulhi
	srliw	a3, a2, 31	# const-div.sign
	addw	s7, a2, a3	# const-div.round
	sw	s7, 0(s1)
	addi	a2, s1, 4
	addi	a3, s3, 4
	addi	a4, s4, 4
	addi	a5, t0, 4
	addi	a6, s5, 4
	addi	a7, s6, 4
	addi	t4, t1, 4
	addi	a0, t2, 4
	lw	a1, 4(s4)
	lw	t3, 4(t1)
	addw	t3, a1, t3
	lw	a1, 4(s3)
	addw	t3, t3, a1
	lw	a1, 4(s6)
	addw	t3, t3, a1
	addw	t3, t3, s7
	lw	a1, 4(t2)
	addw	t3, t3, a1
	lw	a1, 4(t0)
	addw	t3, t3, a1
	li	a1, 1431655766
	mul	t3, t3, a1	# const-div.fullmul
	srai	t3, t3, 32	# const-div.mulhi
	srliw	a1, t3, 31	# const-div.sign
	addw	s5, t3, a1	# const-div.round
	sw	s5, 4(s1)
	addi	t3, a2, 4
	addi	a2, a3, 4
	addi	a3, a4, 4
	addi	a4, a5, 4
	addi	a5, a6, 4
	addi	a6, a7, 4
	addi	a7, t4, 4
	addi	t4, a0, 4
	lw	a0, 8(s4)
	lw	a1, 8(t1)
	addw	a0, a0, a1
	lw	a1, 8(s3)
	addw	a0, a0, a1
	lw	a1, 8(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 8(t2)
	addw	a0, a0, a1
	lw	a1, 8(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 8(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 12(s4)
	lw	a1, 12(t1)
	addw	a0, a0, a1
	lw	a1, 12(s3)
	addw	a0, a0, a1
	lw	a1, 12(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 12(t2)
	addw	a0, a0, a1
	lw	a1, 12(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 12(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 16(s4)
	lw	a1, 16(t1)
	addw	a0, a0, a1
	lw	a1, 16(s3)
	addw	a0, a0, a1
	lw	a1, 16(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 16(t2)
	addw	a0, a0, a1
	lw	a1, 16(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 16(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 20(s4)
	lw	a1, 20(t1)
	addw	a0, a0, a1
	lw	a1, 20(s3)
	addw	a0, a0, a1
	lw	a1, 20(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 20(t2)
	addw	a0, a0, a1
	lw	a1, 20(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 20(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 24(s4)
	lw	a1, 24(t1)
	addw	a0, a0, a1
	lw	a1, 24(s3)
	addw	a0, a0, a1
	lw	a1, 24(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 24(t2)
	addw	a0, a0, a1
	lw	a1, 24(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 24(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 28(s4)
	lw	a1, 28(t1)
	addw	a0, a0, a1
	lw	a1, 28(s3)
	addw	a0, a0, a1
	lw	a1, 28(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 28(t2)
	addw	a0, a0, a1
	lw	a1, 28(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 28(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 32(s4)
	lw	a1, 32(t1)
	addw	a0, a0, a1
	lw	a1, 32(s3)
	addw	a0, a0, a1
	lw	a1, 32(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 32(t2)
	addw	a0, a0, a1
	lw	a1, 32(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 32(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 36(s4)
	lw	a1, 36(t1)
	addw	a0, a0, a1
	lw	a1, 36(s3)
	addw	a0, a0, a1
	lw	a1, 36(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 36(t2)
	addw	a0, a0, a1
	lw	a1, 36(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 36(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 40(s4)
	lw	a1, 40(t1)
	addw	a0, a0, a1
	lw	a1, 40(s3)
	addw	a0, a0, a1
	lw	a1, 40(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 40(t2)
	addw	a0, a0, a1
	lw	a1, 40(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 40(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 44(s4)
	lw	a1, 44(t1)
	addw	a0, a0, a1
	lw	a1, 44(s3)
	addw	a0, a0, a1
	lw	a1, 44(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 44(t2)
	addw	a0, a0, a1
	lw	a1, 44(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 44(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 48(s4)
	lw	a1, 48(t1)
	addw	a0, a0, a1
	lw	a1, 48(s3)
	addw	a0, a0, a1
	lw	a1, 48(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 48(t2)
	addw	a0, a0, a1
	lw	a1, 48(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 48(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 52(s4)
	lw	a1, 52(t1)
	addw	a0, a0, a1
	lw	a1, 52(s3)
	addw	a0, a0, a1
	lw	a1, 52(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 52(t2)
	addw	a0, a0, a1
	lw	a1, 52(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 52(s1)
	addi	a4, a4, 4
	addi	t4, t4, 4
	lw	a0, 56(s4)
	lw	a1, 56(t1)
	addw	a0, a0, a1
	lw	a1, 56(s3)
	addw	a0, a0, a1
	lw	a1, 56(s6)
	addw	a0, a0, a1
	addw	a0, a0, s5
	lw	a1, 56(t2)
	addw	a0, a0, a1
	lw	a1, 56(t0)
	addw	a0, a0, a1
	li	a1, 1431655766
	mul	a0, a0, a1	# const-div.fullmul
	srai	a0, a0, 32	# const-div.mulhi
	srliw	a1, a0, 31	# const-div.sign
	addw	s5, a0, a1	# const-div.round
	sw	s5, 56(s1)
	addi	t3, t3, 52
	addi	a2, a2, 52
	addi	a3, a3, 52
	addi	a4, a4, 4
	addi	a5, a5, 52
	addi	a6, a6, 52
	addi	a7, a7, 52
	addi	t4, t4, 4
	lw	a0, 60(s4)
	lw	t1, 60(t1)
	addw	t1, a0, t1
	lw	a0, 60(s3)
	addw	t1, t1, a0
	lw	a0, 60(s6)
	addw	t1, t1, a0
	addw	t1, t1, s5
	ld	t5, 184(sp)
	addiw	a0, t5, 16
	lw	t2, 60(t2)
	addw	t1, t1, t2
	lw	t0, 60(t0)
	addw	t0, t1, t0
	li	t1, 1431655766
	mul	t0, t0, t1	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	sw	t0, 60(s1)
	addi	t0, t3, 4
	addi	t1, a2, 4
	addi	s4, a3, 4
	addi	t3, a4, 4
	addi	s5, a5, 4
	addi	s6, a6, 4
	addi	a4, a7, 4
	addi	a5, t4, 4
	mv	t2, a5	# phi-copy
	mv	s3, t1	# phi-copy
	mv	t1, a4	# phi-copy
	mv	s1, t0	# phi-copy
	mv	t0, t3	# phi-copy
	sd	a0, 184(sp)
	j	.main.bb19
.main.bb9:
	addiw	t2, t2, 1
	j	.main.bb4
.main.bb22:
	ld	t5, 40(sp)
	addw	t2, t5, s8
	addiw	t2, t2, 4
	la	t5, x	# remat
	add	s9, t5, t2
	addi	s10, s9, -1000
	ld	t5, 32(sp)
	addw	t2, t5, s8
	addiw	t2, t2, 4
	la	t5, x	# remat
	add	s11, t5, t2
	addi	t2, s11, -1004
	addi	t3, s10, 996
	addi	a2, t3, 1004
	ld	t5, 24(sp)
	addw	a3, t5, s8
	addiw	a3, a3, 4
	la	t5, x	# remat
	add	a3, t5, a3
	addi	a4, a2, -996
	li	a5, 1
	sd	a5, 64(sp)
	j	.main.bb16
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb15:
	ld	s4, 48(sp)
	ld	s5, 16(sp)
.main.bb10:
	ld	t5, 56(sp)
	bge	s5, t5, .main.bb12
.main.bb11:
	addiw	t5, s5, 1
	sd	t5, 16(sp)
	li	t1, 250000
	mulw	t5, s5, t1
	sd	t5, 40(sp)
	li	t1, -250000
	addw	t5, t5, t1
	sd	t5, 32(sp)
	li	t1, 250000
	ld	t5, 40(sp)
	addw	t5, t5, t1
	sd	t5, 24(sp)
	li	t1, 1
	sd	t1, 48(sp)
	j	.main.bb13
.main.bb3:
	ld	t5, 72(sp)
	addiw	t5, t5, -1
	sd	t5, 56(sp)
	li	s5, 1
	li	t1, 1
	mv	s4, t1	# phi-copy
	j	.main.bb10
.main.bb12:
	li	a0, 53
	call	_sysy_stoptime
	ld	t5, 72(sp)
	sd	t5, 0(sp)
	la	t5, x
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	ld	t5, 72(sp)
	srliw	t0, t5, 31	# const-div.pow2.bias-fold
	ld	t5, 72(sp)
	addw	t0, t5, t0
	sraiw	t0, t0, 1
	li	t1, 250000
	mulw	t1, t0, t1
	li	t2, 1000
	mulw	t0, t0, t2
	addw	t0, t1, t0
	la	s2, x
	add	t0, s2, t0
	ld	t5, 72(sp)
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	addiw	t0, s5, -1
	li	t1, 250000
	mulw	t0, t0, t1
	addiw	t1, s4, -1
	li	t2, 1000
	mulw	t1, t1, t2
	addw	t0, t0, t1
	add	t0, s2, t0
	ld	t5, 72(sp)
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	li	a0, 0
	ld	s11, 280(sp)
	ld	s10, 272(sp)
	ld	s9, 264(sp)
	ld	s8, 256(sp)
	ld	s7, 248(sp)
	ld	s6, 240(sp)
	ld	s5, 232(sp)
	ld	s4, 224(sp)
	ld	s3, 216(sp)
	ld	s2, 208(sp)
	ld	s1, 200(sp)
	ld	ra, 192(sp)
	addi	sp, sp, 288
	ret
	.size main, .-main
