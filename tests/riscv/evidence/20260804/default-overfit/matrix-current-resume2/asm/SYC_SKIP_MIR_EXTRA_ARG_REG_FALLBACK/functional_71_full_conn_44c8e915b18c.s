
  .text

  .globl model
  .type  model, @function
model:
.model.bb0:
	addi	sp, sp, -416
	sd	s1, 368(sp)
	sd	s2, 376(sp)
	sd	s3, 384(sp)
	sd	s4, 392(sp)
	sd	s5, 400(sp)
	sd	s6, 408(sp)
	mv	t0, a0
	lw	t2, 0(t0)
	li	t3, 85
	mulw	t3, t2, t3
	lw	a2, 4(t0)
	li	a3, 23
	mulw	a3, a2, a3
	addw	t3, t3, a3
	lw	t5, 8(t0)
	sd	t5, 360(sp)
	li	a4, -82
	mulw	a4, t5, a4
	addw	t3, t3, a4
	lw	t5, 12(t0)
	sd	t5, 272(sp)
	li	a5, -103
	mulw	a5, t5, a5
	addw	t3, t3, a5
	lw	t5, 16(t0)
	sd	t5, 152(sp)
	li	a6, -123
	mulw	a6, t5, a6
	addw	t3, t3, a6
	lw	t5, 20(t0)
	sd	t5, 128(sp)
	slliw	t5, t5, 6
	sd	t5, 0(sp)
	addw	t3, t3, t5
	lw	t5, 24(t0)
	sd	t5, 24(sp)
	slliw	a7, t5, 7
	slliw	t5, t5, 3
	sd	t5, 16(sp)
	subw	t5, a7, t5
	sd	t5, 8(sp)
	subw	t3, t3, t5
	lw	t5, 28(t0)
	sd	t5, 112(sp)
	li	a7, 50
	mulw	a7, t5, a7
	addw	t3, t3, a7
	lw	t5, 32(t0)
	sd	t5, 32(sp)
	li	s1, -59
	mulw	t4, t5, s1
	addw	t3, t3, t4
	lw	t5, 36(t0)
	sd	t5, 40(sp)
	li	a7, 47
	mulw	a7, t5, a7
	addw	t3, t3, a7
	lw	t5, 40(t0)
	sd	t5, 48(sp)
	li	t4, -111
	mulw	t4, t5, t4
	addw	t3, t3, t4
	lw	t5, 44(t0)
	sd	t5, 64(sp)
	li	a7, -67
	mulw	t5, t5, a7
	sd	t5, 56(sp)
	addw	t3, t3, t5
	lw	t5, 48(t0)
	sd	t5, 104(sp)
	li	a7, -106
	mulw	a7, t5, a7
	addw	t3, t3, a7
	lw	t5, 52(t0)
	sd	t5, 72(sp)
	li	t4, -75
	mulw	t4, t5, t4
	addw	t3, t3, t4
	lw	t4, 56(t0)
	li	a7, -102
	mulw	a7, t4, a7
	addw	t3, t3, a7
	lw	t5, 60(t0)
	sd	t5, 80(sp)
	slliw	t5, t5, 5
	sd	t5, 96(sp)
	ld	t5, 80(sp)
	slliw	a7, t5, 1
	ld	t5, 96(sp)
	addw	t5, t5, a7
	sd	t5, 88(sp)
	addw	t3, t3, t5
	lw	a7, 64(t0)
	li	s3, -39
	mulw	s1, a7, s3
	addw	t3, t3, s1
	lw	s2, 68(t0)
	slliw	t5, s2, 6
	sd	t5, 120(sp)
	addw	s1, t5, s2
	addw	t3, t3, s1
	lw	t5, 72(t0)
	sd	t5, 136(sp)
	li	a6, 47
	mulw	a6, t5, a6
	addw	t3, t3, a6
	lw	t5, 76(t0)
	sd	t5, 144(sp)
	li	s1, 113
	mulw	a6, t5, s1
	addw	t3, t3, a6
	lw	t5, 80(t0)
	sd	t5, 160(sp)
	li	a5, 110
	mulw	a5, t5, a5
	addw	t3, t3, a5
	lw	t5, 84(t0)
	sd	t5, 320(sp)
	li	a6, 47
	mulw	a6, t5, a6
	addw	t3, t3, a6
	lw	t5, 88(t0)
	sd	t5, 176(sp)
	slliw	t5, t5, 2
	sd	t5, 168(sp)
	subw	t3, t3, t5
	lw	s1, 92(t0)
	slliw	a6, s1, 6
	slliw	t5, s1, 4
	sd	t5, 184(sp)
	addw	a6, a6, t5
	addw	t3, t3, a6
	lw	t0, 96(t0)
	li	t1, 46
	mulw	t1, t0, t1
	addw	t1, t3, t1
	li	t3, 127
	blt	t3, t1, .model.bb3
.model.bb4:
	blt	t1, zero, .model.bb5
.model.bb6:
.model.bb2:
	li	t3, 39
	mulw	t1, t1, t3
	li	t3, -106
	mulw	t3, t2, t3
	slliw	a6, a2, 7
	slliw	s3, a2, 1
	subw	a6, a6, s3
	addw	t3, t3, a6
	ld	t5, 360(sp)
	slliw	a6, t5, 4
	ld	t5, 360(sp)
	slliw	t5, t5, 1
	sd	t5, 200(sp)
	addw	a6, a6, t5
	subw	t3, t3, a6
	ld	t5, 272(sp)
	slliw	t5, t5, 5
	sd	t5, 192(sp)
	ld	t6, 272(sp)
	subw	a6, t5, t6
	subw	t3, t3, a6
	ld	t5, 152(sp)
	slliw	t5, t5, 3
	sd	t5, 208(sp)
	subw	t3, t3, t5
	li	s3, 47
	ld	t5, 128(sp)
	mulw	a6, t5, s3
	addw	t3, t3, a6
	ld	t5, 24(sp)
	slliw	a6, t5, 2
	subw	t3, t3, a6
	li	a6, 67
	ld	t5, 112(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -94
	ld	t5, 32(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -121
	ld	t5, 40(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 48(sp)
	slliw	a6, t5, 3
	ld	t5, 48(sp)
	subw	a6, a6, t5
	addw	t3, t3, a6
	li	a6, -21
	ld	t5, 64(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 104(sp)
	slliw	a6, t5, 6
	ld	t5, 104(sp)
	slliw	s3, t5, 2
	subw	a6, a6, s3
	subw	t3, t3, a6
	li	a6, -43
	ld	t5, 72(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 105
	mulw	a6, t4, a6
	addw	t3, t3, a6
	li	a6, -42
	ld	t5, 80(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 87
	mulw	a6, a7, a6
	addw	t3, t3, a6
	li	a6, 29
	mulw	a6, s2, a6
	addw	t3, t3, a6
	li	a6, -106
	ld	t5, 136(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 144(sp)
	slliw	a6, t5, 5
	ld	t5, 144(sp)
	subw	a6, a6, t5
	subw	t3, t3, a6
	li	a6, -110
	ld	t5, 160(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -100
	ld	t5, 320(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -22
	ld	t5, 176(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -75
	mulw	a6, s1, a6
	addw	t3, t3, a6
	li	a6, -125
	mulw	a6, t0, a6
	addw	t3, t3, a6
	li	a6, 127
	blt	a6, t3, .model.bb8
.model.bb9:
	blt	t3, zero, .model.bb10
.model.bb11:
.model.bb7:
	li	a6, 77
	mulw	t3, t3, a6
	addw	t1, t1, t3
	li	t3, 26
	mulw	t3, t2, t3
	li	a6, 76
	mulw	a6, a2, a6
	addw	t3, t3, a6
	li	a6, -70
	ld	t5, 360(sp)
	mulw	t5, t5, a6
	sd	t5, 216(sp)
	addw	t3, t3, t5
	li	a6, 29
	ld	t5, 272(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -95
	ld	t5, 152(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 128(sp)
	slliw	t5, t5, 5
	sd	t5, 224(sp)
	ld	t5, 0(sp)
	ld	t6, 224(sp)
	addw	a6, t5, t6
	addw	t3, t3, a6
	li	a6, 52
	ld	t5, 24(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 112(sp)
	slliw	a6, t5, 6
	ld	t5, 112(sp)
	slliw	s4, t5, 2
	addw	a6, a6, s4
	subw	t3, t3, a6
	ld	t5, 32(sp)
	slliw	t5, t5, 2
	sd	t5, 232(sp)
	ld	t6, 32(sp)
	addw	a6, t5, t6
	subw	t3, t3, a6
	ld	t5, 40(sp)
	slliw	a6, t5, 5
	ld	t5, 40(sp)
	slliw	t5, t5, 1
	sd	t5, 248(sp)
	addw	a6, a6, t5
	addw	t3, t3, a6
	ld	t5, 48(sp)
	slliw	a6, t5, 5
	ld	t5, 48(sp)
	slliw	t5, t5, 1
	sd	t5, 256(sp)
	addw	a6, a6, t5
	subw	t3, t3, a6
	li	a6, 102
	ld	t5, 64(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 104(sp)
	slliw	a6, t5, 1
	addw	a6, s3, a6
	addw	t3, t3, a6
	li	a6, -38
	ld	t5, 72(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 27
	mulw	a6, t4, a6
	addw	t3, t3, a6
	li	a6, 110
	ld	t5, 80(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 116
	mulw	a6, a7, a6
	addw	t3, t3, a6
	li	a6, 39
	mulw	a6, s2, a6
	addw	t3, t3, a6
	ld	t5, 136(sp)
	slliw	a6, t5, 6
	ld	t5, 136(sp)
	subw	a6, a6, t5
	subw	t3, t3, a6
	li	a6, -99
	ld	t5, 144(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 160(sp)
	slliw	a6, t5, 6
	ld	t5, 160(sp)
	addw	t5, a6, t5
	sd	t5, 240(sp)
	addw	t3, t3, t5
	ld	t5, 320(sp)
	slliw	a6, t5, 7
	ld	t5, 320(sp)
	slliw	s5, t5, 3
	subw	a6, a6, s5
	addw	t3, t3, a6
	li	a6, -39
	ld	t5, 176(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	slliw	a6, s1, 2
	slliw	s6, s1, 1
	addw	a6, a6, s6
	subw	t3, t3, a6
	li	a6, 94
	mulw	a6, t0, a6
	addw	t3, t3, a6
	li	a6, 127
	blt	a6, t3, .model.bb13
.model.bb14:
	blt	t3, zero, .model.bb15
.model.bb16:
.model.bb12:
	slliw	a6, t3, 7
	subw	t3, a6, t3
	addw	t1, t1, t3
	li	t3, -23
	mulw	t3, t2, t3
	slliw	t5, a2, 6
	sd	t5, 264(sp)
	subw	a6, t5, a2
	subw	t3, t3, a6
	li	a6, 49
	ld	t5, 360(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 50
	ld	t5, 272(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 152(sp)
	slliw	a6, t5, 6
	ld	t5, 208(sp)
	addw	a6, a6, t5
	addw	t3, t3, a6
	li	a6, 85
	ld	t5, 128(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 24(sp)
	slliw	t5, t5, 5
	sd	t5, 280(sp)
	ld	t5, 24(sp)
	slliw	a4, t5, 1
	ld	t5, 280(sp)
	subw	a4, t5, a4
	subw	t3, t3, a4
	ld	t5, 112(sp)
	slliw	a4, t5, 3
	addw	a4, a4, s4
	addw	t3, t3, a4
	li	a4, 125
	ld	t5, 32(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a6, -117
	ld	t5, 40(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 48(sp)
	slliw	t5, t5, 6
	sd	t5, 288(sp)
	ld	t6, 48(sp)
	addw	a6, t5, t6
	subw	t3, t3, a6
	ld	t5, 56(sp)
	addw	t3, t3, t5
	li	a6, 125
	ld	t5, 104(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 110
	ld	t5, 72(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	slliw	a6, t4, 5
	subw	a6, a6, t4
	subw	t3, t3, a6
	li	a6, -123
	ld	t5, 80(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 83
	mulw	a6, a7, a6
	addw	t3, t3, a6
	li	a6, 122
	mulw	a6, s2, a6
	addw	t3, t3, a6
	li	a6, 11
	ld	t5, 136(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -23
	ld	t5, 144(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -47
	ld	t5, 160(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 320(sp)
	slliw	t5, t5, 5
	sd	t5, 296(sp)
	subw	t3, t3, t5
	li	a6, -117
	ld	t5, 176(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 95
	mulw	a6, s1, a6
	addw	t3, t3, a6
	li	a6, 118
	mulw	a6, t0, a6
	addw	t3, t3, a6
	li	a6, 127
	blt	a6, t3, .model.bb18
.model.bb19:
	blt	t3, zero, .model.bb20
.model.bb21:
.model.bb17:
	li	a6, -106
	mulw	t3, t3, a6
	addw	t1, t1, t3
	slliw	t3, t2, 3
	li	a6, 82
	mulw	t5, a2, a6
	sd	t5, 304(sp)
	addw	t3, t3, t5
	li	a6, -104
	ld	t5, 360(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, 101
	ld	t5, 272(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -116
	ld	t5, 152(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 0(sp)
	ld	t6, 128(sp)
	subw	a6, t5, t6
	subw	t3, t3, a6
	ld	t5, 24(sp)
	slliw	a6, t5, 4
	subw	t3, t3, a6
	li	a6, -70
	ld	t5, 112(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	addw	t3, t3, a4
	li	a4, 75
	ld	t5, 40(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	ld	t5, 288(sp)
	ld	t6, 256(sp)
	addw	a4, t5, t6
	addw	t3, t3, a4
	ld	t5, 64(sp)
	slliw	a4, t5, 6
	ld	t5, 64(sp)
	slliw	a6, t5, 5
	addw	a4, a4, a6
	subw	t3, t3, a4
	li	a4, -101
	ld	t5, 104(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, -114
	ld	t5, 72(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, 59
	mulw	a4, t4, a4
	addw	t3, t3, a4
	ld	t5, 80(sp)
	slliw	t5, t5, 3
	sd	t5, 312(sp)
	ld	t5, 80(sp)
	slliw	a6, t5, 2
	ld	t5, 312(sp)
	addw	a6, t5, a6
	addw	t3, t3, a6
	slliw	a6, a7, 2
	addw	a6, a6, a7
	addw	t3, t3, a6
	li	a6, -95
	mulw	a6, s2, a6
	addw	t3, t3, a6
	li	a6, 116
	ld	t5, 136(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	li	a6, -93
	ld	t5, 144(sp)
	mulw	a6, t5, a6
	addw	t3, t3, a6
	ld	t5, 160(sp)
	slliw	a6, t5, 4
	ld	t5, 160(sp)
	subw	a4, a6, t5
	addw	t3, t3, a4
	li	a4, 79
	ld	t5, 320(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	ld	t5, 176(sp)
	slliw	a4, t5, 1
	ld	t5, 176(sp)
	addw	a4, a4, t5
	addw	t3, t3, a4
	li	a4, 49
	mulw	a4, s1, a4
	addw	t3, t3, a4
	slliw	a4, t0, 7
	slliw	a5, t0, 2
	subw	a4, a4, a5
	subw	t3, t3, a4
	li	a4, 127
	blt	a4, t3, .model.bb23
.model.bb24:
	blt	t3, zero, .model.bb25
.model.bb26:
.model.bb22:
	slliw	a4, t3, 1
	addw	t3, a4, t3
	subw	t1, t1, t3
	li	t3, 81
	mulw	t3, t2, t3
	slliw	a4, a2, 2
	ld	t5, 264(sp)
	addw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, -102
	ld	t5, 360(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, -74
	ld	t5, 272(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, 121
	ld	t5, 152(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	ld	t5, 128(sp)
	slliw	a4, t5, 4
	ld	t5, 128(sp)
	subw	t5, a4, t5
	sd	t5, 336(sp)
	subw	t3, t3, t5
	li	a5, 55
	ld	t5, 24(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 101
	ld	t5, 112(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -13
	ld	t5, 32(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 40(sp)
	slliw	a5, t5, 6
	ld	t5, 248(sp)
	subw	t5, a5, t5
	sd	t5, 328(sp)
	subw	t3, t3, t5
	ld	t5, 288(sp)
	addw	t3, t3, t5
	li	a5, 114
	ld	t5, 64(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 38
	ld	t5, 104(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -21
	ld	t5, 72(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	slliw	a5, t4, 7
	slliw	a4, t4, 4
	subw	a4, a5, a4
	addw	t3, t3, a4
	li	a4, 114
	ld	t5, 80(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	slliw	a4, a7, 7
	slliw	t5, a7, 4
	sd	t5, 344(sp)
	subw	a4, a4, t5
	addw	t3, t3, a4
	slliw	a4, s2, 3
	slliw	a5, s2, 1
	addw	a4, a4, a5
	subw	t3, t3, a4
	ld	t5, 136(sp)
	slliw	a4, t5, 4
	subw	t3, t3, a4
	li	a4, -50
	ld	t5, 144(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	ld	t5, 160(sp)
	slliw	a4, t5, 7
	subw	a4, a4, a6
	subw	t3, t3, a4
	li	a4, -116
	ld	t5, 320(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, -54
	ld	t5, 176(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, 82
	mulw	a4, s1, a4
	addw	t3, t3, a4
	slliw	a4, t0, 6
	slliw	a5, t0, 3
	addw	a5, a4, a5
	subw	t3, t3, a5
	li	a5, 127
	blt	a5, t3, .model.bb28
.model.bb29:
	blt	t3, zero, .model.bb30
.model.bb31:
.model.bb27:
	slliw	t3, t3, 5
	addw	t1, t1, t3
	slliw	t3, t2, 4
	subw	t3, t3, t2
	li	a5, -77
	mulw	a5, a2, a5
	addw	t3, t3, a5
	ld	t5, 360(sp)
	slliw	a5, t5, 6
	ld	t5, 200(sp)
	addw	a5, a5, t5
	addw	t3, t3, a5
	li	a5, -90
	ld	t5, 272(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 152(sp)
	slliw	a5, t5, 2
	ld	t5, 152(sp)
	slliw	a6, t5, 1
	addw	a5, a5, a6
	subw	t3, t3, a5
	ld	t5, 128(sp)
	slliw	t5, t5, 1
	sd	t5, 352(sp)
	ld	t5, 224(sp)
	ld	t6, 352(sp)
	subw	a5, t5, t6
	subw	t3, t3, a5
	ld	t5, 16(sp)
	subw	t3, t3, t5
	li	a5, 81
	ld	t5, 112(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 32(sp)
	slliw	a5, t5, 1
	addw	t3, t3, a5
	li	a3, -110
	ld	t5, 40(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	li	a3, -95
	ld	t5, 48(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	li	a3, 59
	ld	t5, 64(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	li	a3, 52
	ld	t5, 104(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	ld	t5, 72(sp)
	slliw	a3, t5, 4
	ld	t5, 72(sp)
	subw	a3, a3, t5
	addw	t3, t3, a3
	li	a3, 55
	mulw	a3, t4, a3
	addw	t3, t3, a3
	ld	t5, 96(sp)
	ld	t6, 80(sp)
	addw	a3, t5, t6
	subw	t3, t3, a3
	slliw	a3, a7, 1
	ld	t5, 344(sp)
	subw	a3, t5, a3
	addw	t3, t3, a3
	li	a3, 58
	mulw	a3, s2, a3
	addw	t3, t3, a3
	li	a3, 67
	ld	t5, 136(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	li	a3, 86
	ld	t5, 144(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	li	a3, -79
	ld	t5, 160(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	ld	t5, 320(sp)
	slliw	a3, t5, 4
	ld	t5, 296(sp)
	addw	a3, t5, a3
	addw	t3, t3, a3
	li	a3, -13
	ld	t5, 176(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	ld	t5, 184(sp)
	subw	a3, t5, s1
	subw	t3, t3, a3
	slliw	a3, t0, 1
	addw	a3, a4, a3
	addw	t3, t3, a3
	li	a3, 127
	blt	a3, t3, .model.bb33
.model.bb34:
	blt	t3, zero, .model.bb35
.model.bb36:
.model.bb32:
	li	a3, -95
	mulw	t3, t3, a3
	addw	t1, t1, t3
	slliw	t3, t2, 5
	addw	t3, t3, t2
	ld	t5, 304(sp)
	addw	t3, t3, t5
	li	a3, 67
	ld	t5, 360(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	ld	t5, 272(sp)
	slliw	a3, t5, 1
	ld	t5, 192(sp)
	subw	a3, t5, a3
	addw	t3, t3, a3
	subw	t3, t3, a6
	ld	t5, 0(sp)
	ld	t6, 128(sp)
	addw	a3, t5, t6
	addw	t3, t3, a3
	ld	t5, 8(sp)
	addw	t3, t3, t5
	li	a3, -13
	ld	t5, 112(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	ld	t5, 32(sp)
	slliw	a3, t5, 4
	addw	a3, a3, a5
	addw	t3, t3, a3
	ld	t5, 40(sp)
	slliw	a3, t5, 2
	ld	t5, 40(sp)
	addw	a3, a3, t5
	addw	t3, t3, a3
	li	a3, 104
	ld	t5, 48(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	li	a3, -119
	ld	t5, 64(sp)
	mulw	a3, t5, a3
	addw	t3, t3, a3
	ld	t5, 104(sp)
	slliw	a3, t5, 3
	ld	t5, 104(sp)
	subw	a3, a3, t5
	subw	t3, t3, a3
	li	a4, 71
	ld	t5, 72(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, 107
	mulw	a4, t4, a4
	addw	t3, t3, a4
	ld	t5, 80(sp)
	slliw	a4, t5, 4
	ld	t5, 312(sp)
	addw	a4, a4, t5
	addw	t3, t3, a4
	li	a4, 82
	mulw	a4, a7, a4
	addw	t3, t3, a4
	slliw	a4, s2, 5
	ld	t5, 120(sp)
	addw	a4, t5, a4
	subw	t3, t3, a4
	li	a4, -104
	ld	t5, 136(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, -121
	ld	t5, 144(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	ld	t5, 240(sp)
	addw	t3, t3, t5
	li	a4, 97
	ld	t5, 320(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, 83
	ld	t5, 176(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, 46
	mulw	a4, s1, a4
	addw	t3, t3, a4
	li	a4, -84
	mulw	a4, t0, a4
	addw	t3, t3, a4
	li	a4, 127
	blt	a4, t3, .model.bb38
.model.bb39:
	blt	t3, zero, .model.bb40
.model.bb41:
.model.bb37:
	li	a4, -50
	mulw	t3, t3, a4
	addw	t1, t1, t3
	li	t3, -29
	mulw	t3, t2, t3
	slliw	a4, a2, 3
	subw	a4, a4, a2
	addw	t3, t3, a4
	ld	t5, 216(sp)
	addw	t3, t3, t5
	li	a4, 38
	ld	t5, 272(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	li	a4, -90
	ld	t5, 152(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	ld	t5, 336(sp)
	subw	t3, t3, t5
	ld	t5, 280(sp)
	subw	t3, t3, t5
	li	a4, 37
	ld	t5, 112(sp)
	mulw	a4, t5, a4
	addw	t3, t3, a4
	ld	t5, 32(sp)
	slliw	a4, t5, 5
	ld	t5, 232(sp)
	addw	a5, a4, t5
	addw	t3, t3, a5
	ld	t5, 328(sp)
	subw	t3, t3, t5
	li	a5, -125
	ld	t5, 48(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -46
	ld	t5, 64(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -70
	ld	t5, 104(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 37
	ld	t5, 72(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -73
	mulw	a5, t4, a5
	addw	t3, t3, a5
	ld	t5, 88(sp)
	subw	t3, t3, t5
	li	a5, -87
	mulw	a5, a7, a5
	addw	t3, t3, a5
	li	a5, -75
	mulw	a5, s2, a5
	addw	t3, t3, a5
	li	a5, 71
	ld	t5, 136(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -77
	ld	t5, 144(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 53
	ld	t5, 160(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 37
	ld	t5, 320(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -103
	ld	t5, 176(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -13
	mulw	a5, s1, a5
	addw	t3, t3, a5
	li	a5, -114
	mulw	a5, t0, a5
	addw	t3, t3, a5
	li	a5, 127
	blt	a5, t3, .model.bb43
.model.bb44:
	blt	t3, zero, .model.bb45
.model.bb46:
.model.bb42:
	li	a5, -23
	mulw	t3, t3, a5
	addw	t1, t1, t3
	li	t3, 67
	mulw	t2, t2, t3
	li	t3, 42
	mulw	t3, a2, t3
	addw	t2, t2, t3
	li	t3, 41
	ld	t5, 360(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, -123
	ld	t5, 272(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, -92
	ld	t5, 152(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	ld	t5, 128(sp)
	slliw	t3, t5, 3
	ld	t5, 352(sp)
	addw	t3, t3, t5
	addw	t2, t2, t3
	li	t3, -77
	ld	t5, 24(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 75
	ld	t5, 112(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	ld	t5, 32(sp)
	slliw	t3, t5, 6
	addw	t3, t3, a4
	addw	t2, t2, t3
	li	t3, -51
	ld	t5, 40(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 109
	ld	t5, 48(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, -74
	ld	t5, 64(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	subw	t2, t2, a3
	li	t3, -122
	ld	t5, 72(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 67
	mulw	t3, t4, t3
	addw	t2, t2, t3
	li	t3, 47
	ld	t5, 80(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 22
	mulw	t3, a7, t3
	addw	t2, t2, t3
	slliw	t3, s2, 2
	ld	t5, 120(sp)
	addw	t3, t5, t3
	subw	t2, t2, t3
	li	t3, 38
	ld	t5, 136(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 29
	ld	t5, 144(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 115
	ld	t5, 160(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, -121
	ld	t5, 320(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	ld	t5, 176(sp)
	slliw	t3, t5, 5
	ld	t5, 168(sp)
	addw	t3, t3, t5
	addw	t2, t2, t3
	li	t3, -49
	mulw	t3, s1, t3
	addw	t2, t2, t3
	li	t3, 85
	mulw	t0, t0, t3
	addw	t0, t2, t0
	li	t2, 127
	blt	t2, t0, .model.bb48
.model.bb49:
	blt	t0, zero, .model.bb50
.model.bb51:
.model.bb47:
	li	t2, 46
	mulw	t0, t0, t2
	addw	t0, t1, t0
	li	t1, 0
	slt	t0, t1, t0
.model.bb1:
	mv	a0, t0
	ld	s6, 408(sp)
	ld	s5, 400(sp)
	ld	s4, 392(sp)
	ld	s3, 384(sp)
	ld	s2, 376(sp)
	ld	s1, 368(sp)
	addi	sp, sp, 416
	ret
.model.bb8:
	li	t3, 127
	j	.model.bb7
.model.bb13:
	li	t3, 127
	j	.model.bb12
.model.bb18:
	li	t3, 127
	j	.model.bb17
.model.bb23:
	li	t3, 127
	j	.model.bb22
.model.bb28:
	li	t3, 127
	j	.model.bb27
.model.bb33:
	li	t3, 127
	j	.model.bb32
.model.bb38:
	li	t3, 127
	j	.model.bb37
.model.bb43:
	li	t3, 127
	j	.model.bb42
.model.bb48:
	li	t0, 127
	j	.model.bb47
.model.bb5:
	li	t1, 0
	j	.model.bb2
.model.bb10:
	li	t3, 0
	j	.model.bb7
.model.bb15:
	li	t3, 0
	j	.model.bb12
.model.bb20:
	li	t3, 0
	j	.model.bb17
.model.bb25:
	li	t3, 0
	j	.model.bb22
.model.bb30:
	li	t3, 0
	j	.model.bb27
.model.bb35:
	li	t3, 0
	j	.model.bb32
.model.bb40:
	li	t3, 0
	j	.model.bb37
.model.bb45:
	li	t3, 0
	j	.model.bb42
.model.bb50:
	li	t0, 0
	j	.model.bb47
.model.bb3:
	li	t1, 127
	j	.model.bb2
	.size model, .-model

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -160
	sd	ra, 112(sp)
	sd	s1, 120(sp)
	sd	s2, 128(sp)
	sd	s3, 136(sp)
	sd	s4, 144(sp)
	call	getint
	mv	t0, a0
	mv	s1, t0	# phi-copy
.main.bb1:
	bge	zero, s1, .main.bb3
.main.bb2:
	li	s3, 0
	li	s2, 5
.main.bb4:
	bge	s3, s2, .main.bb6
.main.bb5:
	slliw	t0, s3, 4
	slliw	t1, s3, 2
	addw	s4, t0, t1
	call	getint
	mv	t0, a0
	addi	t1, sp, 8
	add	s4, t1, s4
	sw	t0, 0(s4)
	call	getint
	mv	t0, a0
	sw	t0, 4(s4)
	call	getint
	mv	t0, a0
	sw	t0, 8(s4)
	call	getint
	mv	t0, a0
	sw	t0, 12(s4)
	call	getint
	mv	t0, a0
	sw	t0, 16(s4)
	addiw	s3, s3, 1
	blt	s3, s2, .main.bb5	# rotated-loop-branch
.main.bb6:
	addi	t5, sp, 8
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	model
	mv	t0, a0
	bne	t0, zero, .main.bb7
.main.bb8:
	li	t5, 100
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 111
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 103
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
.main.bb9:
	addiw	s1, s1, -1
	blt	zero, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 0
	ld	s4, 144(sp)
	ld	s3, 136(sp)
	ld	s2, 128(sp)
	ld	s1, 120(sp)
	ld	ra, 112(sp)
	addi	sp, sp, 160
	ret
.main.bb7:
	li	t5, 99
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 97
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 116
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb9
	.size main, .-main
