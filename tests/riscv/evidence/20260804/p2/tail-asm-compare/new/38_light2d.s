
  .section .rodata
  .globl W
  .align 2
  .type  W, @object
  .size  W, 4
W:
  .word 12

  .section .rodata
  .globl H
  .align 2
  .type  H, @object
  .size  H, 4
H:
  .word 12

  .section .rodata
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .word 24

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
  .globl MAX_STEP
  .align 2
  .type  MAX_STEP, @object
  .size  MAX_STEP, 4
MAX_STEP:
  .word 10

  .section .rodata
  .globl MAX_DISTANCE
  .align 2
  .type  MAX_DISTANCE, @object
  .size  MAX_DISTANCE, 4
MAX_DISTANCE:
  .word 1073741824

  .section .rodata
  .globl EPSILON
  .align 2
  .type  EPSILON, @object
  .size  EPSILON, 4
EPSILON:
  .word 897988541

  .section .rodata
  .globl RAND_MAX
  .align 2
  .type  RAND_MAX, @object
  .size  RAND_MAX, 4
RAND_MAX:
  .word 100000006

  .bss
  .globl seed
  .align 2
  .type  seed, @object
  .size  seed, 4
seed:
  .zero 4

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

  .globl circle_sdf
  .type  circle_sdf, @function
circle_sdf:
.circle_sdf.bb0:
	addi	sp, sp, -16
	fsw	fs0, 0(sp)
	fsw	fs1, 4(sp)
	fmv.s	ft0, fa0
	fmv.s	ft1, fa1
	fmv.s	ft2, fa2
	fmv.s	ft3, fa3
	fmv.s	ft4, fa4
	fsub.s	ft0, ft0, ft2
	fsub.s	ft1, ft1, ft3
	fmul.s	ft0, ft0, ft0
	fmul.s	ft1, ft1, ft1
	fadd.s	fs0, ft0, ft1
	li	t0, 1090519040
	fmv.w.x	ft0, t0
	fdiv.s	ft0, fs0, ft0
	li	t0, 1056964608
	fmv.w.x	ft1, t0
	fadd.s	ft0, ft0, ft1
	li	t0, 1073741824
	fmv.w.x	fs1, t0
	fmul.s	ft1, fs1, fs0
	li	t0, 1082130432
	fmv.w.x	ft2, t0
	fadd.s	ft2, ft2, fs0
	fdiv.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	li	t0, 10
.circle_sdf.bb1:
	beq	t0, zero, .circle_sdf.bb3
.circle_sdf.bb2:
	fdiv.s	ft1, fs0, ft0
	addiw	t0, t0, -1
	fadd.s	ft0, ft0, ft1
	fdiv.s	ft0, ft0, fs1
	bne	t0, zero, .circle_sdf.bb2	# rotated-loop-branch
.circle_sdf.bb3:
	fsub.s	fa0, ft0, ft4
	flw	fs1, 4(sp)
	flw	fs0, 0(sp)
	addi	sp, sp, 16
	ret
	.size circle_sdf, .-circle_sdf

  .globl scene
  .type  scene, @function
scene:
.scene.bb0:
	addi	sp, sp, -80
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	fsw	fs0, 56(sp)
	fsw	fs1, 60(sp)
	fsw	fs2, 64(sp)
	fmv.s	fs0, fa0
	fmv.s	fs1, fa1
	mv	s1, a0
	li	t0, 1053609165
	fmv.w.x	fa3, t0
	li	t0, 1036831949
	fmv.w.x	fa4, t0
	fmv.s	fa0, fs0	# call-arg-copy
	fmv.s	fa1, fs1	# call-arg-copy
	fmv.s	fa2, fa3	# call-arg-copy
	call	circle_sdf
	fmv.s	fs2, fa0
	li	t0, 1058642330
	fmv.w.x	fa3, t0
	li	t0, 1028443341
	fmv.w.x	ft1, t0
	fmv.s	fa0, fs0	# call-arg-copy
	fmv.s	fa1, fs1	# call-arg-copy
	fmv.s	fa2, fa3	# call-arg-copy
	fmv.s	fa4, ft1	# call-arg-copy
	call	circle_sdf
	fmv.s	ft0, fa0
	flt.s	t0, fs2, ft0
	li	t1, 1077936128
	fmv.w.x	ft1, t1
	li	t1, 0
	fmv.w.x	ft2, t1
	bnez	t0, .scene.bb1
.scene.bb2:
	fsw	ft0, 0(s1)
	fsw	ft2, 4(s1)
.scene.bb3:
	flw	fs2, 64(sp)
	flw	fs1, 60(sp)
	flw	fs0, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 80
	ret
.scene.bb1:
	fsw	fs2, 0(s1)
	fsw	ft1, 4(s1)
	j	.scene.bb3
	.size scene, .-scene

  .globl trace
  .type  trace, @function
trace:
.trace.bb0:
	addi	sp, sp, -96
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	fsw	fs0, 48(sp)
	fsw	fs1, 52(sp)
	fsw	fs2, 56(sp)
	fsw	fs3, 60(sp)
	fsw	fs4, 64(sp)
	fsw	fs5, 68(sp)
	fsw	fs6, 72(sp)
	fsw	fs7, 76(sp)
	fsw	fs8, 80(sp)
	fmv.s	fs0, fa0
	fmv.s	fs1, fa1
	fmv.s	fs2, fa2
	fmv.s	fs3, fa3
	li	t0, 1073741824
	fmv.w.x	fs4, t0
	li	t0, 897988541
	fmv.w.x	fs5, t0
	li	s1, 0
	fmv.w.x	fs6, s1
	fmv.s	fs7, fs6	# phi-copy
.trace.bb2:
	li	t0, 10
	flt.s	t1, fs7, fs4
	slt	t0, s1, t0
	and	t0, t0, t1
	beq	t0, zero, .trace.bb1
.trace.bb3:
	fmul.s	ft0, fs2, fs7
	fadd.s	fa0, fs0, ft0
	fmul.s	ft1, fs3, fs7
	fadd.s	fa1, fs1, ft1
	addi	a0, sp, 24
	call	scene
	addi	t0, sp, 24
	flw	fs8, 0(t0)
	flt.s	t0, fs8, fs5
	bnez	t0, .trace.bb5
.trace.bb6:
	fadd.s	fs7, fs7, fs8
	addiw	s1, s1, 1
	j	.trace.bb2
.trace.bb4:
.trace.bb1:
	fmv.s	fa0, fs6
	flw	fs8, 80(sp)
	flw	fs7, 76(sp)
	flw	fs6, 72(sp)
	flw	fs5, 68(sp)
	flw	fs4, 64(sp)
	flw	fs3, 60(sp)
	flw	fs2, 56(sp)
	flw	fs1, 52(sp)
	flw	fs0, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 96
	ret
.trace.bb5:
	addi	t0, sp, 24
	flw	fs6, 4(t0)
	j	.trace.bb1
	.size trace, .-trace

  .globl sample
  .type  sample, @function
sample:
.sample.bb0:
	addi	sp, sp, -96
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	fsw	fs0, 64(sp)
	fsw	fs1, 68(sp)
	fsw	fs2, 72(sp)
	fsw	fs3, 76(sp)
	fsw	fs4, 80(sp)
	fsw	fs5, 84(sp)
	fsw	fs6, 88(sp)
	fsw	fs7, 92(sp)
	fmv.s	fs0, fa0
	fmv.s	fs1, fa1
	li	t0, 1103101952
	fmv.w.x	fs2, t0
	li	t0, 1287568417
	fmv.w.x	fs3, t0
	li	t0, 1086918619
	fmv.w.x	fs4, t0
	li	s2, 0
	li	t1, 0
	fmv.w.x	fs5, t1
	li	s1, 24
.sample.bb1:
	bge	s2, s1, .sample.bb3
.sample.bb2:
	la	t0, seed
	li	t1, 19980130
	lw	t0, 0(t0)
	mulw	t0, t0, t1
	li	t1, 23333
	addw	s3, t0, t1
	li	t0, 360287945
	mul	t0, s3, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 23	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 100000007
	mulw	t0, t0, t1	# const-rem.mul
	subw	s3, s3, t0	# const-rem.sub
	bge	s3, zero, .sample.bb6
.sample.bb4:
	li	t0, 100000007
	addw	t0, s3, t0
.sample.bb5:
	la	t1, seed
	sw	t0, 0(t1)
	fcvt.s.w	ft0, t0
	fdiv.s	ft0, ft0, fs3
	fcvt.s.w	ft1, s2
	fadd.s	ft0, ft1, ft0
	fmul.s	ft0, fs4, ft0
	fdiv.s	fs6, ft0, fs2
	fmv.s	fa0, fs6	# call-arg-copy
	call	my_cos
	fmv.s	fs7, fa0
	fmv.s	fa0, fs6	# call-arg-copy
	call	my_sin
	fmv.s	fa3, fa0
	fmv.s	fa0, fs0	# call-arg-copy
	fmv.s	fa1, fs1	# call-arg-copy
	fmv.s	fa2, fs7	# call-arg-copy
	call	trace
	fmv.s	ft0, fa0
	fadd.s	fs5, fs5, ft0
	addiw	s2, s2, 1
	blt	s2, s1, .sample.bb2	# rotated-loop-branch
.sample.bb3:
	fdiv.s	fa0, fs5, fs2
	flw	fs7, 92(sp)
	flw	fs6, 88(sp)
	flw	fs5, 84(sp)
	flw	fs4, 80(sp)
	flw	fs3, 76(sp)
	flw	fs2, 72(sp)
	flw	fs1, 68(sp)
	flw	fs0, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 96
	ret
.sample.bb6:
	mv	t0, s3	# phi-copy
	j	.sample.bb5
	.size sample, .-sample

  .globl write_pgm
  .type  write_pgm, @function
write_pgm:
.write_pgm.bb0:
	addi	sp, sp, -80
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	fsw	fs0, 56(sp)
	fsw	fs1, 60(sp)
	fsw	fs2, 64(sp)
	li	a0, 80
	call	putch
	li	a0, 50
	call	putch
	li	a0, 10
	call	putch
	li	a0, 12
	call	putint
	li	a0, 32
	call	putch
	li	a0, 12
	call	putint
	li	a0, 32
	call	putch
	li	a0, 255
	call	putint
	li	a0, 10
	call	putch
	li	t0, 1094713344
	fmv.w.x	fs0, t0
	li	t0, 1132396544
	fmv.w.x	fs1, t0
	li	s1, 0
.write_pgm.bb1:
	li	t0, 12
	bge	s1, t0, .write_pgm.bb3
.write_pgm.bb2:
	fcvt.s.w	fs2, s1
	li	s3, 0
	li	s2, 12
.write_pgm.bb4:
	bge	s3, s2, .write_pgm.bb6
.write_pgm.bb5:
	fcvt.s.w	ft0, s3
	fdiv.s	fa0, ft0, fs0
	fdiv.s	fa1, fs2, fs0
	call	sample
	fmv.s	ft0, fa0
	fmul.s	ft0, ft0, fs1
	fcvt.w.s	s4, ft0, rtz
	li	t0, 255
	bge	t0, s4, .write_pgm.bb9
.write_pgm.bb7:
	li	t0, 255
.write_pgm.bb8:
	mv	a0, t0
	call	putint
	li	a0, 32
	call	putch
	addiw	s3, s3, 1
	j	.write_pgm.bb4
.write_pgm.bb9:
	mv	t0, s4	# phi-copy
	j	.write_pgm.bb8
.write_pgm.bb6:
	li	a0, 10
	call	putch
	addiw	s1, s1, 1
	j	.write_pgm.bb1
.write_pgm.bb3:
	flw	fs2, 64(sp)
	flw	fs1, 60(sp)
	flw	fs0, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 80
	ret
	.size write_pgm, .-write_pgm

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 0(sp)
	call	write_pgm
	li	a0, 0
	ld	ra, 0(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
