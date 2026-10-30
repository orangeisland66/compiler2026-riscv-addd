
  .bss
  .globl data
  .align 2
  .type  data, @object
  .size  data, 400000
data:
  .zero 400000

  .bss
  .globl size
  .align 2
  .type  size, @object
  .size  size, 4
size:
  .zero 4

  .bss
  .globl out
  .align 2
  .type  out, @object
  .size  out, 400000
out:
  .zero 400000

  .bss
  .globl out_num
  .align 2
  .type  out_num, @object
  .size  out_num, 4
out_num:
  .zero 4

  .bss
  .globl pos
  .align 2
  .type  pos, @object
  .size  pos, 4
pos:
  .zero 4

  .bss
  .globl buf
  .align 2
  .type  buf, @object
  .size  buf, 4
buf:
  .zero 4

  .bss
  .globl bits
  .align 2
  .type  bits, @object
  .size  bits, 4
bits:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	la	t0, pos
	lw	s1, 0(t0)
	la	t0, buf
	lw	s2, 0(t0)
	la	t0, bits
	lw	s3, 0(t0)
	la	t0, out_num
	lw	s4, 0(t0)
	la	t5, data
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s5, a0
	li	t5, 153
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	la	t0, data
	li	t1, 2000
.main.bb1:
	blt	t1, zero, .main.bb3
.main.bb2:
	li	t2, 0
	li	t3, 0
	mv	s1, t0	# phi-copy
.main.bb7:
	li	a2, 1
	slt	a3, t2, s5
	slt	a2, t3, a2
	and	a2, a2, a3
	beq	a2, zero, .main.bb11
.main.bb8:
	lw	a2, 0(s1)
.main.bb10:
	or	s2, s2, a2
	addiw	t3, t3, 8
	addiw	t2, t2, 1
	addi	s1, s1, 4
	j	.main.bb7
.main.bb33:
	li	a5, 5
	slt	s1, t2, s5
	slt	a5, t3, a5
	and	a5, a5, s1
	beq	a5, zero, .main.bb38
.main.bb34:
	lw	s1, 0(a4)
	li	a5, 1
	slt	a5, t3, a5
	xori	a5, a5, 1
	beqz	a5, .main.bb60
.main.bb37:
	sllw	a5, s1, t3
.main.bb36:
	or	a3, a3, a5
	addiw	t3, t3, 8
	addiw	t2, t2, 1
	addi	a4, a4, 4
	j	.main.bb33
.main.bb60:
	mv	a5, s1	# phi-copy
	j	.main.bb36
.main.bb35:
.main.bb38:
	li	a4, 32
	sraiw	a5, a3, 31
	addiw	a4, a4, -1
	andi	a5, a5, 31
	and	a4, a3, a4
	addw	a3, a3, a5
	sraiw	s2, a3, 5
.main.bb39:
	addiw	s3, t3, -5
	addiw	s4, a4, 64
	li	a3, 144
	li	t3, 64
	slt	a3, a3, s4
	slt	t3, t3, s4
	xori	a3, a3, 1
	and	t3, t3, a3
	bne	t3, zero, .main.bb24
.main.bb25:
	beqz	s1, .main.bb27
.main.bb26:
	mv	t3, s3	# phi-copy
	mv	a3, s2	# phi-copy
.main.bb23:
	slliw	a4, t2, 2
	la	a5, data
	add	a4, a5, a4
	j	.main.bb33
.main.bb24:
	slliw	t3, a2, 2
	la	a3, out
	add	t3, a3, t3
	sw	s4, 0(t3)
	addiw	a2, a2, 1
	mv	t3, s3	# phi-copy
	mv	a3, s2	# phi-copy
	j	.main.bb23
.main.bb28:
	li	a4, 1
	slt	a5, t2, s5
	slt	a4, t3, a4
	and	a4, a4, a5
	beq	a4, zero, .main.bb30
.main.bb29:
	lw	a4, 0(a2)
.main.bb31:
	or	a3, a3, a4
	addiw	t3, t3, 8
	addiw	t2, t2, 1
	addi	a2, a2, 4
	j	.main.bb28
.main.bb30:
	srliw	a2, a3, 31	# const-div.pow2.bias-fold
	addw	a2, a3, a2
	sraiw	a2, a2, 1
.main.bb32:
	addiw	t3, t3, -1
	mv	a3, a2	# phi-copy
.main.bb20:
	bge	zero, t3, .main.bb22
.main.bb21:
	slliw	a2, t2, 2
	la	a4, data
	add	a2, a4, a2
	j	.main.bb28
.main.bb9:
.main.bb11:
	li	a2, 2
	srliw	a3, s2, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, -1
	addw	a3, s2, a3
	and	a2, s2, a2
	sraiw	a3, a3, 1
.main.bb12:
	addi	a4, sp, 8
	addiw	t3, t3, -1
	sw	a2, 0(a4)
	slliw	a2, t2, 2
	la	a4, data
	add	a2, a4, a2
.main.bb13:
	li	a4, 2
	slt	a5, t2, s5
	slt	a4, t3, a4
	and	a4, a4, a5
	beq	a4, zero, .main.bb18
.main.bb14:
	lw	s1, 0(a2)
	li	a4, 1
	slt	a4, t3, a4
	xori	a4, a4, 1
	beqz	a4, .main.bb59
.main.bb17:
	slliw	a4, s1, 1
.main.bb16:
	or	a3, a3, a4
	addiw	t3, t3, 8
	addiw	t2, t2, 1
	addi	a2, a2, 4
	j	.main.bb13
.main.bb59:
	mv	a4, s1	# phi-copy
	j	.main.bb16
.main.bb15:
.main.bb18:
	li	a2, 4
	sraiw	a4, a3, 31
	addiw	a2, a2, -1
	andi	a4, a4, 3
	and	a2, a3, a2
	addw	a3, a3, a4
	sraiw	a3, a3, 2
.main.bb19:
	addi	a4, sp, 12
	addiw	t3, t3, -2
	sw	a2, 0(a4)
	j	.main.bb20
.main.bb22:
	li	a2, 0
	j	.main.bb23
.main.bb27:
	addiw	t1, t1, -1
	mv	s1, t2	# phi-copy
	mv	s4, a2	# phi-copy
	bge	t1, zero, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t5, 162
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_stoptime
	addi	t0, sp, 8
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addi	t0, sp, 12
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 1000
	bge	t0, s4, .main.bb44
.main.bb45:
	li	t0, 10000
	slt	t0, t0, s4
	xori	t0, t0, 1
	bne	t0, zero, .main.bb50
.main.bb51:
	li	t1, 100000
	li	t0, 10000
	slt	t1, t1, s4
	slt	t0, t0, s4
	xori	t1, t1, 1
	and	t0, t0, t1
	beq	t0, zero, .main.bb58
.main.bb57:
	li	s6, 0
.main.bb46:
	blt	s6, s4, .main.bb47
.main.bb6:
	li	a0, 10
	call	putch
	la	t0, out_num
	sw	s4, 0(t0)
	la	t0, bits
	sw	s3, 0(t0)
	la	t0, buf
	sw	s2, 0(t0)
	la	t0, pos
	sw	s1, 0(t0)
	la	t0, size
	sw	s5, 0(t0)
	li	a0, 0
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 80
	ret
.main.bb47:
	li	t0, 1374389535
	mul	t0, s6, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 4	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 50
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s6, t0	# const-rem.sub
	beq	t0, zero, .main.bb48
.main.bb49:
	addiw	s6, s6, 1
	blt	s6, s4, .main.bb47	# rotated-loop-branch
	j	.main.bb6	# rotated-loop-exit
.main.bb48:
	slliw	t0, s6, 2
	la	t1, out
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb49
.main.bb41:
	li	t0, 1717986919
	mul	t0, s6, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 1	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 5
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s6, t0	# const-rem.sub
	beq	t0, zero, .main.bb42
.main.bb43:
	addiw	s6, s6, 1
	blt	s6, s4, .main.bb41	# rotated-loop-branch
	j	.main.bb6	# rotated-loop-exit
.main.bb42:
	slliw	t0, s6, 2
	la	t1, out
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb43
.main.bb53:
	li	t0, 1374389535
	mul	t0, s6, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 5	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 100
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s6, t0	# const-rem.sub
	bne	t0, zero, .main.bb54
.main.bb55:
	slliw	t0, s6, 2
	la	t1, out
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
.main.bb54:
	addiw	s6, s6, 1
	blt	s6, s4, .main.bb53	# rotated-loop-branch
	j	.main.bb6	# rotated-loop-exit
.main.bb56:
	j	.main.bb54
.main.bb5:
	slliw	t0, s6, 2
	la	t1, out
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s6, s6, 1
	blt	s6, s4, .main.bb5	# rotated-loop-branch
	j	.main.bb6	# rotated-loop-exit
.main.bb44:
	li	s6, 0
.main.bb4:
	bge	s6, s4, .main.bb6
	j	.main.bb5
.main.bb50:
	li	s6, 0
.main.bb40:
	bge	s6, s4, .main.bb6
	j	.main.bb41
.main.bb58:
	li	s6, 0
.main.bb52:
	bge	s6, s4, .main.bb6
	j	.main.bb53
	.size main, .-main
