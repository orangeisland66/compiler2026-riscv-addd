
  .section .rodata
  .globl MAX_DIM_X
  .align 2
  .type  MAX_DIM_X, @object
  .size  MAX_DIM_X, 4
MAX_DIM_X:
  .word 8

  .section .rodata
  .globl MAX_DIM_Y
  .align 2
  .type  MAX_DIM_Y, @object
  .size  MAX_DIM_Y, 4
MAX_DIM_Y:
  .word 8

  .bss
  .globl test_block
  .align 2
  .type  test_block, @object
  .size  test_block, 256
test_block:
  .zero 256

  .bss
  .globl test_dct
  .align 2
  .type  test_dct, @object
  .size  test_dct, 256
test_dct:
  .zero 256

  .bss
  .globl test_idct
  .align 2
  .type  test_idct, @object
  .size  test_idct, 256
test_idct:
  .zero 256

  .section .rodata
  .globl PI
  .align 2
  .type  PI, @object
  .size  PI, 4
PI:
  .word 1078530011

  .section .rodata
  .globl TWO_PI
  .align 2
  .type  TWO_PI, @object
  .size  TWO_PI, 4
TWO_PI:
  .word 1086918619

  .section .rodata
  .globl EPSILON
  .align 2
  .type  EPSILON, @object
  .size  EPSILON, 4
EPSILON:
  .word 897988541

  .text

  .globl my_sin_impl
  .type  my_sin_impl, @function
my_sin_impl:
.my_sin_impl.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	fsw	fs0, 16(sp)
	fsw	fs1, 20(sp)
	fmv.s	ft0, fa0
	li	t0, 0
	fmv.w.x	ft1, t0
	flt.s	t0, ft1, ft0
	li	t1, 1077936128
	fmv.w.x	fs0, t1
	li	t1, 897988541
	fmv.w.x	ft2, t1
	li	t1, 1082130432
	fmv.w.x	fs1, t1
	beqz	t0, .my_sin_impl.bb6
.my_sin_impl.bb5:
	fmv.s	ft3, ft0	# phi-copy
.my_sin_impl.bb4:
	fle.s	t0, ft3, ft2
	bnez	t0, .my_sin_impl.bb1
.my_sin_impl.bb3:
	fdiv.s	ft0, ft0, fs0
	flt.s	t0, ft1, ft0
	beqz	t0, .my_sin_impl.bb12
.my_sin_impl.bb11:
	fmv.s	ft3, ft0	# phi-copy
.my_sin_impl.bb10:
	fle.s	t0, ft3, ft2
	bnez	t0, .my_sin_impl.bb7
.my_sin_impl.bb9:
	fdiv.s	ft0, ft0, fs0
	flt.s	t0, ft1, ft0
	beqz	t0, .my_sin_impl.bb18
.my_sin_impl.bb17:
	fmv.s	ft3, ft0	# phi-copy
.my_sin_impl.bb16:
	fle.s	t0, ft3, ft2
	bnez	t0, .my_sin_impl.bb13
.my_sin_impl.bb15:
	fdiv.s	ft0, ft0, fs0
	flt.s	t0, ft1, ft0
	beqz	t0, .my_sin_impl.bb24
.my_sin_impl.bb23:
	fmv.s	ft3, ft0	# phi-copy
.my_sin_impl.bb22:
	fle.s	t0, ft3, ft2
	bnez	t0, .my_sin_impl.bb19
.my_sin_impl.bb21:
	fdiv.s	ft0, ft0, fs0
	flt.s	t0, ft1, ft0
	beqz	t0, .my_sin_impl.bb30
.my_sin_impl.bb29:
	fmv.s	ft1, ft0	# phi-copy
.my_sin_impl.bb28:
	fle.s	t0, ft1, ft2
	bnez	t0, .my_sin_impl.bb25
	j	.my_sin_impl.bb27
.my_sin_impl.bb26:
.my_sin_impl.bb25:
	fmul.s	ft1, fs1, ft0
	fmul.s	ft2, fs0, ft0
	fmul.s	ft1, ft1, ft0
	fmul.s	ft0, ft1, ft0
	fsub.s	ft0, ft2, ft0
.my_sin_impl.bb19:
	fmul.s	ft1, fs1, ft0
	fmul.s	ft2, fs0, ft0
	fmul.s	ft1, ft1, ft0
	fmul.s	ft0, ft1, ft0
	fsub.s	ft0, ft2, ft0
.my_sin_impl.bb13:
	fmul.s	ft1, fs1, ft0
	fmul.s	ft2, fs0, ft0
	fmul.s	ft1, ft1, ft0
	fmul.s	ft0, ft1, ft0
	fsub.s	ft0, ft2, ft0
.my_sin_impl.bb7:
	fmul.s	ft1, fs1, ft0
	fmul.s	ft2, fs0, ft0
	fmul.s	ft1, ft1, ft0
	fmul.s	ft0, ft1, ft0
	fsub.s	ft0, ft2, ft0
.my_sin_impl.bb1:
	fmv.s	fa0, ft0
	flw	fs1, 20(sp)
	flw	fs0, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.my_sin_impl.bb2:
	j	.my_sin_impl.bb1
.my_sin_impl.bb8:
	j	.my_sin_impl.bb7
.my_sin_impl.bb14:
	j	.my_sin_impl.bb13
.my_sin_impl.bb20:
	j	.my_sin_impl.bb19
.my_sin_impl.bb27:
	fdiv.s	fa0, ft0, fs0
	call	my_sin_impl
	fmv.s	ft0, fa0
	fmul.s	ft1, fs0, ft0
	fmul.s	ft2, fs1, ft0
	fmul.s	ft2, ft2, ft0
	fmul.s	ft0, ft2, ft0
	fsub.s	ft0, ft1, ft0
	j	.my_sin_impl.bb25
.my_sin_impl.bb12:
	fneg.s	ft3, ft0
	j	.my_sin_impl.bb10
.my_sin_impl.bb18:
	fneg.s	ft3, ft0
	j	.my_sin_impl.bb16
.my_sin_impl.bb24:
	fneg.s	ft3, ft0
	j	.my_sin_impl.bb22
.my_sin_impl.bb30:
	fneg.s	ft1, ft0
	j	.my_sin_impl.bb28
.my_sin_impl.bb6:
	fneg.s	ft3, ft0
	j	.my_sin_impl.bb4
	.size my_sin_impl, .-my_sin_impl

  .globl my_sin
  .type  my_sin, @function
my_sin:
.my_sin.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	fmv.s	ft0, fa0
	li	t0, 1086918619
	fmv.w.x	ft1, t0
	flt.s	t0, ft1, ft0
	li	t1, -1060565029
	fmv.w.x	ft2, t1
	flt.s	t1, ft0, ft2
	or	t0, t0, t1
	li	t1, 1078530011
	fmv.w.x	ft2, t1
	li	t1, -1068953637
	fmv.w.x	ft3, t1
	bne	t0, zero, .my_sin.bb1
.my_sin.bb7:
.my_sin.bb2:
	flt.s	t0, ft2, ft0
	bnez	t0, .my_sin.bb3
.my_sin.bb8:
.my_sin.bb4:
	flt.s	t0, ft0, ft3
	bnez	t0, .my_sin.bb5
.my_sin.bb9:
.my_sin.bb6:
	fmv.s	fa0, ft0	# call-arg-copy
	call	my_sin_impl
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.my_sin.bb3:
	fsub.s	ft0, ft0, ft1
	j	.my_sin.bb4
.my_sin.bb5:
	fadd.s	ft0, ft0, ft1
	j	.my_sin.bb6
.my_sin.bb1:
	fdiv.s	ft4, ft0, ft1
	fcvt.w.s	t0, ft4, rtz
	fcvt.s.w	ft4, t0
	fmul.s	ft4, ft4, ft1
	fsub.s	ft0, ft0, ft4
	j	.my_sin.bb2
	.size my_sin, .-my_sin

  .globl my_cos
  .type  my_cos, @function
my_cos:
.my_cos.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	fmv.s	ft0, fa0
	li	t0, 1070141403
	fmv.w.x	ft1, t0
	fadd.s	fa0, ft0, ft1
	call	my_sin
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size my_cos, .-my_cos

  .globl write_mat
  .type  write_mat, @function
write_mat:
.write_mat.bb0:
	addi	sp, sp, -64
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	li	s4, 0
.write_mat.bb1:
	bge	s4, s2, .write_mat.bb3
.write_mat.bb2:
	slliw	s5, s4, 5
	add	t0, s1, s5
	flw	fa0, 0(t0)
	call	putfloat
	li	s6, 1
.write_mat.bb4:
	bge	s6, s3, .write_mat.bb6
.write_mat.bb5:
	li	a0, 32
	call	putch
	slliw	t0, s6, 2
	addw	t0, s5, t0
	add	t0, s1, t0
	flw	fa0, 0(t0)
	call	putfloat
	addiw	s6, s6, 1
	blt	s6, s3, .write_mat.bb5	# rotated-loop-branch
.write_mat.bb6:
	li	a0, 10
	call	putch
	addiw	s4, s4, 1
	blt	s4, s2, .write_mat.bb2	# rotated-loop-branch
.write_mat.bb3:
	li	a0, 10
	call	putch
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
	.size write_mat, .-write_mat

  .globl dct
  .type  dct, @function
dct:
.dct.bb0:
	addi	sp, sp, -144
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	sd	s9, 80(sp)
	sd	s10, 88(sp)
	fsw	fs0, 96(sp)
	fsw	fs1, 100(sp)
	fsw	fs2, 104(sp)
	fsw	fs3, 108(sp)
	fsw	fs4, 112(sp)
	fsw	fs5, 116(sp)
	fsw	fs6, 120(sp)
	fsw	fs7, 124(sp)
	fsw	fs8, 128(sp)
	fsw	fs9, 132(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	fcvt.s.w	fs0, s3
	fcvt.s.w	fs1, s4
	li	t0, 0
	fmv.w.x	fs2, t0
	li	t0, 1056964608
	fmv.w.x	fs3, t0
	li	t0, 1078530011
	fmv.w.x	fs4, t0
	li	s5, 0
.dct.bb1:
	bge	s5, s3, .dct.bb3
.dct.bb2:
	slliw	t0, s5, 5
	fcvt.s.w	fs5, s5
	add	t0, s1, t0
	li	t1, 0
	mv	s6, t0	# phi-copy
	mv	s7, t1	# phi-copy
.dct.bb4:
	bge	s7, s4, .dct.bb6
.dct.bb5:
	fsw	fs2, 0(s6)
	fcvt.s.w	fs6, s7
	li	s8, 0
.dct.bb7:
	bge	s8, s3, .dct.bb9
.dct.bb8:
	fcvt.s.w	ft0, s8
	slliw	s9, s8, 5
	fadd.s	fs7, ft0, fs3
	li	s10, 0
.dct.bb10:
	bge	s10, s4, .dct.bb12
.dct.bb11:
	flw	fs8, 0(s6)
	slliw	t0, s10, 2
	addw	t0, s9, t0
	add	t0, s2, t0
	flw	fs9, 0(t0)
	fdiv.s	ft0, fs4, fs0
	fmul.s	ft0, ft0, fs7
	fmul.s	fa0, ft0, fs5
	call	my_cos
	fmv.s	ft0, fa0
	fmul.s	fs9, fs9, ft0
	fdiv.s	ft0, fs4, fs1
	fcvt.s.w	ft1, s10
	fadd.s	ft1, ft1, fs3
	fmul.s	ft0, ft0, ft1
	fmul.s	fa0, ft0, fs6
	call	my_cos
	fmv.s	ft0, fa0
	fmul.s	ft0, fs9, ft0
	fadd.s	ft0, fs8, ft0
	fsw	ft0, 0(s6)
	addiw	s10, s10, 1
	blt	s10, s4, .dct.bb11	# rotated-loop-branch
.dct.bb12:
	addiw	s8, s8, 1
	j	.dct.bb7
.dct.bb9:
	addiw	t0, s7, 1
	addi	s6, s6, 4
	mv	s7, t0	# phi-copy
	j	.dct.bb4
.dct.bb6:
	addiw	s5, s5, 1
	blt	s5, s3, .dct.bb2	# rotated-loop-branch
.dct.bb3:
	flw	fs9, 132(sp)
	flw	fs8, 128(sp)
	flw	fs7, 124(sp)
	flw	fs6, 120(sp)
	flw	fs5, 116(sp)
	flw	fs4, 112(sp)
	flw	fs3, 108(sp)
	flw	fs2, 104(sp)
	flw	fs1, 100(sp)
	flw	fs0, 96(sp)
	ld	s10, 88(sp)
	ld	s9, 80(sp)
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 144
	ret
	.size dct, .-dct

  .globl idct
  .type  idct, @function
idct:
.idct.bb0:
	addi	sp, sp, -160
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	sd	s9, 80(sp)
	sd	s10, 88(sp)
	sd	s11, 96(sp)
	fsw	fs0, 104(sp)
	fsw	fs1, 108(sp)
	fsw	fs2, 112(sp)
	fsw	fs3, 116(sp)
	fsw	fs4, 120(sp)
	fsw	fs5, 124(sp)
	fsw	fs6, 128(sp)
	fsw	fs7, 132(sp)
	fsw	fs8, 136(sp)
	fsw	fs9, 140(sp)
	fsw	fs10, 144(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	mv	t0, s2
	fcvt.s.w	fs0, s3
	fcvt.s.w	fs1, s4
	addi	s5, t0, 4
	li	t0, 1056964608
	fmv.w.x	fs2, t0
	li	t0, 1048576000
	fmv.w.x	fs3, t0
	li	t0, 1073741824
	fmv.w.x	fs4, t0
	li	t0, 1078530011
	fmv.w.x	fs5, t0
	li	s6, 0
.idct.bb1:
	bge	s6, s3, .idct.bb3
.idct.bb2:
	fcvt.s.w	ft0, s6
	slliw	t0, s6, 5
	fadd.s	fs6, ft0, fs2
	add	t0, s1, t0
	li	t1, 0
	mv	s7, t0	# phi-copy
	mv	s8, t1	# phi-copy
.idct.bb4:
	bge	s8, s4, .idct.bb6
.idct.bb5:
	flw	ft0, 0(s2)
	li	t0, 1
	fmul.s	ft0, fs3, ft0
	fsw	ft0, 0(s7)
.idct.bb7:
	bge	t0, s3, .idct.bb9
.idct.bb19:
	addiw	t1, t0, 3
	blt	t1, s3, .idct.bb20
.idct.bb8:
	flw	ft0, 0(s7)
	slliw	t1, t0, 5
	add	t1, s2, t1
	addiw	t0, t0, 1
	flw	ft1, 0(t1)
	fmul.s	ft1, fs2, ft1
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 0(s7)
	j	.idct.bb7
.idct.bb20:
	flw	ft0, 0(s7)
	slliw	t1, t0, 5
	add	s9, s2, t1
	addiw	t0, t0, 4
	flw	ft1, 0(s9)
	flw	ft2, 32(s9)
	flw	ft3, 64(s9)
	flw	ft4, 96(s9)
	fmul.s	ft1, fs2, ft1
	fmul.s	ft2, fs2, ft2
	fmul.s	ft3, fs2, ft3
	fmul.s	ft4, fs2, ft4
	fadd.s	ft0, ft0, ft1
	fadd.s	ft0, ft0, ft2
	fadd.s	ft0, ft0, ft3
	fadd.s	ft0, ft0, ft4
	fsw	ft0, 0(s7)
	j	.idct.bb7
.idct.bb9:
	li	t0, 1
	mv	s9, s5	# phi-copy
.idct.bb10:
	bge	t0, s4, .idct.bb12
.idct.bb21:
	addiw	t1, t0, 3
	blt	t1, s4, .idct.bb22
.idct.bb11:
	flw	ft0, 0(s9)
	addiw	t0, t0, 1
	flw	ft1, 0(s7)
	addi	t1, s9, 4
	fmul.s	ft0, fs2, ft0
	fadd.s	ft0, ft1, ft0
	fsw	ft0, 0(s7)
	mv	s9, t1	# phi-copy
	j	.idct.bb10
.idct.bb22:
	flw	ft0, 0(s9)
	addi	t1, s9, 4
	flw	ft1, 0(s7)
	addi	t1, t1, 4
	flw	ft2, 4(s9)
	addi	t1, t1, 4
	flw	ft3, 8(s9)
	addiw	t0, t0, 4
	flw	ft4, 12(s9)
	addi	t1, t1, 4
	fmul.s	ft0, fs2, ft0
	fmul.s	ft2, fs2, ft2
	fmul.s	ft3, fs2, ft3
	fmul.s	ft4, fs2, ft4
	fadd.s	ft0, ft1, ft0
	fadd.s	ft0, ft0, ft2
	fadd.s	ft0, ft0, ft3
	fadd.s	ft0, ft0, ft4
	fsw	ft0, 0(s7)
	mv	s9, t1	# phi-copy
	j	.idct.bb10
.idct.bb12:
	fcvt.s.w	ft0, s8
	li	t0, 1
	fadd.s	fs7, ft0, fs2
	mv	s9, t0	# phi-copy
.idct.bb13:
	bge	s9, s3, .idct.bb15
.idct.bb14:
	slliw	s10, s9, 5
	fcvt.s.w	fs8, s9
	li	s11, 1
.idct.bb16:
	bge	s11, s4, .idct.bb18
.idct.bb17:
	flw	fs9, 0(s7)
	slliw	t0, s11, 2
	addw	t0, s10, t0
	add	t0, s2, t0
	flw	fs10, 0(t0)
	fdiv.s	ft0, fs5, fs0
	fmul.s	ft0, ft0, fs6
	fmul.s	fa0, ft0, fs8
	call	my_cos
	fmv.s	ft0, fa0
	fmul.s	fs10, fs10, ft0
	fdiv.s	ft0, fs5, fs1
	fmul.s	ft0, ft0, fs7
	fcvt.s.w	ft1, s11
	fmul.s	fa0, ft0, ft1
	call	my_cos
	fmv.s	ft0, fa0
	fmul.s	ft0, fs10, ft0
	fadd.s	ft0, fs9, ft0
	fsw	ft0, 0(s7)
	addiw	s11, s11, 1
	blt	s11, s4, .idct.bb17	# rotated-loop-branch
.idct.bb18:
	addiw	s9, s9, 1
	j	.idct.bb13
.idct.bb15:
	flw	ft0, 0(s7)
	addiw	t0, s8, 1
	addi	t1, s7, 4
	fmul.s	ft0, ft0, fs4
	fdiv.s	ft0, ft0, fs0
	fmul.s	ft0, ft0, fs4
	fdiv.s	ft0, ft0, fs1
	fsw	ft0, 0(s7)
	mv	s7, t1	# phi-copy
	mv	s8, t0	# phi-copy
	j	.idct.bb4
.idct.bb6:
	addiw	s6, s6, 1
	blt	s6, s3, .idct.bb2	# rotated-loop-branch
.idct.bb3:
	flw	fs10, 144(sp)
	flw	fs9, 140(sp)
	flw	fs8, 136(sp)
	flw	fs7, 132(sp)
	flw	fs6, 128(sp)
	flw	fs5, 124(sp)
	flw	fs4, 120(sp)
	flw	fs3, 116(sp)
	flw	fs2, 112(sp)
	flw	fs1, 108(sp)
	flw	fs0, 104(sp)
	ld	s11, 96(sp)
	ld	s10, 88(sp)
	ld	s9, 80(sp)
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 160
	ret
	.size idct, .-idct

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	li	s3, 0
.main.bb1:
	bge	s3, s1, .main.bb3
.main.bb2:
	slliw	s4, s3, 5
	li	s5, 0
.main.bb4:
	bge	s5, s2, .main.bb6
.main.bb5:
	call	getfloat
	fmv.s	ft0, fa0
	slliw	t0, s5, 2
	addw	t0, s4, t0
	la	t1, test_block
	add	t0, t1, t0
	fsw	ft0, 0(t0)
	addiw	s5, s5, 1
	blt	s5, s2, .main.bb5	# rotated-loop-branch
.main.bb6:
	addiw	s3, s3, 1
	blt	s3, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	la	t5, test_dct
	sd	t5, 0(sp)
	la	t5, test_block
	sd	t5, 8(sp)
	mv	t5, s1
	sd	t5, 16(sp)
	mv	t5, s2
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	dct
	la	t5, test_dct
	sd	t5, 0(sp)
	mv	t5, s1
	sd	t5, 8(sp)
	mv	t5, s2
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	write_mat
	la	t5, test_idct
	sd	t5, 0(sp)
	la	t5, test_dct
	sd	t5, 8(sp)
	mv	t5, s1
	sd	t5, 16(sp)
	mv	t5, s2
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	idct
	la	t5, test_idct
	sd	t5, 0(sp)
	mv	t5, s1
	sd	t5, 8(sp)
	mv	t5, s2
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	write_mat
	li	a0, 0
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
