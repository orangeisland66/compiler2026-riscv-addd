
  .bss
  .globl M
  .align 2
  .type  M, @object
  .size  M, 4
M:
  .zero 4

  .bss
  .globl L
  .align 2
  .type  L, @object
  .size  L, 4
L:
  .zero 4

  .bss
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .zero 4

  .text

  .globl mul
  .type  mul, @function
mul:
.mul.bb0:
	addi	sp, sp, -16
	sd	s0, 0(sp)
	addi	s0, sp, 16
	mv	t0, a0
	mv	t1, a1
	mv	t2, a2
	mv	t3, a3
	mv	a2, a4
	mv	a3, a5
	mv	a4, a6
	mv	a5, a7
	ld	a6, 0(s0)
	flw	ft0, 0(t0)
	mv	t4, t3
	flw	ft1, 0(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t0)
	flw	ft2, 0(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t0)
	flw	ft2, 0(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 0(a4)
	flw	ft0, 0(t0)
	addi	t4, t4, 4
	flw	ft1, 4(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t0)
	flw	ft2, 4(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t0)
	flw	ft2, 4(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 4(a4)
	flw	ft0, 0(t0)
	addi	t4, t4, 4
	flw	ft1, 8(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t0)
	flw	ft2, 8(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t0)
	flw	ft2, 8(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 8(a4)
	flw	ft0, 0(t1)
	flw	ft1, 0(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t1)
	flw	ft2, 0(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t1)
	flw	ft2, 0(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 0(a5)
	flw	ft0, 0(t1)
	flw	ft1, 4(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t1)
	flw	ft2, 4(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t1)
	flw	ft2, 4(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 4(a5)
	flw	ft0, 0(t1)
	flw	ft1, 8(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t1)
	flw	ft2, 8(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t1)
	flw	ft2, 8(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 8(a5)
	flw	ft0, 0(t2)
	flw	ft1, 0(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t2)
	flw	ft2, 0(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t2)
	flw	ft2, 0(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 0(a6)
	flw	ft0, 0(t2)
	flw	ft1, 4(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t2)
	flw	ft2, 4(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t2)
	flw	ft2, 4(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 4(a6)
	flw	ft0, 0(t2)
	flw	ft1, 8(t3)
	fmul.s	ft0, ft0, ft1
	flw	ft1, 4(t2)
	flw	ft2, 8(a2)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	flw	ft1, 8(t2)
	flw	ft2, 8(a3)
	fmul.s	ft1, ft1, ft2
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 8(a6)
	li	a0, 0
	ld	s0, 0(sp)
	addi	sp, sp, 16
	ret
	.size mul, .-mul

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -288
	sd	ra, 224(sp)
	sd	s1, 232(sp)
	sd	s2, 240(sp)
	sd	s3, 248(sp)
	sd	s4, 256(sp)
	sd	s5, 264(sp)
	sd	s6, 272(sp)
	fsw	fs0, 280(sp)
	fsw	fs1, 284(sp)
	li	t0, 0
	li	t1, 3
.main.bb7:
	addiw	s1, t0, 2
	bge	t0, t1, .main.bb3
.main.bb8:
	li	t2, 3
	bge	s1, t2, .main.bb9
.main.bb10:
	mv	t2, s1	# phi-copy
	j	.main.bb1
.main.bb11:
.main.bb1:
	bge	t0, t2, .main.bb7
.main.bb13:
	addiw	s1, t0, 1
	blt	s1, t2, .main.bb14
.main.bb2:
	slliw	s2, t0, 2
	fcvt.s.w	fs0, t0
	addi	t0, sp, 72
	addi	t3, sp, 88
	add	t0, t0, s2
	add	t3, t3, s2
	fsw	fs0, 0(t0)
	addi	t0, sp, 104
	fsw	fs0, 0(t3)
	add	t0, t0, s2
	addi	t3, sp, 120
	fsw	fs0, 0(t0)
	add	t0, t3, s2
	addi	t3, sp, 136
	fsw	fs0, 0(t0)
	add	t0, t3, s2
	addi	t3, sp, 152
	fsw	fs0, 0(t0)
	add	t0, t3, s2
	fsw	fs0, 0(t0)
	mv	t0, s1	# phi-copy
	j	.main.bb1
.main.bb14:
	slliw	s2, t0, 2
	fcvt.s.w	fs1, s1
	addi	t3, sp, 72
	fcvt.s.w	fs0, t0
	add	s1, t3, s2
	addiw	t0, t0, 2
	addi	t3, sp, 88
	fsw	fs0, 0(s1)
	add	s3, t3, s2
	addi	t3, sp, 104
	fsw	fs0, 0(s3)
	add	s4, t3, s2
	addi	t3, sp, 120
	fsw	fs0, 0(s4)
	add	s5, t3, s2
	addi	t3, sp, 136
	fsw	fs0, 0(s5)
	add	s6, t3, s2
	addi	t3, sp, 152
	fsw	fs0, 0(s6)
	add	s2, t3, s2
	fsw	fs0, 0(s2)
	fsw	fs1, 4(s1)
	fsw	fs1, 4(s3)
	fsw	fs1, 4(s4)
	fsw	fs1, 4(s5)
	fsw	fs1, 4(s6)
	fsw	fs1, 4(s2)
	j	.main.bb1
.main.bb12:
	j	.main.bb7
.main.bb9:
	li	t2, 3
	j	.main.bb1
.main.bb3:
	addi	t5, sp, 72
	sd	t5, 0(sp)
	addi	t5, sp, 88
	sd	t5, 8(sp)
	addi	t5, sp, 104
	sd	t5, 16(sp)
	addi	t5, sp, 120
	sd	t5, 24(sp)
	addi	t5, sp, 136
	sd	t5, 32(sp)
	addi	t5, sp, 152
	sd	t5, 40(sp)
	addi	t5, sp, 200
	sd	t5, 48(sp)
	addi	t5, sp, 168
	sd	t5, 56(sp)
	addi	t5, sp, 184
	sd	t5, 64(sp)
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
	call	mul
	mv	t0, a0
	li	s1, 3
	mv	s2, t0	# phi-copy
.main.bb4:
	bge	s2, s1, .main.bb6
.main.bb5:
	slliw	t0, s2, 2
	addi	t1, sp, 200
	add	t0, t1, t0
	flw	ft0, 0(t0)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	t0, s2, 1
	mv	s2, t0	# phi-copy
	blt	s2, s1, .main.bb5	# rotated-loop-branch
.main.bb6:
	li	a0, 10
	call	putch
	addi	s1, sp, 168
	flw	ft0, 0(s1)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	flw	ft0, 4(s1)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	flw	ft0, 8(s1)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addi	s1, sp, 184
	flw	ft0, 0(s1)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	flw	ft0, 4(s1)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	flw	ft0, 8(s1)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	flw	fs1, 284(sp)
	flw	fs0, 280(sp)
	ld	s6, 272(sp)
	ld	s5, 264(sp)
	ld	s4, 256(sp)
	ld	s3, 248(sp)
	ld	s2, 240(sp)
	ld	s1, 232(sp)
	ld	ra, 224(sp)
	addi	sp, sp, 288
	ret
	.size main, .-main
