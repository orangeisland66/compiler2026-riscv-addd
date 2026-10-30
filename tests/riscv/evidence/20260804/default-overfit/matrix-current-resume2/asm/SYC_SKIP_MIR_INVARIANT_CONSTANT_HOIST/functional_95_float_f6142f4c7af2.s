
  .section .rodata
  .globl RADIUS
  .align 2
  .type  RADIUS, @object
  .size  RADIUS, 4
RADIUS:
  .word 1085276160

  .section .rodata
  .globl PI
  .align 2
  .type  PI, @object
  .size  PI, 4
PI:
  .word 1078530011

  .section .rodata
  .globl EPS
  .align 2
  .type  EPS, @object
  .size  EPS, 4
EPS:
  .word 897988541

  .section .rodata
  .globl PI_HEX
  .align 2
  .type  PI_HEX, @object
  .size  PI_HEX, 4
PI_HEX:
  .word 1078530011

  .section .rodata
  .globl HEX2
  .align 2
  .type  HEX2, @object
  .size  HEX2, 4
HEX2:
  .word 1033895936

  .section .rodata
  .globl FACT
  .align 2
  .type  FACT, @object
  .size  FACT, 4
FACT:
  .word -956241920

  .section .rodata
  .globl EVAL1
  .align 2
  .type  EVAL1, @object
  .size  EVAL1, 4
EVAL1:
  .word 1119752446

  .section .rodata
  .globl EVAL2
  .align 2
  .type  EVAL2, @object
  .size  EVAL2, 4
EVAL2:
  .word 1107966695

  .section .rodata
  .globl EVAL3
  .align 2
  .type  EVAL3, @object
  .size  EVAL3, 4
EVAL3:
  .word 1107966695

  .section .rodata
  .globl CONV1
  .align 2
  .type  CONV1, @object
  .size  CONV1, 4
CONV1:
  .word 1130954752

  .section .rodata
  .globl CONV2
  .align 2
  .type  CONV2, @object
  .size  CONV2, 4
CONV2:
  .word 1166012416

  .section .rodata
  .globl MAX
  .align 2
  .type  MAX, @object
  .size  MAX, 4
MAX:
  .word 1000000000

  .section .rodata
  .globl TWO
  .align 2
  .type  TWO, @object
  .size  TWO, 4
TWO:
  .word 2

  .section .rodata
  .globl THREE
  .align 2
  .type  THREE, @object
  .size  THREE, 4
THREE:
  .word 3

  .section .rodata
  .globl FIVE
  .align 2
  .type  FIVE, @object
  .size  FIVE, 4
FIVE:
  .word 5

  .text

  .globl float_eq
  .type  float_eq, @function
float_eq:
.float_eq.bb0:
	fmv.s	ft0, fa0
	fmv.s	ft1, fa1
	fsub.s	ft0, ft0, ft1
	li	t0, 0
	fmv.w.x	ft1, t0
	flt.s	t0, ft0, ft1
	li	t1, 897988541
	fmv.w.x	ft1, t1
	beqz	t0, .float_eq.bb2
.float_eq.bb3:
	fneg.s	ft0, ft0
.float_eq.bb2:
	flt.s	t0, ft0, ft1
.float_eq.bb1:
	mv	a0, t0
	ret
.float_eq.bb4:
	j	.float_eq.bb2
	.size float_eq, .-float_eq

  .globl error
  .type  error, @function
error:
.error.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 101
	call	putch
	li	a0, 114
	call	putch
	li	a0, 114
	call	putch
	li	a0, 111
	call	putch
	li	a0, 114
	call	putch
	li	a0, 10
	call	putch
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size error, .-error

  .globl ok
  .type  ok, @function
ok:
.ok.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 111
	call	putch
	li	a0, 107
	call	putch
	li	a0, 10
	call	putch
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size ok, .-ok

  .globl assert
  .type  assert, @function
assert:
.assert.bb0:
	addi	sp, sp, -16
	sd	ra, 0(sp)
	mv	t0, a0
	beq	t0, zero, .assert.bb1
.assert.bb2:
	call	ok
.assert.bb3:
	ld	ra, 0(sp)
	addi	sp, sp, 16
	ret
.assert.bb1:
	call	error
	j	.assert.bb3
	.size assert, .-assert

  .globl assert_not
  .type  assert_not, @function
assert_not:
.assert_not.bb0:
	addi	sp, sp, -16
	sd	ra, 0(sp)
	mv	t0, a0
	bne	t0, zero, .assert_not.bb1
.assert_not.bb2:
	call	ok
.assert_not.bb3:
	ld	ra, 0(sp)
	addi	sp, sp, 16
	ret
.assert_not.bb1:
	call	error
	j	.assert_not.bb3
	.size assert_not, .-assert_not

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -128
	sd	ra, 56(sp)
	sd	s1, 64(sp)
	sd	s2, 72(sp)
	sd	s3, 80(sp)
	sd	s4, 88(sp)
	fsw	fs0, 96(sp)
	fsw	fs1, 100(sp)
	fsw	fs2, 104(sp)
	fsw	fs3, 108(sp)
	fsw	fs4, 112(sp)
	li	t0, 1033895936
	fmv.w.x	ft10, t0
	fsw	ft10, 0(sp)
	li	t0, -956241920
	fmv.w.x	ft10, t0
	fsw	ft10, 8(sp)
	flw	fa0, 0(sp)
	flw	fa1, 8(sp)
	call	float_eq
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	assert_not
	li	t0, 1119752446
	fmv.w.x	ft10, t0
	fsw	ft10, 0(sp)
	li	t0, 1107966695
	fmv.w.x	fs0, t0
	fmv.s	ft10, fs0
	fsw	ft10, 8(sp)
	flw	fa0, 0(sp)
	flw	fa1, 8(sp)
	call	float_eq
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	assert_not
	fmv.s	ft10, fs0
	fsw	ft10, 0(sp)
	fmv.s	ft10, fs0
	fsw	ft10, 8(sp)
	flw	fa0, 0(sp)
	flw	fa1, 8(sp)
	call	float_eq
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	assert
	li	t0, 1117590627
	fmv.w.x	ft0, t0
	fmv.s	ft10, ft0
	fsw	ft10, 0(sp)
	fmv.s	ft10, ft0
	fsw	ft10, 8(sp)
	flw	fa0, 0(sp)
	flw	fa1, 8(sp)
	call	float_eq
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	assert
	li	t0, 1130954752
	fmv.w.x	ft10, t0
	fsw	ft10, 0(sp)
	li	t0, 1166012416
	fmv.w.x	ft10, t0
	fsw	ft10, 8(sp)
	flw	fa0, 0(sp)
	flw	fa1, 8(sp)
	call	float_eq
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	assert_not
	call	ok
	call	ok
	call	ok
	addi	t5, sp, 16
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	li	t0, 1065353216
	fmv.w.x	ft0, t0
	addi	t0, sp, 16
	fsw	ft0, 0(t0)
	li	t1, 1073741824
	fmv.w.x	fs0, t1
	fsw	fs0, 4(t0)
	addi	t5, sp, 16
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getfarray
	mv	s1, a0
	li	t0, 1078530011
	fmv.w.x	fs1, t0
	li	t0, 1092616192
	fmv.w.x	fs2, t0
	li	s3, 0
	li	s2, 1
.main.bb1:
	li	t0, 1000000000
	bge	s2, t0, .main.bb3
.main.bb2:
	call	getfloat
	fmv.s	fs3, fa0
	fmul.s	ft0, fs1, fs3
	fmul.s	ft0, ft0, fs3
	fcvt.w.s	s4, fs3, rtz
	fcvt.s.w	ft1, s4
	fmul.s	ft2, fs1, ft1
	fmul.s	ft1, ft2, ft1
	mulw	t0, s4, s4
	fcvt.s.w	ft2, t0
	fmul.s	ft2, ft2, fs1
	fadd.s	ft1, ft1, ft2
	fdiv.s	fs4, ft1, fs0
	slliw	t0, s3, 2
	addi	t1, sp, 16
	add	s4, t1, t0
	flw	ft1, 0(s4)
	fadd.s	ft1, ft1, fs3
	fsw	ft1, 0(s4)
	fmv.s	ft10, ft0
	fsw	ft10, 0(sp)
	flw	fa0, 0(sp)
	call	putfloat
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fcvt.w.s	t5, fs4, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fcvt.s.w	ft0, s2
	fmul.s	ft0, ft0, fs2
	fcvt.w.s	s2, ft0, rtz
	addiw	s3, s3, 1
	j	.main.bb1
.main.bb3:
	mv	t5, s1
	sd	t5, 0(sp)
	addi	t5, sp, 16
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putfarray
	li	a0, 0
	flw	fs4, 112(sp)
	flw	fs3, 108(sp)
	flw	fs2, 104(sp)
	flw	fs1, 100(sp)
	flw	fs0, 96(sp)
	ld	s4, 88(sp)
	ld	s3, 80(sp)
	ld	s2, 72(sp)
	ld	s1, 64(sp)
	ld	ra, 56(sp)
	addi	sp, sp, 128
	ret
	.size main, .-main
