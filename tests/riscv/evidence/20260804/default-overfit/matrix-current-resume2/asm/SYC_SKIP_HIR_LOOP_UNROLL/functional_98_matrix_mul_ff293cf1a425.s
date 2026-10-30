
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
	addi	sp, sp, -256
	sd	ra, 224(sp)
	sd	s1, 232(sp)
	sd	s2, 240(sp)
	fsw	fs0, 248(sp)
	li	t0, 0
.main.bb13:
	addiw	s1, t0, 2
	li	t1, 3
	bge	t0, t1, .main.bb3
.main.bb14:
	li	t1, 3
	bge	s1, t1, .main.bb15
.main.bb16:
	mv	t1, s1	# phi-copy
.main.bb17:
	slliw	s1, t0, 2
	addi	t2, sp, 72
	addi	t3, sp, 88
	add	t2, t2, s1
	add	t3, t3, s1
.main.bb1:
	bge	t0, t1, .main.bb13
.main.bb2:
	fcvt.s.w	fs0, t0
	slliw	s1, t0, 2
	fsw	fs0, 0(t2)
	addi	a2, sp, 104
	fsw	fs0, 0(t3)
	add	a2, a2, s1
	addi	a3, sp, 120
	fsw	fs0, 0(a2)
	add	a2, a3, s1
	addiw	t0, t0, 1
	addi	a3, sp, 136
	fsw	fs0, 0(a2)
	add	a2, a3, s1
	addi	t2, t2, 4
	addi	a3, sp, 152
	fsw	fs0, 0(a2)
	add	a2, a3, s1
	addi	t3, t3, 4
	fsw	fs0, 0(a2)
	blt	t0, t1, .main.bb2	# rotated-loop-branch
	j	.main.bb13	# rotated-loop-exit
.main.bb18:
	j	.main.bb13
.main.bb15:
	li	t1, 3
	j	.main.bb17
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
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb5	# rotated-loop-branch
.main.bb6:
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	s2, 0
	li	s1, 3
.main.bb7:
	bge	s2, s1, .main.bb9
.main.bb8:
	slliw	t0, s2, 2
	addi	t1, sp, 168
	add	t0, t1, t0
	flw	ft0, 0(t0)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	s2, 0
	li	s1, 3
.main.bb10:
	bge	s2, s1, .main.bb12
.main.bb11:
	slliw	t0, s2, 2
	addi	t1, sp, 184
	add	t0, t1, t0
	flw	ft0, 0(t0)
	fcvt.w.s	t5, ft0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb11	# rotated-loop-branch
.main.bb12:
	li	a0, 10
	call	putch
	li	a0, 0
	flw	fs0, 248(sp)
	ld	s2, 240(sp)
	ld	s1, 232(sp)
	ld	ra, 224(sp)
	addi	sp, sp, 256
	ret
	.size main, .-main
