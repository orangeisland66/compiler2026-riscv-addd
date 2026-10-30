
  .bss
  .globl interchange_input
  .align 2
  .type  interchange_input, @object
  .size  interchange_input, 1024
interchange_input:
  .zero 1024

  .bss
  .globl interchange_aux
  .align 2
  .type  interchange_aux, @object
  .size  interchange_aux, 1024
interchange_aux:
  .zero 1024

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	li	t0, 0
	li	t1, 256
	la	s1, interchange_aux	# loop.addr.hoisted
	la	s2, interchange_input	# loop.addr.hoisted
.main.bb4:
	addiw	s3, t0, 8
	bge	t0, t1, .main.bb3
.main.bb5:
	li	t2, 256
	bge	s3, t2, .main.bb6
.main.bb7:
	mv	t2, s3	# phi-copy
	j	.main.bb1
.main.bb8:
.main.bb1:
	bge	t0, t2, .main.bb4
.main.bb10:
	addiw	s3, t0, 3
	blt	s3, t2, .main.bb11
.main.bb2:
	slliw	s4, t0, 2
	addiw	s3, t0, 1
	add	t3, s2, s4
	addiw	t0, t0, 2
	sw	s3, 0(t3)
	add	t3, s1, s4
	sw	t0, 0(t3)
	mv	t0, s3	# phi-copy
	j	.main.bb1
.main.bb11:
	slliw	s4, t0, 2
	addiw	t3, t0, 1
	add	s5, s2, s4
	addiw	s6, t0, 2
	sw	t3, 0(s5)
	add	s4, s1, s4
	sw	s6, 0(s4)
	sw	s6, 4(s5)
	sw	s3, 4(s4)
	sw	s3, 8(s5)
	addiw	s3, t0, 4
	sw	s3, 8(s4)
	addiw	t0, t0, 5
	sw	s3, 12(s5)
	sw	t0, 12(s4)
	mv	t0, s3	# phi-copy
	j	.main.bb1
.main.bb9:
	j	.main.bb4
.main.bb12:
	bge	t0, t3, .main.bb15
.main.bb21:
	addiw	a2, t0, 3
	blt	a2, t3, .main.bb22
.main.bb13:
	slliw	s3, t0, 2
	add	a2, s2, s3
	addiw	t0, t0, 1
	lw	a2, 0(a2)
	addw	t1, t1, a2
	add	a2, s1, s3
	lw	a2, 0(a2)
	addw	t1, t1, a2
	j	.main.bb12
.main.bb22:
	slliw	s3, t0, 2
	add	s4, s2, s3
	addiw	t0, t0, 4
	lw	a2, 0(s4)
	add	s3, s1, s3
	lw	a3, 4(s4)
	lw	a4, 4(s3)
	addw	t1, t1, a2
	lw	a2, 0(s3)
	addw	t1, t1, a2
	addw	t1, t1, a3
	addw	t1, t1, a4
	lw	a4, 8(s4)
	lw	a2, 12(s4)
	addw	t1, t1, a4
	lw	a4, 8(s3)
	addw	t1, t1, a4
	addw	t1, t1, a2
	lw	a2, 12(s3)
	addw	t1, t1, a2
	j	.main.bb12
.main.bb20:
.main.bb15:
	addiw	s3, t0, 8
	bge	t0, t2, .main.bb14
.main.bb16:
	li	t3, 64
	bge	s3, t3, .main.bb17
.main.bb18:
	mv	t3, s3	# phi-copy
	j	.main.bb12
.main.bb19:
	j	.main.bb12
.main.bb23:
	bge	t0, t3, .main.bb26
.main.bb32:
	addiw	a2, t0, 3
	blt	a2, t3, .main.bb33
.main.bb24:
	addiw	a2, t0, 64
	slliw	s3, a2, 2
	addiw	t0, t0, 1
	add	a2, s2, s3
	lw	a2, 0(a2)
	addw	t1, t1, a2
	add	a2, s1, s3
	lw	a2, 0(a2)
	addw	t1, t1, a2
	j	.main.bb23
.main.bb33:
	addiw	a2, t0, 64
	slliw	s3, a2, 2
	addiw	t0, t0, 4
	add	s4, s2, s3
	lw	a2, 0(s4)
	add	s3, s1, s3
	lw	a3, 4(s4)
	lw	a4, 4(s3)
	addw	t1, t1, a2
	lw	a2, 0(s3)
	addw	t1, t1, a2
	addw	t1, t1, a3
	addw	t1, t1, a4
	lw	a4, 8(s4)
	lw	a2, 12(s4)
	addw	t1, t1, a4
	lw	a4, 8(s3)
	addw	t1, t1, a4
	addw	t1, t1, a2
	lw	a2, 12(s3)
	addw	t1, t1, a2
	j	.main.bb23
.main.bb31:
.main.bb26:
	addiw	s3, t0, 8
	bge	t0, t2, .main.bb25
.main.bb27:
	li	t3, 64
	bge	s3, t3, .main.bb28
.main.bb29:
	mv	t3, s3	# phi-copy
	j	.main.bb23
.main.bb30:
	j	.main.bb23
.main.bb34:
	bge	t0, t3, .main.bb37
.main.bb43:
	addiw	a2, t0, 3
	blt	a2, t3, .main.bb44
.main.bb35:
	addiw	a2, t0, 128
	slliw	s3, a2, 2
	addiw	t0, t0, 1
	add	a2, s2, s3
	lw	a2, 0(a2)
	addw	t1, t1, a2
	add	a2, s1, s3
	lw	a2, 0(a2)
	addw	t1, t1, a2
	j	.main.bb34
.main.bb44:
	addiw	a2, t0, 128
	slliw	s3, a2, 2
	addiw	t0, t0, 4
	add	s4, s2, s3
	lw	a2, 0(s4)
	add	s3, s1, s3
	lw	a3, 4(s4)
	lw	a4, 4(s3)
	addw	t1, t1, a2
	lw	a2, 0(s3)
	addw	t1, t1, a2
	addw	t1, t1, a3
	addw	t1, t1, a4
	lw	a4, 8(s4)
	lw	a2, 12(s4)
	addw	t1, t1, a4
	lw	a4, 8(s3)
	addw	t1, t1, a4
	addw	t1, t1, a2
	lw	a2, 12(s3)
	addw	t1, t1, a2
	j	.main.bb34
.main.bb42:
.main.bb37:
	addiw	s3, t0, 8
	bge	t0, t2, .main.bb36
.main.bb38:
	li	t3, 64
	bge	s3, t3, .main.bb39
.main.bb40:
	mv	t3, s3	# phi-copy
	j	.main.bb34
.main.bb41:
	j	.main.bb34
.main.bb45:
	bge	t0, t3, .main.bb48
.main.bb54:
	addiw	a2, t0, 3
	blt	a2, t3, .main.bb55
.main.bb46:
	addiw	a2, t0, 192
	slliw	s3, a2, 2
	addiw	t0, t0, 1
	add	a2, s2, s3
	lw	a2, 0(a2)
	addw	t1, t1, a2
	add	a2, s1, s3
	lw	a2, 0(a2)
	addw	t1, t1, a2
	j	.main.bb45
.main.bb55:
	addiw	a2, t0, 192
	slliw	s3, a2, 2
	addiw	t0, t0, 4
	add	s4, s2, s3
	lw	a2, 0(s4)
	add	s3, s1, s3
	lw	a3, 4(s4)
	lw	a4, 4(s3)
	addw	t1, t1, a2
	lw	a2, 0(s3)
	addw	t1, t1, a2
	addw	t1, t1, a3
	addw	t1, t1, a4
	lw	a4, 8(s4)
	lw	a2, 12(s4)
	addw	t1, t1, a4
	lw	a4, 8(s3)
	addw	t1, t1, a4
	addw	t1, t1, a2
	lw	a2, 12(s3)
	addw	t1, t1, a2
	j	.main.bb45
.main.bb53:
.main.bb48:
	addiw	s3, t0, 8
	bge	t0, t2, .main.bb47
.main.bb49:
	li	t3, 64
	bge	s3, t3, .main.bb50
.main.bb51:
	mv	t3, s3	# phi-copy
	j	.main.bb45
.main.bb52:
	j	.main.bb45
.main.bb6:
	li	t2, 256
	j	.main.bb1
.main.bb17:
	li	t3, 64
	j	.main.bb12
.main.bb28:
	li	t3, 64
	j	.main.bb23
.main.bb39:
	li	t3, 64
	j	.main.bb34
.main.bb50:
	li	t3, 64
	j	.main.bb45
.main.bb3:
	li	t0, 0
	li	t1, 0
	li	t2, 64
	la	s1, interchange_aux	# loop.addr.hoisted
	la	s2, interchange_input	# loop.addr.hoisted
	j	.main.bb15
.main.bb14:
	li	t0, 0
	li	t2, 64
	la	s1, interchange_aux	# loop.addr.hoisted
	la	s2, interchange_input	# loop.addr.hoisted
	j	.main.bb26
.main.bb25:
	li	t0, 0
	li	t2, 64
	la	s1, interchange_aux	# loop.addr.hoisted
	la	s2, interchange_input	# loop.addr.hoisted
	j	.main.bb37
.main.bb36:
	li	t0, 0
	li	t2, 64
	la	s1, interchange_aux	# loop.addr.hoisted
	la	s2, interchange_input	# loop.addr.hoisted
	j	.main.bb48
.main.bb47:
	mv	a0, t1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main
