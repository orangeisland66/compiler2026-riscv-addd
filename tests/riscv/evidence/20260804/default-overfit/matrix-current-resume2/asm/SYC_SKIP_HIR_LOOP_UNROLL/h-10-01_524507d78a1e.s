
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
	addi	sp, sp, -80
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	fsw	fs0, 72(sp)
	fsw	fs1, 76(sp)
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
	slliw	t1, t0, 12
	li	t2, 0
.main.bb43:
	addiw	s2, t2, 8
	bge	t2, s1, .main.bb18
.main.bb44:
	bge	s2, s1, .main.bb45
.main.bb46:
.main.bb47:
	slliw	t3, t2, 2
	la	a2, B
	addw	s3, t1, t3
	la	t3, C
	add	a2, a2, s3
	add	t3, t3, s3
.main.bb16:
	bge	t2, s2, .main.bb43
.main.bb17:
	flw	ft0, 0(t3)
	addiw	t2, t2, 1
	addi	a3, a2, 4
	addi	t3, t3, 4
	fsw	ft0, 0(a2)
	mv	a2, a3	# phi-copy
	blt	t2, s2, .main.bb17	# rotated-loop-branch
	j	.main.bb43	# rotated-loop-exit
.main.bb48:
	j	.main.bb43
.main.bb35:
	flw	ft1, 0(a3)
	addiw	t3, t3, 1
	flw	ft2, 0(a2)
	addi	a4, a2, 4
	fmul.s	ft1, ft0, ft1
	addi	a3, a3, 4
	fsub.s	ft1, ft2, ft1
	fsw	ft1, 0(a2)
	mv	a2, a4	# phi-copy
	blt	t3, s5, .main.bb35	# rotated-loop-branch
	j	.main.bb37	# rotated-loop-exit
.main.bb42:
.main.bb37:
	addiw	s5, t3, 16
	bge	t3, s1, .main.bb36
.main.bb38:
	bge	s5, s1, .main.bb39
.main.bb40:
.main.bb41:
	slliw	s6, t3, 2
	la	s7, B
	addw	a2, t2, s6
	addw	a3, s2, s6
	add	a2, s7, a2
	add	a3, s7, a3
.main.bb34:
	bge	t3, s5, .main.bb37
	j	.main.bb35
.main.bb39:
	mv	s5, s1	# phi-copy
	j	.main.bb41
.main.bb36:
	li	t3, 4096
	addiw	s4, s4, 1
	add	t1, t1, t3
.main.bb31:
	bge	s4, s1, .main.bb33
.main.bb32:
	flw	ft0, 0(t1)
	slliw	t2, s4, 12
	li	t3, 0
	j	.main.bb37
.main.bb45:
	mv	s2, s1	# phi-copy
	j	.main.bb47
.main.bb18:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb14	# rotated-loop-branch
.main.bb15:
	la	t0, A
	li	t1, 0
.main.bb25:
	bge	t1, s1, .main.bb27
.main.bb26:
	flw	ft0, 0(t0)
	slliw	s2, t1, 12
	la	t2, B
	li	t3, 0
	add	t2, t2, s2
.main.bb28:
	bge	t3, s1, .main.bb30
.main.bb29:
	flw	ft1, 0(t2)
	addiw	t3, t3, 1
	addi	a2, t2, 4
	fdiv.s	ft1, ft1, ft0
	fadd.s	ft1, ft1, fs0
	fsw	ft1, 0(t2)
	mv	t2, a2	# phi-copy
	blt	t3, s1, .main.bb29	# rotated-loop-branch
.main.bb30:
	addiw	s3, t1, 1
	slliw	t1, t1, 2
	slliw	t2, s3, 12
	addw	t1, t1, t2
	la	t2, A
	add	t1, t2, t1
	mv	s4, s3	# phi-copy
	j	.main.bb31
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
.main.bb19:
	bge	t0, s1, .main.bb21
.main.bb20:
	slliw	t1, t0, 12
	la	t2, B
	add	t1, t2, t1
	li	t2, 0
.main.bb22:
	bge	t2, s1, .main.bb24
.main.bb23:
	flw	ft0, 0(t1)
	addiw	t2, t2, 1
	fadd.s	fs1, fs1, ft0
	addi	t1, t1, 4
	blt	t2, s1, .main.bb23	# rotated-loop-branch
.main.bb24:
	addiw	t0, t0, 1
	blt	t0, s1, .main.bb20	# rotated-loop-branch
.main.bb21:
	fmv.s	fa0, fs1
	call	putfloat
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	flw	fs1, 76(sp)
	flw	fs0, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
