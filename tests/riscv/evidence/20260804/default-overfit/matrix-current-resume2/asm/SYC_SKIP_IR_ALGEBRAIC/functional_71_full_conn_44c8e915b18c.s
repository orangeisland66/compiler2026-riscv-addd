
  .text

  .globl model
  .type  model, @function
model:
.model.bb0:
	addi	sp, sp, -464
	sd	s1, 368(sp)
	sd	s2, 376(sp)
	sd	s3, 384(sp)
	sd	s4, 392(sp)
	sd	s5, 400(sp)
	sd	s6, 408(sp)
	sd	s7, 416(sp)
	sd	s8, 424(sp)
	sd	s9, 432(sp)
	sd	s10, 440(sp)
	sd	s11, 448(sp)
	mv	t0, a0
	lw	t2, 0(t0)
	li	t3, 85
	mulw	t3, t2, t3
	lw	t5, 4(t0)
	sd	t5, 144(sp)
	li	a3, 23
	mulw	a3, t5, a3
	addw	t3, t3, a3
	lw	a3, 8(t0)
	li	a4, -82
	mulw	a4, a3, a4
	addw	t3, t3, a4
	lw	a4, 12(t0)
	li	a5, -103
	mulw	a5, a4, a5
	addw	t3, t3, a5
	lw	t5, 16(t0)
	sd	t5, 360(sp)
	li	a6, -123
	mulw	a6, t5, a6
	addw	t3, t3, a6
	lw	a6, 20(t0)
	slliw	t5, a6, 6
	sd	t5, 48(sp)
	addw	t3, t3, t5
	lw	t5, 24(t0)
	sd	t5, 232(sp)
	slliw	a0, t5, 7
	slliw	t5, t5, 3
	sd	t5, 24(sp)
	subw	t5, a0, t5
	sd	t5, 0(sp)
	subw	a0, zero, t5
	addw	t3, t3, a0
	lw	t5, 28(t0)
	sd	t5, 280(sp)
	li	a0, 50
	mulw	a0, t5, a0
	addw	t3, t3, a0
	lw	t5, 32(t0)
	sd	t5, 256(sp)
	li	a0, -59
	mulw	a0, t5, a0
	addw	t3, t3, a0
	lw	t5, 36(t0)
	sd	t5, 264(sp)
	li	a0, 47
	mulw	a0, t5, a0
	addw	t3, t3, a0
	lw	t5, 40(t0)
	sd	t5, 248(sp)
	li	a0, -111
	mulw	a0, t5, a0
	addw	t3, t3, a0
	lw	t5, 44(t0)
	sd	t5, 120(sp)
	li	a0, -67
	mulw	t5, t5, a0
	sd	t5, 40(sp)
	addw	t3, t3, t5
	lw	t5, 48(t0)
	sd	t5, 272(sp)
	li	a0, -106
	mulw	a0, t5, a0
	addw	t3, t3, a0
	lw	t5, 52(t0)
	sd	t5, 88(sp)
	li	a0, -75
	mulw	a0, t5, a0
	addw	t3, t3, a0
	lw	a0, 56(t0)
	li	a1, -102
	mulw	a1, a0, a1
	addw	t3, t3, a1
	lw	t5, 60(t0)
	sd	t5, 72(sp)
	slliw	t5, t5, 5
	sd	t5, 16(sp)
	ld	t5, 72(sp)
	slliw	s2, t5, 1
	ld	t5, 16(sp)
	addw	t5, t5, s2
	sd	t5, 8(sp)
	addw	t3, t3, t5
	lw	s2, 64(t0)
	li	a1, -39
	mulw	a1, s2, a1
	addw	t3, t3, a1
	lw	a1, 68(t0)
	slliw	t5, a1, 6
	sd	t5, 32(sp)
	addw	s1, t5, a1
	addw	t3, t3, s1
	lw	t5, 72(t0)
	sd	t5, 56(sp)
	li	s8, 47
	mulw	a7, t5, s8
	addw	t3, t3, a7
	lw	t5, 76(t0)
	sd	t5, 64(sp)
	li	s1, 113
	mulw	a7, t5, s1
	addw	t3, t3, a7
	lw	t5, 80(t0)
	sd	t5, 80(sp)
	li	s11, 110
	mulw	a7, t5, s11
	addw	t3, t3, a7
	lw	t5, 84(t0)
	sd	t5, 96(sp)
	li	s10, 47
	mulw	a7, t5, s10
	addw	t3, t3, a7
	lw	t5, 88(t0)
	sd	t5, 112(sp)
	slliw	t5, t5, 2
	sd	t5, 104(sp)
	subw	s1, zero, t5
	addw	t3, t3, s1
	lw	a7, 92(t0)
	slliw	s7, a7, 6
	slliw	t5, a7, 4
	sd	t5, 128(sp)
	addw	s1, s7, t5
	addw	t3, t3, s1
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
	mulw	t5, t1, t3
	sd	t5, 136(sp)
	li	t3, -106
	mulw	t3, t2, t3
	ld	t5, 144(sp)
	slliw	t1, t5, 7
	ld	t5, 144(sp)
	slliw	a2, t5, 1
	subw	t1, t1, a2
	addw	t1, t3, t1
	slliw	t3, a3, 4
	slliw	t5, a3, 1
	sd	t5, 160(sp)
	addw	t3, t3, t5
	subw	t3, zero, t3
	addw	t1, t1, t3
	slliw	t5, a4, 5
	sd	t5, 152(sp)
	subw	t3, t5, a4
	subw	t3, zero, t3
	addw	t1, t1, t3
	ld	t5, 360(sp)
	slliw	t5, t5, 3
	sd	t5, 168(sp)
	subw	a2, zero, t5
	addw	t1, t1, a2
	li	a2, 47
	mulw	a2, a6, a2
	addw	t1, t1, a2
	ld	t5, 232(sp)
	slliw	a2, t5, 2
	subw	a2, zero, a2
	addw	t1, t1, a2
	li	a2, 67
	ld	t5, 280(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	li	a2, -94
	ld	t5, 256(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	li	a2, -121
	ld	t5, 264(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	ld	t5, 248(sp)
	slliw	a2, t5, 3
	ld	t5, 248(sp)
	subw	a2, a2, t5
	addw	t1, t1, a2
	li	a2, -21
	ld	t5, 120(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	ld	t5, 272(sp)
	slliw	a2, t5, 6
	ld	t5, 272(sp)
	slliw	s1, t5, 2
	subw	a2, a2, s1
	subw	a2, zero, a2
	addw	t1, t1, a2
	li	a2, -43
	ld	t5, 88(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	li	a2, 105
	mulw	a2, a0, a2
	addw	t1, t1, a2
	li	a2, -42
	ld	t5, 72(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	li	a2, 87
	mulw	a2, s2, a2
	addw	t1, t1, a2
	li	a2, 29
	mulw	a2, a1, a2
	addw	t1, t1, a2
	li	a2, -106
	ld	t5, 56(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	ld	t5, 64(sp)
	slliw	a2, t5, 5
	ld	t5, 64(sp)
	subw	a2, a2, t5
	subw	a2, zero, a2
	addw	t1, t1, a2
	li	a2, -110
	ld	t5, 80(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	li	a2, -100
	ld	t5, 96(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	li	a2, -22
	ld	t5, 112(sp)
	mulw	a2, t5, a2
	addw	t1, t1, a2
	li	a2, -75
	mulw	a2, a7, a2
	addw	t1, t1, a2
	li	a2, -125
	mulw	a2, t0, a2
	addw	t1, t1, a2
	li	a2, 127
	blt	a2, t1, .model.bb8
.model.bb9:
	blt	t1, zero, .model.bb10
.model.bb11:
.model.bb7:
	li	a2, 77
	mulw	t1, t1, a2
	ld	t5, 136(sp)
	addw	t1, t5, t1
	li	a2, 26
	mulw	a2, t2, a2
	li	t3, 76
	ld	t5, 144(sp)
	mulw	t3, t5, t3
	addw	t3, a2, t3
	li	a2, -70
	mulw	t5, a3, a2
	sd	t5, 176(sp)
	addw	t3, t3, t5
	li	a2, 29
	mulw	a2, a4, a2
	addw	t3, t3, a2
	li	a2, -95
	ld	t5, 360(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	slliw	t5, a6, 5
	sd	t5, 184(sp)
	ld	t5, 48(sp)
	ld	t6, 184(sp)
	addw	a2, t5, t6
	addw	t3, t3, a2
	li	a2, 52
	ld	t5, 232(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 280(sp)
	slliw	a2, t5, 6
	ld	t5, 280(sp)
	slliw	s7, t5, 2
	addw	a2, a2, s7
	subw	a2, zero, a2
	addw	t3, t3, a2
	ld	t5, 256(sp)
	slliw	t5, t5, 2
	sd	t5, 192(sp)
	ld	t6, 256(sp)
	addw	a2, t5, t6
	subw	a2, zero, a2
	addw	t3, t3, a2
	ld	t5, 264(sp)
	slliw	a2, t5, 5
	ld	t5, 264(sp)
	slliw	t5, t5, 1
	sd	t5, 208(sp)
	addw	a2, a2, t5
	addw	t3, t3, a2
	ld	t5, 248(sp)
	slliw	a2, t5, 5
	ld	t5, 248(sp)
	slliw	t5, t5, 1
	sd	t5, 216(sp)
	addw	a2, a2, t5
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, 102
	ld	t5, 120(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 272(sp)
	slliw	a2, t5, 1
	addw	a2, s1, a2
	addw	t3, t3, a2
	li	a2, -38
	ld	t5, 88(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, 27
	mulw	a2, a0, a2
	addw	t3, t3, a2
	li	a2, 110
	ld	t5, 72(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, 116
	mulw	a2, s2, a2
	addw	t3, t3, a2
	li	a2, 39
	mulw	a2, a1, a2
	addw	t3, t3, a2
	ld	t5, 56(sp)
	slliw	a2, t5, 6
	ld	t5, 56(sp)
	subw	a2, a2, t5
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, -99
	ld	t5, 64(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 80(sp)
	slliw	a2, t5, 6
	ld	t5, 80(sp)
	addw	t5, a2, t5
	sd	t5, 200(sp)
	addw	t3, t3, t5
	ld	t5, 96(sp)
	slliw	a2, t5, 7
	ld	t5, 96(sp)
	slliw	s8, t5, 3
	subw	a2, a2, s8
	addw	t3, t3, a2
	li	a2, -39
	ld	t5, 112(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	slliw	a2, a7, 2
	slliw	s10, a7, 1
	addw	a2, a2, s10
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, 94
	mulw	a2, t0, a2
	addw	t3, t3, a2
	li	a2, 127
	blt	a2, t3, .model.bb13
.model.bb14:
	blt	t3, zero, .model.bb15
.model.bb16:
.model.bb12:
	slliw	a2, t3, 7
	subw	t3, a2, t3
	addw	t1, t1, t3
	li	t3, -23
	mulw	t3, t2, t3
	ld	t5, 144(sp)
	slliw	t5, t5, 6
	sd	t5, 224(sp)
	ld	t6, 144(sp)
	subw	a2, t5, t6
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, 49
	mulw	a2, a3, a2
	addw	t3, t3, a2
	li	a2, 50
	mulw	a2, a4, a2
	addw	t3, t3, a2
	ld	t5, 360(sp)
	slliw	a2, t5, 6
	ld	t5, 168(sp)
	addw	a2, a2, t5
	addw	t3, t3, a2
	li	a2, 85
	mulw	a2, a6, a2
	addw	t3, t3, a2
	ld	t5, 232(sp)
	slliw	t5, t5, 5
	sd	t5, 240(sp)
	ld	t5, 232(sp)
	slliw	t4, t5, 1
	ld	t5, 240(sp)
	subw	t4, t5, t4
	subw	t4, zero, t4
	addw	t3, t3, t4
	ld	t5, 280(sp)
	slliw	t4, t5, 3
	addw	t4, t4, s7
	addw	t3, t3, t4
	li	t4, 125
	ld	t5, 256(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	a2, -117
	ld	t5, 264(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 248(sp)
	slliw	t5, t5, 6
	sd	t5, 288(sp)
	ld	t6, 248(sp)
	addw	s6, t5, t6
	subw	s4, zero, s6
	addw	t3, t3, s4
	ld	t5, 40(sp)
	addw	t3, t3, t5
	li	s5, 125
	ld	t5, 272(sp)
	mulw	s9, t5, s5
	addw	t3, t3, s9
	li	s3, 110
	ld	t5, 88(sp)
	mulw	a2, t5, s3
	addw	t3, t3, a2
	slliw	a2, a0, 5
	subw	a2, a2, a0
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, -123
	ld	t5, 72(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, 83
	mulw	a2, s2, a2
	addw	t3, t3, a2
	li	a2, 122
	mulw	a2, a1, a2
	addw	t3, t3, a2
	li	a2, 11
	ld	t5, 56(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, -23
	ld	t5, 64(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, -47
	ld	t5, 80(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 96(sp)
	slliw	t5, t5, 5
	sd	t5, 296(sp)
	subw	a2, zero, t5
	addw	t3, t3, a2
	li	a2, -117
	ld	t5, 112(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, 95
	mulw	a2, a7, a2
	addw	t3, t3, a2
	li	a2, 118
	mulw	a2, t0, a2
	addw	t3, t3, a2
	li	a2, 127
	blt	a2, t3, .model.bb18
.model.bb19:
	blt	t3, zero, .model.bb20
.model.bb21:
.model.bb17:
	li	a2, -106
	mulw	t3, t3, a2
	addw	t1, t1, t3
	slliw	t3, t2, 3
	li	a2, 82
	ld	t5, 144(sp)
	mulw	t5, t5, a2
	sd	t5, 304(sp)
	addw	t3, t3, t5
	li	a2, -104
	mulw	a2, a3, a2
	addw	t3, t3, a2
	li	a2, 101
	mulw	a2, a4, a2
	addw	t3, t3, a2
	li	a2, -116
	ld	t5, 360(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 48(sp)
	subw	a2, t5, a6
	subw	a2, zero, a2
	addw	t3, t3, a2
	ld	t5, 232(sp)
	slliw	a2, t5, 4
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, -70
	ld	t5, 280(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	addw	t3, t3, t4
	li	a2, 75
	ld	t5, 264(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 288(sp)
	ld	t6, 216(sp)
	addw	a2, t5, t6
	addw	t3, t3, a2
	ld	t5, 120(sp)
	slliw	a2, t5, 6
	ld	t5, 120(sp)
	slliw	t4, t5, 5
	addw	a2, a2, t4
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, -101
	ld	t5, 272(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, -114
	ld	t5, 88(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, 59
	mulw	a2, a0, a2
	addw	t3, t3, a2
	ld	t5, 72(sp)
	slliw	t5, t5, 3
	sd	t5, 312(sp)
	ld	t5, 72(sp)
	slliw	t4, t5, 2
	ld	t5, 312(sp)
	addw	t4, t5, t4
	addw	t3, t3, t4
	slliw	t4, s2, 2
	addw	t4, t4, s2
	addw	t3, t3, t4
	li	t4, -95
	mulw	t4, a1, t4
	addw	t3, t3, t4
	li	t4, 116
	ld	t5, 56(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -93
	ld	t5, 64(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	ld	t5, 80(sp)
	slliw	t5, t5, 4
	sd	t5, 320(sp)
	ld	t6, 80(sp)
	subw	a2, t5, t6
	addw	t3, t3, a2
	li	a2, 79
	ld	t5, 96(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 112(sp)
	slliw	a2, t5, 1
	ld	t5, 112(sp)
	addw	a2, a2, t5
	addw	t3, t3, a2
	li	a2, 49
	mulw	a2, a7, a2
	addw	t3, t3, a2
	slliw	a2, t0, 7
	slliw	t4, t0, 2
	subw	a2, a2, t4
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, 127
	blt	a2, t3, .model.bb23
.model.bb24:
	blt	t3, zero, .model.bb25
.model.bb26:
.model.bb22:
	slliw	a2, t3, 1
	addw	t3, a2, t3
	subw	t3, zero, t3
	addw	t1, t1, t3
	li	t3, 81
	mulw	t3, t2, t3
	ld	t5, 144(sp)
	slliw	a2, t5, 2
	ld	t5, 224(sp)
	addw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, -102
	mulw	a2, a3, a2
	addw	t3, t3, a2
	li	a2, -74
	mulw	a2, a4, a2
	addw	t3, t3, a2
	li	a2, 121
	ld	t5, 360(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	slliw	a2, a6, 4
	subw	a2, a2, a6
	subw	t5, zero, a2
	sd	t5, 336(sp)
	addw	t3, t3, t5
	li	t4, 55
	ld	t5, 232(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, 101
	ld	t5, 280(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -13
	ld	t5, 256(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	ld	t5, 264(sp)
	slliw	t4, t5, 6
	ld	t5, 208(sp)
	subw	t4, t4, t5
	subw	t5, zero, t4
	sd	t5, 328(sp)
	addw	t3, t3, t5
	ld	t5, 288(sp)
	addw	t3, t3, t5
	li	t4, 114
	ld	t5, 120(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, 38
	ld	t5, 272(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -21
	ld	t5, 88(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	slliw	t4, a0, 7
	slliw	a2, a0, 4
	subw	a2, t4, a2
	addw	t3, t3, a2
	li	a2, 114
	ld	t5, 72(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	slliw	a2, s2, 7
	slliw	t5, s2, 4
	sd	t5, 344(sp)
	subw	a2, a2, t5
	addw	t3, t3, a2
	slliw	a2, a1, 3
	slliw	t4, a1, 1
	addw	a2, a2, t4
	subw	a2, zero, a2
	addw	t3, t3, a2
	ld	t5, 56(sp)
	slliw	a2, t5, 4
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, -50
	ld	t5, 64(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 80(sp)
	slliw	a2, t5, 7
	ld	t5, 320(sp)
	subw	a2, a2, t5
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a2, -116
	ld	t5, 96(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, -54
	ld	t5, 112(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, 82
	mulw	a2, a7, a2
	addw	t3, t3, a2
	slliw	a2, t0, 6
	slliw	t4, t0, 3
	addw	t4, a2, t4
	subw	t4, zero, t4
	addw	t3, t3, t4
	li	t4, 127
	blt	t4, t3, .model.bb28
.model.bb29:
	blt	t3, zero, .model.bb30
.model.bb31:
.model.bb27:
	slliw	t3, t3, 5
	addw	t1, t1, t3
	slliw	t3, t2, 4
	subw	t3, t3, t2
	li	t4, -77
	ld	t5, 144(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	slliw	t4, a3, 6
	ld	t5, 160(sp)
	addw	t4, t4, t5
	addw	t3, t3, t4
	li	t4, -90
	mulw	t4, a4, t4
	addw	t3, t3, t4
	ld	t5, 360(sp)
	slliw	t4, t5, 2
	ld	t5, 360(sp)
	slliw	s1, t5, 1
	addw	t4, t4, s1
	subw	t4, zero, t4
	addw	t3, t3, t4
	slliw	t5, a6, 1
	sd	t5, 352(sp)
	ld	t5, 184(sp)
	ld	t6, 352(sp)
	subw	t4, t5, t6
	subw	t4, zero, t4
	addw	t3, t3, t4
	ld	t5, 24(sp)
	subw	t4, zero, t5
	addw	t3, t3, t4
	li	t4, 81
	ld	t5, 280(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	ld	t5, 256(sp)
	slliw	t4, t5, 1
	addw	t3, t3, t4
	li	a5, -110
	ld	t5, 264(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -95
	ld	t5, 248(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 59
	ld	t5, 120(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 52
	ld	t5, 272(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 88(sp)
	slliw	a5, t5, 4
	ld	t5, 88(sp)
	subw	a5, a5, t5
	addw	t3, t3, a5
	li	a5, 55
	mulw	a5, a0, a5
	addw	t3, t3, a5
	ld	t5, 16(sp)
	ld	t6, 72(sp)
	addw	a5, t5, t6
	subw	a5, zero, a5
	addw	t3, t3, a5
	slliw	a5, s2, 1
	ld	t5, 344(sp)
	subw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 58
	mulw	a5, a1, a5
	addw	t3, t3, a5
	li	a5, 67
	ld	t5, 56(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 86
	ld	t5, 64(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -79
	ld	t5, 80(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 96(sp)
	slliw	a5, t5, 4
	ld	t5, 296(sp)
	addw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -13
	ld	t5, 112(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 128(sp)
	subw	a5, t5, a7
	subw	a5, zero, a5
	addw	t3, t3, a5
	slliw	a5, t0, 1
	addw	a2, a2, a5
	addw	t3, t3, a2
	li	a2, 127
	blt	a2, t3, .model.bb33
.model.bb34:
	blt	t3, zero, .model.bb35
.model.bb36:
.model.bb32:
	li	a2, -95
	mulw	t3, t3, a2
	addw	t1, t1, t3
	slliw	t3, t2, 5
	addw	t3, t3, t2
	ld	t5, 304(sp)
	addw	t3, t3, t5
	li	a2, 67
	mulw	a2, a3, a2
	addw	t3, t3, a2
	slliw	a2, a4, 1
	ld	t5, 152(sp)
	subw	a2, t5, a2
	addw	t3, t3, a2
	subw	a2, zero, s1
	addw	t3, t3, a2
	ld	t5, 48(sp)
	addw	a2, t5, a6
	addw	t3, t3, a2
	ld	t5, 0(sp)
	addw	t3, t3, t5
	li	a2, -13
	ld	t5, 280(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 256(sp)
	slliw	a2, t5, 4
	addw	a2, a2, t4
	addw	t3, t3, a2
	ld	t5, 264(sp)
	slliw	a2, t5, 2
	ld	t5, 264(sp)
	addw	a2, a2, t5
	addw	t3, t3, a2
	li	a2, 104
	ld	t5, 248(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	li	a2, -119
	ld	t5, 120(sp)
	mulw	a2, t5, a2
	addw	t3, t3, a2
	ld	t5, 272(sp)
	slliw	a2, t5, 3
	ld	t5, 272(sp)
	subw	a2, a2, t5
	subw	a2, zero, a2
	addw	t3, t3, a2
	li	a5, 71
	ld	t5, 88(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 107
	mulw	a5, a0, a5
	addw	t3, t3, a5
	ld	t5, 72(sp)
	slliw	a5, t5, 4
	ld	t5, 312(sp)
	addw	a5, a5, t5
	addw	t3, t3, a5
	li	a5, 82
	mulw	a5, s2, a5
	addw	t3, t3, a5
	slliw	a5, a1, 5
	ld	t5, 32(sp)
	addw	a5, t5, a5
	subw	a5, zero, a5
	addw	t3, t3, a5
	li	a5, -104
	ld	t5, 56(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, -121
	ld	t5, 64(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 200(sp)
	addw	t3, t3, t5
	li	a5, 97
	ld	t5, 96(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 83
	ld	t5, 112(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	li	a5, 46
	mulw	a5, a7, a5
	addw	t3, t3, a5
	li	a5, -84
	mulw	a5, t0, a5
	addw	t3, t3, a5
	li	a5, 127
	blt	a5, t3, .model.bb38
.model.bb39:
	blt	t3, zero, .model.bb40
.model.bb41:
.model.bb37:
	li	a5, -50
	mulw	t3, t3, a5
	addw	t1, t1, t3
	li	t3, -29
	mulw	t3, t2, t3
	ld	t5, 144(sp)
	slliw	a5, t5, 3
	ld	t5, 144(sp)
	subw	a5, a5, t5
	addw	t3, t3, a5
	ld	t5, 176(sp)
	addw	t3, t3, t5
	li	a5, 38
	mulw	a5, a4, a5
	addw	t3, t3, a5
	li	a5, -90
	ld	t5, 360(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 336(sp)
	addw	t3, t3, t5
	ld	t5, 240(sp)
	subw	a5, zero, t5
	addw	t3, t3, a5
	li	a5, 37
	ld	t5, 280(sp)
	mulw	a5, t5, a5
	addw	t3, t3, a5
	ld	t5, 256(sp)
	slliw	a5, t5, 5
	ld	t5, 192(sp)
	addw	t4, a5, t5
	addw	t3, t3, t4
	ld	t5, 328(sp)
	addw	t3, t3, t5
	li	t4, -125
	ld	t5, 248(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -46
	ld	t5, 120(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -70
	ld	t5, 272(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, 37
	ld	t5, 88(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -73
	mulw	t4, a0, t4
	addw	t3, t3, t4
	ld	t5, 8(sp)
	subw	t4, zero, t5
	addw	t3, t3, t4
	li	t4, -87
	mulw	t4, s2, t4
	addw	t3, t3, t4
	li	t4, -75
	mulw	t4, a1, t4
	addw	t3, t3, t4
	li	t4, 71
	ld	t5, 56(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -77
	ld	t5, 64(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, 53
	ld	t5, 80(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, 37
	ld	t5, 96(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -103
	ld	t5, 112(sp)
	mulw	t4, t5, t4
	addw	t3, t3, t4
	li	t4, -13
	mulw	t4, a7, t4
	addw	t3, t3, t4
	li	t4, -114
	mulw	t4, t0, t4
	addw	t3, t3, t4
	li	t4, 127
	blt	t4, t3, .model.bb43
.model.bb44:
	blt	t3, zero, .model.bb45
.model.bb46:
.model.bb42:
	li	t4, -23
	mulw	t3, t3, t4
	addw	t1, t1, t3
	li	t3, 67
	mulw	t2, t2, t3
	li	t3, 42
	ld	t5, 144(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 41
	mulw	t3, a3, t3
	addw	t2, t2, t3
	li	t3, -123
	mulw	t3, a4, t3
	addw	t2, t2, t3
	li	t3, -92
	ld	t5, 360(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	slliw	t3, a6, 3
	ld	t5, 352(sp)
	addw	t3, t3, t5
	addw	t2, t2, t3
	li	t3, -77
	ld	t5, 232(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 75
	ld	t5, 280(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	ld	t5, 256(sp)
	slliw	t3, t5, 6
	addw	t3, t3, a5
	addw	t2, t2, t3
	li	t3, -51
	ld	t5, 264(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 109
	ld	t5, 248(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, -74
	ld	t5, 120(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	addw	t2, t2, a2
	li	t3, -122
	ld	t5, 88(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 67
	mulw	t3, a0, t3
	addw	t2, t2, t3
	li	t3, 47
	ld	t5, 72(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 22
	mulw	t3, s2, t3
	addw	t2, t2, t3
	slliw	t3, a1, 2
	ld	t5, 32(sp)
	addw	t3, t5, t3
	subw	t3, zero, t3
	addw	t2, t2, t3
	li	t3, 38
	ld	t5, 56(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 29
	ld	t5, 64(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, 115
	ld	t5, 80(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	li	t3, -121
	ld	t5, 96(sp)
	mulw	t3, t5, t3
	addw	t2, t2, t3
	ld	t5, 112(sp)
	slliw	t3, t5, 5
	ld	t5, 104(sp)
	addw	t3, t3, t5
	addw	t2, t2, t3
	li	t3, -49
	mulw	t3, a7, t3
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
	ld	s11, 448(sp)
	ld	s10, 440(sp)
	ld	s9, 432(sp)
	ld	s8, 424(sp)
	ld	s7, 416(sp)
	ld	s6, 408(sp)
	ld	s5, 400(sp)
	ld	s4, 392(sp)
	ld	s3, 384(sp)
	ld	s2, 376(sp)
	ld	s1, 368(sp)
	addi	sp, sp, 464
	ret
.model.bb8:
	li	t1, 127
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
	li	t1, 0
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
