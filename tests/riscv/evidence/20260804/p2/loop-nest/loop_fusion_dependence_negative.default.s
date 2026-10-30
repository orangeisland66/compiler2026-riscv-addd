
  .bss
  .globl fusion_values
  .align 2
  .type  fusion_values, @object
  .size  fusion_values, 512
fusion_values:
  .zero 512

  .bss
  .globl fusion_values_aux
  .align 2
  .type  fusion_values_aux, @object
  .size  fusion_values_aux, 512
fusion_values_aux:
  .zero 512

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
	li	t1, 64
	la	s1, fusion_values	# loop.addr.hoisted
	la	s2, fusion_values_aux	# loop.addr.hoisted
.main.bb7:
	addiw	s3, t0, 8
	bge	t0, t1, .main.bb3
.main.bb8:
	li	t2, 64
	bge	s3, t2, .main.bb9
.main.bb10:
	mv	t2, s3	# phi-copy
	j	.main.bb1
.main.bb11:
.main.bb1:
	bge	t0, t2, .main.bb7
.main.bb19:
	addiw	t3, t0, 3
	blt	t3, t2, .main.bb20
.main.bb2:
	slliw	s3, t0, 2
	addiw	t3, t0, 7
	add	a2, s1, s3
	sw	t3, 0(a2)
	addiw	t3, t0, 9
	add	a2, s2, s3
	sw	t3, 0(a2)
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb20:
	slliw	s3, t0, 2
	addiw	t3, t0, 7
	add	s4, s1, s3
	addiw	s5, t0, 9
	sw	t3, 0(s4)
	add	s3, s2, s3
	sw	s5, 0(s3)
	addiw	t3, t0, 8
	sw	t3, 4(s4)
	addiw	s6, t0, 10
	sw	s6, 4(s3)
	sw	s5, 8(s4)
	addiw	a3, t0, 11
	sw	a3, 8(s3)
	sw	s6, 12(s4)
	addiw	a2, t0, 12
	sw	a2, 12(s3)
	addiw	t0, t0, 4
	j	.main.bb1
.main.bb4:
	bge	t1, t3, .main.bb13
.main.bb21:
	addiw	a2, t1, 3
	blt	a2, t3, .main.bb22
.main.bb5:
	slliw	s3, t1, 2
	add	a2, s1, s3
	addiw	t1, t1, 1
	lw	a2, 0(a2)
	addw	t0, t0, a2
	add	a2, s2, s3
	lw	a2, 0(a2)
	addw	t0, t0, a2
	j	.main.bb4
.main.bb22:
	slliw	s3, t1, 2
	add	s4, s1, s3
	addiw	t1, t1, 4
	lw	a2, 0(s4)
	add	s3, s2, s3
	lw	a3, 4(s4)
	lw	a4, 4(s3)
	addw	t0, t0, a2
	lw	a2, 0(s3)
	addw	t0, t0, a2
	addw	t0, t0, a3
	addw	t0, t0, a4
	lw	a4, 8(s4)
	lw	a2, 12(s4)
	addw	t0, t0, a4
	lw	a4, 8(s3)
	addw	t0, t0, a4
	addw	t0, t0, a2
	lw	a2, 12(s3)
	addw	t0, t0, a2
	j	.main.bb4
.main.bb12:
	j	.main.bb7
.main.bb18:
.main.bb13:
	addiw	s3, t1, 8
	bge	t1, t2, .main.bb6
.main.bb14:
	li	t3, 64
	bge	s3, t3, .main.bb15
.main.bb16:
	mv	t3, s3	# phi-copy
	j	.main.bb4
.main.bb17:
	j	.main.bb4
.main.bb9:
	li	t2, 64
	j	.main.bb1
.main.bb15:
	li	t3, 64
	j	.main.bb4
.main.bb3:
	li	t0, 0
	li	t1, 0
	li	t2, 64
	la	s1, fusion_values	# loop.addr.hoisted
	la	s2, fusion_values_aux	# loop.addr.hoisted
	j	.main.bb13
.main.bb6:
	mv	a0, t0
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
