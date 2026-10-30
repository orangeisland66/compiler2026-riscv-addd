
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
	addi	sp, sp, -752
	sd	ra, 656(sp)
	sd	s1, 664(sp)
	sd	s2, 672(sp)
	sd	s3, 680(sp)
	sd	s4, 688(sp)
	sd	s5, 696(sp)
	sd	s6, 704(sp)
	sd	s7, 712(sp)
	sd	s8, 720(sp)
	sd	s9, 728(sp)
	sd	s10, 736(sp)
	sd	s11, 744(sp)
	call	getint
	mv	t5, a0
	sd	t5, 72(sp)
	call	getint
	mv	t5, a0
	sd	t5, 416(sp)
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
	la	s2, x	# loop.addr.hoisted
	mulw	t3, t2, t3
	addw	s1, t1, t3
	li	t3, 0
.main.bb23:
	addiw	s3, t3, 8
	ld	t5, 72(sp)
	bge	t3, t5, .main.bb9
.main.bb24:
	ld	t5, 72(sp)
	bge	s3, t5, .main.bb25
.main.bb26:
	j	.main.bb7
.main.bb27:
.main.bb7:
	bge	t3, s3, .main.bb23
.main.bb29:
	addiw	a2, t3, 3
	blt	a2, s3, .main.bb30
.main.bb8:
	slliw	a2, t3, 2
	li	a3, 1
	addw	a2, s1, a2
	addiw	t3, t3, 1
	add	a2, s2, a2
	sw	a3, 0(a2)
	j	.main.bb7
.main.bb30:
	slliw	a2, t3, 2
	li	a3, 1
	addw	a2, s1, a2
	addiw	t3, t3, 4
	add	s4, s2, a2
	li	a2, 1
	sw	a3, 4(s4)
	sw	a2, 0(s4)
	sw	a3, 8(s4)
	sw	a2, 12(s4)
	j	.main.bb7
.main.bb28:
	j	.main.bb23
.main.bb25:
	ld	s3, 72(sp)
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
	ld	t5, 416(sp)
	divw	a6, a6, t5
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
	ld	t5, 416(sp)
	divw	s8, a6, t5
	sw	s8, 0(s9)
	addi	a6, s9, 4
	addi	a7, s10, 4
	addi	t4, s11, 4
	addi	s7, t2, 4
	addi	t0, a2, 4
	addi	s6, a3, 4
	addi	s5, a4, 4
	lw	s4, 4(s11)
	lw	t1, 4(a3)
	addw	t1, s4, t1
	lw	s3, 4(s10)
	addw	t1, t1, s3
	lw	a5, 4(a2)
	addw	t1, t1, a5
	addw	t1, t1, s8
	lw	a5, 4(a4)
	addw	t1, t1, a5
	lw	a5, 4(t2)
	addw	t1, t1, a5
	ld	t5, 416(sp)
	divw	s3, t1, t5
	sw	s3, 4(s9)
	addi	t1, a6, 4
	addi	a5, a7, 4
	addi	a6, t4, 4
	addi	a7, s7, 4
	addi	t5, t0, 4
	sd	t5, 96(sp)
	addi	t5, s6, 4
	sd	t5, 88(sp)
	addi	t5, s5, 4
	sd	t5, 80(sp)
	lw	s1, 8(s11)
	lw	t4, 8(a3)
	addw	t4, s1, t4
	lw	t0, 8(s10)
	addw	t0, t4, t0
	lw	t4, 8(a2)
	addw	t0, t0, t4
	addw	t0, t0, s3
	lw	t4, 8(a4)
	addw	t0, t0, t4
	lw	t4, 8(t2)
	addw	t0, t0, t4
	ld	t5, 416(sp)
	divw	s1, t0, t5
	sw	s1, 8(s9)
	addi	t0, t1, 4
	addi	t1, a5, 4
	addi	a5, a6, 4
	addi	a6, a7, 4
	ld	t5, 96(sp)
	addi	t5, t5, 4
	sd	t5, 120(sp)
	ld	t5, 88(sp)
	addi	t5, t5, 4
	sd	t5, 112(sp)
	ld	t5, 80(sp)
	addi	t5, t5, 4
	sd	t5, 104(sp)
	lw	t4, 12(s11)
	lw	a7, 12(a3)
	addw	a7, t4, a7
	lw	t4, 12(s10)
	addw	a7, a7, t4
	lw	t4, 12(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 12(a4)
	addw	a7, a7, t4
	lw	t4, 12(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 12(s9)
	addi	a6, a6, 4
	ld	t5, 120(sp)
	addi	t5, t5, 4
	sd	t5, 144(sp)
	ld	t5, 112(sp)
	addi	t5, t5, 4
	sd	t5, 136(sp)
	ld	t5, 104(sp)
	addi	t5, t5, 4
	sd	t5, 128(sp)
	lw	t4, 16(s11)
	lw	a7, 16(a3)
	addw	a7, t4, a7
	lw	t4, 16(s10)
	addw	a7, a7, t4
	lw	t4, 16(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 16(a4)
	addw	a7, a7, t4
	lw	t4, 16(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 16(s9)
	addi	a6, a6, 4
	ld	t5, 144(sp)
	addi	t5, t5, 4
	sd	t5, 168(sp)
	ld	t5, 136(sp)
	addi	t5, t5, 4
	sd	t5, 160(sp)
	ld	t5, 128(sp)
	addi	t5, t5, 4
	sd	t5, 152(sp)
	lw	t4, 20(s11)
	lw	a7, 20(a3)
	addw	a7, t4, a7
	lw	t4, 20(s10)
	addw	a7, a7, t4
	lw	t4, 20(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 20(a4)
	addw	a7, a7, t4
	lw	t4, 20(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 20(s9)
	addi	a6, a6, 4
	ld	t5, 168(sp)
	addi	t5, t5, 4
	sd	t5, 192(sp)
	ld	t5, 160(sp)
	addi	t5, t5, 4
	sd	t5, 184(sp)
	ld	t5, 152(sp)
	addi	t5, t5, 4
	sd	t5, 176(sp)
	lw	t4, 24(s11)
	lw	a7, 24(a3)
	addw	a7, t4, a7
	lw	t4, 24(s10)
	addw	a7, a7, t4
	lw	t4, 24(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 24(a4)
	addw	a7, a7, t4
	lw	t4, 24(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 24(s9)
	addi	a6, a6, 4
	ld	t5, 192(sp)
	addi	t5, t5, 4
	sd	t5, 216(sp)
	ld	t5, 184(sp)
	addi	t5, t5, 4
	sd	t5, 208(sp)
	ld	t5, 176(sp)
	addi	t5, t5, 4
	sd	t5, 200(sp)
	lw	t4, 28(s11)
	lw	a7, 28(a3)
	addw	a7, t4, a7
	lw	t4, 28(s10)
	addw	a7, a7, t4
	lw	t4, 28(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 28(a4)
	addw	a7, a7, t4
	lw	t4, 28(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 28(s9)
	addi	a6, a6, 4
	ld	t5, 216(sp)
	addi	t5, t5, 4
	sd	t5, 240(sp)
	ld	t5, 208(sp)
	addi	t5, t5, 4
	sd	t5, 232(sp)
	ld	t5, 200(sp)
	addi	t5, t5, 4
	sd	t5, 224(sp)
	lw	t4, 32(s11)
	lw	a7, 32(a3)
	addw	a7, t4, a7
	lw	t4, 32(s10)
	addw	a7, a7, t4
	lw	t4, 32(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 32(a4)
	addw	a7, a7, t4
	lw	t4, 32(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 32(s9)
	addi	a6, a6, 4
	ld	t5, 240(sp)
	addi	t5, t5, 4
	sd	t5, 264(sp)
	ld	t5, 232(sp)
	addi	t5, t5, 4
	sd	t5, 256(sp)
	ld	t5, 224(sp)
	addi	t5, t5, 4
	sd	t5, 248(sp)
	lw	t4, 36(s11)
	lw	a7, 36(a3)
	addw	a7, t4, a7
	lw	t4, 36(s10)
	addw	a7, a7, t4
	lw	t4, 36(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 36(a4)
	addw	a7, a7, t4
	lw	t4, 36(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 36(s9)
	addi	a6, a6, 4
	ld	t5, 264(sp)
	addi	t5, t5, 4
	sd	t5, 288(sp)
	ld	t5, 256(sp)
	addi	t5, t5, 4
	sd	t5, 280(sp)
	ld	t5, 248(sp)
	addi	t5, t5, 4
	sd	t5, 272(sp)
	lw	t4, 40(s11)
	lw	a7, 40(a3)
	addw	a7, t4, a7
	lw	t4, 40(s10)
	addw	a7, a7, t4
	lw	t4, 40(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 40(a4)
	addw	a7, a7, t4
	lw	t4, 40(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 40(s9)
	addi	a6, a6, 4
	ld	t5, 288(sp)
	addi	t5, t5, 4
	sd	t5, 312(sp)
	ld	t5, 280(sp)
	addi	t5, t5, 4
	sd	t5, 304(sp)
	ld	t5, 272(sp)
	addi	t5, t5, 4
	sd	t5, 296(sp)
	lw	t4, 44(s11)
	lw	a7, 44(a3)
	addw	a7, t4, a7
	lw	t4, 44(s10)
	addw	a7, a7, t4
	lw	t4, 44(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 44(a4)
	addw	a7, a7, t4
	lw	t4, 44(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 44(s9)
	addi	a6, a6, 4
	ld	t5, 312(sp)
	addi	t5, t5, 4
	sd	t5, 336(sp)
	ld	t5, 304(sp)
	addi	t5, t5, 4
	sd	t5, 328(sp)
	ld	t5, 296(sp)
	addi	t5, t5, 4
	sd	t5, 320(sp)
	lw	t4, 48(s11)
	lw	a7, 48(a3)
	addw	a7, t4, a7
	lw	t4, 48(s10)
	addw	a7, a7, t4
	lw	t4, 48(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 48(a4)
	addw	a7, a7, t4
	lw	t4, 48(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 48(s9)
	addi	a6, a6, 4
	ld	t5, 336(sp)
	addi	t5, t5, 4
	sd	t5, 360(sp)
	ld	t5, 328(sp)
	addi	t5, t5, 4
	sd	t5, 352(sp)
	ld	t5, 320(sp)
	addi	t5, t5, 4
	sd	t5, 344(sp)
	lw	t4, 52(s11)
	lw	a7, 52(a3)
	addw	a7, t4, a7
	lw	t4, 52(s10)
	addw	a7, a7, t4
	lw	t4, 52(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 52(a4)
	addw	a7, a7, t4
	lw	t4, 52(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 52(s9)
	addi	a6, a6, 4
	ld	t5, 360(sp)
	addi	t5, t5, 4
	sd	t5, 384(sp)
	ld	t5, 352(sp)
	addi	t5, t5, 4
	sd	t5, 376(sp)
	ld	t5, 344(sp)
	addi	t5, t5, 4
	sd	t5, 368(sp)
	lw	t4, 56(s11)
	lw	a7, 56(a3)
	addw	a7, t4, a7
	lw	t4, 56(s10)
	addw	a7, a7, t4
	lw	t4, 56(a2)
	addw	a7, a7, t4
	addw	a7, a7, s1
	lw	t4, 56(a4)
	addw	a7, a7, t4
	lw	t4, 56(t2)
	addw	a7, a7, t4
	ld	t5, 416(sp)
	divw	s1, a7, t5
	sw	s1, 56(s9)
	addi	a5, a5, 48
	addi	a6, a6, 4
	ld	t5, 384(sp)
	addi	a7, t5, 4
	ld	t5, 376(sp)
	addi	t5, t5, 4
	sd	t5, 400(sp)
	ld	t5, 368(sp)
	addi	t5, t5, 4
	sd	t5, 392(sp)
	lw	t4, 60(s11)
	lw	a3, 60(a3)
	addw	a3, t4, a3
	lw	t4, 60(s10)
	addw	a3, a3, t4
	lw	a2, 60(a2)
	addw	a2, a3, a2
	addw	a2, a2, s1
	ld	t5, 64(sp)
	addiw	a3, t5, 16
	lw	a4, 60(a4)
	addw	a2, a2, a4
	lw	t2, 60(t2)
	addw	t2, a2, t2
	ld	t5, 416(sp)
	divw	t2, t2, t5
	sw	t2, 60(s9)
	addi	t0, t0, 52
	addi	t1, t1, 52
	addi	s11, a5, 4
	addi	t2, a6, 4
	addi	t3, t3, 64
	addi	a2, a7, 4
	ld	t5, 400(sp)
	addi	a5, t5, 4
	ld	t5, 392(sp)
	addi	a6, t5, 4
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
	ld	t5, 416(sp)
	bne	t5, t2, .main.bb22
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
	sd	t3, 408(sp)
.main.bb19:
	ld	t5, 408(sp)
	ld	t6, 56(sp)
	bge	t5, t6, .main.bb18
.main.bb33:
	ld	t5, 408(sp)
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
	ld	t5, 408(sp)
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
	sd	t3, 408(sp)
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
	addi	t5, t1, 4
	sd	t5, 432(sp)
	addi	t5, t2, 4
	sd	t5, 424(sp)
	lw	s2, 4(s4)
	lw	t3, 4(t1)
	addw	t3, s2, t3
	lw	t4, 4(s3)
	addw	t3, t3, t4
	lw	t4, 4(s6)
	addw	t3, t3, t4
	addw	t3, t3, s7
	lw	t4, 4(t2)
	addw	t3, t3, t4
	lw	t4, 4(t0)
	addw	t3, t3, t4
	li	t4, 1431655766
	mul	t3, t3, t4	# const-div.fullmul
	srai	t3, t3, 32	# const-div.mulhi
	srliw	t4, t3, 31	# const-div.sign
	addw	s2, t3, t4	# const-div.round
	sw	s2, 4(s1)
	addi	t3, a2, 4
	addi	a2, a3, 4
	addi	a3, a4, 4
	addi	a4, a5, 4
	addi	a5, a6, 4
	addi	a6, a7, 4
	ld	t5, 432(sp)
	addi	t5, t5, 4
	sd	t5, 448(sp)
	ld	t5, 424(sp)
	addi	t5, t5, 4
	sd	t5, 440(sp)
	lw	t4, 8(s4)
	lw	a7, 8(t1)
	addw	a7, t4, a7
	lw	t4, 8(s3)
	addw	a7, a7, t4
	lw	t4, 8(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 8(t2)
	addw	a7, a7, t4
	lw	t4, 8(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 8(s1)
	addi	a4, a4, 4
	ld	t5, 448(sp)
	addi	t5, t5, 4
	sd	t5, 464(sp)
	ld	t5, 440(sp)
	addi	t5, t5, 4
	sd	t5, 456(sp)
	lw	t4, 12(s4)
	lw	a7, 12(t1)
	addw	a7, t4, a7
	lw	t4, 12(s3)
	addw	a7, a7, t4
	lw	t4, 12(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 12(t2)
	addw	a7, a7, t4
	lw	t4, 12(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 12(s1)
	addi	a4, a4, 4
	ld	t5, 464(sp)
	addi	t5, t5, 4
	sd	t5, 480(sp)
	ld	t5, 456(sp)
	addi	t5, t5, 4
	sd	t5, 472(sp)
	lw	t4, 16(s4)
	lw	a7, 16(t1)
	addw	a7, t4, a7
	lw	t4, 16(s3)
	addw	a7, a7, t4
	lw	t4, 16(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 16(t2)
	addw	a7, a7, t4
	lw	t4, 16(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 16(s1)
	addi	a4, a4, 4
	ld	t5, 480(sp)
	addi	t5, t5, 4
	sd	t5, 496(sp)
	ld	t5, 472(sp)
	addi	t5, t5, 4
	sd	t5, 488(sp)
	lw	t4, 20(s4)
	lw	a7, 20(t1)
	addw	a7, t4, a7
	lw	t4, 20(s3)
	addw	a7, a7, t4
	lw	t4, 20(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 20(t2)
	addw	a7, a7, t4
	lw	t4, 20(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 20(s1)
	addi	a4, a4, 4
	ld	t5, 496(sp)
	addi	t5, t5, 4
	sd	t5, 512(sp)
	ld	t5, 488(sp)
	addi	t5, t5, 4
	sd	t5, 504(sp)
	lw	t4, 24(s4)
	lw	a7, 24(t1)
	addw	a7, t4, a7
	lw	t4, 24(s3)
	addw	a7, a7, t4
	lw	t4, 24(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 24(t2)
	addw	a7, a7, t4
	lw	t4, 24(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 24(s1)
	addi	a4, a4, 4
	ld	t5, 512(sp)
	addi	t5, t5, 4
	sd	t5, 528(sp)
	ld	t5, 504(sp)
	addi	t5, t5, 4
	sd	t5, 520(sp)
	lw	t4, 28(s4)
	lw	a7, 28(t1)
	addw	a7, t4, a7
	lw	t4, 28(s3)
	addw	a7, a7, t4
	lw	t4, 28(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 28(t2)
	addw	a7, a7, t4
	lw	t4, 28(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 28(s1)
	addi	a4, a4, 4
	ld	t5, 528(sp)
	addi	t5, t5, 4
	sd	t5, 544(sp)
	ld	t5, 520(sp)
	addi	t5, t5, 4
	sd	t5, 536(sp)
	lw	t4, 32(s4)
	lw	a7, 32(t1)
	addw	a7, t4, a7
	lw	t4, 32(s3)
	addw	a7, a7, t4
	lw	t4, 32(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 32(t2)
	addw	a7, a7, t4
	lw	t4, 32(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 32(s1)
	addi	a4, a4, 4
	ld	t5, 544(sp)
	addi	t5, t5, 4
	sd	t5, 560(sp)
	ld	t5, 536(sp)
	addi	t5, t5, 4
	sd	t5, 552(sp)
	lw	t4, 36(s4)
	lw	a7, 36(t1)
	addw	a7, t4, a7
	lw	t4, 36(s3)
	addw	a7, a7, t4
	lw	t4, 36(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 36(t2)
	addw	a7, a7, t4
	lw	t4, 36(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 36(s1)
	addi	a4, a4, 4
	ld	t5, 560(sp)
	addi	t5, t5, 4
	sd	t5, 576(sp)
	ld	t5, 552(sp)
	addi	t5, t5, 4
	sd	t5, 568(sp)
	lw	t4, 40(s4)
	lw	a7, 40(t1)
	addw	a7, t4, a7
	lw	t4, 40(s3)
	addw	a7, a7, t4
	lw	t4, 40(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 40(t2)
	addw	a7, a7, t4
	lw	t4, 40(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 40(s1)
	addi	a4, a4, 4
	ld	t5, 576(sp)
	addi	t5, t5, 4
	sd	t5, 592(sp)
	ld	t5, 568(sp)
	addi	t5, t5, 4
	sd	t5, 584(sp)
	lw	t4, 44(s4)
	lw	a7, 44(t1)
	addw	a7, t4, a7
	lw	t4, 44(s3)
	addw	a7, a7, t4
	lw	t4, 44(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 44(t2)
	addw	a7, a7, t4
	lw	t4, 44(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 44(s1)
	addi	a4, a4, 4
	ld	t5, 592(sp)
	addi	t5, t5, 4
	sd	t5, 608(sp)
	ld	t5, 584(sp)
	addi	t5, t5, 4
	sd	t5, 600(sp)
	lw	t4, 48(s4)
	lw	a7, 48(t1)
	addw	a7, t4, a7
	lw	t4, 48(s3)
	addw	a7, a7, t4
	lw	t4, 48(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 48(t2)
	addw	a7, a7, t4
	lw	t4, 48(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 48(s1)
	addi	a4, a4, 4
	ld	t5, 608(sp)
	addi	t5, t5, 4
	sd	t5, 624(sp)
	ld	t5, 600(sp)
	addi	t5, t5, 4
	sd	t5, 616(sp)
	lw	t4, 52(s4)
	lw	a7, 52(t1)
	addw	a7, t4, a7
	lw	t4, 52(s3)
	addw	a7, a7, t4
	lw	t4, 52(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 52(t2)
	addw	a7, a7, t4
	lw	t4, 52(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 52(s1)
	addi	a4, a4, 4
	ld	t5, 624(sp)
	addi	t5, t5, 4
	sd	t5, 640(sp)
	ld	t5, 616(sp)
	addi	t5, t5, 4
	sd	t5, 632(sp)
	lw	t4, 56(s4)
	lw	a7, 56(t1)
	addw	a7, t4, a7
	lw	t4, 56(s3)
	addw	a7, a7, t4
	lw	t4, 56(s6)
	addw	a7, a7, t4
	addw	a7, a7, s2
	lw	t4, 56(t2)
	addw	a7, a7, t4
	lw	t4, 56(t0)
	addw	a7, a7, t4
	li	t4, 1431655766
	mul	a7, a7, t4	# const-div.fullmul
	srai	a7, a7, 32	# const-div.mulhi
	srliw	t4, a7, 31	# const-div.sign
	addw	s2, a7, t4	# const-div.round
	sw	s2, 56(s1)
	addi	t3, t3, 52
	addi	a2, a2, 52
	addi	a3, a3, 52
	addi	a4, a4, 4
	addi	a5, a5, 52
	addi	a6, a6, 52
	ld	t5, 640(sp)
	addi	a7, t5, 4
	ld	t5, 632(sp)
	addi	t5, t5, 4
	sd	t5, 648(sp)
	lw	t4, 60(s4)
	lw	t1, 60(t1)
	addw	t1, t4, t1
	lw	t4, 60(s3)
	addw	t1, t1, t4
	lw	t4, 60(s6)
	addw	t1, t1, t4
	addw	t1, t1, s2
	ld	t5, 408(sp)
	addiw	t4, t5, 16
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
	ld	t5, 648(sp)
	addi	a5, t5, 4
	mv	t2, a5	# phi-copy
	mv	s3, t1	# phi-copy
	mv	t1, a4	# phi-copy
	mv	s1, t0	# phi-copy
	mv	t0, t3	# phi-copy
	sd	t4, 408(sp)
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
	li	s4, 1
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
	ld	s11, 744(sp)
	ld	s10, 736(sp)
	ld	s9, 728(sp)
	ld	s8, 720(sp)
	ld	s7, 712(sp)
	ld	s6, 704(sp)
	ld	s5, 696(sp)
	ld	s4, 688(sp)
	ld	s3, 680(sp)
	ld	s2, 672(sp)
	ld	s1, 664(sp)
	ld	ra, 656(sp)
	addi	sp, sp, 752
	ret
	.size main, .-main
