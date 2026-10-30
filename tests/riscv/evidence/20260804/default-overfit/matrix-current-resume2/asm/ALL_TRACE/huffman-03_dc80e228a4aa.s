
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
	addi	sp, sp, -96
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	sd	s6, 80(sp)
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
	la	t5, data
	sd	t5, 8(sp)
	li	t1, 0
	li	t2, 0
	li	t3, 0
	li	a2, 0
	li	a3, 0
	li	a4, 0
	li	a5, 0
	li	a6, 2000
	sd	t2, 16(sp)
.main.bb1:
	bge	zero, a6, .main.bb3
.main.bb2:
	li	a7, 0
	li	t4, 0
	ld	s1, 8(sp)
.main.bb7:
	li	a0, 1
	slt	a1, a7, s5
	slt	a0, t4, a0
	and	a0, a0, a1
	beq	a0, zero, .main.bb11
.main.bb8:
	lw	t0, 0(s1)
.main.bb10:
	addiw	t4, t4, 8
	or	s2, s2, t0
	addiw	a7, a7, 1
	addi	s1, s1, 4
	j	.main.bb7
.main.bb33:
	li	a1, 5
	slt	s1, a7, s5
	slt	a1, t4, a1
	and	a1, a1, s1
	beq	a1, zero, .main.bb38
.main.bb34:
	lw	s1, 0(a0)
	li	a1, 1
	slt	a1, t4, a1
	xori	a1, a1, 1
	beqz	a1, .main.bb63
.main.bb37:
	sllw	a1, s1, t4
.main.bb36:
	or	t2, t2, a1
	addiw	t4, t4, 8
	addiw	a7, a7, 1
	addi	a0, a0, 4
	j	.main.bb33
.main.bb63:
	mv	a1, s1	# phi-copy
	j	.main.bb36
.main.bb35:
.main.bb38:
	li	a0, 32
	sraiw	a1, t2, 31
	addiw	a0, a0, -1
	andi	a1, a1, 31
	and	a0, t2, a0
	addw	t2, t2, a1
	sraiw	s2, t2, 5
.main.bb39:
	addiw	s3, t4, -5
	addiw	s4, a0, 64
	li	t4, 144
	li	t2, 64
	slt	t4, t4, s4
	slt	t2, t2, s4
	xori	t4, t4, 1
	and	t2, t2, t4
	bne	t2, zero, .main.bb24
.main.bb25:
	beqz	s1, .main.bb27
.main.bb26:
	mv	t4, s3	# phi-copy
	mv	t2, s2	# phi-copy
.main.bb23:
	slliw	a0, a7, 2
	la	a1, data
	add	a0, a1, a0
	j	.main.bb33
.main.bb24:
	slliw	t2, t0, 2
	la	t4, out
	add	t2, t4, t2
	addiw	t0, t0, 1
	sw	s4, 0(t2)
	mv	t4, s3	# phi-copy
	mv	t2, s2	# phi-copy
	j	.main.bb23
.main.bb28:
	li	a0, 1
	slt	a1, a7, s5
	slt	a0, t4, a0
	and	a0, a0, a1
	beq	a0, zero, .main.bb30
.main.bb29:
	lw	a0, 0(t0)
.main.bb31:
	or	t2, t2, a0
	addiw	t4, t4, 8
	addiw	a7, a7, 1
	addi	t0, t0, 4
	j	.main.bb28
.main.bb30:
	srliw	t0, t2, 31	# const-div.pow2.bias-fold
	addw	t0, t2, t0
	sraiw	t0, t0, 1
.main.bb32:
	addiw	t4, t4, -1
	mv	t2, t0	# phi-copy
.main.bb20:
	bge	zero, t4, .main.bb22
.main.bb21:
	slliw	t0, a7, 2
	la	a0, data
	add	t0, a0, t0
	j	.main.bb28
.main.bb9:
.main.bb11:
	li	a0, 2
	srliw	a1, s2, 31	# const-div.pow2.bias-fold
	addiw	a0, a0, -1
	addw	a1, s2, a1
	and	a0, s2, a0
	sraiw	a1, a1, 1
.main.bb12:
	addi	t0, sp, 24
	addiw	t4, t4, -1
	sw	a0, 0(t0)
	slliw	t0, a7, 2
	la	a0, data
	add	t0, a0, t0
.main.bb13:
	li	a0, 2
	slt	t2, a7, s5
	slt	a0, t4, a0
	and	t2, a0, t2
	beq	t2, zero, .main.bb18
.main.bb14:
	lw	s1, 0(t0)
	li	t2, 1
	slt	t2, t4, t2
	xori	t2, t2, 1
	beqz	t2, .main.bb62
.main.bb17:
	slliw	t2, s1, 1
.main.bb16:
	or	a1, a1, t2
	addiw	t4, t4, 8
	addiw	a7, a7, 1
	addi	t0, t0, 4
	j	.main.bb13
.main.bb62:
	mv	t2, s1	# phi-copy
	j	.main.bb16
.main.bb15:
.main.bb18:
	sraiw	t2, a1, 31
	li	t0, 4
	andi	t2, t2, 3
	addiw	t0, t0, -1
	addw	t2, a1, t2
	and	t0, a1, t0
	sraiw	t2, t2, 2
.main.bb19:
	addi	a0, sp, 28
	addiw	t4, t4, -2
	sw	t0, 0(a0)
	j	.main.bb20
.main.bb22:
	li	t0, 0
	j	.main.bb23
.main.bb27:
	addiw	t2, a6, -1
	subw	a5, t0, a5
	sltiu	a5, a5, 1
	and	t1, t1, a5
	subw	a4, s3, a4
	sltiu	a4, a4, 1
	and	t1, t1, a4
	subw	a3, s2, a3
	sltiu	a3, a3, 1
	and	t1, t1, a3
	subw	a2, a7, a2
	sltiu	a2, a2, 1
	and	t1, t1, a2
	addi	a2, sp, 24
	lw	s1, 0(a2)
	subw	t3, s1, t3
	sltiu	t3, t3, 1
	and	t1, t1, t3
	addi	t3, sp, 28
	lw	s4, 0(t3)
	ld	t5, 16(sp)
	subw	t3, s4, t5
	sltiu	t3, t3, 1
	and	t1, t1, t3
	bnez	t1, .main.bb61
.main.bb59:
	li	t1, 1
	mv	a6, t2	# phi-copy
	mv	a5, t0	# phi-copy
	mv	a4, s3	# phi-copy
	mv	a3, s2	# phi-copy
	mv	a2, a7	# phi-copy
	mv	t3, s1	# phi-copy
	sd	s4, 16(sp)
	mv	s1, a7	# phi-copy
	mv	s4, t0	# phi-copy
	blt	zero, a6, .main.bb2	# rotated-loop-branch
	j	.main.bb3	# rotated-loop-exit
.main.bb60:
.main.bb3:
	li	t5, 162
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_stoptime
	addi	t0, sp, 24
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addi	t0, sp, 28
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
	ld	s6, 80(sp)
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 96
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
.main.bb61:
	mv	s1, a7	# phi-copy
	mv	s4, t0	# phi-copy
	j	.main.bb3
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
