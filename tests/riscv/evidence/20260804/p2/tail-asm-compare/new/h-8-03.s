
  .data
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .word 50

  .bss
  .globl seq
  .align 2
  .type  seq, @object
  .size  seq, 5600
seq:
  .zero 5600

  .bss
  .globl table
  .align 2
  .type  table, @object
  .size  table, 7840000
table:
  .zero 7840000

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -96
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	sd	s8, 80(sp)
	la	t5, seq
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	la	t5, table
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	li	t5, 79
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	la	t0, seq
	addi	t0, t0, 196
	li	t1, 49
.main.bb1:
	blt	t1, zero, .main.bb3
.main.bb2:
	li	t2, 5600
	addiw	s1, t1, 1
	mulw	s2, t1, t2
	slliw	s3, s1, 2
	la	t3, table
	li	a3, 5600
	mulw	a3, s1, a3
	la	a2, seq
	addw	t2, s2, s3
	add	a2, a2, s3
	add	s4, t3, t2
	lw	t3, 0(t0)
	li	t2, 5596
	add	t2, s4, t2
	mv	s3, s4	# phi-copy
	mv	s5, s1	# phi-copy
.main.bb4:
	li	a4, 50
	bge	s5, a4, .main.bb6
.main.bb5:
	slliw	s6, s5, 2
	la	a6, table
	addw	a5, s2, s6
	addiw	a4, s5, -1
	add	s7, a6, a5
	lw	a5, 0(s3)
	lw	s8, -4(s7)
	blt	a5, s8, .main.bb7
.main.bb8:
	lw	a5, 0(s3)
	li	a6, 5600
	add	a6, s7, a6
	lw	a6, 0(a6)
	blt	a5, a6, .main.bb9
.main.bb10:
	blt	t1, a4, .main.bb11
.main.bb12:
	lw	a4, 0(s3)
	lw	s7, 0(t2)
	blt	a4, s7, .main.bb17
	j	.main.bb13
.main.bb18:
.main.bb13:
	li	a4, 5600
	la	a5, table
	addw	a4, s6, a4
	addw	a4, a4, a3
	add	a4, a5, a4
	mv	s6, s1	# phi-copy
	mv	s7, s4	# phi-copy
.main.bb19:
	bge	s6, s5, .main.bb21
.main.bb20:
	lw	s8, 0(s7)
	addiw	a6, s6, 1
	lw	s6, 0(a4)
	lw	a5, 0(s3)
	addw	a7, s8, s6
	blt	a5, a7, .main.bb22
.main.bb23:
	li	a5, 5600
	add	a4, a4, a5
	addi	s7, s7, 4
	mv	s6, a6	# phi-copy
	j	.main.bb19
.main.bb22:
	slliw	a5, s8, 1
	addw	a5, a5, s6
	sw	a5, 0(s3)
	j	.main.bb23
.main.bb21:
	addiw	s5, s5, 1
	addi	s3, s3, 4
	addi	t2, t2, 4
	addi	a2, a2, 4
	j	.main.bb4
.main.bb9:
	slliw	a5, a6, 1
	sw	a5, 0(s3)
	j	.main.bb10
.main.bb11:
	lw	a4, 0(a2)
	li	a5, 3
	addw	a4, t3, a4
	subw	a4, a4, a5
	sltiu	a4, a4, 1
	mulw	a4, a4, a5
.main.bb14:
	lw	a6, 0(t2)
	lw	a5, 0(s3)
	addw	a4, a6, a4
	blt	a5, a4, .main.bb15
.main.bb16:
	j	.main.bb13
.main.bb7:
	sw	s8, 0(s3)
	j	.main.bb8
.main.bb15:
	sw	a4, 0(s3)
	j	.main.bb13
.main.bb17:
	sw	s7, 0(s3)
	j	.main.bb13
.main.bb28:
	lw	s1, 0(a2)
	addiw	t2, t2, 1
	li	a3, 780903145
	mul	a3, s1, a3	# const-div.fullmul
	srai	a3, a3, 32	# const-div.mulhi
	sraiw	a3, a3, 1	# const-div.shift
	srliw	a4, a3, 31	# const-div.sign
	addw	a3, a3, a4	# const-div.round
	li	a4, 11
	mulw	a3, a3, a4	# const-rem.mul
	subw	a3, s1, a3	# const-rem.sub
	sw	a3, 0(a2)
	addi	a2, a2, 4
	blt	t2, t3, .main.bb28	# rotated-loop-branch
	j	.main.bb30	# rotated-loop-exit
.main.bb35:
.main.bb30:
	addiw	s1, t2, 8
	li	t3, 50
	bge	t2, t3, .main.bb29
.main.bb31:
	li	t3, 50
	bge	s1, t3, .main.bb32
.main.bb33:
	mv	t3, s1	# phi-copy
.main.bb34:
	slliw	a2, t2, 2
	la	a3, table
	addw	a2, t1, a2
	add	a2, a3, a2
.main.bb27:
	bge	t2, t3, .main.bb30
	j	.main.bb28
.main.bb32:
	li	t3, 50
	j	.main.bb34
.main.bb6:
	addiw	t1, t1, -1
	addi	t0, t0, -4
	bge	t1, zero, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
.main.bb24:
	li	t1, 50
	bge	t0, t1, .main.bb26
.main.bb25:
	li	t1, 5600
	li	t2, 0
	mulw	t1, t0, t1
	j	.main.bb30
.main.bb29:
	addiw	t0, t0, 1
	j	.main.bb24
.main.bb26:
	li	a0, 81
	call	_sysy_stoptime
	li	t5, 2500
	sd	t5, 0(sp)
	la	t5, table
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	li	a0, 0
	ld	s8, 80(sp)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 96
	ret
	.size main, .-main
