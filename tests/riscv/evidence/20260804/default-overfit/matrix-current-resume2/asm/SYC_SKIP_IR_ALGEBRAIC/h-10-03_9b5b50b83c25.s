
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
	fsw	fs0, 88(sp)
	fsw	fs1, 92(sp)
	fsw	fs2, 96(sp)
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
	li	t0, 0
.main.bb13:
	bge	t0, s1, .main.bb15
.main.bb14:
	slliw	s2, t0, 12
	li	t1, 0
	la	s3, B	# loop.addr.hoisted
	la	s4, C	# loop.addr.hoisted
.main.bb43:
	addiw	s5, t1, 8
	bge	t1, s1, .main.bb18
.main.bb44:
	bge	s5, s1, .main.bb45
.main.bb46:
	j	.main.bb16
.main.bb47:
.main.bb16:
	bge	t1, s5, .main.bb43
.main.bb49:
	addiw	t2, t1, 3
	blt	t2, s5, .main.bb50
.main.bb17:
	slliw	t2, t1, 2
	addw	t2, s2, t2
	addiw	t1, t1, 1
	add	t3, s4, t2
	flw	ft0, 0(t3)
	add	t2, s3, t2
	fsw	ft0, 0(t2)
	j	.main.bb16
.main.bb50:
	slliw	t2, t1, 2
	addw	s6, s2, t2
	addiw	t1, t1, 1
	add	s7, s4, s6
	addiw	t1, t1, 1
	flw	ft0, 0(s7)
	add	s6, s3, s6
	fsw	ft0, 0(s6)
	addiw	t1, t1, 1
	flw	ft0, 4(s7)
	addiw	t1, t1, 1
	fsw	ft0, 4(s6)
	flw	ft0, 8(s7)
	fsw	ft0, 8(s6)
	flw	ft0, 12(s7)
	fsw	ft0, 12(s6)
	j	.main.bb16
.main.bb34:
	bge	t2, s7, .main.bb37
.main.bb55:
	addiw	t3, t2, 3
	blt	t3, s7, .main.bb56
.main.bb35:
	slliw	s8, t2, 2
	addw	t3, s2, s8
	addw	a2, s5, s8
	add	t3, s6, t3
	add	s8, s6, a2
	flw	ft0, 0(t3)
	addiw	t2, t2, 1
	flw	ft1, 0(s8)
	fmul.s	ft0, fs2, ft0
	fsub.s	ft0, ft1, ft0
	fsw	ft0, 0(s8)
	j	.main.bb34
.main.bb56:
	slliw	s8, t2, 2
	addw	t3, s2, s8
	addw	a2, s5, s8
	add	s8, s6, t3
	add	s9, s6, a2
	flw	ft0, 0(s8)
	addiw	t2, t2, 1
	flw	ft1, 0(s9)
	flw	ft2, 4(s9)
	flw	ft3, 8(s9)
	addiw	t2, t2, 1
	flw	ft4, 12(s9)
	fmul.s	ft0, fs2, ft0
	addiw	t2, t2, 1
	addiw	t2, t2, 1
	fsub.s	ft0, ft1, ft0
	fsw	ft0, 0(s9)
	flw	ft0, 4(s8)
	fmul.s	ft0, fs2, ft0
	fsub.s	ft0, ft2, ft0
	fsw	ft0, 4(s9)
	flw	ft0, 8(s8)
	fmul.s	ft0, fs2, ft0
	fsub.s	ft0, ft3, ft0
	fsw	ft0, 8(s9)
	flw	ft0, 12(s8)
	fmul.s	ft0, fs2, ft0
	fsub.s	ft0, ft4, ft0
	fsw	ft0, 12(s9)
	j	.main.bb34
.main.bb42:
.main.bb37:
	addiw	s7, t2, 16
	bge	t2, s1, .main.bb36
.main.bb38:
	bge	s7, s1, .main.bb39
.main.bb40:
	j	.main.bb34
.main.bb41:
	j	.main.bb34
.main.bb39:
	mv	s7, s1	# phi-copy
	j	.main.bb34
.main.bb48:
	j	.main.bb43
.main.bb36:
	li	t3, 4096
	addiw	s4, s4, 1
	add	t1, t1, t3
.main.bb31:
	bge	s4, s1, .main.bb33
.main.bb32:
	flw	fs2, 0(t1)
	slliw	s5, s4, 12
	li	t2, 0
	la	s6, B	# loop.addr.hoisted
	j	.main.bb37
.main.bb45:
	mv	s5, s1	# phi-copy
	j	.main.bb16
.main.bb22:
	bge	t1, s1, .main.bb24
.main.bb51:
	addiw	t2, t1, 3
	blt	t2, s1, .main.bb52
.main.bb23:
	slliw	t2, t1, 2
	addw	t2, s3, t2
	addiw	t1, t1, 1
	add	t2, s2, t2
	flw	ft0, 0(t2)
	fadd.s	fs1, fs1, ft0
	j	.main.bb22
.main.bb52:
	slliw	t2, t1, 2
	addw	t2, s3, t2
	addiw	t1, t1, 1
	add	s4, s2, t2
	addiw	t1, t1, 1
	flw	ft0, 0(s4)
	flw	ft1, 4(s4)
	addiw	t1, t1, 1
	flw	ft2, 8(s4)
	flw	ft3, 12(s4)
	fadd.s	ft0, fs1, ft0
	addiw	t1, t1, 1
	fadd.s	ft0, ft0, ft1
	fadd.s	ft0, ft0, ft2
	fadd.s	fs1, ft0, ft3
	j	.main.bb22
.main.bb24:
	addiw	t0, t0, 1
	bge	t0, s1, .main.bb21	# rotated-loop-branch
.main.bb20:
	slliw	s3, t0, 12
	li	t1, 0
	j	.main.bb22
.main.bb28:
	bge	t2, s1, .main.bb30
.main.bb53:
	addiw	s3, t2, 1
	blt	s3, s1, .main.bb54
.main.bb29:
	slliw	t2, t2, 2
	la	t3, B
	addw	t2, s2, t2
	add	s4, t3, t2
	flw	ft0, 0(s4)
	fdiv.s	ft0, ft0, fs2
	fadd.s	ft0, ft0, fs0
	fsw	ft0, 0(s4)
	mv	t2, s3	# phi-copy
	j	.main.bb28
.main.bb54:
	slliw	t2, t2, 2
	la	t3, B
	addw	t2, s2, t2
	addiw	a2, s3, 1
	add	s3, t3, t2
	flw	ft0, 0(s3)
	flw	ft1, 4(s3)
	fdiv.s	ft0, ft0, fs2
	fdiv.s	ft1, ft1, fs2
	fadd.s	ft0, ft0, fs0
	fadd.s	ft1, ft1, fs0
	fsw	ft0, 0(s3)
	fsw	ft1, 4(s3)
	mv	t2, a2	# phi-copy
	j	.main.bb28
.main.bb30:
	addiw	s3, t1, 1
	slliw	t1, t1, 2
	slliw	t2, s3, 12
	addw	t1, t1, t2
	la	t2, A
	add	t1, t2, t1
	mv	s4, s3	# phi-copy
	j	.main.bb31
.main.bb18:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb14	# rotated-loop-branch
.main.bb15:
	la	t0, A
	li	t1, 0
.main.bb25:
	bge	t1, s1, .main.bb27
.main.bb26:
	flw	fs2, 0(t0)
	slliw	s2, t1, 12
	li	t2, 0
	j	.main.bb28
.main.bb33:
	li	t1, 4100
	add	t0, t0, t1
	mv	t1, s3	# phi-copy
	blt	t1, s1, .main.bb26	# rotated-loop-branch
.main.bb27:
	li	t5, 70
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_stoptime
	li	t0, 0
	la	s2, B	# loop.addr.hoisted
.main.bb19:
	blt	t0, s1, .main.bb20
.main.bb21:
	fmv.s	fa0, fs1
	call	putfloat
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	flw	fs2, 96(sp)
	flw	fs1, 92(sp)
	flw	fs0, 88(sp)
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
