
  .text

  .globl param16
  .type  param16, @function
param16:
.param16.bb0:
	addi	sp, sp, -304
	sd	s0, 208(sp)
	sd	s1, 216(sp)
	sd	s2, 224(sp)
	sd	s3, 232(sp)
	sd	s4, 240(sp)
	sd	s5, 248(sp)
	sd	s6, 256(sp)
	sd	s7, 264(sp)
	sd	s8, 272(sp)
	sd	s9, 280(sp)
	sd	s10, 288(sp)
	sd	s11, 296(sp)
	addi	s0, sp, 304
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	mv	s5, a4
	mv	s6, a5
	mv	s7, a6
	mv	s8, a7
	lw	s9, 0(s0)
	lw	s10, 8(s0)
	lw	s11, 16(s0)
	lw	t0, 24(s0)
	lw	t1, 32(s0)
	lw	t2, 40(s0)
	lw	t3, 48(s0)
	lw	a2, 56(s0)
	addi	t5, sp, 144
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	addi	a3, sp, 144
	addi	a4, sp, 144
	sw	s1, 0(a4)
	sw	s2, 4(a4)
	sw	s3, 8(a4)
	sw	s4, 12(a4)
	sw	s5, 16(a4)
	sw	s6, 20(a4)
	sw	s7, 24(a4)
	sw	s8, 28(a4)
	sw	s9, 32(a4)
	sw	s10, 36(a4)
	sw	s11, 40(a4)
	sw	t0, 44(a4)
	sw	t1, 48(a4)
	sw	t2, 52(a4)
	sw	t3, 56(a4)
	sw	a2, 60(a4)
	li	a4, 0
.param16.bb1:
	li	a5, 15
	bge	a4, a5, .param16.bb3
.param16.bb2:
	addiw	a4, a4, 1
	addi	a6, sp, 144
	slliw	a5, a4, 2
	add	a5, a6, a5
	li	a6, 16
	mv	a7, a5	# phi-copy
	mv	a5, a4	# phi-copy
.param16.bb4:
	bge	a5, a6, .param16.bb6
.param16.bb5:
	lw	t4, 0(a3)
	lw	a0, 0(a7)
	blt	t4, a0, .param16.bb7
.param16.bb8:
	addiw	a5, a5, 1
	addi	a7, a7, 4
	j	.param16.bb4
.param16.bb7:
	sw	a0, 0(a3)
	sw	t4, 0(a7)
	j	.param16.bb8
.param16.bb6:
	addi	a3, a3, 4
	j	.param16.bb1
.param16.bb3:
	addi	a3, sp, 144
	lw	t5, 0(a3)
	sd	t5, 0(sp)
	lw	t5, 4(a3)
	sd	t5, 8(sp)
	lw	t5, 8(a3)
	sd	t5, 16(sp)
	lw	t5, 12(a3)
	sd	t5, 24(sp)
	lw	t5, 16(a3)
	sd	t5, 32(sp)
	lw	t5, 20(a3)
	sd	t5, 40(sp)
	lw	t5, 24(a3)
	sd	t5, 48(sp)
	lw	t5, 28(a3)
	sd	t5, 56(sp)
	lw	a5, 32(a3)
	lw	a6, 36(a3)
	lw	a7, 40(a3)
	lw	t4, 44(a3)
	lw	a0, 48(a3)
	lw	a1, 52(a3)
	lw	a4, 56(a3)
	lw	a3, 60(a3)
	ld	t5, 24(sp)
	sd	t5, 64(sp)
	ld	t5, 32(sp)
	sd	t5, 72(sp)
	ld	t5, 40(sp)
	sd	t5, 80(sp)
	ld	t5, 48(sp)
	sd	t5, 88(sp)
	ld	t5, 56(sp)
	sd	t5, 96(sp)
	sd	a5, 104(sp)
	sd	a6, 112(sp)
	sd	a7, 120(sp)
	sd	t4, 128(sp)
	sd	a0, 136(sp)
	mv	a5, t2	# phi-copy
	mv	t2, s8	# phi-copy
	mv	s8, s5	# phi-copy
	mv	s5, s2	# phi-copy
	ld	s2, 0(sp)
	mv	a6, t3	# phi-copy
	mv	t3, s9	# phi-copy
	mv	s9, s6	# phi-copy
	mv	s6, s3	# phi-copy
	mv	s3, a3	# phi-copy
	mv	a3, t1	# phi-copy
	mv	t1, s7	# phi-copy
	mv	s7, s4	# phi-copy
	mv	s4, s1	# phi-copy
	ld	s1, 8(sp)
	mv	a7, a2	# phi-copy
	mv	a2, t0	# phi-copy
	ld	t0, 16(sp)
.param16.bb9:
	beq	s2, zero, .param16.bb10
.param16.bb11:
	addw	s1, s1, t0
	addiw	t4, s2, -1
	li	t0, 288737297
	mul	t0, s1, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 26	# const-div.shift
	srliw	a0, t0, 31	# const-div.sign
	addw	t0, t0, a0	# const-div.round
	li	a0, 998244353
	mulw	t0, t0, a0	# const-rem.mul
	subw	t0, s1, t0	# const-rem.sub
	li	a0, 0
	mv	s1, t0	# phi-copy
	mv	s2, t4	# phi-copy
	ld	t0, 64(sp)
	ld	t5, 72(sp)
	sd	t5, 64(sp)
	ld	t5, 80(sp)
	sd	t5, 72(sp)
	ld	t5, 88(sp)
	sd	t5, 80(sp)
	ld	t5, 96(sp)
	sd	t5, 88(sp)
	ld	t5, 104(sp)
	sd	t5, 96(sp)
	ld	t5, 112(sp)
	sd	t5, 104(sp)
	ld	t5, 120(sp)
	sd	t5, 112(sp)
	ld	t5, 128(sp)
	sd	t5, 120(sp)
	ld	t5, 136(sp)
	sd	t5, 128(sp)
	sd	a1, 136(sp)
	mv	a1, a4	# phi-copy
	mv	a4, s3	# phi-copy
	mv	s3, s4	# phi-copy
	mv	s4, s5	# phi-copy
	mv	s5, s6	# phi-copy
	mv	s6, s7	# phi-copy
	mv	s7, s8	# phi-copy
	mv	s8, s9	# phi-copy
	mv	s9, t1	# phi-copy
	mv	t1, t2	# phi-copy
	mv	t2, t3	# phi-copy
	mv	t3, s10	# phi-copy
	mv	s10, s11	# phi-copy
	mv	s11, a2	# phi-copy
	mv	a2, a3	# phi-copy
	mv	a3, a5	# phi-copy
	mv	a5, a6	# phi-copy
	mv	a6, a7	# phi-copy
	mv	a7, a0	# phi-copy
	bne	s2, zero, .param16.bb11	# rotated-loop-branch
.param16.bb10:
	mv	a0, s1
	ld	s11, 296(sp)
	ld	s10, 288(sp)
	ld	s9, 280(sp)
	ld	s8, 272(sp)
	ld	s7, 264(sp)
	ld	s6, 256(sp)
	ld	s5, 248(sp)
	ld	s4, 240(sp)
	ld	s3, 232(sp)
	ld	s2, 224(sp)
	ld	s1, 216(sp)
	ld	s0, 208(sp)
	addi	sp, sp, 304
	ret
	.size param16, .-param16

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -512
	sd	ra, 416(sp)
	sd	s1, 424(sp)
	sd	s2, 432(sp)
	sd	s3, 440(sp)
	sd	s4, 448(sp)
	sd	s5, 456(sp)
	sd	s6, 464(sp)
	sd	s7, 472(sp)
	sd	s8, 480(sp)
	sd	s9, 488(sp)
	sd	s10, 496(sp)
	sd	s11, 504(sp)
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
	call	getint
	mv	s10, a0
	call	getint
	mv	s11, a0
	call	getint
	mv	t5, a0
	sd	t5, 128(sp)
	call	getint
	mv	t5, a0
	sd	t5, 136(sp)
	call	getint
	mv	t5, a0
	sd	t5, 144(sp)
	call	getint
	mv	t5, a0
	sd	t5, 152(sp)
	call	getint
	mv	t0, a0
	addi	t5, sp, 160
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
	sd	zero, 160(t5)	# memset-zero
	sd	zero, 168(t5)	# memset-zero
	sd	zero, 176(t5)	# memset-zero
	sd	zero, 184(t5)	# memset-zero
	sd	zero, 192(t5)	# memset-zero
	sd	zero, 200(t5)	# memset-zero
	sd	zero, 208(t5)	# memset-zero
	sd	zero, 216(t5)	# memset-zero
	sd	zero, 224(t5)	# memset-zero
	sd	zero, 232(t5)	# memset-zero
	sd	zero, 240(t5)	# memset-zero
	sd	zero, 248(t5)	# memset-zero
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, s2
	sd	t5, 8(sp)
	mv	t5, s3
	sd	t5, 16(sp)
	mv	t5, s4
	sd	t5, 24(sp)
	mv	t5, s5
	sd	t5, 32(sp)
	mv	t5, s6
	sd	t5, 40(sp)
	mv	t5, s7
	sd	t5, 48(sp)
	mv	t5, s8
	sd	t5, 56(sp)
	mv	t5, s9
	sd	t5, 64(sp)
	mv	t5, s10
	sd	t5, 72(sp)
	mv	t5, s11
	sd	t5, 80(sp)
	ld	t5, 128(sp)
	sd	t5, 88(sp)
	ld	t5, 136(sp)
	sd	t5, 96(sp)
	ld	t5, 144(sp)
	sd	t5, 104(sp)
	ld	t5, 152(sp)
	sd	t5, 112(sp)
	mv	t5, t0
	sd	t5, 120(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	ld	a5, 40(sp)
	ld	a6, 48(sp)
	ld	a7, 56(sp)
	ld	t5, 64(sp)
	sd	t5, 0(sp)
	ld	t5, 72(sp)
	sd	t5, 8(sp)
	ld	t5, 80(sp)
	sd	t5, 16(sp)
	ld	t5, 88(sp)
	sd	t5, 24(sp)
	ld	t5, 96(sp)
	sd	t5, 32(sp)
	ld	t5, 104(sp)
	sd	t5, 40(sp)
	ld	t5, 112(sp)
	sd	t5, 48(sp)
	ld	t5, 120(sp)
	sd	t5, 56(sp)
	call	param16
	mv	t0, a0
	addi	s1, sp, 160
	sw	t0, 0(s1)
	li	t0, 8848
	sw	t0, 4(s1)
	li	t0, 1
.main.bb1:
	li	t1, 32
	bge	t0, t1, .main.bb3
.main.bb4:
	addiw	t1, t0, 3
	li	t2, 32
	blt	t1, t2, .main.bb5
.main.bb2:
	addiw	t1, t0, -1
	slliw	t1, t1, 3
	addiw	t0, t0, 1
	addiw	t1, t1, 4
	add	s2, s1, t1
	lw	t1, 0(s2)
	addiw	t1, t1, -1
	sw	t1, 4(s2)
	lw	t2, -4(s2)
	addiw	t2, t2, -2
	sw	t2, 8(s2)
	j	.main.bb1
.main.bb5:
	addiw	t1, t0, -1
	slliw	t1, t1, 3
	addiw	t0, t0, 4
	addiw	t1, t1, 4
	add	s2, s1, t1
	lw	t1, 0(s2)
	addiw	t1, t1, -1
	sw	t1, 4(s2)
	lw	t2, -4(s2)
	addiw	s3, t2, -2
	addiw	t2, s3, -1
	sw	s3, 8(s2)
	sw	t2, 12(s2)
	lw	t2, 4(s2)
	addiw	s3, t2, -2
	addiw	t2, s3, -1
	sw	s3, 16(s2)
	sw	t2, 20(s2)
	lw	t2, 12(s2)
	addiw	s3, t2, -2
	addiw	t2, s3, -1
	sw	s3, 24(s2)
	sw	t2, 28(s2)
	lw	t2, 20(s2)
	addiw	t2, t2, -2
	sw	t2, 32(s2)
	j	.main.bb1
.main.bb3:
	lw	t0, 0(s1)
	lw	t1, 4(s1)
	addw	t0, t0, t1
	lw	t1, 8(s1)
	addw	t0, t0, t1
	lw	t1, 12(s1)
	addw	t0, t0, t1
	lw	t1, 16(s1)
	addw	t0, t0, t1
	lw	t1, 20(s1)
	addw	t0, t0, t1
	lw	t1, 24(s1)
	addw	t0, t0, t1
	lw	t1, 28(s1)
	addw	t0, t0, t1
	lw	t1, 32(s1)
	addw	t0, t0, t1
	lw	t1, 36(s1)
	addw	t0, t0, t1
	lw	t1, 40(s1)
	addw	t0, t0, t1
	lw	t1, 44(s1)
	addw	t0, t0, t1
	lw	t1, 48(s1)
	addw	t0, t0, t1
	lw	t1, 52(s1)
	addw	t0, t0, t1
	lw	t1, 56(s1)
	addw	t0, t0, t1
	lw	t1, 60(s1)
	addw	t0, t0, t1
	lw	t1, 64(s1)
	addw	t0, t0, t1
	lw	t1, 68(s1)
	addw	t0, t0, t1
	lw	t1, 72(s1)
	addw	t0, t0, t1
	lw	t1, 76(s1)
	addw	t0, t0, t1
	lw	t1, 80(s1)
	addw	t0, t0, t1
	lw	t1, 84(s1)
	addw	t0, t0, t1
	lw	t1, 88(s1)
	addw	t0, t0, t1
	lw	t1, 92(s1)
	addw	t0, t0, t1
	lw	t1, 96(s1)
	addw	t0, t0, t1
	lw	t1, 100(s1)
	addw	t0, t0, t1
	lw	t1, 104(s1)
	addw	t0, t0, t1
	lw	t1, 108(s1)
	addw	t0, t0, t1
	lw	t1, 112(s1)
	addw	t0, t0, t1
	lw	t1, 116(s1)
	addw	t0, t0, t1
	lw	t1, 120(s1)
	addw	t0, t0, t1
	lw	t1, 124(s1)
	addw	t0, t0, t1
	lw	t1, 128(s1)
	addw	t0, t0, t1
	lw	t1, 132(s1)
	addw	t0, t0, t1
	lw	t1, 136(s1)
	addw	t0, t0, t1
	lw	t1, 140(s1)
	addw	t0, t0, t1
	lw	t1, 144(s1)
	addw	t0, t0, t1
	lw	t1, 148(s1)
	addw	t0, t0, t1
	lw	t1, 152(s1)
	addw	t0, t0, t1
	lw	t1, 156(s1)
	addw	t0, t0, t1
	lw	t1, 160(s1)
	addw	t0, t0, t1
	lw	t1, 164(s1)
	addw	t0, t0, t1
	lw	t1, 168(s1)
	addw	t0, t0, t1
	lw	t1, 172(s1)
	addw	t0, t0, t1
	lw	t1, 176(s1)
	addw	t0, t0, t1
	lw	t1, 180(s1)
	addw	t0, t0, t1
	lw	t1, 184(s1)
	addw	t0, t0, t1
	lw	t1, 188(s1)
	addw	t0, t0, t1
	lw	t1, 192(s1)
	addw	t0, t0, t1
	lw	t1, 196(s1)
	addw	t0, t0, t1
	lw	t1, 200(s1)
	addw	t0, t0, t1
	lw	t1, 204(s1)
	addw	t0, t0, t1
	lw	t1, 208(s1)
	addw	t0, t0, t1
	lw	t1, 212(s1)
	addw	t0, t0, t1
	lw	t1, 216(s1)
	addw	t0, t0, t1
	lw	t1, 220(s1)
	addw	t0, t0, t1
	lw	t1, 224(s1)
	addw	t0, t0, t1
	lw	t1, 228(s1)
	addw	t0, t0, t1
	lw	t1, 232(s1)
	addw	t0, t0, t1
	lw	t1, 236(s1)
	addw	t0, t0, t1
	lw	t1, 240(s1)
	addw	t0, t0, t1
	lw	t1, 244(s1)
	addw	t0, t0, t1
	lw	t1, 248(s1)
	addw	t0, t0, t1
	lw	t1, 252(s1)
	addw	a0, t0, t1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s11, 504(sp)
	ld	s10, 496(sp)
	ld	s9, 488(sp)
	ld	s8, 480(sp)
	ld	s7, 472(sp)
	ld	s6, 464(sp)
	ld	s5, 456(sp)
	ld	s4, 448(sp)
	ld	s3, 440(sp)
	ld	s2, 432(sp)
	ld	s1, 424(sp)
	ld	ra, 416(sp)
	addi	sp, sp, 512
	ret
	.size main, .-main
