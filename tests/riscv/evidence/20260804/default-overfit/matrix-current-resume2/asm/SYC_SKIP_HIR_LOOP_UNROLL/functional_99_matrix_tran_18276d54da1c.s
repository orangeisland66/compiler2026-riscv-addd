
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
	addi	sp, sp, -160
	sd	ra, 128(sp)
	sd	s1, 136(sp)
	sd	s2, 144(sp)
	fsw	fs0, 152(sp)
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
.main.bb19:
	addiw	s1, t0, 2
	bge	t0, t1, .main.bb13
.main.bb20:
	bge	s1, t1, .main.bb21
.main.bb22:
.main.bb23:
	slliw	s2, t0, 2
	addi	a2, sp, 40
	addi	a3, sp, 56
	add	a2, a2, s2
	add	a3, a3, s2
.main.bb1:
	bge	t0, s1, .main.bb19
.main.bb2:
	fcvt.s.w	fs0, t0
	fsw	fs0, 0(t2)
	addiw	t0, t0, 1
	fsw	fs0, 0(t3)
	addi	t2, t2, 4
	fsw	fs0, 0(a2)
	addi	t3, t3, 4
	fsw	fs0, 0(a3)
	addi	a2, a2, 4
	addi	a3, a3, 4
	blt	t0, s1, .main.bb2	# rotated-loop-branch
	j	.main.bb19	# rotated-loop-exit
.main.bb24:
	j	.main.bb19
.main.bb21:
	mv	s1, t1	# phi-copy
	j	.main.bb23
.main.bb18:
	j	.main.bb13
.main.bb15:
	li	t1, 3
	j	.main.bb17
.main.bb3:
	addi	t0, sp, 40
	flw	ft0, 4(t0)
	addi	t1, sp, 72
	addi	a2, sp, 104
	addi	t5, sp, 24
	addi	a3, sp, 8
	fsw	ft0, 8(t1)
	flw	ft0, 8(t5)
	addi	t5, sp, 88
	fsw	ft0, 4(t5)
	addi	t5, sp, 24
	flw	ft0, 0(t5)
	addi	t4, sp, 104
	addi	a4, t4, 4
	fsw	ft0, 4(a2)
	flw	ft0, 0(t0)
	addi	t5, sp, 72
	fsw	ft0, 8(a2)
	addi	t2, t5, 4
	flw	ft0, 4(a3)
	addi	t5, sp, 40
	addi	t2, t5, 8
	addi	t5, sp, 88
	fsw	ft0, 0(t1)
	addi	t2, t5, 8
	flw	ft0, 8(a3)
	addi	t5, sp, 88
	fsw	ft0, 0(t5)
	addi	t5, sp, 24
	flw	ft0, 4(t5)
	li	s2, 0
	li	s1, 3
	fsw	ft0, 4(t1)
	addi	t5, sp, 88
	flw	ft0, 8(t0)
	fsw	ft0, 8(t5)
	flw	ft0, 0(a3)
	fsw	ft0, 0(a2)
.main.bb4:
	bge	s2, s1, .main.bb6
.main.bb5:
	slliw	t0, s2, 2
	addi	t1, sp, 104
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
	addi	t1, sp, 72
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
	addi	t1, sp, 88
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
	flw	fs0, 152(sp)
	ld	s2, 144(sp)
	ld	s1, 136(sp)
	ld	ra, 128(sp)
	addi	sp, sp, 160
	ret
	.size main, .-main
