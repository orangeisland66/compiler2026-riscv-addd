
  .section .rodata
  .globl MAX_WIDTH
  .align 2
  .type  MAX_WIDTH, @object
  .size  MAX_WIDTH, 4
MAX_WIDTH:
  .word 1024

  .section .rodata
  .globl MAX_HEIGHT
  .align 2
  .type  MAX_HEIGHT, @object
  .size  MAX_HEIGHT, 4
MAX_HEIGHT:
  .word 1024

  .bss
  .globl image
  .align 2
  .type  image, @object
  .size  image, 4194304
image:
  .zero 4194304

  .bss
  .globl width
  .align 2
  .type  width, @object
  .size  width, 4
width:
  .zero 4

  .bss
  .globl height
  .align 2
  .type  height, @object
  .size  height, 4
height:
  .zero 4

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

  .globl read_image
  .type  read_image, @function
read_image:
.read_image.bb0:
	addi	sp, sp, -48
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	sd	s3, 24(sp)
	sd	s4, 32(sp)
	sd	s5, 40(sp)
	call	getch
	mv	t0, a0
	li	t1, 80
	subw	t0, t0, t1
	sltu	t0, zero, t0
	bnez	t0, .read_image.bb2
	j	.read_image.bb3
.read_image.bb16:
.read_image.bb2:
	bne	t0, zero, .read_image.bb4
.read_image.bb5:
	call	getint
	mv	s1, a0
	la	t0, width
	sw	s1, 0(t0)
	call	getint
	mv	s2, a0
	la	t0, height
	sw	s2, 0(t0)
	li	t0, 1024
	slt	t0, t0, s1
	li	t1, 1024
	slt	t1, t1, s2
	or	t0, t0, t1
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
	bnez	t0, .read_image.bb6
	j	.read_image.bb7
.read_image.bb17:
.read_image.bb6:
	beq	t0, zero, .read_image.bb9
.read_image.bb8:
	li	t0, -1
.read_image.bb1:
	mv	a0, t0
	ld	s5, 40(sp)
	ld	s4, 32(sp)
	ld	s3, 24(sp)
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 48
	ret
.read_image.bb14:
	call	getint
	mv	t0, a0
	addw	t1, s4, s5
	slliw	t1, t1, 2
	la	t2, image
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s5, s5, 1
	blt	s5, s1, .read_image.bb14	# rotated-loop-branch
.read_image.bb15:
	addiw	s3, s3, 1
	bge	s3, s2, .read_image.bb12	# rotated-loop-branch
.read_image.bb11:
	mulw	s4, s3, s1
	li	s5, 0
.read_image.bb13:
	bge	s5, s1, .read_image.bb15
	j	.read_image.bb14
.read_image.bb12:
	li	t0, 0
	j	.read_image.bb1
.read_image.bb4:
	li	t0, -1
	j	.read_image.bb1
.read_image.bb9:
	li	s3, 0
.read_image.bb10:
	bge	s3, s2, .read_image.bb12
	j	.read_image.bb11
.read_image.bb7:
	call	getint
	mv	t0, a0
	li	t1, 255
	subw	t0, t0, t1
	sltu	t0, zero, t0
	j	.read_image.bb6
.read_image.bb3:
	call	getch
	mv	t0, a0
	li	t1, 50
	subw	t0, t0, t1
	sltu	t0, zero, t0
	j	.read_image.bb2
	.size read_image, .-read_image

  .globl rotate
  .type  rotate, @function
rotate:
.rotate.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	fsw	fs0, 32(sp)
	fsw	fs1, 36(sp)
	mv	s1, a0
	mv	s2, a1
	fmv.s	fs0, fa0
	fmv.s	fa0, fs0	# call-arg-copy
	call	my_sin
	fmv.s	fs1, fa0
	fmv.s	fa0, fs0	# call-arg-copy
	call	my_cos
	fmv.s	ft0, fa0
	la	t0, width
	lw	t0, 0(t0)
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t1, t0, t1
	sraiw	t1, t1, 1
	la	t2, height
	lw	t2, 0(t2)
	srliw	t3, t2, 31	# const-div.pow2.bias-fold
	addw	t3, t2, t3
	sraiw	t3, t3, 1
	subw	a2, s1, t1
	subw	a3, s2, t3
	fcvt.s.w	ft1, a2
	fmul.s	ft2, ft1, ft0
	fcvt.s.w	ft3, a3
	fmul.s	ft4, ft3, fs1
	fsub.s	ft2, ft2, ft4
	fcvt.s.w	ft4, t1
	fadd.s	ft2, ft2, ft4
	fcvt.w.s	t1, ft2, rtz
	fmul.s	ft1, ft1, fs1
	fmul.s	ft0, ft3, ft0
	fadd.s	ft0, ft1, ft0
	fcvt.s.w	ft1, t3
	fadd.s	ft0, ft0, ft1
	fcvt.w.s	t3, ft0, rtz
	li	a2, 0
	slt	a2, t1, a2
	slt	a3, t1, t0
	xori	a3, a3, 1
	or	a2, a2, a3
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
	slt	a3, t3, a3
	or	a2, a2, a3
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
	slt	t2, t3, t2
	xori	t2, t2, 1
	or	t2, a2, t2
	beq	t2, zero, .rotate.bb3
.rotate.bb2:
	li	t0, 0
.rotate.bb1:
	mv	a0, t0
	flw	fs1, 36(sp)
	flw	fs0, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
.rotate.bb3:
	mulw	t0, t3, t0
	addw	t0, t0, t1
	slliw	t0, t0, 2
	la	t1, image
	add	t0, t1, t0
	lw	t0, 0(t0)
	j	.rotate.bb1
	.size rotate, .-rotate

  .globl write_pgm
  .type  write_pgm, @function
write_pgm:
.write_pgm.bb0:
	addi	sp, sp, -80
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	sd	s3, 48(sp)
	sd	s4, 56(sp)
	fsw	fs0, 64(sp)
	fmv.s	fs0, fa0
	li	a0, 80
	call	putch
	li	a0, 50
	call	putch
	li	a0, 10
	call	putch
	la	t0, width
	lw	s1, 0(t0)
	mv	a0, s1
	call	putint
	li	a0, 32
	call	putch
	la	t0, height
	lw	s2, 0(t0)
	mv	a0, s2
	call	putint
	li	a0, 32
	call	putch
	li	a0, 255
	call	putint
	li	a0, 10
	call	putch
	li	s3, 0
.write_pgm.bb1:
	bge	s3, s2, .write_pgm.bb3
.write_pgm.bb2:
	li	s4, 0
.write_pgm.bb4:
	bge	s4, s1, .write_pgm.bb6
.write_pgm.bb5:
	mv	a0, s4	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	fmv.s	fa0, fs0	# call-arg-copy
	call	rotate
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	a0, 32
	call	putch
	addiw	s4, s4, 1
	blt	s4, s1, .write_pgm.bb5	# rotated-loop-branch
.write_pgm.bb6:
	li	a0, 10
	call	putch
	addiw	s3, s3, 1
	blt	s3, s2, .write_pgm.bb2	# rotated-loop-branch
.write_pgm.bb3:
	flw	fs0, 64(sp)
	ld	s4, 56(sp)
	ld	s3, 48(sp)
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 80
	ret
	.size write_pgm, .-write_pgm

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	fsw	fs0, 16(sp)
	call	getfloat
	fmv.s	fs0, fa0
	call	getch
	call	read_image
	mv	t0, a0
	bge	t0, zero, .main.bb3
.main.bb2:
	li	t0, -1
.main.bb1:
	mv	a0, t0
	flw	fs0, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.main.bb3:
	fmv.s	fa0, fs0
	call	write_pgm
	li	t0, 0
	j	.main.bb1
	.size main, .-main
