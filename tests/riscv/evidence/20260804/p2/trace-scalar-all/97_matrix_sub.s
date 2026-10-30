
  .bss
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .zero 4

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

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -192
	sd	ra, 160(sp)
	sd	s1, 168(sp)
	sd	s2, 176(sp)
	fsw	fs0, 184(sp)
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
	addi	t2, sp, 8
	addi	t3, sp, 24
	add	t2, t2, s1
	add	t3, t3, s1
.main.bb1:
	bge	t0, t1, .main.bb13
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
	j	.main.bb13	# rotated-loop-exit
.main.bb18:
	j	.main.bb13
.main.bb5:
	flw	ft0, 0(t2)
	slliw	s2, t0, 2
	flw	ft1, 0(t3)
	addi	a2, sp, 136
	add	a2, a2, s2
	addi	a3, sp, 24
	addi	a4, sp, 72
	add	a3, a3, s2
	fsub.s	ft0, ft0, ft1
	add	a4, a4, s2
	addi	a5, sp, 104
	addi	a6, sp, 40
	add	a5, a5, s2
	addi	a7, sp, 88
	add	a6, a6, s2
	add	a7, a7, s2
	fsw	ft0, 0(a2)
	addiw	t0, t0, 1
	flw	ft0, 0(a3)
	addi	a2, sp, 120
	flw	ft1, 0(a4)
	add	a2, a2, s2
	addi	t2, t2, 4
	addi	t3, t3, 4
	fsub.s	ft0, ft0, ft1
	fsw	ft0, 0(a5)
	flw	ft0, 0(a6)
	flw	ft1, 0(a7)
	fsub.s	ft0, ft0, ft1
	fsw	ft0, 0(a2)
	blt	t0, s1, .main.bb5	# rotated-loop-branch
	j	.main.bb19	# rotated-loop-exit
.main.bb24:
.main.bb19:
	addiw	s1, t0, 2
	bge	t0, t1, .main.bb7
.main.bb20:
	bge	s1, t1, .main.bb21
.main.bb22:
.main.bb23:
	slliw	s2, t0, 2
	addi	t2, sp, 8
	addi	t3, sp, 56
	add	t2, t2, s2
	add	t3, t3, s2
.main.bb4:
	bge	t0, s1, .main.bb19
	j	.main.bb5
.main.bb21:
	mv	s1, t1	# phi-copy
	j	.main.bb23
.main.bb12:
.main.bb7:
	addiw	s1, t0, 2
	li	t1, 3
	bge	t0, t1, .main.bb6
.main.bb8:
	li	t1, 3
	bge	s1, t1, .main.bb9
.main.bb10:
	mv	t1, s1	# phi-copy
	j	.main.bb19
.main.bb11:
	j	.main.bb19
.main.bb9:
	li	t1, 3
	j	.main.bb19
.main.bb15:
	li	t1, 3
	j	.main.bb17
.main.bb3:
	li	t0, 0
	j	.main.bb7
.main.bb6:
	addi	s1, sp, 136
	flw	ft0, 0(s1)
	fcvt.w.s	a0, ft0, rtz
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
	addi	s1, sp, 104
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
	addi	s1, sp, 120
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
	flw	fs0, 184(sp)
	ld	s2, 176(sp)
	ld	s1, 168(sp)
	ld	ra, 160(sp)
	addi	sp, sp, 192
	ret
	.size main, .-main
