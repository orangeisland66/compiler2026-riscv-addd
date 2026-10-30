
  .bss
  .globl seed
  .align 2
  .type  seed, @object
  .size  seed, 4
seed:
  .zero 4

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
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	li	t5, 52
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 0
	li	t1, 0
	mv	s3, s2	# phi-copy
	mv	s2, t0	# phi-copy
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	li	t2, 19980130
	andi	t0, s2, 1
	mulw	s3, s3, t2
	bne	t0, zero, .main.bb5
.main.bb4:
	li	t0, 23333
	addw	s3, s3, t0
	li	t0, 360287945
	mul	t0, s3, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 23	# const-div.shift
	srliw	t2, t0, 31	# const-div.sign
	addw	t0, t0, t2	# const-div.round
	li	t2, 100000007
	mulw	t0, t0, t2	# const-rem.mul
	subw	s3, s3, t0	# const-rem.sub
	bge	s3, zero, .main.bb35
.main.bb29:
	li	t0, 100000007
	addw	t0, s3, t0
.main.bb30:
	li	t2, 1757988013
	mul	t2, t0, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	sraiw	t2, t2, 12	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	subw	s3, t1, t2
	li	t1, 0
	mv	s4, s3	# phi-copy
.main.bb7:
	andi	t2, s4, 1
	bne	t2, zero, .main.bb9
.main.bb8:
	sraiw	t2, s4, 31
	addiw	t1, t1, 1
	andi	t2, t2, 1
	addw	t2, s4, t2
	sraiw	s4, t2, 1
	j	.main.bb7
.main.bb13:
	remw	t3, s4, t2
	bne	t3, zero, .main.bb15
.main.bb14:
	divw	s4, s4, t2
	addiw	t1, t1, 1
	j	.main.bb13
.main.bb15:
	addiw	t2, t2, 2
.main.bb10:
	mulw	t3, t2, t2
	blt	s4, t3, .main.bb12
.main.bb11:
	j	.main.bb13
.main.bb24:
	remw	t3, s4, t2
	bne	t3, zero, .main.bb26
.main.bb25:
	divw	s4, s4, t2
	addiw	t1, t1, 1
	j	.main.bb24
.main.bb26:
	addiw	t2, t2, 2
.main.bb21:
	mulw	t3, t2, t2
	blt	s4, t3, .main.bb23
.main.bb22:
	j	.main.bb24
.main.bb12:
	li	t2, 2
	bge	t2, s4, .main.bb17
.main.bb16:
	addiw	t1, t1, 1
.main.bb17:
	mv	s4, t0	# phi-copy
.main.bb6:
	addw	s3, s3, t1
	sraiw	t0, s3, 31
	andi	s5, t0, 255
	addw	t0, s3, s5
	andi	t0, t0, 255
	subw	s3, t0, s5
	mv	t5, s3
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s2, s2, 1
	mv	t1, s3	# phi-copy
	mv	s3, s4	# phi-copy
	blt	s2, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 76
	call	_sysy_stoptime
	la	t0, seed
	sw	s3, 0(t0)
	li	a0, 0
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 64
	ret
.main.bb23:
	li	t2, 2
	bge	t2, s4, .main.bb28
.main.bb27:
	addiw	t1, t1, 1
.main.bb28:
	mv	s4, t0	# phi-copy
	j	.main.bb6
.main.bb9:
	li	t2, 3
	j	.main.bb10
.main.bb5:
	li	t0, 23333
	addw	s3, s3, t0
	li	t0, 360287945
	mul	t0, s3, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 23	# const-div.shift
	srliw	t2, t0, 31	# const-div.sign
	addw	t0, t0, t2	# const-div.round
	li	t2, 100000007
	mulw	t0, t0, t2	# const-rem.mul
	subw	s3, s3, t0	# const-rem.sub
	bge	s3, zero, .main.bb36
.main.bb31:
	li	t0, 100000007
	addw	t0, s3, t0
.main.bb32:
	li	t2, -1757988013
	mul	t2, t0, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	sraiw	t2, t2, 12	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	li	t3, -10007
	mulw	t2, t2, t3	# const-rem.mul
	subw	t2, t0, t2	# const-rem.sub
	addw	s3, t1, t2
	li	t1, 0
	mv	s4, s3	# phi-copy
.main.bb18:
	andi	t2, s4, 1
	bne	t2, zero, .main.bb20
.main.bb19:
	sraiw	t2, s4, 31
	addiw	t1, t1, 1
	andi	t2, t2, 1
	addw	t2, s4, t2
	sraiw	s4, t2, 1
	j	.main.bb18
.main.bb20:
	li	t2, 3
	j	.main.bb21
.main.bb33:
	j	.main.bb17
.main.bb34:
	j	.main.bb28
.main.bb35:
	mv	t0, s3	# phi-copy
	j	.main.bb30
.main.bb36:
	mv	t0, s3	# phi-copy
	j	.main.bb32
	.size main, .-main
