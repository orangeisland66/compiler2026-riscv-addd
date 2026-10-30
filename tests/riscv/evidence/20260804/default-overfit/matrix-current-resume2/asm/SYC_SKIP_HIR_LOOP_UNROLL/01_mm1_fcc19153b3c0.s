
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
	li	t6, -8256
	add	sp, sp, t6
	li	t6, 8200
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 8208
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 8216
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 8224
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 8232
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 8240
	add	t6, sp, t6
	sd	s5, 0(t6)
	li	t6, 8248
	add	t6, sp, t6
	sd	s6, 0(t6)
	call	getint
	mv	s1, a0
	li	s2, 0
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	slliw	s3, s2, 12
	li	s4, 0
.main.bb4:
	bge	s4, s1, .main.bb6
.main.bb5:
	call	getint
	mv	t0, a0
	slliw	t1, s4, 2
	addw	t1, s3, t1
	la	t2, A
	add	t1, t2, t1
	sw	t0, 0(t1)
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
	call	getint
	mv	t0, a0
	slliw	t1, s4, 2
	addw	t1, s3, t1
	la	t2, B
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s4, s4, 1
	blt	s4, s1, .main.bb11	# rotated-loop-branch
.main.bb12:
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	li	t5, 65
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	addi	s2, sp, 8
	li	t0, 0
	mv	s3, s2	# phi-copy
.main.bb13:
	bge	t0, s1, .main.bb15
.main.bb14:
	slliw	t1, t0, 12
	la	t2, B
	add	t1, t2, t1
	li	t3, 0
	li	t2, 0
.main.bb16:
	bge	t2, s1, .main.bb18
.main.bb17:
	lw	a2, 0(t1)
	addiw	t2, t2, 1
	addw	t3, t3, a2
	addi	t1, t1, 4
	blt	t2, s1, .main.bb17	# rotated-loop-branch
.main.bb18:
	sw	t3, 0(s3)
	addiw	t0, t0, 1
	addi	s3, s3, 4
	blt	t0, s1, .main.bb14	# rotated-loop-branch
.main.bb15:
	li	s3, 4104
	add	s3, sp, s3
	li	t0, 0
.main.bb19:
	li	t1, 5
	bge	t0, t1, .main.bb21
.main.bb20:
	li	t1, 0
	mv	s4, s3	# phi-copy
.main.bb22:
	bge	t1, s1, .main.bb24
.main.bb23:
	li	t2, 0
	addiw	t1, t1, 1
	sw	t2, 0(s4)
	addi	s4, s4, 4
	blt	t1, s1, .main.bb23	# rotated-loop-branch
.main.bb24:
	li	t1, 0
	mv	s4, s2	# phi-copy
.main.bb25:
	bge	t1, s1, .main.bb27
.main.bb26:
	lw	t2, 0(s4)
	slliw	t3, t1, 2
	la	a2, A
	add	t3, a2, t3
	li	a2, 0
	mv	s5, s3	# phi-copy
.main.bb28:
	bge	a2, s1, .main.bb30
.main.bb29:
	lw	s6, 0(t3)
	li	a3, 1
	bne	s6, a3, .main.bb31
.main.bb32:
	li	a4, 4096
	addiw	a2, a2, 1
	addi	s5, s5, 4
	add	t3, t3, a4
	j	.main.bb28
.main.bb31:
	lw	a3, 0(s5)
	mulw	a3, a3, s6
	addw	a3, a3, t2
	sw	a3, 0(s5)
	j	.main.bb32
.main.bb30:
	addiw	t1, t1, 1
	addi	s4, s4, 4
	j	.main.bb25
.main.bb39:
	bge	a2, s1, .main.bb41
.main.bb40:
	lw	s6, 0(t3)
	li	a3, 1
	bne	s6, a3, .main.bb42
.main.bb43:
	li	a4, 4096
	addiw	a2, a2, 1
	addi	s5, s5, 4
	add	t3, t3, a4
	j	.main.bb39
.main.bb42:
	lw	a3, 0(s5)
	mulw	a3, a3, s6
	addw	a3, a3, t2
	sw	a3, 0(s5)
	j	.main.bb43
.main.bb41:
	addiw	t1, t1, 1
	addi	s4, s4, 4
.main.bb36:
	bge	t1, s1, .main.bb38
.main.bb37:
	lw	t2, 0(s4)
	slliw	t3, t1, 2
	la	a2, A
	add	t3, a2, t3
	li	a2, 0
	mv	s5, s2	# phi-copy
	j	.main.bb39
.main.bb27:
	li	t1, 0
	mv	s4, s2	# phi-copy
.main.bb33:
	bge	t1, s1, .main.bb35
.main.bb34:
	li	t2, 0
	addiw	t1, t1, 1
	sw	t2, 0(s4)
	addi	s4, s4, 4
	blt	t1, s1, .main.bb34	# rotated-loop-branch
.main.bb35:
	li	t1, 0
	mv	s4, s3	# phi-copy
	j	.main.bb36
.main.bb38:
	addiw	t0, t0, 1
	j	.main.bb19
.main.bb21:
	li	t0, 0
	li	t1, 0
	mv	s3, s2	# phi-copy
	mv	s2, t0	# phi-copy
.main.bb44:
	bge	t1, s1, .main.bb46
.main.bb45:
	lw	t0, 0(s3)
	addiw	t1, t1, 1
	addi	s3, s3, 4
	addw	s2, s2, t0
	blt	t1, s1, .main.bb45	# rotated-loop-branch
.main.bb46:
	li	a0, 84
	call	_sysy_stoptime
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	li	t6, 8248
	add	t6, sp, t6
	ld	s6, 0(t6)
	li	t6, 8240
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 8232
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 8224
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 8216
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 8208
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 8200
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 8256
	add	sp, sp, t6
	ret
	.size main, .-main
