
  .section .rodata
  .globl e
  .align 2
  .type  e, @object
  .size  e, 4
e:
  .word 1076754516

  .text

  .globl my_pow
  .type  my_pow, @function
my_pow:
.my_pow.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	fsw	fs0, 24(sp)
	fsw	fs1, 28(sp)
	fmv.s	ft0, fa0
	mv	t0, a0
	li	t1, 1065353216
	fmv.w.x	fs0, t1
	bge	t0, zero, .my_pow.bb4
.my_pow.bb2:
	subw	t0, zero, t0
	bge	t0, zero, .my_pow.bb11
.my_pow.bb10:
	subw	t0, zero, t0
	bge	t0, zero, .my_pow.bb19
.my_pow.bb18:
	subw	t0, zero, t0
	bge	t0, zero, .my_pow.bb27
.my_pow.bb26:
	subw	t0, zero, t0
	bge	t0, zero, .my_pow.bb35
.my_pow.bb34:
	subw	a0, zero, t0
	fmv.s	fa0, ft0	# call-arg-copy
	call	my_pow
	fmv.s	ft0, fa0
	fdiv.s	fs1, fs0, ft0
.my_pow.bb33:
	fdiv.s	fs1, fs0, fs1
.my_pow.bb25:
	fdiv.s	fs1, fs0, fs1
.my_pow.bb17:
	fdiv.s	fs1, fs0, fs1
.my_pow.bb9:
	fdiv.s	fs0, fs0, fs1
.my_pow.bb1:
	fmv.s	fa0, fs0
	flw	fs1, 28(sp)
	flw	fs0, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
.my_pow.bb5:
	andi	t1, t0, 1
	beq	t1, zero, .my_pow.bb41
.my_pow.bb7:
	fmul.s	ft1, fs0, ft0
.my_pow.bb8:
	fmul.s	ft0, ft0, ft0
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t1
	sraiw	t0, t0, 1
	fmv.s	fs0, ft1	# phi-copy
	bne	t0, zero, .my_pow.bb5	# rotated-loop-branch
	j	.my_pow.bb1	# rotated-loop-exit
.my_pow.bb6:
	j	.my_pow.bb1
.my_pow.bb41:
	fmv.s	ft1, fs0	# phi-copy
	j	.my_pow.bb8
.my_pow.bb13:
	andi	t1, t0, 1
	beq	t1, zero, .my_pow.bb42
.my_pow.bb15:
	fmul.s	ft1, fs1, ft0
.my_pow.bb16:
	fmul.s	ft0, ft0, ft0
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t1
	sraiw	t0, t0, 1
	fmv.s	fs1, ft1	# phi-copy
	bne	t0, zero, .my_pow.bb13	# rotated-loop-branch
	j	.my_pow.bb9	# rotated-loop-exit
.my_pow.bb14:
	j	.my_pow.bb9
.my_pow.bb42:
	fmv.s	ft1, fs1	# phi-copy
	j	.my_pow.bb16
.my_pow.bb21:
	andi	t1, t0, 1
	beq	t1, zero, .my_pow.bb43
.my_pow.bb23:
	fmul.s	ft1, fs1, ft0
.my_pow.bb24:
	fmul.s	ft0, ft0, ft0
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t1
	sraiw	t0, t0, 1
	fmv.s	fs1, ft1	# phi-copy
	bne	t0, zero, .my_pow.bb21	# rotated-loop-branch
	j	.my_pow.bb17	# rotated-loop-exit
.my_pow.bb22:
	j	.my_pow.bb17
.my_pow.bb43:
	fmv.s	ft1, fs1	# phi-copy
	j	.my_pow.bb24
.my_pow.bb29:
	andi	t1, t0, 1
	beq	t1, zero, .my_pow.bb44
.my_pow.bb31:
	fmul.s	ft1, fs1, ft0
.my_pow.bb32:
	fmul.s	ft0, ft0, ft0
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t1
	sraiw	t0, t0, 1
	fmv.s	fs1, ft1	# phi-copy
	bne	t0, zero, .my_pow.bb29	# rotated-loop-branch
	j	.my_pow.bb25	# rotated-loop-exit
.my_pow.bb30:
	j	.my_pow.bb25
.my_pow.bb44:
	fmv.s	ft1, fs1	# phi-copy
	j	.my_pow.bb32
.my_pow.bb37:
	andi	t1, t0, 1
	beq	t1, zero, .my_pow.bb45
.my_pow.bb39:
	fmul.s	ft1, fs1, ft0
.my_pow.bb40:
	fmul.s	ft0, ft0, ft0
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t1
	sraiw	t0, t0, 1
	fmv.s	fs1, ft1	# phi-copy
	bne	t0, zero, .my_pow.bb37	# rotated-loop-branch
	j	.my_pow.bb33	# rotated-loop-exit
.my_pow.bb38:
	j	.my_pow.bb33
.my_pow.bb45:
	fmv.s	ft1, fs1	# phi-copy
	j	.my_pow.bb40
.my_pow.bb11:
	fmv.s	fs1, fs0	# phi-copy
.my_pow.bb12:
	beq	t0, zero, .my_pow.bb9
	j	.my_pow.bb13
.my_pow.bb19:
	fmv.s	fs1, fs0	# phi-copy
.my_pow.bb20:
	beq	t0, zero, .my_pow.bb17
	j	.my_pow.bb21
.my_pow.bb27:
	fmv.s	fs1, fs0	# phi-copy
.my_pow.bb28:
	beq	t0, zero, .my_pow.bb25
	j	.my_pow.bb29
.my_pow.bb35:
	fmv.s	fs1, fs0	# phi-copy
.my_pow.bb36:
	beq	t0, zero, .my_pow.bb33
	j	.my_pow.bb37
.my_pow.bb3:
.my_pow.bb4:
	beq	t0, zero, .my_pow.bb1
	j	.my_pow.bb5
	.size my_pow, .-my_pow

  .globl my_sqrt
  .type  my_sqrt, @function
my_sqrt:
.my_sqrt.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	fsw	fs0, 16(sp)
	fsw	fs1, 20(sp)
	fsw	fs2, 24(sp)
	fmv.s	fs0, fa0
	li	t0, 1120403456
	fmv.w.x	ft0, t0
	flt.s	t0, ft0, fs0
	li	t1, 1090519040
	fmv.w.x	ft1, t1
	li	t1, 1056964608
	fmv.w.x	ft2, t1
	li	t1, 1073741824
	fmv.w.x	fs1, t1
	li	t1, 1082130432
	fmv.w.x	ft3, t1
	li	t1, 1092616192
	fmv.w.x	fs2, t1
	beqz	t0, .my_sqrt.bb3
.my_sqrt.bb2:
	fdiv.s	fs0, fs0, ft0
	flt.s	t0, ft0, fs0
	beqz	t0, .my_sqrt.bb9
.my_sqrt.bb8:
	fdiv.s	fs0, fs0, ft0
	flt.s	t0, ft0, fs0
	beqz	t0, .my_sqrt.bb15
.my_sqrt.bb14:
	fdiv.s	fs0, fs0, ft0
	flt.s	t0, ft0, fs0
	beqz	t0, .my_sqrt.bb21
.my_sqrt.bb20:
	fdiv.s	fs0, fs0, ft0
	flt.s	t0, ft0, fs0
	beqz	t0, .my_sqrt.bb27
.my_sqrt.bb26:
	fdiv.s	fa0, fs0, ft0
	call	my_sqrt
	fmv.s	ft0, fa0
	fmul.s	ft0, fs2, ft0
.my_sqrt.bb25:
	fmul.s	ft0, fs2, ft0
.my_sqrt.bb19:
	fmul.s	ft0, fs2, ft0
.my_sqrt.bb13:
	fmul.s	ft0, fs2, ft0
.my_sqrt.bb7:
	fmul.s	ft0, fs2, ft0
.my_sqrt.bb1:
	fmv.s	fa0, ft0
	flw	fs2, 24(sp)
	flw	fs1, 20(sp)
	flw	fs0, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.my_sqrt.bb5:
	fdiv.s	ft1, fs0, ft0
	addiw	t0, t0, -1
	fadd.s	ft0, ft0, ft1
	fdiv.s	ft0, ft0, fs1
	bne	t0, zero, .my_sqrt.bb5	# rotated-loop-branch
	j	.my_sqrt.bb1	# rotated-loop-exit
.my_sqrt.bb6:
	j	.my_sqrt.bb1
.my_sqrt.bb11:
	fdiv.s	ft1, fs0, ft0
	addiw	t0, t0, -1
	fadd.s	ft0, ft0, ft1
	fdiv.s	ft0, ft0, fs1
	bne	t0, zero, .my_sqrt.bb11	# rotated-loop-branch
	j	.my_sqrt.bb7	# rotated-loop-exit
.my_sqrt.bb12:
	j	.my_sqrt.bb7
.my_sqrt.bb17:
	fdiv.s	ft1, fs0, ft0
	addiw	t0, t0, -1
	fadd.s	ft0, ft0, ft1
	fdiv.s	ft0, ft0, fs1
	bne	t0, zero, .my_sqrt.bb17	# rotated-loop-branch
	j	.my_sqrt.bb13	# rotated-loop-exit
.my_sqrt.bb18:
	j	.my_sqrt.bb13
.my_sqrt.bb23:
	fdiv.s	ft1, fs0, ft0
	addiw	t0, t0, -1
	fadd.s	ft0, ft0, ft1
	fdiv.s	ft0, ft0, fs1
	bne	t0, zero, .my_sqrt.bb23	# rotated-loop-branch
	j	.my_sqrt.bb19	# rotated-loop-exit
.my_sqrt.bb24:
	j	.my_sqrt.bb19
.my_sqrt.bb29:
	fdiv.s	ft1, fs0, ft0
	addiw	t0, t0, -1
	fadd.s	ft0, ft0, ft1
	fdiv.s	ft0, ft0, fs1
	bne	t0, zero, .my_sqrt.bb29	# rotated-loop-branch
	j	.my_sqrt.bb25	# rotated-loop-exit
.my_sqrt.bb30:
	j	.my_sqrt.bb25
.my_sqrt.bb9:
	fdiv.s	ft0, fs0, ft1
	li	t0, 10
	fadd.s	ft3, ft3, fs0
	fmul.s	ft1, fs1, fs0
	fadd.s	ft0, ft0, ft2
	fdiv.s	ft1, ft1, ft3
	fadd.s	ft0, ft0, ft1
.my_sqrt.bb10:
	beq	t0, zero, .my_sqrt.bb7
	j	.my_sqrt.bb11
.my_sqrt.bb15:
	fdiv.s	ft0, fs0, ft1
	li	t0, 10
	fadd.s	ft3, ft3, fs0
	fmul.s	ft1, fs1, fs0
	fadd.s	ft0, ft0, ft2
	fdiv.s	ft1, ft1, ft3
	fadd.s	ft0, ft0, ft1
.my_sqrt.bb16:
	beq	t0, zero, .my_sqrt.bb13
	j	.my_sqrt.bb17
.my_sqrt.bb21:
	fdiv.s	ft0, fs0, ft1
	li	t0, 10
	fadd.s	ft3, ft3, fs0
	fmul.s	ft1, fs1, fs0
	fadd.s	ft0, ft0, ft2
	fdiv.s	ft1, ft1, ft3
	fadd.s	ft0, ft0, ft1
.my_sqrt.bb22:
	beq	t0, zero, .my_sqrt.bb19
	j	.my_sqrt.bb23
.my_sqrt.bb27:
	fdiv.s	ft0, fs0, ft1
	li	t0, 10
	fadd.s	ft3, ft3, fs0
	fmul.s	ft1, fs1, fs0
	fadd.s	ft0, ft0, ft2
	fdiv.s	ft1, ft1, ft3
	fadd.s	ft0, ft0, ft1
.my_sqrt.bb28:
	beq	t0, zero, .my_sqrt.bb25
	j	.my_sqrt.bb29
.my_sqrt.bb3:
	fdiv.s	ft0, fs0, ft1
	li	t0, 10
	fadd.s	ft3, ft3, fs0
	fmul.s	ft1, fs1, fs0
	fadd.s	ft0, ft0, ft2
	fdiv.s	ft1, ft1, ft3
	fadd.s	ft0, ft0, ft1
.my_sqrt.bb4:
	beq	t0, zero, .my_sqrt.bb1
	j	.my_sqrt.bb5
	.size my_sqrt, .-my_sqrt

  .globl F2
  .type  F2, @function
F2:
.F2.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	fsw	fs0, 16(sp)
	fmv.s	ft0, fa0
	fmul.s	ft0, ft0, ft0
	li	t0, 1065353216
	fmv.w.x	fs0, t0
	fsub.s	fa0, fs0, ft0
	call	my_sqrt
	fmv.s	ft0, fa0
	fdiv.s	fa0, fs0, ft0
	flw	fs0, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size F2, .-F2

  .globl simpson
  .type  simpson, @function
simpson:
.simpson.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	fsw	fs0, 16(sp)
	fsw	fs1, 20(sp)
	fsw	fs2, 24(sp)
	fsw	fs3, 28(sp)
	fsw	fs4, 32(sp)
	fsw	fs5, 36(sp)
	fmv.s	ft0, fa0
	fmv.s	fs0, fa1
	mv	t0, a0
	fsub.s	fs1, fs0, ft0
	li	t1, 1073741824
	fmv.w.x	ft1, t1
	fdiv.s	ft1, fs1, ft1
	fadd.s	fs2, ft0, ft1
	li	t1, 1
	li	t2, 1065353216
	fmv.w.x	ft1, t2
	li	t2, 1082130432
	fmv.w.x	fs3, t2
	li	t2, 1086324736
	fmv.w.x	fs4, t2
	li	t2, 0
	fmv.w.x	ft2, t2
	beq	t0, t1, .simpson.bb2
.simpson.bb3:
	li	t1, 2
	beq	t0, t1, .simpson.bb4
.simpson.bb5:
	fmv.s	ft0, ft2	# phi-copy
.simpson.bb1:
	fmv.s	fa0, ft0
	flw	fs5, 36(sp)
	flw	fs4, 32(sp)
	flw	fs3, 28(sp)
	flw	fs2, 24(sp)
	flw	fs1, 20(sp)
	flw	fs0, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
.simpson.bb4:
	fmv.s	fa0, ft0	# call-arg-copy
	call	F2
	fmv.s	fs5, fa0
	fmv.s	fa0, fs2	# call-arg-copy
	call	F2
	fmv.s	ft0, fa0
	fmul.s	ft0, fs3, ft0
	fadd.s	fs2, fs5, ft0
	fmv.s	fa0, fs0	# call-arg-copy
	call	F2
	fmv.s	ft0, fa0
	fadd.s	ft0, fs2, ft0
	fmul.s	ft0, ft0, fs1
	fdiv.s	ft0, ft0, fs4
	j	.simpson.bb1
.simpson.bb2:
	fdiv.s	ft2, ft1, fs2
	fdiv.s	ft0, ft1, ft0
	fdiv.s	ft1, ft1, fs0
	fmul.s	ft2, fs3, ft2
	fadd.s	ft0, ft0, ft2
	fadd.s	ft0, ft0, ft1
	fmul.s	ft0, ft0, fs1
	fdiv.s	ft0, ft0, fs4
	j	.simpson.bb1
	.size simpson, .-simpson

  .globl asr5
  .type  asr5, @function
asr5:
.asr5.bb0:
	addi	sp, sp, -96
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	fsw	fs0, 56(sp)
	fsw	fs1, 60(sp)
	fsw	fs2, 64(sp)
	fsw	fs3, 68(sp)
	fsw	fs4, 72(sp)
	fsw	fs5, 76(sp)
	fsw	fs6, 80(sp)
	fsw	fs7, 84(sp)
	fmv.s	fs0, fa0
	fmv.s	fs1, fa1
	fmv.s	fs2, fa2
	fmv.s	fs3, fa3
	mv	s1, a0
	fsub.s	ft0, fs1, fs0
	li	t0, 1073741824
	fmv.w.x	fs4, t0
	fdiv.s	ft0, ft0, fs4
	fadd.s	fs5, fs0, ft0
	mv	a0, s1	# call-arg-copy
	fmv.s	fa0, fs0	# call-arg-copy
	fmv.s	fa1, fs5	# call-arg-copy
	call	simpson
	fmv.s	fs6, fa0
	mv	a0, s1	# call-arg-copy
	fmv.s	fa0, fs5	# call-arg-copy
	fmv.s	fa1, fs1	# call-arg-copy
	call	simpson
	fmv.s	fs7, fa0
	fadd.s	ft0, fs6, fs7
	fsub.s	ft1, ft0, fs3
	li	t0, 0
	fmv.w.x	ft2, t0
	flt.s	t0, ft2, ft1
	li	t1, 1097859072
	fmv.w.x	ft2, t1
	beqz	t0, .asr5.bb6
.asr5.bb5:
	fmv.s	ft4, ft1	# phi-copy
.asr5.bb4:
	fmul.s	ft3, ft2, fs2
	fle.s	t0, ft4, ft3
	beqz	t0, .asr5.bb3
.asr5.bb2:
	fdiv.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
.asr5.bb1:
	fmv.s	fa0, ft0
	flw	fs7, 84(sp)
	flw	fs6, 80(sp)
	flw	fs5, 76(sp)
	flw	fs4, 72(sp)
	flw	fs3, 68(sp)
	flw	fs2, 64(sp)
	flw	fs1, 60(sp)
	flw	fs0, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 96
	ret
.asr5.bb3:
	fdiv.s	fs2, fs2, fs4
	mv	a0, s1	# call-arg-copy
	fmv.s	fa0, fs0	# call-arg-copy
	fmv.s	fa1, fs5	# call-arg-copy
	fmv.s	fa2, fs2	# call-arg-copy
	fmv.s	fa3, fs6	# call-arg-copy
	call	asr5
	fmv.s	fs0, fa0
	mv	a0, s1	# call-arg-copy
	fmv.s	fa0, fs5	# call-arg-copy
	fmv.s	fa1, fs1	# call-arg-copy
	fmv.s	fa2, fs2	# call-arg-copy
	fmv.s	fa3, fs7	# call-arg-copy
	call	asr5
	fmv.s	ft0, fa0
	fadd.s	ft0, fs0, ft0
	j	.asr5.bb1
.asr5.bb6:
	fneg.s	ft4, ft1
	j	.asr5.bb4
	.size asr5, .-asr5

  .globl asr4
  .type  asr4, @function
asr4:
.asr4.bb0:
	addi	sp, sp, -80
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	fsw	fs0, 56(sp)
	fsw	fs1, 60(sp)
	fsw	fs2, 64(sp)
	fmv.s	fs0, fa0
	fmv.s	fs1, fa1
	fmv.s	fs2, fa2
	mv	s1, a0
	mv	a0, s1	# call-arg-copy
	fmv.s	fa0, fs0	# call-arg-copy
	fmv.s	fa1, fs1	# call-arg-copy
	call	simpson
	fmv.s	fa3, fa0
	mv	a0, s1	# call-arg-copy
	fmv.s	fa0, fs0	# call-arg-copy
	fmv.s	fa1, fs1	# call-arg-copy
	fmv.s	fa2, fs2	# call-arg-copy
	call	asr5
	flw	fs2, 64(sp)
	flw	fs1, 60(sp)
	flw	fs0, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 80
	ret
	.size asr4, .-asr4

  .globl eee
  .type  eee, @function
eee:
.eee.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	fsw	fs0, 24(sp)
	fsw	fs1, 28(sp)
	fsw	fs2, 32(sp)
	fsw	fs3, 36(sp)
	fsw	fs4, 40(sp)
	fmv.s	fs0, fa0
	li	t0, 981668463
	fmv.w.x	ft0, t0
	flt.s	t0, ft0, fs0
	li	t1, 1073741824
	fmv.w.x	ft0, t1
	li	t1, 1065353216
	fmv.w.x	ft1, t1
	li	t1, 1086324736
	fmv.w.x	fs1, t1
	li	t1, 1103101952
	fmv.w.x	fs2, t1
	li	t1, 1123024896
	fmv.w.x	fs3, t1
	beqz	t0, .eee.bb3
.eee.bb2:
	fdiv.s	fa0, fs0, ft0
	call	eee
	fmv.s	ft0, fa0
	fmul.s	ft0, ft0, ft0
.eee.bb1:
	fmv.s	fa0, ft0
	flw	fs4, 40(sp)
	flw	fs3, 36(sp)
	flw	fs2, 32(sp)
	flw	fs1, 28(sp)
	flw	fs0, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.eee.bb3:
	fadd.s	ft1, ft1, fs0
	fmul.s	ft2, fs0, fs0
	fdiv.s	ft0, ft2, ft0
	fadd.s	fs4, ft1, ft0
	fmv.s	fa0, fs0	# call-arg-copy
	li	a0, 3
	call	my_pow
	fmv.s	ft0, fa0
	fdiv.s	ft0, ft0, fs1
	fadd.s	fs1, fs4, ft0
	fmv.s	fa0, fs0	# call-arg-copy
	li	a0, 4
	call	my_pow
	fmv.s	ft0, fa0
	fdiv.s	ft0, ft0, fs2
	fadd.s	fs1, fs1, ft0
	fmv.s	fa0, fs0	# call-arg-copy
	li	a0, 5
	call	my_pow
	fmv.s	ft0, fa0
	fdiv.s	ft0, ft0, fs3
	fadd.s	ft0, fs1, ft0
	j	.eee.bb1
	.size eee, .-eee

  .globl my_exp
  .type  my_exp, @function
my_exp:
.my_exp.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	fsw	fs0, 24(sp)
	fsw	fs1, 28(sp)
	fmv.s	ft0, fa0
	li	t0, 0
	fmv.w.x	ft1, t0
	flt.s	t0, ft0, ft1
	li	t1, 1065353216
	fmv.w.x	fs0, t1
	li	t1, 1076754516
	fmv.w.x	ft1, t1
	beqz	t0, .my_exp.bb3
.my_exp.bb2:
	fneg.s	fa0, ft0
	call	my_exp
	fmv.s	ft0, fa0
	fdiv.s	ft0, fs0, ft0
.my_exp.bb1:
	fmv.s	fa0, ft0
	flw	fs1, 28(sp)
	flw	fs0, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
.my_exp.bb3:
	fcvt.w.s	a0, ft0, rtz
	fcvt.s.w	ft2, a0
	fsub.s	fs0, ft0, ft2
	fmv.s	fa0, ft1	# call-arg-copy
	call	my_pow
	fmv.s	fs1, fa0
	fmv.s	fa0, fs0	# call-arg-copy
	call	eee
	fmv.s	ft0, fa0
	fmul.s	ft0, fs1, ft0
	j	.my_exp.bb1
	.size my_exp, .-my_exp

  .globl my_ln
  .type  my_ln, @function
my_ln:
.my_ln.bb0:
	addi	sp, sp, -48
	sd	ra, 32(sp)
	fmv.s	fa1, fa0
	li	t0, 1065353216
	fmv.w.x	fa0, t0
	li	t0, 841731191
	fmv.w.x	fa2, t0
	li	a0, 1
	call	asr4
	ld	ra, 32(sp)
	addi	sp, sp, 48
	ret
	.size my_ln, .-my_ln

  .globl my_log
  .type  my_log, @function
my_log:
.my_log.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	fsw	fs0, 16(sp)
	fsw	fs1, 20(sp)
	fmv.s	fs0, fa0
	fmv.s	fa0, fa1
	call	my_ln
	fmv.s	fs1, fa0
	fmv.s	fa0, fs0	# call-arg-copy
	call	my_ln
	fmv.s	ft0, fa0
	fdiv.s	fa0, fs1, ft0
	flw	fs1, 20(sp)
	flw	fs0, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size my_log, .-my_log

  .globl my_powf
  .type  my_powf, @function
my_powf:
.my_powf.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	fsw	fs0, 16(sp)
	fmv.s	fs0, fa1
	call	my_ln
	fmv.s	ft0, fa0
	fmul.s	fa0, fs0, ft0
	call	my_exp
	flw	fs0, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size my_powf, .-my_powf

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	fsw	fs0, 40(sp)
	fsw	fs1, 44(sp)
	fsw	fs2, 48(sp)
	call	getint
	mv	t0, a0
	li	t1, 0
	fmv.w.x	fs0, t1
	mv	s1, t0	# phi-copy
.main.bb1:
	beq	s1, zero, .main.bb3
.main.bb2:
	call	getfloat
	fmv.s	fs1, fa0
	call	getfloat
	fmv.s	fs2, fa0
	flt.s	s2, fs0, fs1
	beqz	s2, .main.bb15
.main.bb14:
	fmv.s	ft0, fs1	# phi-copy
.main.bb13:
	fmv.s	ft10, ft0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fmv.s	ft10, fs1
	fsw	ft10, 0(sp)
	li	t5, 2
	sd	t5, 8(sp)
	flw	fa0, 0(sp)
	ld	a0, 8(sp)
	call	my_pow
	fmv.s	ft10, fa0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fmv.s	ft10, fs1
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	my_sqrt
	fmv.s	ft10, fa0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fmv.s	ft10, fs1
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	my_exp
	fmv.s	ft10, fa0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	beqz	s2, .main.bb5
.main.bb4:
	fmv.s	ft10, fs1
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	my_ln
	fmv.s	ft10, fa0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
.main.bb6:
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	flt.s	t0, fs0, fs2
	and	t0, s2, t0
	bne	t0, zero, .main.bb7
.main.bb8:
	li	t5, 45
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
.main.bb9:
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	bnez	s2, .main.bb10
.main.bb11:
	li	t5, 45
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
.main.bb12:
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s1, s1, -1
	bne	s1, zero, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 0
	flw	fs2, 48(sp)
	flw	fs1, 44(sp)
	flw	fs0, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 64
	ret
.main.bb5:
	li	t5, 45
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb6
.main.bb7:
	fmv.s	ft10, fs1
	fsw	ft10, 0(sp)
	fmv.s	ft10, fs2
	fsw	ft10, 8(sp)
	flw	fa0, 0(sp)
	flw	fa1, 8(sp)
	call	my_log
	fmv.s	ft10, fa0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	j	.main.bb9
.main.bb10:
	fmv.s	ft10, fs1
	fsw	ft10, 0(sp)
	fmv.s	ft10, fs2
	fsw	ft10, 8(sp)
	flw	fa0, 0(sp)
	flw	fa1, 8(sp)
	call	my_powf
	fmv.s	ft10, fa0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	j	.main.bb12
.main.bb15:
	fneg.s	ft0, fs1
	j	.main.bb13
	.size main, .-main
