
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
	sd	s0, 192(sp)
	sd	ra, 200(sp)
	sd	s1, 208(sp)
	fsw	fs0, 216(sp)
	fsw	fs1, 220(sp)
	fsw	fs2, 224(sp)
	fsw	fs3, 228(sp)
	fsw	fs4, 232(sp)
	fsw	fs5, 236(sp)
	fsw	fs6, 240(sp)
	fsw	fs7, 244(sp)
	fsw	fs8, 248(sp)
	fsw	fs9, 252(sp)
	fsw	fs10, 256(sp)
	fsw	fs11, 260(sp)
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
	flw	fa0, 24(s0)
	flw	fa1, 32(s0)
	flw	ft0, 40(s0)
	flw	ft1, 48(s0)
	flw	ft2, 56(s0)
	flw	ft3, 64(s0)
	flw	ft4, 72(s0)
	flw	ft5, 80(s0)
	flw	ft6, 88(s0)
	flw	ft7, 96(s0)
	flw	ft8, 104(s0)
	flw	fa2, 112(s0)
	flw	fa3, 120(s0)
	flw	fa4, 128(s0)
	flw	fa5, 136(s0)
	flw	fa6, 144(s0)
	flw	fa7, 152(s0)
	flw	fs0, 160(s0)
	flw	fs1, 168(s0)
	flw	fs2, 176(s0)
	flw	fs3, 184(s0)
	li	t0, 0
	fmv.w.x	ft10, t0
	fsw	ft10, 140(sp)
	fsw	ft3, 136(sp)
	flw	ft3, 72(sp)
	fsw	ft6, 144(sp)
	flw	ft6, 84(sp)
	fsw	ft7, 92(sp)
	flw	ft7, 88(sp)
	fsw	ft8, 96(sp)
	fmv.s	ft8, ft0	# phi-copy
	flw	ft0, 60(sp)
	fsw	fa2, 100(sp)
	fmv.s	fa2, ft1	# phi-copy
	flw	ft1, 64(sp)
	fsw	fa3, 104(sp)
	fmv.s	fa3, ft2	# phi-copy
	flw	ft2, 68(sp)
	fsw	fa4, 108(sp)
	fsw	fa5, 112(sp)
	fmv.s	fa5, ft4	# phi-copy
	flw	ft4, 76(sp)
	fsw	fa6, 116(sp)
	fmv.s	fa6, ft5	# phi-copy
	flw	ft5, 80(sp)
	fsw	fa7, 120(sp)
	fsw	fs0, 124(sp)
	flw	fs0, 16(sp)
	fsw	fs1, 128(sp)
	flw	fs1, 20(sp)
	fsw	fs2, 132(sp)
	flw	fs2, 24(sp)
	fmv.s	fa4, fs3	# phi-copy
	flw	fs3, 28(sp)
.params_f40.bb1:
	flw	ft10, 140(sp)
	feq.s	t0, fa4, ft10
	beqz	t0, .params_f40.bb2
.params_f40.bb3:
	fadd.s	fs4, fs0, fs1
	fadd.s	fa7, fs4, fs2
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
	fmv.s	ft7, fa0	# phi-copy
	fmv.s	fa0, fa1	# phi-copy
	fmv.s	fa1, ft8	# phi-copy
	fmv.s	ft8, fa2	# phi-copy
	fmv.s	fa2, fa3	# phi-copy
	flw	fa3, 136(sp)
	fsw	fa5, 136(sp)
	fmv.s	fa5, fa6	# phi-copy
	flw	fa6, 144(sp)
	flw	ft10, 92(sp)
	fsw	ft10, 144(sp)
	flw	ft10, 96(sp)
	fsw	ft10, 92(sp)
	flw	ft10, 100(sp)
	fsw	ft10, 96(sp)
	flw	ft10, 104(sp)
	fsw	ft10, 100(sp)
	flw	ft10, 108(sp)
	fsw	ft10, 104(sp)
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
	fsw	fa4, 132(sp)
	fmv.s	fa4, fa7	# phi-copy
	j	.params_f40.bb1
.params_f40.bb2:
	addi	t5, sp, 152
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	fadd.s	fa7, fs0, fs1
	fadd.s	fa7, fa7, fs2
	fadd.s	fa7, fa7, fs3
	addi	s1, sp, 152
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
	fadd.s	ft0, ft0, fa0
	fsw	ft0, 16(s1)
	fadd.s	ft0, fa1, ft8
	fadd.s	ft0, ft0, fa2
	fadd.s	ft0, ft0, fa3
	fsw	ft0, 20(s1)
	flw	ft10, 136(sp)
	fadd.s	ft0, ft10, fa5
	fadd.s	ft0, ft0, fa6
	flw	ft10, 144(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 24(s1)
	flw	ft10, 92(sp)
	flw	ft11, 96(sp)
	fadd.s	ft0, ft10, ft11
	flw	ft10, 100(sp)
	fadd.s	ft0, ft0, ft10
	flw	ft10, 104(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 28(s1)
	flw	ft10, 108(sp)
	flw	ft11, 112(sp)
	fadd.s	ft0, ft10, ft11
	flw	ft10, 116(sp)
	fadd.s	ft0, ft0, ft10
	flw	ft10, 120(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 32(s1)
	flw	ft10, 124(sp)
	flw	ft11, 128(sp)
	fadd.s	ft0, ft10, ft11
	flw	ft10, 132(sp)
	fadd.s	ft0, ft0, ft10
	fadd.s	ft0, ft0, fa4
	fsw	ft0, 36(s1)
	li	a0, 10
	addi	a1, sp, 152
	call	putfarray
	la	t0, k
	lw	t0, 0(t0)
	slliw	t0, t0, 2
	add	t0, s1, t0
	flw	fa0, 0(t0)
	flw	fs11, 260(sp)
	flw	fs10, 256(sp)
	flw	fs9, 252(sp)
	flw	fs8, 248(sp)
	flw	fs7, 244(sp)
	flw	fs6, 240(sp)
	flw	fs5, 236(sp)
	flw	fs4, 232(sp)
	flw	fs3, 228(sp)
	flw	fs2, 224(sp)
	flw	fs1, 220(sp)
	flw	fs0, 216(sp)
	ld	s1, 208(sp)
	ld	ra, 200(sp)
	ld	s0, 192(sp)
	addi	sp, sp, 272
	ret
	.size params_f40, .-params_f40

  .globl params_f40_i24
  .type  params_f40_i24, @function
params_f40_i24:
.params_f40_i24.bb0:
	addi	sp, sp, -400
	sd	s0, 248(sp)
	sd	ra, 256(sp)
	sd	s1, 264(sp)
	sd	s2, 272(sp)
	sd	s3, 280(sp)
	sd	s4, 288(sp)
	sd	s5, 296(sp)
	sd	s6, 304(sp)
	sd	s7, 312(sp)
	sd	s8, 320(sp)
	sd	s9, 328(sp)
	sd	s10, 336(sp)
	sd	s11, 344(sp)
	fsw	fs0, 352(sp)
	fsw	fs1, 356(sp)
	fsw	fs2, 360(sp)
	fsw	fs3, 364(sp)
	fsw	fs4, 368(sp)
	fsw	fs5, 372(sp)
	fsw	fs6, 376(sp)
	fsw	fs7, 380(sp)
	fsw	fs8, 384(sp)
	fsw	fs9, 388(sp)
	fsw	fs10, 392(sp)
	fsw	fs11, 396(sp)
	addi	s0, sp, 400
	mv	t5, a0
	sd	t5, 16(sp)
	mv	t5, a1
	sd	t5, 24(sp)
	mv	t5, a2
	sd	t5, 32(sp)
	fmv.s	ft10, fa0
	fsw	ft10, 80(sp)
	mv	t5, a3
	sd	t5, 40(sp)
	mv	t5, a4
	sd	t5, 48(sp)
	mv	t5, a5
	sd	t5, 56(sp)
	fmv.s	ft10, fa1
	fsw	ft10, 84(sp)
	fmv.s	ft10, fa2
	fsw	ft10, 88(sp)
	fmv.s	ft10, fa3
	fsw	ft10, 92(sp)
	mv	a4, a6
	fmv.s	ft10, fa4
	fsw	ft10, 96(sp)
	fmv.s	ft10, fa5
	fsw	ft10, 100(sp)
	mv	a5, a7
	fmv.s	ft10, fa6
	fsw	ft10, 104(sp)
	lw	a6, 0(s0)
	fmv.s	ft10, fa7
	fsw	ft10, 108(sp)
	flw	ft10, 8(s0)
	fsw	ft10, 112(sp)
	flw	ft10, 16(s0)
	fsw	ft10, 116(sp)
	flw	ft10, 24(s0)
	fsw	ft10, 120(sp)
	flw	fs11, 32(s0)
	flw	fs10, 40(s0)
	lw	a7, 48(s0)
	flw	fa6, 56(s0)
	lw	t4, 64(s0)
	lw	s1, 72(s0)
	flw	fa7, 80(s0)
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
	flw	fa0, 200(s0)
	flw	fa1, 208(s0)
	lw	a0, 216(s0)
	flw	ft0, 224(s0)
	lw	a1, 232(s0)
	lw	t0, 240(s0)
	flw	ft1, 248(s0)
	flw	ft2, 256(s0)
	flw	ft3, 264(s0)
	flw	ft4, 272(s0)
	lw	t1, 280(s0)
	lw	t2, 288(s0)
	lw	t3, 296(s0)
	flw	ft5, 304(s0)
	flw	ft6, 312(s0)
	flw	ft7, 320(s0)
	flw	ft8, 328(s0)
	flw	fa2, 336(s0)
	flw	fa3, 344(s0)
	lw	a2, 352(s0)
	flw	fa4, 360(s0)
	lw	a3, 368(s0)
	flw	fa5, 376(s0)
	sd	t2, 72(sp)
	sd	s4, 64(sp)
	mv	s6, s1	# phi-copy
	ld	s1, 40(sp)
	ld	s7, 56(sp)
	ld	s8, 48(sp)
	ld	s9, 32(sp)
	ld	s10, 24(sp)
	ld	s11, 16(sp)
	fsw	fa5, 132(sp)
	fsw	fa3, 156(sp)
	fsw	fa2, 136(sp)
	fsw	ft8, 140(sp)
	fsw	ft7, 148(sp)
	fsw	ft4, 144(sp)
	fsw	ft2, 164(sp)
	fsw	ft1, 168(sp)
	fsw	ft0, 152(sp)
	fsw	fs8, 160(sp)
	fsw	fs4, 172(sp)
	flw	fa5, 120(sp)
	flw	fa2, 116(sp)
	flw	ft8, 112(sp)
	flw	ft4, 108(sp)
	flw	ft7, 104(sp)
	flw	ft0, 100(sp)
	flw	fa3, 96(sp)
	flw	fs8, 92(sp)
	flw	ft2, 88(sp)
	flw	ft1, 84(sp)
	flw	fs4, 80(sp)
.params_f40_i24.bb1:
	bne	s11, zero, .params_f40_i24.bb3
.params_f40_i24.bb4:
	mv	s11, s1	# phi-copy
	beq	s11, zero, .params_f40_i24.bb4	# rotated-loop-branch
.params_f40_i24.bb3:
	addi	t5, sp, 208
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	fadd.s	fa2, fa1, fa2
	fadd.s	fa2, fa2, fa4
	fadd.s	fa2, fa2, fa3
	addi	s4, sp, 208
	fsw	fa2, 0(s4)
	fadd.s	fa2, fs4, fs7
	fadd.s	fa2, fa2, fs10
	fadd.s	fa2, fa2, fs8
	fsw	fa2, 4(s4)
	fadd.s	ft1, ft1, fs2
	fadd.s	ft1, ft1, fa0
	fadd.s	ft1, ft1, fa5
	fsw	ft1, 8(s4)
	fadd.s	ft1, ft8, ft3
	fadd.s	ft1, ft1, fs9
	fadd.s	ft1, ft1, ft2
	fsw	ft1, 12(s4)
	flw	ft10, 156(sp)
	fadd.s	ft1, fs6, ft10
	fadd.s	ft1, ft1, ft5
	fadd.s	ft1, ft1, ft6
	fsw	ft1, 16(s4)
	fadd.s	ft1, fa7, fs5
	flw	ft10, 148(sp)
	fadd.s	ft1, ft1, ft10
	fadd.s	ft1, ft1, fs1
	fsw	ft1, 20(s4)
	flw	ft10, 164(sp)
	fadd.s	ft1, ft10, fs3
	flw	ft10, 140(sp)
	fadd.s	ft1, ft1, ft10
	flw	ft10, 168(sp)
	fadd.s	ft1, ft1, ft10
	fsw	ft1, 24(s4)
	flw	ft10, 144(sp)
	fadd.s	ft0, ft0, ft10
	fadd.s	ft0, ft0, ft4
	flw	ft10, 152(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 28(s4)
	fadd.s	ft0, fa6, fs0
	flw	ft10, 160(sp)
	fadd.s	ft0, ft0, ft10
	flw	ft10, 132(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 32(s4)
	flw	ft10, 136(sp)
	fadd.s	ft0, ft10, ft7
	fadd.s	ft0, ft0, fs11
	flw	ft10, 172(sp)
	fadd.s	ft0, ft0, ft10
	fsw	ft0, 36(s4)
	addi	t5, sp, 176
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	addw	t2, a5, s1
	addw	t2, t2, s10
	addi	s1, sp, 176
	sw	t2, 0(s1)
	addw	t1, t1, s8
	addw	t1, t1, s7
	sw	t1, 4(s1)
	addw	t1, s9, a7
	addw	t1, t1, s2
	sw	t1, 8(s1)
	addw	t1, s5, t4
	addw	t1, t1, a1
	sw	t1, 12(s1)
	addw	t1, a0, s6
	addw	t1, t1, a3
	sw	t1, 16(s1)
	addw	t0, a2, t0
	addw	t0, t0, s3
	sw	t0, 20(s1)
	ld	t5, 64(sp)
	addw	t0, t5, a6
	addw	t0, t0, t3
	sw	t0, 24(s1)
	ld	t5, 72(sp)
	addw	t0, t5, a4
	addw	t0, t0, s11
	sw	t0, 28(s1)
	li	a0, 10
	addi	a1, sp, 208
	call	putfarray
	li	a0, 8
	addi	a1, sp, 176
	call	putarray
	lw	t0, 0(s1)
	flw	ft0, 0(s4)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 0(s1)
	lw	t0, 4(s1)
	flw	ft0, 4(s4)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 4(s1)
	lw	t0, 8(s1)
	flw	ft0, 8(s4)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 8(s1)
	lw	t0, 12(s1)
	flw	ft0, 12(s4)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 12(s1)
	lw	t0, 16(s1)
	flw	ft0, 16(s4)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 16(s1)
	lw	t0, 20(s1)
	flw	ft0, 20(s4)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 20(s1)
	lw	t0, 24(s1)
	flw	ft0, 24(s4)
	fcvt.s.w	ft1, t0
	fsub.s	ft0, ft1, ft0
	fcvt.w.s	t0, ft0, rtz
	sw	t0, 24(s1)
	lw	t0, 28(s1)
	flw	ft0, 28(s4)
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
	flw	fs11, 396(sp)
	flw	fs10, 392(sp)
	flw	fs9, 388(sp)
	flw	fs8, 384(sp)
	flw	fs7, 380(sp)
	flw	fs6, 376(sp)
	flw	fs5, 372(sp)
	flw	fs4, 368(sp)
	flw	fs3, 364(sp)
	flw	fs2, 360(sp)
	flw	fs1, 356(sp)
	flw	fs0, 352(sp)
	ld	s11, 344(sp)
	ld	s10, 336(sp)
	ld	s9, 328(sp)
	ld	s8, 320(sp)
	ld	s7, 312(sp)
	ld	s6, 304(sp)
	ld	s5, 296(sp)
	ld	s4, 288(sp)
	ld	s3, 280(sp)
	ld	s2, 272(sp)
	ld	s1, 264(sp)
	ld	ra, 256(sp)
	ld	s0, 248(sp)
	addi	sp, sp, 400
	ret
	.size params_f40_i24, .-params_f40_i24

  .globl params_fa40
  .type  params_fa40, @function
params_fa40:
.params_fa40.bb0:
	addi	sp, sp, -464
	sd	s0, 352(sp)
	sd	ra, 360(sp)
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
	fsw	fs0, 456(sp)
	fsw	fs1, 460(sp)
	addi	s0, sp, 464
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
	ld	s9, 88(s0)
	ld	s10, 96(s0)
	ld	s11, 104(s0)
	ld	a0, 112(s0)
	ld	a1, 120(s0)
	ld	t0, 128(s0)
	ld	t1, 136(s0)
	ld	t2, 144(s0)
	ld	t3, 152(s0)
	ld	a2, 160(s0)
	ld	a3, 168(s0)
	ld	a4, 176(s0)
	ld	a5, 184(s0)
	ld	a6, 192(s0)
	ld	a7, 200(s0)
	ld	t4, 208(s0)
	ld	s1, 216(s0)
	ld	s2, 224(s0)
	ld	s3, 232(s0)
	ld	s4, 240(s0)
	ld	s5, 248(s0)
	la	s6, k
	lw	s7, 0(s6)
	slliw	s6, s7, 2
	li	s8, 0
	fmv.w.x	fs0, s8
	sd	s5, 168(sp)
	sd	s4, 176(sp)
	sd	s3, 184(sp)
	sd	s2, 192(sp)
	sd	s1, 200(sp)
	sd	t4, 208(sp)
	sd	a7, 216(sp)
	sd	a6, 224(sp)
	sd	a5, 232(sp)
	sd	a4, 240(sp)
	sd	a3, 248(sp)
	sd	a2, 256(sp)
	sd	t3, 264(sp)
	sd	t2, 272(sp)
	sd	t1, 280(sp)
	sd	t0, 288(sp)
	sd	a1, 296(sp)
	ld	s7, 160(sp)
	ld	s8, 152(sp)
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
	ld	t5, 48(sp)
	sd	t5, 304(sp)
	ld	t2, 40(sp)
	ld	t1, 32(sp)
	ld	t0, 24(sp)
	ld	a1, 16(sp)
.params_fa40.bb1:
	addi	t5, sp, 312
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	add	a1, a1, s6
	flw	ft0, 0(a1)
	add	a1, t0, s6
	flw	ft1, 0(a1)
	fadd.s	ft0, ft0, ft1
	add	a1, t1, s6
	flw	ft1, 0(a1)
	fadd.s	ft0, ft0, ft1
	add	a1, t2, s6
	flw	ft1, 0(a1)
	fadd.s	ft0, ft0, ft1
	addi	a1, sp, 312
	fsw	ft0, 0(a1)
	ld	t5, 304(sp)
	add	t3, t5, s6
	flw	ft0, 0(t3)
	add	t3, a2, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, a3, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, a4, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 4(a1)
	add	t3, a5, s6
	flw	ft0, 0(t3)
	add	t3, a6, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, a7, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, t4, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 8(a1)
	add	t3, s1, s6
	flw	ft0, 0(t3)
	add	t3, s2, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, s3, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, s4, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 12(a1)
	add	t3, s5, s6
	flw	ft0, 0(t3)
	add	t3, s8, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, s7, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, s9, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 16(a1)
	add	t3, s10, s6
	flw	ft0, 0(t3)
	add	t3, s11, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	add	t3, a0, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 296(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 20(a1)
	ld	t5, 288(sp)
	add	t3, t5, s6
	flw	ft0, 0(t3)
	ld	t5, 280(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 272(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 264(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 24(a1)
	ld	t5, 256(sp)
	add	t3, t5, s6
	flw	ft0, 0(t3)
	ld	t5, 248(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 240(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 232(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 28(a1)
	ld	t5, 224(sp)
	add	t3, t5, s6
	flw	ft0, 0(t3)
	ld	t5, 216(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 208(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 200(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 32(a1)
	ld	t5, 192(sp)
	add	t3, t5, s6
	flw	ft0, 0(t3)
	ld	t5, 184(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 176(sp)
	add	t3, t5, s6
	flw	ft1, 0(t3)
	fadd.s	ft0, ft0, ft1
	ld	t5, 168(sp)
	add	t3, t5, s6
	flw	fs1, 0(t3)
	fadd.s	ft0, ft0, fs1
	fsw	ft0, 36(a1)
	feq.s	t3, fs1, fs0
	xori	t3, t3, 1
	fcvt.s.w	ft0, t3
	feq.s	t3, ft0, fs0
	xori	t3, t3, 1
	fcvt.s.w	ft0, t3
	feq.s	t3, ft0, fs0
	beqz	t3, .params_fa40.bb3
.params_fa40.bb4:
	addi	t3, sp, 312
	mv	a1, t0	# phi-copy
	mv	t0, t1	# phi-copy
	mv	t1, t2	# phi-copy
	ld	t2, 304(sp)
	sd	a2, 304(sp)
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
	mv	s5, s8	# phi-copy
	mv	s8, s7	# phi-copy
	mv	s7, s9	# phi-copy
	mv	s9, s10	# phi-copy
	mv	s10, s11	# phi-copy
	mv	s11, a0	# phi-copy
	ld	a0, 296(sp)
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
	ld	t5, 176(sp)
	sd	t5, 184(sp)
	ld	t5, 168(sp)
	sd	t5, 176(sp)
	sd	t3, 168(sp)
	j	.params_fa40.bb1
.params_fa40.bb3:
	li	a0, 10
	addi	a1, sp, 312
	call	putfarray
	addi	t0, sp, 312
	add	t0, t0, s6
	flw	ft0, 0(t0)
.params_fa40.bb2:
	fmv.s	fa0, ft0
	flw	fs1, 460(sp)
	flw	fs0, 456(sp)
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
	ld	ra, 360(sp)
	ld	s0, 352(sp)
	addi	sp, sp, 464
	ret
	.size params_fa40, .-params_fa40

  .globl params_mix
  .type  params_mix, @function
params_mix:
.params_mix.bb0:
	addi	sp, sp, -624
	sd	s0, 496(sp)
	sd	ra, 504(sp)
	sd	s1, 512(sp)
	sd	s2, 520(sp)
	sd	s3, 528(sp)
	sd	s4, 536(sp)
	sd	s5, 544(sp)
	sd	s6, 552(sp)
	sd	s7, 560(sp)
	sd	s8, 568(sp)
	sd	s9, 576(sp)
	sd	s10, 584(sp)
	sd	s11, 592(sp)
	fsw	fs0, 600(sp)
	fsw	fs1, 604(sp)
	fsw	fs2, 608(sp)
	fsw	fs3, 612(sp)
	fsw	fs4, 616(sp)
	addi	s0, sp, 624
	fmv.s	ft10, fa0
	fsw	ft10, 376(sp)
	mv	t5, a0
	sd	t5, 16(sp)
	mv	t5, a1
	sd	t5, 24(sp)
	mv	t5, a2
	sd	t5, 32(sp)
	fmv.s	ft10, fa1
	fsw	ft10, 380(sp)
	mv	t5, a3
	sd	t5, 40(sp)
	fmv.s	ft10, fa2
	fsw	ft10, 384(sp)
	fmv.s	ft3, fa3
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
	fmv.s	ft4, fa4
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
	lw	a0, 144(s0)
	ld	a1, 152(s0)
	ld	t0, 160(s0)
	flw	fa5, 168(s0)
	flw	fa6, 176(s0)
	ld	t1, 184(s0)
	ld	t2, 192(s0)
	lw	t3, 200(s0)
	lw	a2, 208(s0)
	flw	fa7, 216(s0)
	flw	fs0, 224(s0)
	ld	a3, 232(s0)
	lw	a4, 240(s0)
	ld	a5, 248(s0)
	lw	a6, 256(s0)
	ld	a7, 264(s0)
	ld	t4, 272(s0)
	flw	fa0, 280(s0)
	flw	fa1, 288(s0)
	ld	s1, 296(s0)
	lw	s2, 304(s0)
	ld	s3, 312(s0)
	ld	s4, 320(s0)
	flw	ft0, 328(s0)
	lw	s5, 336(s0)
	flw	ft1, 344(s0)
	ld	s6, 352(s0)
	ld	s7, 360(s0)
	flw	ft2, 368(s0)
	lw	s8, 376(s0)
	la	s9, k
	lw	s10, 0(s9)
	slliw	t5, s10, 2
	sd	t5, 368(sp)
	li	s11, 0
	fmv.w.x	ft10, s11
	fsw	ft10, 388(sp)
	sd	s8, 192(sp)
	sd	s5, 200(sp)
	sd	s3, 216(sp)
	sd	s2, 208(sp)
	sd	t4, 224(sp)
	sd	a7, 240(sp)
	sd	a6, 232(sp)
	sd	a4, 248(sp)
	sd	a3, 264(sp)
	sd	a2, 256(sp)
	sd	t3, 272(sp)
	sd	t2, 280(sp)
	sd	t1, 288(sp)
	sd	a0, 296(sp)
	ld	t5, 184(sp)
	sd	t5, 304(sp)
	ld	s11, 176(sp)
	ld	t5, 168(sp)
	sd	t5, 312(sp)
	ld	t5, 160(sp)
	sd	t5, 320(sp)
	ld	s2, 152(sp)
	ld	t5, 144(sp)
	sd	t5, 336(sp)
	ld	t5, 136(sp)
	sd	t5, 328(sp)
	ld	a6, 128(sp)
	ld	a7, 120(sp)
	ld	a4, 112(sp)
	ld	a2, 104(sp)
	ld	a3, 96(sp)
	ld	t3, 88(sp)
	ld	t5, 80(sp)
	sd	t5, 360(sp)
	ld	t1, 72(sp)
	ld	a0, 64(sp)
	ld	s10, 56(sp)
	ld	t5, 48(sp)
	sd	t5, 352(sp)
	ld	s5, 40(sp)
	ld	t4, 32(sp)
	ld	s3, 24(sp)
	ld	t5, 16(sp)
	sd	t5, 344(sp)
	fsw	ft2, 404(sp)
	fsw	ft3, 408(sp)
	flw	ft10, 384(sp)
	fsw	ft10, 400(sp)
	flw	ft10, 380(sp)
	fsw	ft10, 396(sp)
	flw	ft10, 376(sp)
	fsw	ft10, 392(sp)
.params_mix.bb1:
	addi	t5, sp, 416
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	ld	t5, 368(sp)
	add	t4, t4, t5
	flw	fs1, 0(t4)
	flw	ft10, 392(sp)
	fadd.s	fs4, ft10, fs1
	flw	ft10, 396(sp)
	fadd.s	fs3, fs4, ft10
	flw	ft10, 400(sp)
	fadd.s	fs2, fs3, ft10
	addi	t4, sp, 416
	fsw	fs2, 0(t4)
	ld	t5, 352(sp)
	ld	t6, 368(sp)
	add	s8, t5, t6
	flw	ft2, 0(s8)
	flw	ft10, 408(sp)
	fadd.s	ft2, ft10, ft2
	ld	t5, 360(sp)
	ld	t6, 368(sp)
	add	t2, t5, t6
	flw	ft3, 0(t2)
	fadd.s	ft2, ft2, ft3
	ld	t5, 368(sp)
	add	t2, a4, t5
	flw	ft3, 0(t2)
	fadd.s	ft2, ft2, ft3
	fsw	ft2, 4(t4)
	ld	t5, 368(sp)
	add	t2, a7, t5
	flw	ft2, 0(t2)
	fadd.s	ft2, ft2, ft4
	fadd.s	ft2, ft2, ft5
	fadd.s	ft2, ft2, ft6
	fsw	ft2, 8(t4)
	ld	t5, 368(sp)
	add	t2, a6, t5
	flw	ft2, 0(t2)
	fadd.s	ft2, ft2, ft7
	fadd.s	ft2, ft2, ft8
	fadd.s	ft2, ft2, fa2
	fsw	ft2, 12(t4)
	ld	t5, 368(sp)
	add	t2, s2, t5
	flw	ft2, 0(t2)
	ld	t5, 368(sp)
	add	t2, s11, t5
	flw	ft3, 0(t2)
	fadd.s	ft2, ft2, ft3
	fadd.s	ft2, ft2, fa3
	fadd.s	ft2, ft2, fa4
	fsw	ft2, 16(t4)
	ld	t5, 368(sp)
	add	t2, a1, t5
	flw	ft2, 0(t2)
	ld	t5, 368(sp)
	add	t2, t0, t5
	flw	ft3, 0(t2)
	fadd.s	ft2, ft2, ft3
	fadd.s	ft2, ft2, fa5
	fadd.s	ft2, ft2, fa6
	fsw	ft2, 20(t4)
	fadd.s	ft2, fa7, fs0
	ld	t5, 368(sp)
	add	t2, a5, t5
	flw	ft3, 0(t2)
	fadd.s	ft2, ft2, ft3
	fadd.s	ft2, ft2, fa0
	fsw	ft2, 24(t4)
	ld	t5, 368(sp)
	add	t2, s1, t5
	flw	ft2, 0(t2)
	fadd.s	ft2, fa1, ft2
	ld	t5, 368(sp)
	add	t2, s4, t5
	flw	ft3, 0(t2)
	fadd.s	ft2, ft2, ft3
	fadd.s	ft2, ft2, ft0
	fsw	ft2, 28(t4)
	ld	t5, 368(sp)
	add	t2, s6, t5
	flw	ft2, 0(t2)
	fadd.s	ft2, ft1, ft2
	ld	t5, 368(sp)
	add	t2, s7, t5
	flw	ft3, 0(t2)
	fadd.s	ft2, ft2, ft3
	flw	ft10, 404(sp)
	fadd.s	fs1, ft2, ft10
	fsw	fs1, 32(t4)
	addi	t5, sp, 456
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	ld	t5, 344(sp)
	ld	t6, 368(sp)
	add	t2, t5, t6
	lw	t2, 0(t2)
	addw	t2, t2, s3
	addw	t2, t2, s5
	addi	s8, sp, 456
	sw	t2, 0(s8)
	ld	t5, 368(sp)
	add	t2, s10, t5
	lw	t2, 0(t2)
	addw	t2, t2, a0
	addw	t2, t2, t1
	addi	t4, sp, 456
	addi	t4, t4, 4
	sw	t2, 4(s8)
	ld	t5, 368(sp)
	add	t2, t3, t5
	lw	t2, 0(t2)
	ld	t5, 368(sp)
	add	t4, a3, t5
	lw	t4, 0(t4)
	addw	t2, t2, t4
	addw	t2, t2, a2
	addi	t4, sp, 456
	addi	t4, t4, 8
	sw	t2, 8(s8)
	ld	t5, 336(sp)
	ld	t6, 368(sp)
	add	t2, t5, t6
	lw	t2, 0(t2)
	ld	t5, 328(sp)
	addw	t2, t5, t2
	ld	t5, 320(sp)
	ld	t6, 368(sp)
	add	t4, t5, t6
	lw	t4, 0(t4)
	addw	t2, t2, t4
	addi	t4, sp, 456
	addi	t4, t4, 12
	sw	t2, 12(s8)
	ld	t5, 312(sp)
	ld	t6, 368(sp)
	add	t2, t5, t6
	lw	t2, 0(t2)
	ld	t5, 304(sp)
	ld	t6, 368(sp)
	add	t4, t5, t6
	lw	t4, 0(t4)
	addw	t2, t2, t4
	ld	t5, 296(sp)
	addw	t2, t2, t5
	addi	t4, sp, 456
	addi	t4, t4, 16
	sw	t2, 16(s8)
	ld	t5, 288(sp)
	ld	t6, 368(sp)
	add	t2, t5, t6
	lw	t2, 0(t2)
	ld	t5, 280(sp)
	ld	t6, 368(sp)
	add	t4, t5, t6
	lw	t4, 0(t4)
	addw	t2, t2, t4
	ld	t5, 272(sp)
	addw	t2, t2, t5
	addi	t4, sp, 456
	addi	t4, t4, 20
	sw	t2, 20(s8)
	ld	t5, 264(sp)
	ld	t6, 368(sp)
	add	t2, t5, t6
	lw	t2, 0(t2)
	ld	t5, 256(sp)
	addw	t2, t5, t2
	ld	t5, 248(sp)
	addw	t2, t2, t5
	addi	t4, sp, 456
	addi	t4, t4, 24
	sw	t2, 24(s8)
	ld	t5, 240(sp)
	ld	t6, 368(sp)
	add	t2, t5, t6
	lw	t2, 0(t2)
	ld	t5, 232(sp)
	addw	t2, t5, t2
	ld	t5, 224(sp)
	ld	t6, 368(sp)
	add	t4, t5, t6
	lw	t4, 0(t4)
	addw	t2, t2, t4
	addi	t4, sp, 456
	addi	t4, t4, 28
	sw	t2, 28(s8)
	ld	t5, 216(sp)
	ld	t6, 368(sp)
	add	t2, t5, t6
	lw	t2, 0(t2)
	ld	t5, 208(sp)
	addw	t2, t5, t2
	ld	t5, 200(sp)
	addw	t2, t2, t5
	ld	t5, 192(sp)
	addw	t2, t2, t5
	addi	t4, sp, 456
	addi	t4, t4, 32
	sw	t2, 32(s8)
	ld	t5, 192(sp)
	bne	t5, zero, .params_mix.bb3
.params_mix.bb4:
	flw	ft10, 404(sp)
	fcvt.w.s	t2, ft10, rtz
	addi	t4, sp, 456
	addi	s9, sp, 416
	sd	t2, 192(sp)
	sd	t4, 344(sp)
	mv	t4, s9	# phi-copy
	flw	ft10, 388(sp)
	fsw	ft10, 404(sp)
	j	.params_mix.bb1
.params_mix.bb3:
	li	a0, 10
	addi	a1, sp, 416
	call	putfarray
	li	a0, 10
	addi	a1, sp, 456
	call	putarray
	addi	t0, sp, 456
	lw	t1, 0(t0)
	addi	t2, sp, 416
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
	ld	t5, 368(sp)
	add	t0, t0, t5
	lw	t0, 0(t0)
	fcvt.s.w	ft0, t0
	fmul.s	ft0, ft0, fs1
	fcvt.w.s	t0, ft0, rtz
.params_mix.bb2:
	mv	a0, t0
	flw	fs4, 616(sp)
	flw	fs3, 612(sp)
	flw	fs2, 608(sp)
	flw	fs1, 604(sp)
	flw	fs0, 600(sp)
	ld	s11, 592(sp)
	ld	s10, 584(sp)
	ld	s9, 576(sp)
	ld	s8, 568(sp)
	ld	s7, 560(sp)
	ld	s6, 552(sp)
	ld	s5, 544(sp)
	ld	s4, 536(sp)
	ld	s3, 528(sp)
	ld	s2, 520(sp)
	ld	s1, 512(sp)
	ld	ra, 504(sp)
	ld	s0, 496(sp)
	addi	sp, sp, 624
	ret
	.size params_mix, .-params_mix

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -1584
	sd	ra, 1448(sp)
	sd	s1, 1456(sp)
	sd	s2, 1464(sp)
	sd	s3, 1472(sp)
	sd	s4, 1480(sp)
	sd	s5, 1488(sp)
	sd	s6, 1496(sp)
	sd	s7, 1504(sp)
	sd	s8, 1512(sp)
	sd	s9, 1520(sp)
	sd	s10, 1528(sp)
	fsw	fs0, 1536(sp)
	fsw	fs1, 1540(sp)
	fsw	fs2, 1544(sp)
	fsw	fs3, 1548(sp)
	fsw	fs4, 1552(sp)
	fsw	fs5, 1556(sp)
	fsw	fs6, 1560(sp)
	fsw	fs7, 1564(sp)
	fsw	fs8, 1568(sp)
	fsw	fs9, 1572(sp)
	fsw	fs10, 1576(sp)
	fsw	fs11, 1580(sp)
	call	getint
	mv	t0, a0
	la	t1, k
	sw	t0, 0(t1)
	addi	s1, sp, 968
	addi	s2, sp, 680
	li	t0, 0
	li	s3, 40
	mv	s4, t0	# phi-copy
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
	li	t0, 0
	li	s3, 24
	mv	s4, t0	# phi-copy
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
	flw	fa0, 324(t0)
	flw	fa1, 336(t0)
	flw	ft10, 348(t0)
	fsw	ft10, 584(sp)
	flw	ft10, 360(t0)
	fsw	ft10, 588(sp)
	flw	ft10, 372(t0)
	fsw	ft10, 592(sp)
	flw	ft10, 384(t0)
	fsw	ft10, 596(sp)
	flw	ft10, 396(t0)
	fsw	ft10, 600(sp)
	flw	ft10, 408(t0)
	fsw	ft10, 604(sp)
	flw	ft10, 420(t0)
	fsw	ft10, 608(sp)
	flw	ft10, 432(t0)
	fsw	ft10, 612(sp)
	flw	ft10, 444(t0)
	fsw	ft10, 616(sp)
	flw	ft10, 456(t0)
	fsw	ft10, 620(sp)
	flw	ft10, 468(t0)
	fsw	ft10, 624(sp)
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
	fmv.s	ft10, fa0
	fsw	ft10, 216(sp)
	fmv.s	ft10, fa1
	fsw	ft10, 224(sp)
	flw	ft10, 584(sp)
	fsw	ft10, 232(sp)
	flw	ft10, 588(sp)
	fsw	ft10, 240(sp)
	flw	ft10, 592(sp)
	fsw	ft10, 248(sp)
	flw	ft10, 596(sp)
	fsw	ft10, 256(sp)
	flw	ft10, 600(sp)
	fsw	ft10, 264(sp)
	flw	ft10, 604(sp)
	fsw	ft10, 272(sp)
	flw	ft10, 608(sp)
	fsw	ft10, 280(sp)
	flw	ft10, 612(sp)
	fsw	ft10, 288(sp)
	flw	ft10, 616(sp)
	fsw	ft10, 296(sp)
	flw	ft10, 620(sp)
	fsw	ft10, 304(sp)
	flw	ft10, 624(sp)
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
	fsw	ft10, 628(sp)
	ld	t5, 512(sp)
	lw	t0, 0(t5)
	slliw	t0, t0, 2
	addiw	t1, t0, 276
	add	t1, s2, t1
	lw	t2, 0(t1)
	lw	a2, -252(t1)
	lw	a3, -204(t1)
	addiw	t0, t0, 48
	add	t0, s1, t0
	flw	ft0, 0(t0)
	lw	a4, -264(t1)
	lw	a5, -228(t1)
	lw	a6, -216(t1)
	addi	a7, t0, 48
	flw	ft1, 48(t0)
	addi	a7, a7, 84
	flw	ft2, 132(t0)
	addi	a7, a7, -96
	flw	ft3, 36(t0)
	lw	t4, -12(t1)
	addi	a7, a7, -48
	flw	ft4, -12(t0)
	addi	a7, a7, 300
	flw	ft5, 288(t0)
	lw	s4, -276(t1)
	addi	a7, a7, 108
	flw	ft6, 396(t0)
	lw	s5, -48(t1)
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
	lw	s6, -192(t1)
	addi	a7, a7, 312
	flw	fa6, 336(t0)
	lw	s7, -156(t1)
	lw	s8, -120(t1)
	addi	a7, a7, -144
	flw	fa7, 192(t0)
	addi	a7, a7, 156
	flw	fs1, 348(t0)
	addi	a0, a7, -120
	flw	fs2, 228(t0)
	addi	a0, a0, -168
	flw	fs3, 60(t0)
	addi	a0, a0, 192
	flw	fs4, 252(t0)
	lw	s9, -180(t1)
	addi	a0, a0, 168
	flw	fs5, 420(t0)
	lw	s10, -72(t1)
	addi	a0, a0, -216
	flw	fs6, 204(t0)
	addi	a0, a0, -60
	flw	fs7, 144(t0)
	flw	fs8, 12(t0)
	addi	a0, a0, 216
	flw	fs9, 360(t0)
	lw	t5, -60(t1)
	sd	t5, 528(sp)
	lw	t5, -168(t1)
	sd	t5, 520(sp)
	addi	s3, a0, -240
	flw	fs10, 120(t0)
	addi	a1, s3, -48
	flw	fs11, 72(t0)
	flw	fa0, -48(t0)
	lw	t5, -132(t1)
	sd	t5, 536(sp)
	flw	fa1, 324(t0)
	lw	t5, -144(t1)
	sd	t5, 552(sp)
	lw	t5, -84(t1)
	sd	t5, 544(sp)
	flw	fs0, 276(t0)
	addi	a1, a1, 168
	flw	ft10, 240(t0)
	fsw	ft10, 632(sp)
	addi	a1, a1, -132
	flw	ft10, 108(t0)
	fsw	ft10, 636(sp)
	flw	ft10, 300(t0)
	fsw	ft10, 640(sp)
	lw	s3, -240(t1)
	lw	t5, -24(t1)
	sd	t5, 560(sp)
	lw	t5, -36(t1)
	sd	t5, 568(sp)
	addi	a1, a1, 60
	flw	ft10, 168(t0)
	fsw	ft10, 644(sp)
	addi	a1, a1, 12
	flw	ft10, 180(t0)
	fsw	ft10, 648(sp)
	addi	a1, a1, 36
	flw	ft10, 216(t0)
	fsw	ft10, 652(sp)
	addi	t5, a1, 48
	sd	t5, 576(sp)
	flw	ft10, 264(t0)
	fsw	ft10, 656(sp)
	flw	ft10, 384(t0)
	fsw	ft10, 660(sp)
	flw	ft10, 156(t0)
	fsw	ft10, 664(sp)
	lw	t3, -96(t1)
	flw	ft10, -24(t0)
	fsw	ft10, 668(sp)
	lw	t1, -108(t1)
	flw	ft10, 372(t0)
	fsw	ft10, 672(sp)
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
	mv	t5, a5
	sd	t5, 40(sp)
	mv	t5, a6
	sd	t5, 48(sp)
	fmv.s	ft10, ft1
	fsw	ft10, 56(sp)
	fmv.s	ft10, ft2
	fsw	ft10, 64(sp)
	fmv.s	ft10, ft3
	fsw	ft10, 72(sp)
	mv	t5, t4
	sd	t5, 80(sp)
	fmv.s	ft10, ft4
	fsw	ft10, 88(sp)
	fmv.s	ft10, ft5
	fsw	ft10, 96(sp)
	mv	t5, s4
	sd	t5, 104(sp)
	fmv.s	ft10, ft6
	fsw	ft10, 112(sp)
	mv	t5, s5
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
	mv	t5, s6
	sd	t5, 176(sp)
	fmv.s	ft10, fa6
	fsw	ft10, 184(sp)
	mv	t5, s7
	sd	t5, 192(sp)
	mv	t5, s8
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
	mv	t5, s9
	sd	t5, 248(sp)
	fmv.s	ft10, fs5
	fsw	ft10, 256(sp)
	mv	t5, s10
	sd	t5, 264(sp)
	fmv.s	ft10, fs6
	fsw	ft10, 272(sp)
	fmv.s	ft10, fs7
	fsw	ft10, 280(sp)
	fmv.s	ft10, fs8
	fsw	ft10, 288(sp)
	fmv.s	ft10, fs9
	fsw	ft10, 296(sp)
	ld	t5, 528(sp)
	sd	t5, 304(sp)
	ld	t5, 520(sp)
	sd	t5, 312(sp)
	fmv.s	ft10, fs10
	fsw	ft10, 320(sp)
	fmv.s	ft10, fs11
	fsw	ft10, 328(sp)
	fmv.s	ft10, fa0
	fsw	ft10, 336(sp)
	ld	t5, 536(sp)
	sd	t5, 344(sp)
	fmv.s	ft10, fa1
	fsw	ft10, 352(sp)
	ld	t5, 552(sp)
	sd	t5, 360(sp)
	ld	t5, 544(sp)
	sd	t5, 368(sp)
	fmv.s	ft10, fs0
	fsw	ft10, 376(sp)
	flw	ft10, 632(sp)
	fsw	ft10, 384(sp)
	flw	ft10, 636(sp)
	fsw	ft10, 392(sp)
	flw	ft10, 640(sp)
	fsw	ft10, 400(sp)
	mv	t5, s3
	sd	t5, 408(sp)
	ld	t5, 560(sp)
	sd	t5, 416(sp)
	ld	t5, 568(sp)
	sd	t5, 424(sp)
	flw	ft10, 644(sp)
	fsw	ft10, 432(sp)
	flw	ft10, 648(sp)
	fsw	ft10, 440(sp)
	flw	ft10, 652(sp)
	fsw	ft10, 448(sp)
	flw	ft10, 656(sp)
	fsw	ft10, 456(sp)
	flw	ft10, 660(sp)
	fsw	ft10, 464(sp)
	flw	ft10, 664(sp)
	fsw	ft10, 472(sp)
	mv	t5, t3
	sd	t5, 480(sp)
	flw	ft10, 668(sp)
	fsw	ft10, 488(sp)
	mv	t5, t1
	sd	t5, 496(sp)
	flw	ft10, 672(sp)
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
	addi	t5, sp, 968
	sd	t5, 0(sp)
	addi	t5, sp, 968
	addi	t5, t5, 12
	sd	t5, 8(sp)
	addi	t5, sp, 968
	addi	t5, t5, 24
	sd	t5, 16(sp)
	addi	t5, sp, 968
	addi	t5, t5, 36
	sd	t5, 24(sp)
	addi	t5, sp, 968
	addi	t5, t5, 48
	sd	t5, 32(sp)
	addi	t5, sp, 968
	addi	t5, t5, 60
	sd	t5, 40(sp)
	addi	t5, sp, 968
	addi	t5, t5, 72
	sd	t5, 48(sp)
	addi	t5, sp, 968
	addi	t5, t5, 84
	sd	t5, 56(sp)
	addi	t5, sp, 968
	addi	t5, t5, 96
	sd	t5, 64(sp)
	addi	t5, sp, 968
	addi	t5, t5, 108
	sd	t5, 72(sp)
	addi	t5, sp, 968
	addi	t5, t5, 120
	sd	t5, 80(sp)
	addi	t5, sp, 968
	addi	t5, t5, 132
	sd	t5, 88(sp)
	addi	t5, sp, 968
	addi	t5, t5, 144
	sd	t5, 96(sp)
	addi	t5, sp, 968
	addi	t5, t5, 156
	sd	t5, 104(sp)
	addi	t5, sp, 968
	addi	t5, t5, 168
	sd	t5, 112(sp)
	addi	t5, sp, 968
	addi	t5, t5, 180
	sd	t5, 120(sp)
	addi	t5, sp, 968
	addi	t5, t5, 192
	sd	t5, 128(sp)
	addi	t5, sp, 968
	addi	t5, t5, 204
	sd	t5, 136(sp)
	addi	t5, sp, 968
	addi	t5, t5, 216
	sd	t5, 144(sp)
	addi	t5, sp, 968
	addi	t5, t5, 228
	sd	t5, 152(sp)
	addi	t5, sp, 968
	addi	t5, t5, 240
	sd	t5, 160(sp)
	addi	t5, sp, 968
	addi	t5, t5, 252
	sd	t5, 168(sp)
	addi	t5, sp, 968
	addi	t5, t5, 264
	sd	t5, 176(sp)
	addi	t5, sp, 968
	addi	t5, t5, 276
	sd	t5, 184(sp)
	addi	t5, sp, 968
	addi	t5, t5, 288
	sd	t5, 192(sp)
	addi	t5, sp, 968
	addi	t5, t5, 300
	sd	t5, 200(sp)
	addi	t5, sp, 968
	addi	t5, t5, 312
	sd	t5, 208(sp)
	addi	t5, sp, 968
	addi	t5, t5, 324
	sd	t5, 216(sp)
	addi	t5, sp, 968
	addi	t5, t5, 336
	sd	t5, 224(sp)
	addi	t5, sp, 968
	addi	t5, t5, 348
	sd	t5, 232(sp)
	addi	t5, sp, 968
	addi	t5, t5, 360
	sd	t5, 240(sp)
	addi	t5, sp, 968
	addi	t5, t5, 372
	sd	t5, 248(sp)
	addi	t5, sp, 968
	addi	t5, t5, 384
	sd	t5, 256(sp)
	addi	t5, sp, 968
	addi	t5, t5, 396
	sd	t5, 264(sp)
	addi	t5, sp, 968
	addi	t5, t5, 408
	sd	t5, 272(sp)
	addi	t5, sp, 968
	addi	t5, t5, 420
	sd	t5, 280(sp)
	addi	t5, sp, 968
	addi	t5, t5, 432
	sd	t5, 288(sp)
	addi	t5, sp, 968
	addi	t5, t5, 444
	sd	t5, 296(sp)
	addi	t5, sp, 968
	addi	t5, t5, 456
	sd	t5, 304(sp)
	addi	t5, sp, 968
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
	addi	t4, sp, 680
	addi	t4, t4, 120
	addi	t4, sp, 680
	addi	t4, t4, 132
	addi	t4, sp, 680
	addi	t4, t4, 144
	flw	fa3, 216(t1)
	flw	fa4, 228(t1)
	addi	t4, sp, 680
	addi	t4, t4, 156
	lw	t4, 156(t0)
	flw	fa5, 264(t1)
	flw	fa6, 276(t1)
	lw	s1, 192(t0)
	lw	s2, 204(t0)
	flw	fa7, 288(t1)
	flw	fs2, 300(t1)
	lw	s3, 228(t0)
	lw	s4, 240(t0)
	flw	fs3, 324(t1)
	flw	fs4, 336(t1)
	lw	s5, -12(t0)
	flw	fs5, 372(t1)
	flw	fs6, 384(t1)
	flw	fs7, 420(t1)
	lw	t0, 24(t0)
	fmv.s	ft10, ft0
	fsw	ft10, 0(sp)
	addi	t5, sp, 680
	sd	t5, 8(sp)
	mv	t5, t2
	sd	t5, 16(sp)
	addi	t5, sp, 968
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
	addi	t5, sp, 968
	addi	t5, t5, 60
	sd	t5, 64(sp)
	addi	t5, sp, 680
	addi	t5, t5, 36
	sd	t5, 72(sp)
	mv	t5, a4
	sd	t5, 80(sp)
	mv	t5, a5
	sd	t5, 88(sp)
	addi	t5, sp, 968
	addi	t5, t5, 72
	sd	t5, 96(sp)
	addi	t5, sp, 680
	addi	t5, t5, 72
	sd	t5, 104(sp)
	addi	t5, sp, 680
	addi	t5, t5, 84
	sd	t5, 112(sp)
	mv	t5, a6
	sd	t5, 120(sp)
	addi	t5, sp, 968
	addi	t5, t5, 84
	sd	t5, 128(sp)
	addi	t5, sp, 968
	addi	t5, t5, 96
	sd	t5, 136(sp)
	fmv.s	ft10, ft4
	fsw	ft10, 144(sp)
	fmv.s	ft10, ft5
	fsw	ft10, 152(sp)
	fmv.s	ft10, ft6
	fsw	ft10, 160(sp)
	addi	t5, sp, 968
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
	addi	t5, sp, 680
	addi	t5, t5, 120
	sd	t5, 208(sp)
	addi	t5, sp, 968
	addi	t5, t5, 192
	sd	t5, 216(sp)
	addi	t5, sp, 680
	addi	t5, t5, 132
	sd	t5, 224(sp)
	addi	t5, sp, 680
	addi	t5, t5, 144
	sd	t5, 232(sp)
	addi	t5, sp, 968
	addi	t5, t5, 204
	sd	t5, 240(sp)
	fmv.s	ft10, fa3
	fsw	ft10, 248(sp)
	fmv.s	ft10, fa4
	fsw	ft10, 256(sp)
	addi	t5, sp, 680
	addi	t5, t5, 156
	sd	t5, 264(sp)
	mv	t5, t4
	sd	t5, 272(sp)
	addi	t5, sp, 968
	addi	t5, t5, 240
	sd	t5, 280(sp)
	addi	t5, sp, 968
	addi	t5, t5, 252
	sd	t5, 288(sp)
	fmv.s	ft10, fa5
	fsw	ft10, 296(sp)
	fmv.s	ft10, fa6
	fsw	ft10, 304(sp)
	addi	t5, sp, 680
	addi	t5, t5, 180
	sd	t5, 312(sp)
	addi	t5, sp, 680
	addi	t5, t5, 192
	sd	t5, 320(sp)
	mv	t5, s1
	sd	t5, 328(sp)
	mv	t5, s2
	sd	t5, 336(sp)
	fmv.s	ft10, fa7
	fsw	ft10, 344(sp)
	fmv.s	ft10, fs2
	fsw	ft10, 352(sp)
	addi	t5, sp, 680
	addi	t5, t5, 228
	sd	t5, 360(sp)
	mv	t5, s3
	sd	t5, 368(sp)
	addi	t5, sp, 968
	addi	t5, t5, 312
	sd	t5, 376(sp)
	mv	t5, s4
	sd	t5, 384(sp)
	addi	t5, sp, 680
	addi	t5, t5, 264
	sd	t5, 392(sp)
	addi	t5, sp, 680
	addi	t5, t5, 276
	sd	t5, 400(sp)
	fmv.s	ft10, fs3
	fsw	ft10, 408(sp)
	fmv.s	ft10, fs4
	fsw	ft10, 416(sp)
	addi	t5, sp, 968
	addi	t5, t5, 348
	sd	t5, 424(sp)
	mv	t5, s5
	sd	t5, 432(sp)
	addi	t5, sp, 680
	addi	t5, t5, 12
	sd	t5, 440(sp)
	addi	t5, sp, 968
	addi	t5, t5, 360
	sd	t5, 448(sp)
	fmv.s	ft10, fs5
	fsw	ft10, 456(sp)
	mv	t5, a3
	sd	t5, 464(sp)
	fmv.s	ft10, fs6
	fsw	ft10, 472(sp)
	addi	t5, sp, 968
	addi	t5, t5, 396
	sd	t5, 480(sp)
	addi	t5, sp, 968
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
	flw	fa0, 628(sp)
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
	flw	fs11, 1580(sp)
	flw	fs10, 1576(sp)
	flw	fs9, 1572(sp)
	flw	fs8, 1568(sp)
	flw	fs7, 1564(sp)
	flw	fs6, 1560(sp)
	flw	fs5, 1556(sp)
	flw	fs4, 1552(sp)
	flw	fs3, 1548(sp)
	flw	fs2, 1544(sp)
	flw	fs1, 1540(sp)
	flw	fs0, 1536(sp)
	ld	s10, 1528(sp)
	ld	s9, 1520(sp)
	ld	s8, 1512(sp)
	ld	s7, 1504(sp)
	ld	s6, 1496(sp)
	ld	s5, 1488(sp)
	ld	s4, 1480(sp)
	ld	s3, 1472(sp)
	ld	s2, 1464(sp)
	ld	s1, 1456(sp)
	ld	ra, 1448(sp)
	addi	sp, sp, 1584
	ret
	.size main, .-main
