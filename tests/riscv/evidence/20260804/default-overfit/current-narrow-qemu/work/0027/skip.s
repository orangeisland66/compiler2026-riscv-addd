
  .section .rodata
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .word 1024

  .bss
  .globl A
  .align 2
  .type  A, @object
  .size  A, 4194304
A:
  .zero 4194304

  .bss
  .globl B
  .align 2
  .type  B, @object
  .size  B, 4194304
B:
  .zero 4194304

  .bss
  .globl C
  .align 2
  .type  C, @object
  .size  C, 4194304
C:
  .zero 4194304

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -112
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
	call	getint
	mv	s1, a0
	li	t0, 1065353216
	fmv.w.x	fs0, t0
	li	s2, 0
	li	t1, 0
	fmv.w.x	fs1, t1
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	slliw	s3, s2, 12
	li	s4, 0
.main.bb4:
	bge	s4, s1, .main.bb6
.main.bb5:
	call	getfloat
	fmv.s	ft0, fa0
	slliw	t0, s4, 2
	addw	t0, s3, t0
	la	t1, A
	add	t0, t1, t0
	fsw	ft0, 0(t0)
	addiw	s4, s4, 1
	blt	s4, s1, .main.bb5	# rotated-loop-branch
.main.bb6:
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	s2, 0
.main.bb7:
	bge	s2, s1, .main.bb9
.main.bb8:
	slliw	s3, s2, 12
	li	s4, 0
.main.bb10:
	bge	s4, s1, .main.bb12
.main.bb11:
	call	getfloat
	fmv.s	ft0, fa0
	slliw	t0, s4, 2
	addw	t0, s3, t0
	la	t1, C
	add	t0, t1, t0
	fsw	ft0, 0(t0)
	addiw	s4, s4, 1
	blt	s4, s1, .main.bb11	# rotated-loop-branch
.main.bb12:
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	li	t5, 55
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	la	t0, A
	li	t1, 0
.main.bb13:
	li	t2, 5
	bge	t1, t2, .main.bb15
.main.bb14:
	li	t2, 0
.main.bb16:
	bge	t2, s1, .main.bb18
.main.bb17:
	slliw	s2, t2, 12
	li	t3, 0
	la	s3, B	# loop.addr.hoisted
	la	s4, C	# loop.addr.hoisted
.main.bb46:
	addiw	s5, t3, 8
	bge	t3, s1, .main.bb21
.main.bb47:
	bge	s5, s1, .main.bb48
.main.bb49:
	j	.main.bb19
.main.bb50:
.main.bb19:
	bge	t3, s5, .main.bb46
.main.bb52:
	addiw	a2, t3, 3
	blt	a2, s5, .main.bb53
.main.bb20:
	slliw	a2, t3, 2
	addw	a2, s2, a2
	addiw	t3, t3, 1
	add	a3, s4, a2
	flw	ft0, 0(a3)
	add	a2, s3, a2
	fsw	ft0, 0(a2)
	j	.main.bb19
.main.bb53:
	slliw	a2, t3, 2
	addw	a2, s2, a2
	addiw	t3, t3, 4
	add	s6, s4, a2
	add	s7, s3, a2
	flw	ft0, 0(s6)
	fsw	ft0, 0(s7)
	flw	ft0, 4(s6)
	fsw	ft0, 4(s7)
	flw	ft0, 8(s6)
	fsw	ft0, 8(s7)
	flw	ft0, 12(s6)
	fsw	ft0, 12(s7)
	j	.main.bb19
.main.bb37:
	bge	t3, s8, .main.bb40
.main.bb58:
	addiw	a2, t3, 3
	blt	a2, s8, .main.bb59
.main.bb38:
	slliw	s9, t3, 2
	addw	a2, s3, s9
	addw	a3, s6, s9
	add	a2, s7, a2
	add	s9, s7, a3
	flw	ft0, 0(a2)
	addiw	t3, t3, 1
	flw	ft1, 0(s9)
	fmul.s	ft0, fs2, ft0
	fsub.s	ft0, ft1, ft0
	fsw	ft0, 0(s9)
	j	.main.bb37
.main.bb59:
	slliw	s9, t3, 2
	addw	a2, s3, s9
	addw	a3, s6, s9
	add	s9, s7, a2
	add	s10, s7, a3
	flw	ft0, 0(s9)
	flw	ft1, 0(s10)
	flw	ft2, 4(s10)
	flw	ft3, 8(s10)
	flw	ft4, 12(s10)
	fmul.s	ft0, fs2, ft0
	addiw	t3, t3, 4
	fsub.s	ft0, ft1, ft0
	fsw	ft0, 0(s10)
	flw	ft0, 4(s9)
	fmul.s	ft0, fs2, ft0
	fsub.s	ft0, ft2, ft0
	fsw	ft0, 4(s10)
	flw	ft0, 8(s9)
	fmul.s	ft0, fs2, ft0
	fsub.s	ft0, ft3, ft0
	fsw	ft0, 8(s10)
	flw	ft0, 12(s9)
	fmul.s	ft0, fs2, ft0
	fsub.s	ft0, ft4, ft0
	fsw	ft0, 12(s10)
	j	.main.bb37
.main.bb45:
.main.bb40:
	addiw	s8, t3, 16
	bge	t3, s1, .main.bb39
.main.bb41:
	bge	s8, s1, .main.bb42
.main.bb43:
	j	.main.bb37
.main.bb44:
	j	.main.bb37
.main.bb42:
	mv	s8, s1	# phi-copy
	j	.main.bb37
.main.bb51:
	j	.main.bb46
.main.bb39:
	li	a2, 4096
	addiw	s5, s5, 1
	add	t2, t2, a2
.main.bb34:
	bge	s5, s1, .main.bb36
.main.bb35:
	flw	fs2, 0(t2)
	slliw	s6, s5, 12
	li	t3, 0
	la	s7, B	# loop.addr.hoisted
	j	.main.bb40
.main.bb48:
	mv	s5, s1	# phi-copy
	j	.main.bb19
.main.bb31:
	bge	t3, s1, .main.bb33
.main.bb56:
	addiw	s4, t3, 1
	blt	s4, s1, .main.bb57
.main.bb32:
	slliw	t3, t3, 2
	la	a2, B
	addw	t3, s3, t3
	add	s5, a2, t3
	flw	ft0, 0(s5)
	fdiv.s	ft0, ft0, fs2
	fadd.s	ft0, ft0, fs0
	fsw	ft0, 0(s5)
	mv	t3, s4	# phi-copy
	j	.main.bb31
.main.bb57:
	slliw	a2, t3, 2
	la	a3, B
	addw	a2, s3, a2
	addiw	t3, t3, 2
	add	s4, a3, a2
	flw	ft0, 0(s4)
	flw	ft1, 4(s4)
	fdiv.s	ft0, ft0, fs2
	fdiv.s	ft1, ft1, fs2
	fadd.s	ft0, ft0, fs0
	fadd.s	ft1, ft1, fs0
	fsw	ft0, 0(s4)
	fsw	ft1, 4(s4)
	j	.main.bb31
.main.bb33:
	addiw	s4, t2, 1
	slliw	t2, t2, 2
	slliw	t3, s4, 12
	addw	t2, t2, t3
	la	t3, A
	add	t2, t3, t2
	mv	s5, s4	# phi-copy
	j	.main.bb34
.main.bb21:
	addiw	t2, t2, 1
	j	.main.bb16
.main.bb36:
	li	t2, 4100
	add	s2, s2, t2
	mv	t2, s4	# phi-copy
.main.bb28:
	bge	t2, s1, .main.bb30
.main.bb29:
	flw	fs2, 0(s2)
	slliw	s3, t2, 12
	li	t3, 0
	j	.main.bb31
.main.bb25:
	bge	t1, s1, .main.bb27
.main.bb54:
	addiw	t2, t1, 3
	blt	t2, s1, .main.bb55
.main.bb26:
	slliw	t2, t1, 2
	addw	t2, s3, t2
	addiw	t1, t1, 1
	add	t2, s2, t2
	flw	ft0, 0(t2)
	fadd.s	fs1, fs1, ft0
	j	.main.bb25
.main.bb55:
	slliw	t2, t1, 2
	addw	t2, s3, t2
	addiw	t1, t1, 4
	add	s4, s2, t2
	flw	ft0, 0(s4)
	flw	ft1, 4(s4)
	flw	ft2, 8(s4)
	flw	ft3, 12(s4)
	fadd.s	ft0, fs1, ft0
	fadd.s	ft0, ft0, ft1
	fadd.s	ft0, ft0, ft2
	fadd.s	fs1, ft0, ft3
	j	.main.bb25
.main.bb27:
	addiw	t0, t0, 1
	bge	t0, s1, .main.bb24	# rotated-loop-branch
.main.bb23:
	slliw	s3, t0, 12
	li	t1, 0
	j	.main.bb25
.main.bb18:
	li	t2, 0
	mv	s2, t0	# phi-copy
	j	.main.bb28
.main.bb30:
	addiw	t1, t1, 1
	j	.main.bb13
.main.bb15:
	li	t5, 70
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_stoptime
	li	t0, 0
	la	s2, B	# loop.addr.hoisted
.main.bb22:
	blt	t0, s1, .main.bb23
.main.bb24:
	fmv.s	fa0, fs1
	call	putfloat
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
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
	addi	sp, sp, 112
	ret
	.size main, .-main
