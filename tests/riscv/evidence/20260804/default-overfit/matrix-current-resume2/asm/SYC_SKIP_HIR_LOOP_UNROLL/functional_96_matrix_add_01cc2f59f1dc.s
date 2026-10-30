
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
	addi	sp, sp, -208
	sd	ra, 160(sp)
	sd	s1, 168(sp)
	sd	s2, 176(sp)
	sd	s3, 184(sp)
	sd	s4, 192(sp)
	fsw	fs0, 200(sp)
	li	t0, 3
	la	t1, M
	sw	t0, 0(t1)
	li	t0, 0
.main.bb22:
	addiw	s1, t0, 2
	li	t1, 3
	bge	t0, t1, .main.bb3
.main.bb23:
	li	t1, 3
	bge	s1, t1, .main.bb24
.main.bb25:
	mv	t1, s1	# phi-copy
.main.bb26:
	slliw	s1, t0, 2
	addi	t2, sp, 8
	addi	t3, sp, 24
	add	t2, t2, s1
	add	t3, t3, s1
.main.bb1:
	bge	t0, t1, .main.bb22
.main.bb2:
	fcvt.s.w	fs0, t0
	slliw	s1, t0, 2
	fsw	fs0, 0(t2)
	addi	a2, sp, 40
	fsw	fs0, 0(t3)
	add	a2, a2, s1
	addi	a3, sp, 56
	fsw	fs0, 0(a2)
	add	a2, a3, s1
	addiw	t0, t0, 1
	addi	a3, sp, 72
	fsw	fs0, 0(a2)
	add	a2, a3, s1
	addi	t2, t2, 4
	addi	a3, sp, 88
	fsw	fs0, 0(a2)
	add	a2, a3, s1
	addi	t3, t3, 4
	fsw	fs0, 0(a2)
	blt	t0, t1, .main.bb2	# rotated-loop-branch
	j	.main.bb22	# rotated-loop-exit
.main.bb27:
	j	.main.bb22
.main.bb14:
	flw	ft0, 0(t1)
	slliw	s4, t0, 2
	flw	ft1, 0(t2)
	addi	t3, sp, 136
	add	t3, t3, s4
	addi	a2, sp, 24
	addi	a3, sp, 72
	add	a2, a2, s4
	fadd.s	ft0, ft0, ft1
	add	a3, a3, s4
	addi	a4, sp, 104
	addi	a5, sp, 40
	add	a4, a4, s4
	addi	a6, sp, 88
	add	a5, a5, s4
	add	a6, a6, s4
	fsw	ft0, 0(t3)
	addiw	t0, t0, 1
	flw	ft0, 0(a2)
	addi	t3, sp, 120
	flw	ft1, 0(a3)
	add	t3, t3, s4
	addi	t1, t1, 4
	addi	t2, t2, 4
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 0(a4)
	flw	ft0, 0(a5)
	flw	ft1, 0(a6)
	fadd.s	ft0, ft0, ft1
	fsw	ft0, 0(t3)
	blt	t0, s3, .main.bb14	# rotated-loop-branch
	j	.main.bb28	# rotated-loop-exit
.main.bb33:
.main.bb28:
	addiw	s3, t0, 2
	bge	t0, s2, .main.bb16
.main.bb29:
	bge	s3, s2, .main.bb30
.main.bb31:
.main.bb32:
	slliw	s4, t0, 2
	addi	t1, sp, 8
	addi	t2, sp, 56
	add	t1, t1, s4
	add	t2, t2, s4
.main.bb13:
	bge	t0, s3, .main.bb28
	j	.main.bb14
.main.bb30:
	mv	s3, s2	# phi-copy
	j	.main.bb32
.main.bb21:
.main.bb16:
	addiw	s2, t0, 32
	bge	t0, s1, .main.bb15
.main.bb17:
	bge	s2, s1, .main.bb18
.main.bb19:
	j	.main.bb28
.main.bb20:
	j	.main.bb28
.main.bb18:
	mv	s2, s1	# phi-copy
	j	.main.bb28
.main.bb24:
	li	t1, 3
	j	.main.bb26
.main.bb3:
	la	t0, M
	lw	s1, 0(t0)
	li	t0, 0
	j	.main.bb16
.main.bb15:
	li	s2, 0
	li	s1, 3
.main.bb4:
	bge	s2, s1, .main.bb6
.main.bb5:
	slliw	t0, s2, 2
	addi	t1, sp, 136
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
	addi	t1, sp, 104
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
	addi	t1, sp, 120
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
	flw	fs0, 200(sp)
	ld	s4, 192(sp)
	ld	s3, 184(sp)
	ld	s2, 176(sp)
	ld	s1, 168(sp)
	ld	ra, 160(sp)
	addi	sp, sp, 208
	ret
	.size main, .-main
