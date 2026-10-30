
  .bss
  .globl k
  .align 2
  .type  k, @object
  .size  k, 4
k:
  .zero 4

  .text

  .globl params_f40
  .type  params_f40, @function
params_f40:
.params_f40.bb0:
	addi	sp, sp, -272
	sd	s0, 200(sp)
	sd	ra, 208(sp)
	sd	s1, 216(sp)
	fsw	fs0, 224(sp)
	fsw	fs1, 228(sp)
	fsw	fs2, 232(sp)
	fsw	fs3, 236(sp)
	fsw	fs4, 240(sp)
	fsw	fs5, 244(sp)
	fsw	fs6, 248(sp)
	fsw	fs7, 252(sp)
	fsw	fs8, 256(sp)
	fsw	fs9, 260(sp)
	fsw	fs10, 264(sp)
	fsw	fs11, 268(sp)
	addi	s0, sp, 272
	fmv.s	ft10, fa0
	fsw	ft10, 16(sp)
	fmv.s	ft10, fa1
	fsw	ft10, 20(sp)
	fmv.s	ft10, fa2
	fsw	ft10, 24(sp)
	fmv.s	ft10, fa3
	fsw	ft10, 28(sp)
	fmv.s	fs5, fa4
	fmv.s	fs6, fa5
	fmv.s	fs7, fa6
	fmv.s	fs8, fa7
	fmv.w.x	fs9, a0
	fmv.w.x	fs10, a1
	fmv.w.x	fs11, a2
	fmv.w.x	ft10, a3
	fsw	ft10, 60(sp)
	fmv.w.x	ft10, a4
	fsw	ft10, 64(sp)
	fmv.w.x	ft10, a5
	fsw	ft10, 68(sp)
	fmv.w.x	ft10, a6
	fsw	ft10, 72(sp)
	fmv.w.x	ft10, a7
	fsw	ft10, 76(sp)
	flw	ft10, 0(s0)
	fsw	ft10, 80(sp)
	flw	ft10, 8(s0)
	fsw	ft10, 84(sp)
	flw	ft10, 16(s0)
	fsw	ft10, 88(sp)
	flw	ft10, 24(s0)
	fsw	ft10, 92(sp)
	flw	ft10, 32(s0)
	fsw	ft10, 96(sp)
	flw	ft2, 40(s0)
	flw	ft3, 48(s0)
	flw	ft4, 56(s0)
	flw	ft5, 64(s0)
	flw	ft6, 72(s0)
	flw	ft7, 80(s0)
	flw	ft8, 88(s0)
	flw	fa2, 96(s0)
	flw	fa3, 104(s0)
	flw	fa4, 112(s0)
	flw	fa5, 120(s0)
	flw	fa6, 128(s0)
	flw	fa7, 136(s0)
	flw	fs0, 144(s0)
	flw	fs1, 152(s0)
	flw	fs2, 160(s0)
	flw	fs3, 168(s0)
	flw	ft0, 176(s0)
	flw	ft1, 184(s0)
	li	t0, 0
	fmv.w.x	ft10, t0
	fsw	ft10, 100(sp)
	fsw	ft5, 104(sp)
	flw	ft5, 80(sp)
	fsw	ft6, 156(sp)
	flw	ft6, 84(sp)
	fsw	ft7, 152(sp)
	flw	ft7, 88(sp)
	fsw	fa2, 108(sp)
	flw	fa2, 96(sp)
	fsw	fa3, 112(sp)
	fmv.s	fa3, ft2	# phi-copy
	flw	ft2, 68(sp)
	fsw	fa4, 116(sp)
	fmv.s	fa4, ft3	# phi-copy
	flw	ft3, 72(sp)
	fsw	fa5, 120(sp)
	fmv.s	fa5, ft4	# phi-copy
	flw	ft4, 76(sp)
	fsw	fa6, 124(sp)
	fmv.s	fa6, ft8	# phi-copy
	flw	ft8, 92(sp)
	fsw	fa7, 128(sp)
	fsw	fs0, 132(sp)
	flw	fs0, 16(sp)
	fsw	fs1, 136(sp)
	flw	fs1, 20(sp)
	fsw	fs2, 140(sp)
	flw	fs2, 24(sp)
	fsw	fs3, 144(sp)
	flw	fs3, 28(sp)
	fsw	ft0, 148(sp)
	flw	ft0, 60(sp)
	fmv.s	fs4, ft1	# phi-copy
	flw	ft1, 64(sp)
.params_f40.bb1:
	flw	ft10, 100(sp)
	feq.s	t0, fs4, ft10
	beqz	t0, .params_f40.bb2
.params_f40.bb3:
	fadd.s	fa7, fs0, fs1
	fadd.s	fa7, fa7, fs2
	fmv.s	fs0, fs1	# phi-copy
	fmv.s	fs1, fs2	# phi-copy
	fmv.s	fs2, fs3	# phi-copy
	fmv.s	fs3, fs5	# phi-copy
	fmv.s	fs5, fs6	# phi-copy
	fmv.s	fs6, fs7	# phi-copy
	fmv.s	fs7, fs8	# phi-copy
	fmv.s	fs8, fs9	# phi-copy
	fmv.s	fs9, fs10	# phi-copy
	fmv.s	fs10, fs11	# phi-copy
	fmv.s	fs11, ft0	# phi-copy
	fmv.s	ft0, ft1	# phi-copy
	fmv.s	ft1, ft2	# phi-copy
	fmv.s	ft2, ft3	# phi-copy
	fmv.s	ft3, ft4	# phi-copy
	fmv.s	ft4, ft5	# phi-copy
	fmv.s	ft5, ft6	# phi-copy
	fmv.s	ft6, ft7	# phi-copy
	fmv.s	ft7, ft8	# phi-copy
	fmv.s	ft8, fa2	# phi-copy
	fmv.s	fa2, fa3	# phi-copy
	fmv.s	fa3, fa4	# phi-copy
	fmv.s	fa4, fa5	# phi-copy
	flw	fa5, 104(sp)
	flw	ft10, 156(sp)
	fsw	ft10, 104(sp)
	flw	ft10, 152(sp)
	fsw	ft10, 156(sp)
	fsw	fa6, 152(sp)
	flw	fa6, 108(sp)
	flw	ft10, 112(sp)
	fsw	ft10, 108(sp)
	flw	ft10, 116(sp)
	fsw	ft10, 112(sp)
	flw	ft10, 120(sp)
	fsw	ft10, 116(sp)
	flw	ft10, 124(sp)
	fsw	ft10, 120(sp)
	flw	ft10, 128(sp)
	fsw	ft10, 124(sp)
	flw	ft10, 132(sp)
	fsw	ft10, 128(sp)
	flw	ft10, 136(sp)
	fsw	ft10, 132(sp)
	flw	ft10, 140(sp)
	fsw	ft10, 136(sp)
	flw	ft10, 144(sp)
	fsw	ft10, 140(sp)
	flw	ft10, 148(sp)
	fsw	ft10, 144(sp)
	fsw	fs4, 148(sp)
	fmv.s	fs4, fa7	# phi-copy
	j	.params_f40.bb1
.params_f40.bb2:
	addi	t5, sp, 160
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	fadd.s	fa7, fs0, fs1
	fadd.s	fa7, fa7, fs2
	fadd.s	fa7, fa7, fs3
	addi	s1, sp, 160
	fsw	fa7, 0(s1)
	fadd.s	fa7, fs5, fs6
	fadd.s	fa7, fa7, fs7
	fadd.s	fa7, fa7, fs8
	fsw	fa7, 4(s1)
	fadd.s	fa7, fs9, fs10
	fadd.s	fa7, fa7, fs11
	fadd.s	ft0, fa7, ft0
	fsw	ft0, 8(s1)
	fadd.s	ft0, ft1, ft2
	fadd.s	ft0, ft0, ft3
	fadd.s	ft0, ft0, ft4
	fsw	ft0, 12(s1)
	fadd.s	ft0, ft5, ft6
	fadd.s	ft0, ft0, ft7
	fadd.s	ft0, ft0, ft8
	fsw	ft0, 16(s1)
	fadd.s	ft0, fa2, fa3
	fadd.s	ft0, ft0, fa4
	fadd.s	ft0, ft0, fa5
	fsw	ft0, 20(s1)
	flw	ft10, 104(sp)
	flw	ft11, 156(sp)
	fadd.s	ft0, ft10, ft11
	flw	ft10, 152(sp)
	fadd.s	ft0, ft0, ft10
	fadd.s	ft0, ft0, fa6
	fsw	ft0, 24(s1)
	flw	ft10, 108(sp)
	flw	ft11, 112(sp)
	fadd.s	ft0, ft10, ft11
	flw	ft10, 116(sp)
	fadd.s	ft0, ft0, ft10
	flw	ft10, 120(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 28(s1)
	flw	ft10, 124(sp)
	flw	ft11, 128(sp)
	fadd.s	ft0, ft10, ft11
	flw	ft10, 132(sp)
	fadd.s	ft0, ft0, ft10
	flw	ft10, 136(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 32(s1)
	flw	ft10, 140(sp)
	flw	ft11, 144(sp)
	fadd.s	ft0, ft10, ft11
	flw	ft10, 148(sp)
	fadd.s	ft0, ft0, ft10
	fadd.s	ft0, ft0, fs4
	fsw	ft0, 36(s1)
	li	a0, 10
	addi	a1, sp, 160
	call	putfarray
	la	t0, k
	lw	t0, 0(t0)
	slliw	t0, t0, 2
	add	t0, s1, t0
	flw	fa0, 0(t0)
	flw	fs11, 268(sp)
	flw	fs10, 264(sp)
	flw	fs9, 260(sp)
	flw	fs8, 256(sp)
	flw	fs7, 252(sp)
	flw	fs6, 248(sp)
	flw	fs5, 244(sp)
	flw	fs4, 240(sp)
	flw	fs3, 236(sp)
	flw	fs2, 232(sp)
	flw	fs1, 228(sp)
	flw	fs0, 224(sp)
	ld	s1, 216(sp)
	ld	ra, 208(sp)
	ld	s0, 200(sp)
	addi	sp, sp, 272
	ret
	.size params_f40, .-params_f40

  .globl params_f40_i24
  .type  params_f40_i24, @function
params_f40_i24:
.params_f40_i24.bb0:
	addi	sp, sp, -464
	sd	s0, 304(sp)
	sd	ra, 312(sp)
	sd	s1, 320(sp)
	sd	s2, 328(sp)
	sd	s3, 336(sp)
	sd	s4, 344(sp)
	sd	s5, 352(sp)
	sd	s6, 360(sp)
	sd	s7, 368(sp)
	sd	s8, 376(sp)
	sd	s9, 384(sp)
	sd	s10, 392(sp)
	sd	s11, 400(sp)
	fsw	fs0, 408(sp)
	fsw	fs1, 412(sp)
	fsw	fs2, 416(sp)
	fsw	fs3, 420(sp)
	fsw	fs4, 424(sp)
	fsw	fs5, 428(sp)
	fsw	fs6, 432(sp)
	fsw	fs7, 436(sp)
	fsw	fs8, 440(sp)
	fsw	fs9, 444(sp)
	fsw	fs10, 448(sp)
	fsw	fs11, 452(sp)
	addi	s0, sp, 464
	mv	t5, a0
	sd	t5, 16(sp)
	mv	t5, a1
	sd	t5, 24(sp)
	mv	t5, a2
	sd	t5, 32(sp)
	fmv.s	ft10, fa0
	fsw	ft10, 120(sp)
	mv	t5, a3
	sd	t5, 40(sp)
	mv	t5, a4
	sd	t5, 48(sp)
	mv	t5, a5
	sd	t5, 56(sp)
	fmv.s	ft10, fa1
	fsw	ft10, 124(sp)
	fmv.s	ft10, fa2
	fsw	ft10, 128(sp)
	fmv.s	ft10, fa3
	fsw	ft10, 132(sp)
	mv	t5, a6
	sd	t5, 64(sp)
	fmv.s	ft10, fa4
	fsw	ft10, 136(sp)
	fmv.s	ft10, fa5
	fsw	ft10, 140(sp)
	mv	t5, a7
	sd	t5, 72(sp)
	fmv.s	ft10, fa6
	fsw	ft10, 144(sp)
	lw	a6, 0(s0)
	fmv.s	ft10, fa7
	fsw	ft10, 148(sp)
	flw	ft10, 8(s0)
	fsw	ft10, 152(sp)
	flw	ft10, 16(s0)
	fsw	ft10, 156(sp)
	flw	ft10, 24(s0)
	fsw	ft10, 160(sp)
	flw	ft10, 32(s0)
	fsw	ft10, 164(sp)
	flw	ft10, 40(s0)
	fsw	ft10, 168(sp)
	lw	a7, 48(s0)
	flw	fs11, 56(s0)
	lw	t4, 64(s0)
	lw	s1, 72(s0)
	flw	fs10, 80(s0)
	flw	fs0, 88(s0)
	flw	fs1, 96(s0)
	flw	fs2, 104(s0)
	flw	fs3, 112(s0)
	lw	s2, 120(s0)
	flw	fs4, 128(s0)
	lw	s3, 136(s0)
	flw	fs5, 144(s0)
	flw	fs6, 152(s0)
	flw	fs7, 160(s0)
	flw	fs8, 168(s0)
	lw	s4, 176(s0)
	lw	s5, 184(s0)
	flw	fs9, 192(s0)
	flw	ft0, 200(s0)
	flw	ft1, 208(s0)
	lw	t0, 216(s0)
	flw	ft2, 224(s0)
	lw	t1, 232(s0)
	lw	t2, 240(s0)
	flw	ft3, 248(s0)
	flw	ft4, 256(s0)
	flw	ft5, 264(s0)
	flw	ft6, 272(s0)
	lw	t3, 280(s0)
	lw	a2, 288(s0)
	lw	a3, 296(s0)
	flw	ft7, 304(s0)
	flw	ft8, 312(s0)
	flw	fa2, 320(s0)
	flw	fa3, 328(s0)
	flw	fa4, 336(s0)
	flw	fa5, 344(s0)
	lw	a4, 352(s0)
	flw	fa6, 360(s0)
	lw	a5, 368(s0)
	flw	fa7, 376(s0)
	sd	a3, 88(sp)
	sd	a2, 80(sp)
	sd	s4, 104(sp)
	mv	s6, s1	# phi-copy
	ld	s1, 40(sp)
	ld	s7, 72(sp)
	ld	t5, 64(sp)
	sd	t5, 96(sp)
	ld	s9, 56(sp)
	ld	s10, 48(sp)
	ld	s11, 32(sp)
	ld	a2, 24(sp)
	ld	t5, 16(sp)
	sd	t5, 112(sp)
	fsw	fa7, 180(sp)
	fsw	fa5, 204(sp)
	fsw	fa4, 184(sp)
	fsw	fa3, 188(sp)
	fsw	fa2, 196(sp)
	fsw	ft8, 224(sp)
	fsw	ft7, 228(sp)
	fsw	ft6, 192(sp)
	fsw	ft4, 212(sp)
	fsw	ft3, 216(sp)
	fsw	ft2, 200(sp)
	fsw	fs8, 208(sp)
	fsw	fs4, 220(sp)
	flw	fa7, 168(sp)
	flw	fa4, 164(sp)
	flw	fa3, 160(sp)
	flw	ft6, 156(sp)
	flw	fa2, 152(sp)
	flw	ft2, 148(sp)
	flw	fa5, 144(sp)
	flw	fs8, 140(sp)
	flw	ft4, 136(sp)
	flw	ft3, 132(sp)
	flw	fs4, 128(sp)
	flw	ft8, 124(sp)
	flw	ft7, 120(sp)
.params_f40_i24.bb1:
	ld	t5, 112(sp)
	bne	t5, zero, .params_f40_i24.bb3
.params_f40_i24.bb4:
	sd	s1, 112(sp)
	j	.params_f40_i24.bb1
.params_f40_i24.bb3:
	addi	t5, sp, 264
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	fadd.s	ft1, ft1, ft6
	fadd.s	ft1, ft1, fa6
	fadd.s	ft1, ft1, ft4
	addi	s8, sp, 264
	fsw	ft1, 0(s8)
	fadd.s	ft1, ft7, fs7
	fadd.s	ft1, ft1, fa7
	fadd.s	ft1, ft1, ft3
	fsw	ft1, 4(s8)
	fadd.s	ft1, ft8, fs2
	fadd.s	ft0, ft1, ft0
	fadd.s	ft0, ft0, fa3
	fsw	ft0, 8(s8)
	fadd.s	ft0, fa2, ft5
	fadd.s	ft0, ft0, fs9
	fadd.s	ft0, ft0, fs4
	fsw	ft0, 12(s8)
	flw	ft10, 204(sp)
	fadd.s	ft0, fs6, ft10
	flw	ft10, 228(sp)
	fadd.s	ft0, ft0, ft10
	flw	ft10, 224(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 16(s8)
	fadd.s	ft0, fs10, fs5
	flw	ft10, 196(sp)
	fadd.s	ft0, ft0, ft10
	fadd.s	ft0, ft0, fs1
	fsw	ft0, 20(s8)
	flw	ft10, 212(sp)
	fadd.s	ft0, ft10, fs3
	flw	ft10, 188(sp)
	fadd.s	ft0, ft0, ft10
	flw	ft10, 216(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 24(s8)
	flw	ft10, 192(sp)
	fadd.s	ft0, fs8, ft10
	fadd.s	ft0, ft0, ft2
	flw	ft10, 200(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 28(s8)
	fadd.s	ft0, fs11, fs0
	flw	ft10, 208(sp)
	fadd.s	ft0, ft0, ft10
	flw	ft10, 180(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 32(s8)
	flw	ft10, 184(sp)
	fadd.s	ft0, ft10, fa5
	fadd.s	ft0, ft0, fa4
	flw	ft10, 220(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 36(s8)
	addi	t5, sp, 232
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	addw	a3, s7, s1
	addw	a2, a3, a2
	addi	s1, sp, 232
	sw	a2, 0(s1)
	addw	t3, t3, s10
	addw	t3, t3, s9
	sw	t3, 4(s1)
	addw	t3, s11, a7
	addw	t3, t3, s2
	sw	t3, 8(s1)
	addw	t3, s5, t4
	addw	t1, t3, t1
	sw	t1, 12(s1)
	addw	t0, t0, s6
	addw	t0, t0, a5
	sw	t0, 16(s1)
	addw	t0, a4, t2
	addw	t0, t0, s3
	sw	t0, 20(s1)
	ld	t5, 104(sp)
	addw	t0, t5, a6
	ld	t5, 88(sp)
	addw	t0, t0, t5
	sw	t0, 24(s1)
	ld	t5, 80(sp)
	ld	t6, 96(sp)
	addw	t0, t5, t6
	ld	t5, 112(sp)
	addw	t0, t0, t5
	sw	t0, 28(s1)
	li	a0, 10
	addi	a1, sp, 264
	call	putfarray
	li	a0, 8
	addi	a1, sp, 232
	call	putarray
	lw	t0, 0(s1)
	flw	ft0, 0(s8)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 0(s1)
	lw	t0, 4(s1)
	flw	ft0, 4(s8)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 4(s1)
	lw	t0, 8(s1)
	flw	ft0, 8(s8)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 8(s1)
	lw	t0, 12(s1)
	flw	ft0, 12(s8)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 12(s1)
	lw	t0, 16(s1)
	flw	ft0, 16(s8)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 16(s1)
	lw	t0, 20(s1)
	flw	ft0, 20(s8)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 20(s1)
	lw	t0, 24(s1)
	flw	ft0, 24(s8)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 24(s1)
	lw	t0, 28(s1)
	flw	ft0, 28(s8)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 28(s1)
	la	t0, k
	lw	t0, 0(t0)
	slliw	t0, t0, 2
	add	t0, s1, t0
	lw	t0, 0(t0)
	fcvt.s.w	ft0, t0
.params_f40_i24.bb2:
	fmv.s	fa0, ft0
	flw	fs11, 452(sp)
	flw	fs10, 448(sp)
	flw	fs9, 444(sp)
	flw	fs8, 440(sp)
	flw	fs7, 436(sp)
	flw	fs6, 432(sp)
	flw	fs5, 428(sp)
	flw	fs4, 424(sp)
	flw	fs3, 420(sp)
	flw	fs2, 416(sp)
	flw	fs1, 412(sp)
	flw	fs0, 408(sp)
	ld	s11, 400(sp)
	ld	s10, 392(sp)
	ld	s9, 384(sp)
	ld	s8, 376(sp)
	ld	s7, 368(sp)
	ld	s6, 360(sp)
	ld	s5, 352(sp)
	ld	s4, 344(sp)
	ld	s3, 336(sp)
	ld	s2, 328(sp)
	ld	s1, 320(sp)
	ld	ra, 312(sp)
	ld	s0, 304(sp)
	addi	sp, sp, 464
	ret
	.size params_f40_i24, .-params_f40_i24

  .globl params_fa40
  .type  params_fa40, @function
params_fa40:
.params_fa40.bb0:
	addi	sp, sp, -512
	sd	s0, 392(sp)
	sd	ra, 400(sp)
	sd	s1, 408(sp)
	sd	s2, 416(sp)
	sd	s3, 424(sp)
	sd	s4, 432(sp)
	sd	s5, 440(sp)
	sd	s6, 448(sp)
	sd	s7, 456(sp)
	sd	s8, 464(sp)
	sd	s9, 472(sp)
	sd	s10, 480(sp)
	sd	s11, 488(sp)
	fsw	fs0, 496(sp)
	fsw	fs1, 500(sp)
	addi	s0, sp, 512
	mv	t5, a0
	sd	t5, 16(sp)
	mv	t5, a1
	sd	t5, 24(sp)
	mv	t5, a2
	sd	t5, 32(sp)
	mv	t5, a3
	sd	t5, 40(sp)
	mv	t5, a4
	sd	t5, 48(sp)
	mv	t5, a5
	sd	t5, 56(sp)
	mv	t5, a6
	sd	t5, 64(sp)
	mv	t5, a7
	sd	t5, 72(sp)
	ld	t5, 0(s0)
	sd	t5, 80(sp)
	ld	t5, 8(s0)
	sd	t5, 88(sp)
	ld	t5, 16(s0)
	sd	t5, 96(sp)
	ld	t5, 24(s0)
	sd	t5, 104(sp)
	ld	t5, 32(s0)
	sd	t5, 112(sp)
	ld	t5, 40(s0)
	sd	t5, 120(sp)
	ld	t5, 48(s0)
	sd	t5, 128(sp)
	ld	t5, 56(s0)
	sd	t5, 136(sp)
	ld	t5, 64(s0)
	sd	t5, 144(sp)
	ld	t5, 72(s0)
	sd	t5, 152(sp)
	ld	t5, 80(s0)
	sd	t5, 160(sp)
	ld	t5, 88(s0)
	sd	t5, 168(sp)
	ld	t5, 96(s0)
	ld	s11, 104(s0)
	ld	t0, 112(s0)
	ld	t1, 120(s0)
	ld	t2, 128(s0)
	ld	t3, 136(s0)
	ld	a2, 144(s0)
	ld	a3, 152(s0)
	ld	a4, 160(s0)
	ld	a5, 168(s0)
	ld	a6, 176(s0)
	ld	a7, 184(s0)
	ld	t4, 192(s0)
	ld	s1, 200(s0)
	ld	s2, 208(s0)
	ld	s3, 216(s0)
	ld	s4, 224(s0)
	ld	s5, 232(s0)
	ld	s6, 240(s0)
	ld	s7, 248(s0)
	la	s8, k
	lw	s9, 0(s8)
	slliw	s8, s9, 2
	li	s10, 0
	fmv.w.x	fs0, s10
	sd	s7, 184(sp)
	sd	s6, 192(sp)
	sd	s5, 200(sp)
	sd	s4, 208(sp)
	sd	s3, 216(sp)
	sd	s2, 224(sp)
	sd	s1, 232(sp)
	sd	t4, 240(sp)
	sd	a7, 248(sp)
	sd	a6, 256(sp)
	sd	a5, 264(sp)
	sd	a4, 272(sp)
	sd	a3, 280(sp)
	sd	a2, 288(sp)
	sd	t3, 296(sp)
	sd	t2, 304(sp)
	sd	t1, 312(sp)
	sd	t0, 320(sp)
	sd	s11, 328(sp)
	sd	t5, 336(sp)
	ld	s10, 168(sp)
	ld	s7, 160(sp)
	ld	s6, 152(sp)
	ld	s5, 144(sp)
	ld	s4, 136(sp)
	ld	s3, 128(sp)
	ld	s2, 120(sp)
	ld	s1, 112(sp)
	ld	t4, 104(sp)
	ld	a7, 96(sp)
	ld	a6, 88(sp)
	ld	a5, 80(sp)
	ld	a4, 72(sp)
	ld	a3, 64(sp)
	ld	a2, 56(sp)
	ld	t3, 48(sp)
	ld	t2, 40(sp)
	ld	t1, 32(sp)
	ld	t5, 24(sp)
	sd	t5, 344(sp)
	ld	s11, 16(sp)
.params_fa40.bb1:
	addi	t5, sp, 352
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	add	s9, s11, s8
	flw	ft0, 0(s9)
	ld	t5, 344(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, t1, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, t2, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	addi	s9, sp, 352
	fsw	ft0, 0(s9)
	add	t0, t3, s8
	flw	ft0, 0(t0)
	add	t0, a2, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, a3, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, a4, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 4(s9)
	add	t0, a5, s8
	flw	ft0, 0(t0)
	add	t0, a6, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, a7, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, t4, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 8(s9)
	add	t0, s1, s8
	flw	ft0, 0(t0)
	add	t0, s2, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, s3, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, s4, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 12(s9)
	add	t0, s5, s8
	flw	ft0, 0(t0)
	add	t0, s6, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, s7, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	add	t0, s10, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 16(s9)
	ld	t5, 336(sp)
	add	t0, t5, s8
	flw	ft0, 0(t0)
	ld	t5, 328(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 320(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 312(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 20(s9)
	ld	t5, 304(sp)
	add	t0, t5, s8
	flw	ft0, 0(t0)
	ld	t5, 296(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 288(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 280(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 24(s9)
	ld	t5, 272(sp)
	add	t0, t5, s8
	flw	ft0, 0(t0)
	ld	t5, 264(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 256(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 248(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 28(s9)
	ld	t5, 240(sp)
	add	t0, t5, s8
	flw	ft0, 0(t0)
	ld	t5, 232(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 224(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 216(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 32(s9)
	ld	t5, 208(sp)
	add	t0, t5, s8
	flw	ft0, 0(t0)
	ld	t5, 200(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 192(sp)
	add	t0, t5, s8
	flw	ft1, 0(t0)
	fadd.s	ft0, ft0, ft1
	ld	t5, 184(sp)
	add	t0, t5, s8
	flw	fs1, 0(t0)
	fadd.s	ft0, ft0, fs1
	fsw	ft0, 36(s9)
	feq.s	t0, fs1, fs0
	xori	t0, t0, 1
	fcvt.s.w	ft0, t0
	feq.s	t0, ft0, fs0
	xori	t0, t0, 1
	fcvt.s.w	ft0, t0
	feq.s	t0, ft0, fs0
	beqz	t0, .params_fa40.bb3
.params_fa40.bb4:
	addi	t0, sp, 352
	ld	s11, 344(sp)
	sd	t1, 344(sp)
	mv	t1, t2	# phi-copy
	mv	t2, t3	# phi-copy
	mv	t3, a2	# phi-copy
	mv	a2, a3	# phi-copy
	mv	a3, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, a6	# phi-copy
	mv	a6, a7	# phi-copy
	mv	a7, t4	# phi-copy
	mv	t4, s1	# phi-copy
	mv	s1, s2	# phi-copy
	mv	s2, s3	# phi-copy
	mv	s3, s4	# phi-copy
	mv	s4, s5	# phi-copy
	mv	s5, s6	# phi-copy
	mv	s6, s7	# phi-copy
	mv	s7, s10	# phi-copy
	ld	s10, 336(sp)
	ld	t5, 328(sp)
	sd	t5, 336(sp)
	ld	t5, 320(sp)
	sd	t5, 328(sp)
	ld	t5, 312(sp)
	sd	t5, 320(sp)
	ld	t5, 304(sp)
	sd	t5, 312(sp)
	ld	t5, 296(sp)
	sd	t5, 304(sp)
	ld	t5, 288(sp)
	sd	t5, 296(sp)
	ld	t5, 280(sp)
	sd	t5, 288(sp)
	ld	t5, 272(sp)
	sd	t5, 280(sp)
	ld	t5, 264(sp)
	sd	t5, 272(sp)
	ld	t5, 256(sp)
	sd	t5, 264(sp)
	ld	t5, 248(sp)
	sd	t5, 256(sp)
	ld	t5, 240(sp)
	sd	t5, 248(sp)
	ld	t5, 232(sp)
	sd	t5, 240(sp)
	ld	t5, 224(sp)
	sd	t5, 232(sp)
	ld	t5, 216(sp)
	sd	t5, 224(sp)
	ld	t5, 208(sp)
	sd	t5, 216(sp)
	ld	t5, 200(sp)
	sd	t5, 208(sp)
	ld	t5, 192(sp)
	sd	t5, 200(sp)
	ld	t5, 184(sp)
	sd	t5, 192(sp)
	sd	t0, 184(sp)
	j	.params_fa40.bb1
.params_fa40.bb3:
	li	a0, 10
	addi	a1, sp, 352
	call	putfarray
	addi	t0, sp, 352
	add	t0, t0, s8
	flw	ft0, 0(t0)
.params_fa40.bb2:
	fmv.s	fa0, ft0
	flw	fs1, 500(sp)
	flw	fs0, 496(sp)
	ld	s11, 488(sp)
	ld	s10, 480(sp)
	ld	s9, 472(sp)
	ld	s8, 464(sp)
	ld	s7, 456(sp)
	ld	s6, 448(sp)
	ld	s5, 440(sp)
	ld	s4, 432(sp)
	ld	s3, 424(sp)
	ld	s2, 416(sp)
	ld	s1, 408(sp)
	ld	ra, 400(sp)
	ld	s0, 392(sp)
	addi	sp, sp, 512
	ret
	.size params_fa40, .-params_fa40

  .globl params_mix
  .type  params_mix, @function
params_mix:
.params_mix.bb0:
	addi	sp, sp, -672
	sd	s0, 528(sp)
	sd	ra, 536(sp)
	sd	s1, 544(sp)
	sd	s2, 552(sp)
	sd	s3, 560(sp)
	sd	s4, 568(sp)
	sd	s5, 576(sp)
	sd	s6, 584(sp)
	sd	s7, 592(sp)
	sd	s8, 600(sp)
	sd	s9, 608(sp)
	sd	s10, 616(sp)
	sd	s11, 624(sp)
	fsw	fs0, 632(sp)
	fsw	fs1, 636(sp)
	fsw	fs2, 640(sp)
	fsw	fs3, 644(sp)
	fsw	fs4, 648(sp)
	fsw	fs5, 652(sp)
	fsw	fs6, 656(sp)
	addi	s0, sp, 672
	fmv.s	ft10, fa0
	fsw	ft10, 400(sp)
	mv	t5, a0
	sd	t5, 16(sp)
	mv	t5, a1
	sd	t5, 24(sp)
	mv	t5, a2
	sd	t5, 32(sp)
	fmv.s	ft10, fa1
	fsw	ft10, 404(sp)
	mv	t5, a3
	sd	t5, 40(sp)
	fmv.s	ft10, fa2
	fsw	ft10, 408(sp)
	fmv.s	ft10, fa3
	fsw	ft10, 412(sp)
	mv	t5, a4
	sd	t5, 48(sp)
	mv	t5, a5
	sd	t5, 56(sp)
	mv	t5, a6
	sd	t5, 64(sp)
	mv	t5, a7
	sd	t5, 72(sp)
	ld	t5, 0(s0)
	sd	t5, 80(sp)
	ld	t5, 8(s0)
	sd	t5, 88(sp)
	ld	t5, 16(s0)
	sd	t5, 96(sp)
	lw	t5, 24(s0)
	sd	t5, 104(sp)
	ld	t5, 32(s0)
	sd	t5, 112(sp)
	ld	t5, 40(s0)
	sd	t5, 120(sp)
	fmv.s	fs2, fa4
	fmv.s	ft5, fa5
	fmv.s	ft6, fa6
	ld	t5, 48(s0)
	sd	t5, 128(sp)
	lw	t5, 56(s0)
	sd	t5, 136(sp)
	fmv.s	ft7, fa7
	flw	ft8, 64(s0)
	flw	fa2, 72(s0)
	ld	t5, 80(s0)
	sd	t5, 144(sp)
	ld	t5, 88(s0)
	sd	t5, 152(sp)
	ld	t5, 96(s0)
	sd	t5, 160(sp)
	ld	t5, 104(s0)
	sd	t5, 168(sp)
	ld	t5, 112(s0)
	sd	t5, 176(sp)
	flw	fa3, 120(s0)
	flw	fa4, 128(s0)
	ld	t5, 136(s0)
	sd	t5, 184(sp)
	lw	t5, 144(s0)
	sd	t5, 192(sp)
	ld	t1, 152(s0)
	ld	t2, 160(s0)
	flw	fa5, 168(s0)
	flw	fa6, 176(s0)
	ld	t3, 184(s0)
	ld	a2, 192(s0)
	lw	a3, 200(s0)
	lw	a4, 208(s0)
	flw	fa7, 216(s0)
	flw	fs0, 224(s0)
	ld	a5, 232(s0)
	lw	a6, 240(s0)
	ld	a7, 248(s0)
	lw	t4, 256(s0)
	ld	s1, 264(s0)
	ld	s2, 272(s0)
	flw	ft0, 280(s0)
	flw	ft1, 288(s0)
	ld	s3, 296(s0)
	lw	s4, 304(s0)
	ld	s5, 312(s0)
	ld	s6, 320(s0)
	flw	ft2, 328(s0)
	lw	s7, 336(s0)
	flw	ft3, 344(s0)
	ld	s8, 352(s0)
	ld	s9, 360(s0)
	flw	ft4, 368(s0)
	lw	s10, 376(s0)
	la	s11, k
	lw	t0, 0(s11)
	slliw	t5, t0, 2
	sd	t5, 392(sp)
	li	t0, 0
	fmv.w.x	ft10, t0
	fsw	ft10, 420(sp)
	sd	s10, 200(sp)
	sd	s7, 208(sp)
	sd	s5, 224(sp)
	sd	s4, 216(sp)
	sd	s2, 232(sp)
	sd	s1, 248(sp)
	sd	t4, 240(sp)
	sd	a6, 256(sp)
	sd	a5, 272(sp)
	sd	a4, 264(sp)
	sd	a3, 280(sp)
	sd	a2, 288(sp)
	sd	t3, 296(sp)
	ld	t5, 192(sp)
	sd	t5, 304(sp)
	ld	t5, 184(sp)
	sd	t5, 312(sp)
	ld	s7, 176(sp)
	ld	t5, 168(sp)
	sd	t5, 320(sp)
	ld	t5, 160(sp)
	sd	t5, 328(sp)
	ld	s2, 152(sp)
	ld	t5, 144(sp)
	sd	t5, 344(sp)
	ld	t5, 136(sp)
	sd	t5, 336(sp)
	ld	a6, 128(sp)
	ld	a4, 120(sp)
	ld	a5, 112(sp)
	ld	t5, 104(sp)
	sd	t5, 352(sp)
	ld	t5, 96(sp)
	sd	t5, 360(sp)
	ld	t3, 88(sp)
	ld	t5, 80(sp)
	sd	t5, 384(sp)
	ld	s10, 72(sp)
	ld	s4, 64(sp)
	ld	s5, 56(sp)
	ld	t5, 48(sp)
	sd	t5, 376(sp)
	ld	t4, 40(sp)
	ld	a3, 32(sp)
	ld	a2, 24(sp)
	ld	t5, 16(sp)
	sd	t5, 368(sp)
	fsw	ft4, 436(sp)
	fsw	ft3, 444(sp)
	flw	ft10, 412(sp)
	fsw	ft10, 440(sp)
	flw	ft10, 408(sp)
	fsw	ft10, 432(sp)
	flw	ft10, 404(sp)
	fsw	ft10, 428(sp)
	flw	ft10, 400(sp)
	fsw	ft10, 424(sp)
.params_mix.bb1:
	addi	t5, sp, 448
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	ld	t5, 392(sp)
	add	a3, a3, t5
	flw	fs1, 0(a3)
	flw	ft10, 424(sp)
	fadd.s	fs6, ft10, fs1
	flw	ft10, 428(sp)
	fadd.s	fs5, fs6, ft10
	flw	ft10, 432(sp)
	fadd.s	fs4, fs5, ft10
	addi	a3, sp, 448
	fsw	fs4, 0(a3)
	ld	t5, 376(sp)
	ld	t6, 392(sp)
	add	s1, t5, t6
	flw	ft4, 0(s1)
	flw	ft10, 440(sp)
	fadd.s	ft4, ft10, ft4
	ld	t5, 384(sp)
	ld	t6, 392(sp)
	add	t0, t5, t6
	flw	fs3, 0(t0)
	fadd.s	ft4, ft4, fs3
	ld	t5, 392(sp)
	add	t0, a5, t5
	flw	ft3, 0(t0)
	fadd.s	ft3, ft4, ft3
	fsw	ft3, 4(a3)
	ld	t5, 392(sp)
	add	t0, a4, t5
	flw	ft3, 0(t0)
	fadd.s	ft3, ft3, fs2
	fadd.s	ft3, ft3, ft5
	fadd.s	ft3, ft3, ft6
	fsw	ft3, 8(a3)
	ld	t5, 392(sp)
	add	t0, a6, t5
	flw	ft3, 0(t0)
	fadd.s	ft3, ft3, ft7
	fadd.s	ft3, ft3, ft8
	fadd.s	ft3, ft3, fa2
	fsw	ft3, 12(a3)
	ld	t5, 392(sp)
	add	t0, s2, t5
	flw	ft3, 0(t0)
	ld	t5, 392(sp)
	add	t0, s7, t5
	flw	ft4, 0(t0)
	fadd.s	ft3, ft3, ft4
	fadd.s	ft3, ft3, fa3
	fadd.s	ft3, ft3, fa4
	fsw	ft3, 16(a3)
	ld	t5, 392(sp)
	add	t0, t1, t5
	flw	ft3, 0(t0)
	ld	t5, 392(sp)
	add	t0, t2, t5
	flw	ft4, 0(t0)
	fadd.s	ft3, ft3, ft4
	fadd.s	ft3, ft3, fa5
	fadd.s	ft3, ft3, fa6
	fsw	ft3, 20(a3)
	fadd.s	ft3, fa7, fs0
	ld	t5, 392(sp)
	add	t0, a7, t5
	flw	ft4, 0(t0)
	fadd.s	ft3, ft3, ft4
	fadd.s	ft3, ft3, ft0
	fsw	ft3, 24(a3)
	ld	t5, 392(sp)
	add	t0, s3, t5
	flw	ft3, 0(t0)
	fadd.s	ft3, ft1, ft3
	ld	t5, 392(sp)
	add	t0, s6, t5
	flw	ft4, 0(t0)
	fadd.s	ft3, ft3, ft4
	fadd.s	ft3, ft3, ft2
	fsw	ft3, 28(a3)
	ld	t5, 392(sp)
	add	t0, s8, t5
	flw	ft3, 0(t0)
	flw	ft10, 444(sp)
	fadd.s	ft3, ft10, ft3
	ld	t5, 392(sp)
	add	t0, s9, t5
	flw	ft4, 0(t0)
	fadd.s	ft3, ft3, ft4
	flw	ft10, 436(sp)
	fadd.s	fs1, ft3, ft10
	fsw	fs1, 32(a3)
	addi	t5, sp, 488
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	ld	t5, 368(sp)
	ld	t6, 392(sp)
	add	t0, t5, t6
	lw	t0, 0(t0)
	addw	t0, t0, a2
	addw	t0, t0, t4
	addi	s1, sp, 488
	sw	t0, 0(s1)
	ld	t5, 392(sp)
	add	t0, s5, t5
	lw	t0, 0(t0)
	addw	t0, t0, s4
	addw	t0, t0, s10
	sw	t0, 4(s1)
	ld	t5, 392(sp)
	add	t0, t3, t5
	lw	t0, 0(t0)
	ld	t5, 360(sp)
	ld	t6, 392(sp)
	add	a3, t5, t6
	lw	a3, 0(a3)
	addw	t0, t0, a3
	ld	t5, 352(sp)
	addw	t0, t0, t5
	sw	t0, 8(s1)
	ld	t5, 344(sp)
	ld	t6, 392(sp)
	add	t0, t5, t6
	lw	t0, 0(t0)
	ld	t5, 336(sp)
	addw	t0, t5, t0
	ld	t5, 328(sp)
	ld	t6, 392(sp)
	add	a3, t5, t6
	lw	a3, 0(a3)
	addw	t0, t0, a3
	sw	t0, 12(s1)
	ld	t5, 320(sp)
	ld	t6, 392(sp)
	add	t0, t5, t6
	lw	t0, 0(t0)
	ld	t5, 312(sp)
	ld	t6, 392(sp)
	add	a3, t5, t6
	lw	a3, 0(a3)
	addw	t0, t0, a3
	ld	t5, 304(sp)
	addw	t0, t0, t5
	sw	t0, 16(s1)
	ld	t5, 296(sp)
	ld	t6, 392(sp)
	add	t0, t5, t6
	lw	t0, 0(t0)
	ld	t5, 288(sp)
	ld	t6, 392(sp)
	add	a3, t5, t6
	lw	a3, 0(a3)
	addw	t0, t0, a3
	ld	t5, 280(sp)
	addw	t0, t0, t5
	sw	t0, 20(s1)
	ld	t5, 272(sp)
	ld	t6, 392(sp)
	add	t0, t5, t6
	lw	t0, 0(t0)
	ld	t5, 264(sp)
	addw	t0, t5, t0
	ld	t5, 256(sp)
	addw	t0, t0, t5
	sw	t0, 24(s1)
	ld	t5, 248(sp)
	ld	t6, 392(sp)
	add	t0, t5, t6
	lw	t0, 0(t0)
	ld	t5, 240(sp)
	addw	t0, t5, t0
	ld	t5, 232(sp)
	ld	t6, 392(sp)
	add	a3, t5, t6
	lw	a3, 0(a3)
	addw	t0, t0, a3
	sw	t0, 28(s1)
	ld	t5, 224(sp)
	ld	t6, 392(sp)
	add	t0, t5, t6
	lw	t0, 0(t0)
	ld	t5, 216(sp)
	addw	t0, t5, t0
	ld	t5, 208(sp)
	addw	t0, t0, t5
	ld	t5, 200(sp)
	addw	t0, t0, t5
	sw	t0, 32(s1)
	ld	t5, 200(sp)
	bne	t5, zero, .params_mix.bb3
.params_mix.bb4:
	flw	ft10, 436(sp)
	fcvt.w.s	t0, ft10, rtz
	addi	a3, sp, 488
	addi	s11, sp, 448
	sd	t0, 200(sp)
	sd	a3, 368(sp)
	mv	a3, s11	# phi-copy
	flw	ft10, 420(sp)
	fsw	ft10, 436(sp)
	j	.params_mix.bb1
.params_mix.bb3:
	li	a0, 10
	addi	a1, sp, 448
	call	putfarray
	li	a0, 10
	addi	a1, sp, 488
	call	putarray
	addi	t0, sp, 488
	lw	t1, 0(t0)
	addi	t2, sp, 448
	flw	ft0, 0(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 0(t0)
	lw	t1, 4(t0)
	flw	ft0, 4(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 4(t0)
	lw	t1, 8(t0)
	flw	ft0, 8(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 8(t0)
	lw	t1, 12(t0)
	flw	ft0, 12(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 12(t0)
	lw	t1, 16(t0)
	flw	ft0, 16(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 16(t0)
	lw	t1, 20(t0)
	flw	ft0, 20(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 20(t0)
	lw	t1, 24(t0)
	flw	ft0, 24(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 24(t0)
	lw	t1, 28(t0)
	flw	ft0, 28(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 28(t0)
	lw	t1, 32(t0)
	fcvt.s.w	ft0, t1
	fsub.s	ft0, ft0, fs1
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 32(t0)
	lw	t1, 36(t0)
	flw	ft0, 36(t2)
	fcvt.s.w	ft1, t1
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t1, ft0, rtz
	sw	t1, 36(t0)
	ld	t5, 392(sp)
	add	t0, t0, t5
	lw	t0, 0(t0)
	fcvt.s.w	ft0, t0
	fmul.s	ft0, ft0, fs1
	fcvt.w.s	t0, ft0, rtz
.params_mix.bb2:
	mv	a0, t0
	flw	fs6, 656(sp)
	flw	fs5, 652(sp)
	flw	fs4, 648(sp)
	flw	fs3, 644(sp)
	flw	fs2, 640(sp)
	flw	fs1, 636(sp)
	flw	fs0, 632(sp)
	ld	s11, 624(sp)
	ld	s10, 616(sp)
	ld	s9, 608(sp)
	ld	s8, 600(sp)
	ld	s7, 592(sp)
	ld	s6, 584(sp)
	ld	s5, 576(sp)
	ld	s4, 568(sp)
	ld	s3, 560(sp)
	ld	s2, 552(sp)
	ld	s1, 544(sp)
	ld	ra, 536(sp)
	ld	s0, 528(sp)
	addi	sp, sp, 672
	ret
	.size params_mix, .-params_mix

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -1696
	sd	ra, 1568(sp)
	sd	s1, 1576(sp)
	sd	s2, 1584(sp)
	sd	s3, 1592(sp)
	sd	s4, 1600(sp)
	sd	s5, 1608(sp)
	sd	s6, 1616(sp)
	sd	s7, 1624(sp)
	sd	s8, 1632(sp)
	fsw	fs0, 1640(sp)
	fsw	fs1, 1644(sp)
	fsw	fs2, 1648(sp)
	fsw	fs3, 1652(sp)
	fsw	fs4, 1656(sp)
	fsw	fs5, 1660(sp)
	fsw	fs6, 1664(sp)
	fsw	fs7, 1668(sp)
	fsw	fs8, 1672(sp)
	fsw	fs9, 1676(sp)
	fsw	fs10, 1680(sp)
	fsw	fs11, 1684(sp)
	call	getint
	mv	t0, a0
	la	t1, k
	sw	t0, 0(t1)
	addi	s1, sp, 1088
	addi	s2, sp, 800
	li	s4, 0
	li	s3, 40
.main.bb1:
	bge	s4, s3, .main.bb3
.main.bb2:
	slliw	t0, s4, 3
	slliw	t1, s4, 2
	addw	t0, t0, t1
	add	t5, s1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getfarray
	addiw	s4, s4, 1
	blt	s4, s3, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	s4, 0
	li	s3, 24
.main.bb4:
	bge	s4, s3, .main.bb6
.main.bb5:
	slliw	t0, s4, 3
	slliw	t1, s4, 2
	addw	t0, t0, t1
	add	t5, s2, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	addiw	s4, s4, 1
	blt	s4, s3, .main.bb5	# rotated-loop-branch
.main.bb6:
	la	t5, k
	sd	t5, 512(sp)
	lw	t0, 0(t5)
	slliw	t0, t0, 2
	add	t0, s1, t0
	flw	ft0, 0(t0)
	flw	ft1, 12(t0)
	flw	ft2, 24(t0)
	flw	ft3, 36(t0)
	flw	ft4, 48(t0)
	flw	ft5, 60(t0)
	flw	ft6, 72(t0)
	flw	ft7, 84(t0)
	flw	ft8, 96(t0)
	flw	fa2, 108(t0)
	flw	fa3, 120(t0)
	flw	fa4, 132(t0)
	flw	fa5, 144(t0)
	flw	fa6, 156(t0)
	flw	fa7, 168(t0)
	flw	fs0, 180(t0)
	flw	fs1, 192(t0)
	flw	fs2, 204(t0)
	flw	fs3, 216(t0)
	flw	fs4, 228(t0)
	flw	fs5, 240(t0)
	flw	fs6, 252(t0)
	flw	fs7, 264(t0)
	flw	fs8, 276(t0)
	flw	fs9, 288(t0)
	flw	fs10, 300(t0)
	flw	fs11, 312(t0)
	flw	ft10, 324(t0)
	fsw	ft10, 688(sp)
	flw	ft10, 336(t0)
	fsw	ft10, 692(sp)
	flw	ft10, 348(t0)
	fsw	ft10, 696(sp)
	flw	ft10, 360(t0)
	fsw	ft10, 700(sp)
	flw	ft10, 372(t0)
	fsw	ft10, 704(sp)
	flw	ft10, 384(t0)
	fsw	ft10, 708(sp)
	flw	ft10, 396(t0)
	fsw	ft10, 712(sp)
	flw	ft10, 408(t0)
	fsw	ft10, 716(sp)
	flw	ft10, 420(t0)
	fsw	ft10, 720(sp)
	flw	ft10, 432(t0)
	fsw	ft10, 724(sp)
	flw	ft10, 444(t0)
	fsw	ft10, 728(sp)
	flw	ft10, 456(t0)
	fsw	ft10, 732(sp)
	flw	ft10, 468(t0)
	fsw	ft10, 736(sp)
	fmv.s	ft10, ft0
	fsw	ft10, 0(sp)
	fmv.s	ft10, ft1
	fsw	ft10, 8(sp)
	fmv.s	ft10, ft2
	fsw	ft10, 16(sp)
	fmv.s	ft10, ft3
	fsw	ft10, 24(sp)
	fmv.s	ft10, ft4
	fsw	ft10, 32(sp)
	fmv.s	ft10, ft5
	fsw	ft10, 40(sp)
	fmv.s	ft10, ft6
	fsw	ft10, 48(sp)
	fmv.s	ft10, ft7
	fsw	ft10, 56(sp)
	fmv.s	ft10, ft8
	fsw	ft10, 64(sp)
	fmv.s	ft10, fa2
	fsw	ft10, 72(sp)
	fmv.s	ft10, fa3
	fsw	ft10, 80(sp)
	fmv.s	ft10, fa4
	fsw	ft10, 88(sp)
	fmv.s	ft10, fa5
	fsw	ft10, 96(sp)
	fmv.s	ft10, fa6
	fsw	ft10, 104(sp)
	fmv.s	ft10, fa7
	fsw	ft10, 112(sp)
	fmv.s	ft10, fs0
	fsw	ft10, 120(sp)
	fmv.s	ft10, fs1
	fsw	ft10, 128(sp)
	fmv.s	ft10, fs2
	fsw	ft10, 136(sp)
	fmv.s	ft10, fs3
	fsw	ft10, 144(sp)
	fmv.s	ft10, fs4
	fsw	ft10, 152(sp)
	fmv.s	ft10, fs5
	fsw	ft10, 160(sp)
	fmv.s	ft10, fs6
	fsw	ft10, 168(sp)
	fmv.s	ft10, fs7
	fsw	ft10, 176(sp)
	fmv.s	ft10, fs8
	fsw	ft10, 184(sp)
	fmv.s	ft10, fs9
	fsw	ft10, 192(sp)
	fmv.s	ft10, fs10
	fsw	ft10, 200(sp)
	fmv.s	ft10, fs11
	fsw	ft10, 208(sp)
	flw	ft10, 688(sp)
	fsw	ft10, 216(sp)
	flw	ft10, 692(sp)
	fsw	ft10, 224(sp)
	flw	ft10, 696(sp)
	fsw	ft10, 232(sp)
	flw	ft10, 700(sp)
	fsw	ft10, 240(sp)
	flw	ft10, 704(sp)
	fsw	ft10, 248(sp)
	flw	ft10, 708(sp)
	fsw	ft10, 256(sp)
	flw	ft10, 712(sp)
	fsw	ft10, 264(sp)
	flw	ft10, 716(sp)
	fsw	ft10, 272(sp)
	flw	ft10, 720(sp)
	fsw	ft10, 280(sp)
	flw	ft10, 724(sp)
	fsw	ft10, 288(sp)
	flw	ft10, 728(sp)
	fsw	ft10, 296(sp)
	flw	ft10, 732(sp)
	fsw	ft10, 304(sp)
	flw	ft10, 736(sp)
	fsw	ft10, 312(sp)
	flw	fa0, 0(sp)
	flw	fa1, 8(sp)
	flw	fa2, 16(sp)
	flw	fa3, 24(sp)
	flw	fa4, 32(sp)
	flw	fa5, 40(sp)
	flw	fa6, 48(sp)
	flw	fa7, 56(sp)
	lw	a0, 64(sp)
	lw	a1, 72(sp)
	lw	a2, 80(sp)
	lw	a3, 88(sp)
	lw	a4, 96(sp)
	lw	a5, 104(sp)
	lw	a6, 112(sp)
	lw	a7, 120(sp)
	flw	ft10, 128(sp)
	fsw	ft10, 0(sp)
	flw	ft10, 136(sp)
	fsw	ft10, 8(sp)
	flw	ft10, 144(sp)
	fsw	ft10, 16(sp)
	flw	ft10, 152(sp)
	fsw	ft10, 24(sp)
	flw	ft10, 160(sp)
	fsw	ft10, 32(sp)
	flw	ft10, 168(sp)
	fsw	ft10, 40(sp)
	flw	ft10, 176(sp)
	fsw	ft10, 48(sp)
	flw	ft10, 184(sp)
	fsw	ft10, 56(sp)
	flw	ft10, 192(sp)
	fsw	ft10, 64(sp)
	flw	ft10, 200(sp)
	fsw	ft10, 72(sp)
	flw	ft10, 208(sp)
	fsw	ft10, 80(sp)
	flw	ft10, 216(sp)
	fsw	ft10, 88(sp)
	flw	ft10, 224(sp)
	fsw	ft10, 96(sp)
	flw	ft10, 232(sp)
	fsw	ft10, 104(sp)
	flw	ft10, 240(sp)
	fsw	ft10, 112(sp)
	flw	ft10, 248(sp)
	fsw	ft10, 120(sp)
	flw	ft10, 256(sp)
	fsw	ft10, 128(sp)
	flw	ft10, 264(sp)
	fsw	ft10, 136(sp)
	flw	ft10, 272(sp)
	fsw	ft10, 144(sp)
	flw	ft10, 280(sp)
	fsw	ft10, 152(sp)
	flw	ft10, 288(sp)
	fsw	ft10, 160(sp)
	flw	ft10, 296(sp)
	fsw	ft10, 168(sp)
	flw	ft10, 304(sp)
	fsw	ft10, 176(sp)
	flw	ft10, 312(sp)
	fsw	ft10, 184(sp)
	call	params_f40
	fmv.s	ft10, fa0
	fsw	ft10, 740(sp)
	ld	t5, 512(sp)
	lw	t0, 0(t5)
	slliw	t0, t0, 2
	addiw	t1, t0, 276
	add	t1, s2, t1
	lw	t2, 0(t1)
	addi	t3, t1, -252
	lw	a2, -252(t1)
	addi	t3, t3, 48
	lw	a3, -204(t1)
	addiw	t0, t0, 48
	add	t0, s1, t0
	flw	ft0, 0(t0)
	addi	t3, t3, -60
	lw	a4, -264(t1)
	addi	t3, t3, 36
	lw	t5, -228(t1)
	sd	t5, 632(sp)
	addi	t3, t3, 12
	lw	t5, -216(t1)
	sd	t5, 624(sp)
	addi	a7, t0, 48
	flw	ft1, 48(t0)
	addi	a7, a7, 84
	flw	ft2, 132(t0)
	addi	a7, a7, -96
	flw	ft3, 36(t0)
	addi	t3, t3, 204
	lw	t5, -12(t1)
	sd	t5, 600(sp)
	addi	a7, a7, -48
	flw	ft4, -12(t0)
	addi	a7, a7, 300
	flw	ft5, 288(t0)
	addi	t3, t3, -264
	lw	t5, -276(t1)
	sd	t5, 584(sp)
	addi	a7, a7, 108
	flw	ft6, 396(t0)
	addi	t3, t3, 228
	lw	t5, -48(t1)
	sd	t5, 560(sp)
	addi	a7, a7, -84
	flw	ft7, 312(t0)
	addi	a7, a7, -216
	flw	ft8, 96(t0)
	addi	a7, a7, -132
	flw	fa2, -36(t0)
	addi	a7, a7, 120
	flw	fa3, 84(t0)
	addi	a7, a7, 324
	flw	fa4, 408(t0)
	addi	a7, a7, -384
	flw	fa5, 24(t0)
	addi	t3, t3, -144
	lw	t5, -192(t1)
	sd	t5, 552(sp)
	addi	a7, a7, 312
	flw	fa6, 336(t0)
	addi	t3, t3, 36
	lw	t5, -156(t1)
	sd	t5, 528(sp)
	addi	t3, t3, 36
	lw	t5, -120(t1)
	sd	t5, 520(sp)
	addi	a7, a7, -144
	flw	fa7, 192(t0)
	addi	a7, a7, 156
	flw	fs1, 348(t0)
	addi	s3, a7, -120
	flw	fs2, 228(t0)
	addi	s8, s3, -168
	flw	fs3, 60(t0)
	addi	s7, s8, 192
	flw	fs4, 252(t0)
	addi	t3, t3, -60
	lw	t5, -180(t1)
	sd	t5, 536(sp)
	addi	s3, s7, 168
	flw	fs5, 420(t0)
	addi	t3, t3, 108
	lw	t5, -72(t1)
	sd	t5, 544(sp)
	addi	s7, s3, -216
	flw	fs6, 204(t0)
	addi	s6, s7, -60
	flw	fs7, 144(t0)
	flw	fs8, 12(t0)
	addi	s5, s6, 216
	flw	fs9, 360(t0)
	addi	t3, t3, 12
	lw	t5, -60(t1)
	sd	t5, 576(sp)
	addi	t3, t3, -108
	lw	t5, -168(t1)
	sd	t5, 568(sp)
	addi	s6, s5, -240
	flw	fs10, 120(t0)
	addi	s3, s6, -48
	flw	fs11, 72(t0)
	flw	fs0, -48(t0)
	addi	s4, t3, 36
	lw	t5, -132(t1)
	sd	t5, 592(sp)
	flw	ft10, 324(t0)
	fsw	ft10, 744(sp)
	addi	t4, s4, -12
	lw	t5, -144(t1)
	sd	t5, 616(sp)
	lw	t5, -84(t1)
	sd	t5, 608(sp)
	flw	ft10, 276(t0)
	fsw	ft10, 748(sp)
	addi	s4, s3, 168
	flw	ft10, 240(t0)
	fsw	ft10, 752(sp)
	addi	a6, s4, -132
	flw	ft10, 108(t0)
	fsw	ft10, 756(sp)
	flw	ft10, 300(t0)
	fsw	ft10, 760(sp)
	addi	t4, t4, -96
	lw	s3, -240(t1)
	lw	s4, -24(t1)
	lw	s5, -36(t1)
	addi	a6, a6, 60
	flw	ft10, 168(t0)
	fsw	ft10, 764(sp)
	addi	a6, a6, 12
	flw	ft10, 180(t0)
	fsw	ft10, 768(sp)
	addi	a6, a6, 36
	flw	ft10, 216(t0)
	fsw	ft10, 772(sp)
	addi	t5, a6, 48
	sd	t5, 640(sp)
	flw	ft10, 264(t0)
	fsw	ft10, 776(sp)
	flw	ft10, 384(t0)
	fsw	ft10, 780(sp)
	flw	ft10, 156(t0)
	fsw	ft10, 784(sp)
	lw	t3, -96(t1)
	flw	ft10, -24(t0)
	fsw	ft10, 788(sp)
	addi	a6, t4, 132
	lw	t1, -108(t1)
	flw	ft10, 372(t0)
	fsw	ft10, 792(sp)
	mv	t5, t2
	sd	t5, 0(sp)
	mv	t5, a2
	sd	t5, 8(sp)
	mv	t5, a3
	sd	t5, 16(sp)
	fmv.s	ft10, ft0
	fsw	ft10, 24(sp)
	mv	t5, a4
	sd	t5, 32(sp)
	ld	t5, 632(sp)
	sd	t5, 40(sp)
	ld	t5, 624(sp)
	sd	t5, 48(sp)
	fmv.s	ft10, ft1
	fsw	ft10, 56(sp)
	fmv.s	ft10, ft2
	fsw	ft10, 64(sp)
	fmv.s	ft10, ft3
	fsw	ft10, 72(sp)
	ld	t5, 600(sp)
	sd	t5, 80(sp)
	fmv.s	ft10, ft4
	fsw	ft10, 88(sp)
	fmv.s	ft10, ft5
	fsw	ft10, 96(sp)
	ld	t5, 584(sp)
	sd	t5, 104(sp)
	fmv.s	ft10, ft6
	fsw	ft10, 112(sp)
	ld	t5, 560(sp)
	sd	t5, 120(sp)
	fmv.s	ft10, ft7
	fsw	ft10, 128(sp)
	fmv.s	ft10, ft8
	fsw	ft10, 136(sp)
	fmv.s	ft10, fa2
	fsw	ft10, 144(sp)
	fmv.s	ft10, fa3
	fsw	ft10, 152(sp)
	fmv.s	ft10, fa4
	fsw	ft10, 160(sp)
	fmv.s	ft10, fa5
	fsw	ft10, 168(sp)
	ld	t5, 552(sp)
	sd	t5, 176(sp)
	fmv.s	ft10, fa6
	fsw	ft10, 184(sp)
	ld	t5, 528(sp)
	sd	t5, 192(sp)
	ld	t5, 520(sp)
	sd	t5, 200(sp)
	fmv.s	ft10, fa7
	fsw	ft10, 208(sp)
	fmv.s	ft10, fs1
	fsw	ft10, 216(sp)
	fmv.s	ft10, fs2
	fsw	ft10, 224(sp)
	fmv.s	ft10, fs3
	fsw	ft10, 232(sp)
	fmv.s	ft10, fs4
	fsw	ft10, 240(sp)
	ld	t5, 536(sp)
	sd	t5, 248(sp)
	fmv.s	ft10, fs5
	fsw	ft10, 256(sp)
	ld	t5, 544(sp)
	sd	t5, 264(sp)
	fmv.s	ft10, fs6
	fsw	ft10, 272(sp)
	fmv.s	ft10, fs7
	fsw	ft10, 280(sp)
	fmv.s	ft10, fs8
	fsw	ft10, 288(sp)
	fmv.s	ft10, fs9
	fsw	ft10, 296(sp)
	ld	t5, 576(sp)
	sd	t5, 304(sp)
	ld	t5, 568(sp)
	sd	t5, 312(sp)
	fmv.s	ft10, fs10
	fsw	ft10, 320(sp)
	fmv.s	ft10, fs11
	fsw	ft10, 328(sp)
	fmv.s	ft10, fs0
	fsw	ft10, 336(sp)
	ld	t5, 592(sp)
	sd	t5, 344(sp)
	flw	ft10, 744(sp)
	fsw	ft10, 352(sp)
	ld	t5, 616(sp)
	sd	t5, 360(sp)
	ld	t5, 608(sp)
	sd	t5, 368(sp)
	flw	ft10, 748(sp)
	fsw	ft10, 376(sp)
	flw	ft10, 752(sp)
	fsw	ft10, 384(sp)
	flw	ft10, 756(sp)
	fsw	ft10, 392(sp)
	flw	ft10, 760(sp)
	fsw	ft10, 400(sp)
	mv	t5, s3
	sd	t5, 408(sp)
	mv	t5, s4
	sd	t5, 416(sp)
	mv	t5, s5
	sd	t5, 424(sp)
	flw	ft10, 764(sp)
	fsw	ft10, 432(sp)
	flw	ft10, 768(sp)
	fsw	ft10, 440(sp)
	flw	ft10, 772(sp)
	fsw	ft10, 448(sp)
	flw	ft10, 776(sp)
	fsw	ft10, 456(sp)
	flw	ft10, 780(sp)
	fsw	ft10, 464(sp)
	flw	ft10, 784(sp)
	fsw	ft10, 472(sp)
	mv	t5, t3
	sd	t5, 480(sp)
	flw	ft10, 788(sp)
	fsw	ft10, 488(sp)
	mv	t5, t1
	sd	t5, 496(sp)
	flw	ft10, 792(sp)
	fsw	ft10, 504(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	flw	fa0, 24(sp)
	ld	a3, 32(sp)
	ld	a4, 40(sp)
	ld	a5, 48(sp)
	flw	fa1, 56(sp)
	flw	fa2, 64(sp)
	flw	fa3, 72(sp)
	ld	a6, 80(sp)
	flw	fa4, 88(sp)
	flw	fa5, 96(sp)
	ld	a7, 104(sp)
	flw	fa6, 112(sp)
	flw	fa7, 128(sp)
	ld	t5, 120(sp)
	sd	t5, 0(sp)
	flw	ft10, 136(sp)
	fsw	ft10, 8(sp)
	flw	ft10, 144(sp)
	fsw	ft10, 16(sp)
	flw	ft10, 152(sp)
	fsw	ft10, 24(sp)
	flw	ft10, 160(sp)
	fsw	ft10, 32(sp)
	flw	ft10, 168(sp)
	fsw	ft10, 40(sp)
	ld	t5, 176(sp)
	sd	t5, 48(sp)
	flw	ft10, 184(sp)
	fsw	ft10, 56(sp)
	ld	t5, 192(sp)
	sd	t5, 64(sp)
	ld	t5, 200(sp)
	sd	t5, 72(sp)
	flw	ft10, 208(sp)
	fsw	ft10, 80(sp)
	flw	ft10, 216(sp)
	fsw	ft10, 88(sp)
	flw	ft10, 224(sp)
	fsw	ft10, 96(sp)
	flw	ft10, 232(sp)
	fsw	ft10, 104(sp)
	flw	ft10, 240(sp)
	fsw	ft10, 112(sp)
	ld	t5, 248(sp)
	sd	t5, 120(sp)
	flw	ft10, 256(sp)
	fsw	ft10, 128(sp)
	ld	t5, 264(sp)
	sd	t5, 136(sp)
	flw	ft10, 272(sp)
	fsw	ft10, 144(sp)
	flw	ft10, 280(sp)
	fsw	ft10, 152(sp)
	flw	ft10, 288(sp)
	fsw	ft10, 160(sp)
	flw	ft10, 296(sp)
	fsw	ft10, 168(sp)
	ld	t5, 304(sp)
	sd	t5, 176(sp)
	ld	t5, 312(sp)
	sd	t5, 184(sp)
	flw	ft10, 320(sp)
	fsw	ft10, 192(sp)
	flw	ft10, 328(sp)
	fsw	ft10, 200(sp)
	flw	ft10, 336(sp)
	fsw	ft10, 208(sp)
	ld	t5, 344(sp)
	sd	t5, 216(sp)
	flw	ft10, 352(sp)
	fsw	ft10, 224(sp)
	ld	t5, 360(sp)
	sd	t5, 232(sp)
	ld	t5, 368(sp)
	sd	t5, 240(sp)
	flw	ft10, 376(sp)
	fsw	ft10, 248(sp)
	flw	ft10, 384(sp)
	fsw	ft10, 256(sp)
	flw	ft10, 392(sp)
	fsw	ft10, 264(sp)
	flw	ft10, 400(sp)
	fsw	ft10, 272(sp)
	ld	t5, 408(sp)
	sd	t5, 280(sp)
	ld	t5, 416(sp)
	sd	t5, 288(sp)
	ld	t5, 424(sp)
	sd	t5, 296(sp)
	flw	ft10, 432(sp)
	fsw	ft10, 304(sp)
	flw	ft10, 440(sp)
	fsw	ft10, 312(sp)
	flw	ft10, 448(sp)
	fsw	ft10, 320(sp)
	flw	ft10, 456(sp)
	fsw	ft10, 328(sp)
	flw	ft10, 464(sp)
	fsw	ft10, 336(sp)
	flw	ft10, 472(sp)
	fsw	ft10, 344(sp)
	ld	t5, 480(sp)
	sd	t5, 352(sp)
	flw	ft10, 488(sp)
	fsw	ft10, 360(sp)
	ld	t5, 496(sp)
	sd	t5, 368(sp)
	flw	ft10, 504(sp)
	fsw	ft10, 376(sp)
	call	params_f40_i24
	fmv.s	fs0, fa0
	addi	t5, sp, 1088
	sd	t5, 0(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 12
	sd	t5, 8(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 24
	sd	t5, 16(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 36
	sd	t5, 24(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 48
	sd	t5, 32(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 60
	sd	t5, 40(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 72
	sd	t5, 48(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 84
	sd	t5, 56(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 96
	sd	t5, 64(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 108
	sd	t5, 72(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 120
	sd	t5, 80(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 132
	sd	t5, 88(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 144
	sd	t5, 96(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 156
	sd	t5, 104(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 168
	sd	t5, 112(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 180
	sd	t5, 120(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 192
	sd	t5, 128(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 204
	sd	t5, 136(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 216
	sd	t5, 144(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 228
	sd	t5, 152(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 240
	sd	t5, 160(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 252
	sd	t5, 168(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 264
	sd	t5, 176(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 276
	sd	t5, 184(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 288
	sd	t5, 192(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 300
	sd	t5, 200(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 312
	sd	t5, 208(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 324
	sd	t5, 216(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 336
	sd	t5, 224(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 348
	sd	t5, 232(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 360
	sd	t5, 240(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 372
	sd	t5, 248(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 384
	sd	t5, 256(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 396
	sd	t5, 264(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 408
	sd	t5, 272(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 420
	sd	t5, 280(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 432
	sd	t5, 288(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 444
	sd	t5, 296(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 456
	sd	t5, 304(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 468
	sd	t5, 312(sp)
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
	ld	t5, 128(sp)
	sd	t5, 64(sp)
	ld	t5, 136(sp)
	sd	t5, 72(sp)
	ld	t5, 144(sp)
	sd	t5, 80(sp)
	ld	t5, 152(sp)
	sd	t5, 88(sp)
	ld	t5, 160(sp)
	sd	t5, 96(sp)
	ld	t5, 168(sp)
	sd	t5, 104(sp)
	ld	t5, 176(sp)
	sd	t5, 112(sp)
	ld	t5, 184(sp)
	sd	t5, 120(sp)
	ld	t5, 192(sp)
	sd	t5, 128(sp)
	ld	t5, 200(sp)
	sd	t5, 136(sp)
	ld	t5, 208(sp)
	sd	t5, 144(sp)
	ld	t5, 216(sp)
	sd	t5, 152(sp)
	ld	t5, 224(sp)
	sd	t5, 160(sp)
	ld	t5, 232(sp)
	sd	t5, 168(sp)
	ld	t5, 240(sp)
	sd	t5, 176(sp)
	ld	t5, 248(sp)
	sd	t5, 184(sp)
	ld	t5, 256(sp)
	sd	t5, 192(sp)
	ld	t5, 264(sp)
	sd	t5, 200(sp)
	ld	t5, 272(sp)
	sd	t5, 208(sp)
	ld	t5, 280(sp)
	sd	t5, 216(sp)
	ld	t5, 288(sp)
	sd	t5, 224(sp)
	ld	t5, 296(sp)
	sd	t5, 232(sp)
	ld	t5, 304(sp)
	sd	t5, 240(sp)
	ld	t5, 312(sp)
	sd	t5, 248(sp)
	call	params_fa40
	fmv.s	fs1, fa0
	ld	t5, 512(sp)
	lw	t0, 0(t5)
	slliw	t0, t0, 2
	add	t1, s1, t0
	flw	ft0, 0(t1)
	addiw	t0, t0, 12
	add	t0, s2, t0
	lw	t2, 0(t0)
	flw	ft1, 24(t1)
	lw	a3, 12(t0)
	flw	ft2, 36(t1)
	flw	ft3, 48(t1)
	lw	a4, 36(t0)
	lw	a5, 48(t0)
	lw	a6, 84(t0)
	flw	ft4, 108(t1)
	flw	ft5, 120(t1)
	flw	ft6, 132(t1)
	lw	a7, 96(t0)
	flw	ft7, 156(t1)
	flw	ft8, 168(t1)
	flw	fa2, 180(t1)
	addi	t4, sp, 800
	addi	t4, t4, 120
	addi	t4, sp, 800
	addi	t4, t4, 132
	addi	t4, sp, 800
	addi	t4, t4, 144
	flw	fa3, 216(t1)
	flw	fa4, 228(t1)
	addi	t4, sp, 800
	addi	t4, t4, 156
	lw	t5, 156(t0)
	sd	t5, 648(sp)
	flw	fa5, 264(t1)
	flw	fa6, 276(t1)
	addi	t4, sp, 800
	addi	t4, t4, 180
	addi	t4, sp, 800
	addi	t4, t4, 192
	lw	t4, 192(t0)
	lw	t5, 204(t0)
	sd	t5, 656(sp)
	flw	fa7, 288(t1)
	flw	fs2, 300(t1)
	lw	t5, 228(t0)
	sd	t5, 672(sp)
	lw	t5, 240(t0)
	sd	t5, 664(sp)
	flw	fs3, 324(t1)
	flw	fs4, 336(t1)
	lw	t5, -12(t0)
	sd	t5, 680(sp)
	flw	fs5, 372(t1)
	flw	fs6, 384(t1)
	flw	fs7, 420(t1)
	lw	t0, 24(t0)
	fmv.s	ft10, ft0
	fsw	ft10, 0(sp)
	addi	t5, sp, 800
	sd	t5, 8(sp)
	mv	t5, t2
	sd	t5, 16(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 12
	sd	t5, 24(sp)
	fmv.s	ft10, ft1
	fsw	ft10, 32(sp)
	mv	t5, a3
	sd	t5, 40(sp)
	fmv.s	ft10, ft2
	fsw	ft10, 48(sp)
	fmv.s	ft10, ft3
	fsw	ft10, 56(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 60
	sd	t5, 64(sp)
	addi	t5, sp, 800
	addi	t5, t5, 36
	sd	t5, 72(sp)
	mv	t5, a4
	sd	t5, 80(sp)
	mv	t5, a5
	sd	t5, 88(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 72
	sd	t5, 96(sp)
	addi	t5, sp, 800
	addi	t5, t5, 72
	sd	t5, 104(sp)
	addi	t5, sp, 800
	addi	t5, t5, 84
	sd	t5, 112(sp)
	mv	t5, a6
	sd	t5, 120(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 84
	sd	t5, 128(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 96
	sd	t5, 136(sp)
	fmv.s	ft10, ft4
	fsw	ft10, 144(sp)
	fmv.s	ft10, ft5
	fsw	ft10, 152(sp)
	fmv.s	ft10, ft6
	fsw	ft10, 160(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 144
	sd	t5, 168(sp)
	mv	t5, a7
	sd	t5, 176(sp)
	fmv.s	ft10, ft7
	fsw	ft10, 184(sp)
	fmv.s	ft10, ft8
	fsw	ft10, 192(sp)
	fmv.s	ft10, fa2
	fsw	ft10, 200(sp)
	addi	t5, sp, 800
	addi	t5, t5, 120
	sd	t5, 208(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 192
	sd	t5, 216(sp)
	addi	t5, sp, 800
	addi	t5, t5, 132
	sd	t5, 224(sp)
	addi	t5, sp, 800
	addi	t5, t5, 144
	sd	t5, 232(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 204
	sd	t5, 240(sp)
	fmv.s	ft10, fa3
	fsw	ft10, 248(sp)
	fmv.s	ft10, fa4
	fsw	ft10, 256(sp)
	addi	t5, sp, 800
	addi	t5, t5, 156
	sd	t5, 264(sp)
	ld	t5, 648(sp)
	sd	t5, 272(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 240
	sd	t5, 280(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 252
	sd	t5, 288(sp)
	fmv.s	ft10, fa5
	fsw	ft10, 296(sp)
	fmv.s	ft10, fa6
	fsw	ft10, 304(sp)
	addi	t5, sp, 800
	addi	t5, t5, 180
	sd	t5, 312(sp)
	addi	t5, sp, 800
	addi	t5, t5, 192
	sd	t5, 320(sp)
	mv	t5, t4
	sd	t5, 328(sp)
	ld	t5, 656(sp)
	sd	t5, 336(sp)
	fmv.s	ft10, fa7
	fsw	ft10, 344(sp)
	fmv.s	ft10, fs2
	fsw	ft10, 352(sp)
	addi	t5, sp, 800
	addi	t5, t5, 228
	sd	t5, 360(sp)
	ld	t5, 672(sp)
	sd	t5, 368(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 312
	sd	t5, 376(sp)
	ld	t5, 664(sp)
	sd	t5, 384(sp)
	addi	t5, sp, 800
	addi	t5, t5, 264
	sd	t5, 392(sp)
	addi	t5, sp, 800
	addi	t5, t5, 276
	sd	t5, 400(sp)
	fmv.s	ft10, fs3
	fsw	ft10, 408(sp)
	fmv.s	ft10, fs4
	fsw	ft10, 416(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 348
	sd	t5, 424(sp)
	ld	t5, 680(sp)
	sd	t5, 432(sp)
	addi	t5, sp, 800
	addi	t5, t5, 12
	sd	t5, 440(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 360
	sd	t5, 448(sp)
	fmv.s	ft10, fs5
	fsw	ft10, 456(sp)
	mv	t5, a3
	sd	t5, 464(sp)
	fmv.s	ft10, fs6
	fsw	ft10, 472(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 396
	sd	t5, 480(sp)
	addi	t5, sp, 1088
	addi	t5, t5, 408
	sd	t5, 488(sp)
	fmv.s	ft10, fs7
	fsw	ft10, 496(sp)
	mv	t5, t0
	sd	t5, 504(sp)
	flw	fa0, 0(sp)
	ld	a0, 8(sp)
	ld	a1, 16(sp)
	ld	a2, 24(sp)
	flw	fa1, 32(sp)
	ld	a3, 40(sp)
	flw	fa2, 48(sp)
	flw	fa3, 56(sp)
	ld	a4, 64(sp)
	ld	a5, 72(sp)
	ld	a6, 80(sp)
	ld	a7, 88(sp)
	flw	fa4, 144(sp)
	flw	fa5, 152(sp)
	flw	fa6, 160(sp)
	flw	fa7, 184(sp)
	ld	t5, 96(sp)
	sd	t5, 0(sp)
	ld	t5, 104(sp)
	sd	t5, 8(sp)
	ld	t5, 112(sp)
	sd	t5, 16(sp)
	ld	t5, 120(sp)
	sd	t5, 24(sp)
	ld	t5, 128(sp)
	sd	t5, 32(sp)
	ld	t5, 136(sp)
	sd	t5, 40(sp)
	ld	t5, 168(sp)
	sd	t5, 48(sp)
	ld	t5, 176(sp)
	sd	t5, 56(sp)
	flw	ft10, 192(sp)
	fsw	ft10, 64(sp)
	flw	ft10, 200(sp)
	fsw	ft10, 72(sp)
	ld	t5, 208(sp)
	sd	t5, 80(sp)
	ld	t5, 216(sp)
	sd	t5, 88(sp)
	ld	t5, 224(sp)
	sd	t5, 96(sp)
	ld	t5, 232(sp)
	sd	t5, 104(sp)
	ld	t5, 240(sp)
	sd	t5, 112(sp)
	flw	ft10, 248(sp)
	fsw	ft10, 120(sp)
	flw	ft10, 256(sp)
	fsw	ft10, 128(sp)
	ld	t5, 264(sp)
	sd	t5, 136(sp)
	ld	t5, 272(sp)
	sd	t5, 144(sp)
	ld	t5, 280(sp)
	sd	t5, 152(sp)
	ld	t5, 288(sp)
	sd	t5, 160(sp)
	flw	ft10, 296(sp)
	fsw	ft10, 168(sp)
	flw	ft10, 304(sp)
	fsw	ft10, 176(sp)
	ld	t5, 312(sp)
	sd	t5, 184(sp)
	ld	t5, 320(sp)
	sd	t5, 192(sp)
	ld	t5, 328(sp)
	sd	t5, 200(sp)
	ld	t5, 336(sp)
	sd	t5, 208(sp)
	flw	ft10, 344(sp)
	fsw	ft10, 216(sp)
	flw	ft10, 352(sp)
	fsw	ft10, 224(sp)
	ld	t5, 360(sp)
	sd	t5, 232(sp)
	ld	t5, 368(sp)
	sd	t5, 240(sp)
	ld	t5, 376(sp)
	sd	t5, 248(sp)
	ld	t5, 384(sp)
	sd	t5, 256(sp)
	ld	t5, 392(sp)
	sd	t5, 264(sp)
	ld	t5, 400(sp)
	sd	t5, 272(sp)
	flw	ft10, 408(sp)
	fsw	ft10, 280(sp)
	flw	ft10, 416(sp)
	fsw	ft10, 288(sp)
	ld	t5, 424(sp)
	sd	t5, 296(sp)
	ld	t5, 432(sp)
	sd	t5, 304(sp)
	ld	t5, 440(sp)
	sd	t5, 312(sp)
	ld	t5, 448(sp)
	sd	t5, 320(sp)
	flw	ft10, 456(sp)
	fsw	ft10, 328(sp)
	ld	t5, 464(sp)
	sd	t5, 336(sp)
	flw	ft10, 472(sp)
	fsw	ft10, 344(sp)
	ld	t5, 480(sp)
	sd	t5, 352(sp)
	ld	t5, 488(sp)
	sd	t5, 360(sp)
	flw	ft10, 496(sp)
	fsw	ft10, 368(sp)
	ld	t5, 504(sp)
	sd	t5, 376(sp)
	call	params_mix
	mv	s1, a0
	flw	fa0, 740(sp)
	call	putfloat
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fmv.s	ft10, fs0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fmv.s	ft10, fs1
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	t5, s1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	flw	fs11, 1684(sp)
	flw	fs10, 1680(sp)
	flw	fs9, 1676(sp)
	flw	fs8, 1672(sp)
	flw	fs7, 1668(sp)
	flw	fs6, 1664(sp)
	flw	fs5, 1660(sp)
	flw	fs4, 1656(sp)
	flw	fs3, 1652(sp)
	flw	fs2, 1648(sp)
	flw	fs1, 1644(sp)
	flw	fs0, 1640(sp)
	ld	s8, 1632(sp)
	ld	s7, 1624(sp)
	ld	s6, 1616(sp)
	ld	s5, 1608(sp)
	ld	s4, 1600(sp)
	ld	s3, 1592(sp)
	ld	s2, 1584(sp)
	ld	s1, 1576(sp)
	ld	ra, 1568(sp)
	addi	sp, sp, 1696
	ret
	.size main, .-main
