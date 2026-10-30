
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

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -128
	sd	ra, 56(sp)
	sd	s1, 64(sp)
	sd	s2, 72(sp)
	sd	s3, 80(sp)
	fsw	fs0, 88(sp)
	fsw	fs1, 92(sp)
	fsw	fs2, 96(sp)
	fsw	fs3, 100(sp)
	fsw	fs4, 104(sp)
	fsw	fs5, 108(sp)
	fsw	fs6, 112(sp)
	fsw	fs7, 116(sp)
	li	t0, 0
.main.bb4:
	addiw	s1, t0, 2
	li	t1, 3
	bge	t0, t1, .main.bb3
.main.bb5:
	li	t1, 3
	bge	s1, t1, .main.bb6
.main.bb7:
	mv	t1, s1	# phi-copy
	j	.main.bb1
.main.bb8:
.main.bb1:
	bge	t0, t1, .main.bb4
.main.bb10:
	addiw	s1, t0, 1
	blt	s1, t1, .main.bb11
.main.bb2:
	slliw	s2, t0, 2
	fcvt.s.w	fs0, t0
	addi	t0, sp, 8
	addi	t2, sp, 24
	add	t0, t0, s2
	add	t2, t2, s2
	fsw	fs0, 0(t0)
	addi	t0, sp, 40
	fsw	fs0, 0(t2)
	add	t0, t0, s2
	fsw	fs0, 0(t0)
	mv	t0, s1	# phi-copy
	j	.main.bb1
.main.bb11:
	slliw	s2, t0, 2
	fcvt.s.w	fs1, s1
	addi	t2, sp, 8
	fcvt.s.w	fs0, t0
	add	s1, t2, s2
	addiw	t0, t0, 2
	addi	t2, sp, 24
	fsw	fs0, 0(s1)
	add	s3, t2, s2
	addi	t2, sp, 40
	fsw	fs0, 0(s3)
	add	s2, t2, s2
	fsw	fs0, 0(s2)
	fsw	fs1, 4(s1)
	fsw	fs1, 4(s3)
	fsw	fs1, 4(s2)
	j	.main.bb1
.main.bb9:
	j	.main.bb4
.main.bb6:
	li	t1, 3
	j	.main.bb1
.main.bb3:
	addi	t0, sp, 40
	flw	fs0, 4(t0)
	addi	t1, sp, 24
	flw	fs1, 8(t1)
	flw	fs2, 0(t1)
	flw	fs3, 0(t0)
	addi	t2, sp, 8
	flw	fs4, 4(t2)
	flw	fs5, 8(t2)
	flw	fs6, 4(t1)
	flw	fs7, 8(t0)
	flw	ft0, 0(t2)
	fcvt.w.s	a0, ft0, rtz
	call	putint
	fcvt.w.s	t5, fs2, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	fcvt.w.s	t5, fs3, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fcvt.w.s	t5, fs4, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	fcvt.w.s	t5, fs6, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	fcvt.w.s	t5, fs0, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	fcvt.w.s	t5, fs5, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	fcvt.w.s	t5, fs1, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	fcvt.w.s	t5, fs7, rtz
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	flw	fs7, 116(sp)
	flw	fs6, 112(sp)
	flw	fs5, 108(sp)
	flw	fs4, 104(sp)
	flw	fs3, 100(sp)
	flw	fs2, 96(sp)
	flw	fs1, 92(sp)
	flw	fs0, 88(sp)
	ld	s3, 80(sp)
	ld	s2, 72(sp)
	ld	s1, 64(sp)
	ld	ra, 56(sp)
	addi	sp, sp, 128
	ret
	.size main, .-main
