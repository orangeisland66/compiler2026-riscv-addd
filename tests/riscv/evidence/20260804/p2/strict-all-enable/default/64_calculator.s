
  .bss
  .globl ints
  .align 2
  .type  ints, @object
  .size  ints, 40000
ints:
  .zero 40000

  .bss
  .globl intt
  .align 2
  .type  intt, @object
  .size  intt, 4
intt:
  .zero 4

  .bss
  .globl chas
  .align 2
  .type  chas, @object
  .size  chas, 40000
chas:
  .zero 40000

  .bss
  .globl chat
  .align 2
  .type  chat, @object
  .size  chat, 4
chat:
  .zero 4

  .bss
  .globl i
  .align 2
  .type  i, @object
  .size  i, 4
i:
  .zero 4

  .data
  .globl ii
  .align 2
  .type  ii, @object
  .size  ii, 4
ii:
  .word 1

  .bss
  .globl c
  .align 2
  .type  c, @object
  .size  c, 4
c:
  .zero 4

  .bss
  .globl get
  .align 2
  .type  get, @object
  .size  get, 40000
get:
  .zero 40000

  .bss
  .globl get2
  .align 2
  .type  get2, @object
  .size  get2, 40000
get2:
  .zero 40000

  .text

  .globl getstr
  .type  getstr, @function
getstr:
.getstr.bb0:
	addi	sp, sp, -32
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	mv	s1, a0
	call	getch
	mv	t0, a0
	li	t1, 0
.getstr.bb1:
	li	t2, 13
	li	t3, 10
	subw	t2, t0, t2
	subw	t3, t0, t3
	sltu	t2, zero, t2
	sltu	t3, zero, t3
	and	t2, t2, t3
	beq	t2, zero, .getstr.bb3
.getstr.bb2:
	slliw	t2, t1, 2
	add	t2, s1, t2
	sw	t0, 0(t2)
	addiw	s2, t1, 1
	call	getch
	mv	t0, a0
	mv	t1, s2	# phi-copy
	j	.getstr.bb1
.getstr.bb3:
	mv	a0, t1
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 32
	ret
	.size getstr, .-getstr

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -96
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	sd	s9, 80(sp)
	la	t0, i
	lw	s1, 0(t0)
	la	t0, ii
	lw	s2, 0(t0)
	la	t0, c
	lw	s3, 0(t0)
	la	t5, get
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getstr
	mv	t0, a0
	li	t1, 0
.main.bb1:
	bge	s1, t0, .main.bb3
.main.bb2:
	slliw	t2, s1, 2
	la	t3, get
	add	s4, t3, t2
	lw	s5, 0(s4)
	li	t2, 48
	li	t3, 57
	slt	t2, s5, t2
	slt	t3, t3, s5
	xori	t2, t2, 1
	xori	t3, t3, 1
	and	t2, t2, t3
.main.bb77:
	bne	t2, zero, .main.bb4
.main.bb5:
	li	t2, 40
	bne	s5, t2, .main.bb96
.main.bb7:
	addiw	s5, t1, 1
	la	t2, chas
	slliw	t1, s5, 2
	add	t1, t2, t1
	li	t2, 40
	sw	t2, 0(t1)
.main.bb8:
	lw	s4, 0(s4)
	li	t1, 94
	beq	s4, t1, .main.bb9
.main.bb97:
.main.bb10:
	li	t1, 41
	beq	s4, t1, .main.bb11
.main.bb98:
	mv	t1, s5	# phi-copy
	mv	t2, s3	# phi-copy
.main.bb12:
	li	t3, 43
	beq	s4, t3, .main.bb18
	j	.main.bb99
.main.bb16:
.main.bb18:
	slliw	t3, t1, 2
	la	a2, chas
	add	t3, a2, t3
	lw	s3, 0(t3)
	li	a2, 45
	li	t3, 43
	subw	a2, s3, a2
	subw	t3, s3, t3
	sltiu	a2, a2, 1
	sltiu	t3, t3, 1
	or	t3, t3, a2
	li	a2, 42
	subw	a2, s3, a2
	sltiu	a2, a2, 1
	or	t3, t3, a2
	li	a2, 47
	subw	a2, s3, a2
	sltiu	a2, a2, 1
	or	t3, t3, a2
	li	a2, 37
	subw	a2, s3, a2
	sltiu	a2, a2, 1
	or	t3, t3, a2
	li	a2, 94
	subw	a2, s3, a2
	sltiu	a2, a2, 1
	or	t3, t3, a2
	beq	t3, zero, .main.bb100
.main.bb19:
	addiw	s5, t1, -1
	la	t2, get2
	slliw	t1, s2, 2
	add	s6, t2, t1
	addiw	s2, s2, 2
	li	t1, 32
	sw	s3, 4(s6)
	sw	t1, 0(s6)
	bne	s5, zero, .main.bb83
.main.bb82:
	li	t1, 0
.main.bb81:
	beq	t1, zero, .main.bb20
.main.bb22:
	mv	t1, s5	# phi-copy
	mv	t2, s3	# phi-copy
	j	.main.bb18
.main.bb83:
	li	t1, 1
	j	.main.bb81
.main.bb21:
.main.bb20:
	addiw	s5, s5, 1
	la	t2, chas
	slliw	t1, s5, 2
	add	t1, t2, t1
	li	t2, 43
	sw	t2, 0(t1)
.main.bb17:
	li	t1, 45
	beq	s4, t1, .main.bb25
	j	.main.bb101
.main.bb23:
.main.bb25:
	slliw	t1, s5, 2
	la	t2, chas
	add	t1, t2, t1
	lw	s6, 0(t1)
	li	t2, 45
	li	t1, 43
	subw	t2, s6, t2
	subw	t1, s6, t1
	sltiu	t2, t2, 1
	sltiu	t1, t1, 1
	or	t1, t1, t2
	li	t2, 42
	subw	t2, s6, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	li	t2, 47
	subw	t2, s6, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	li	t2, 37
	subw	t2, s6, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	li	t2, 94
	subw	t2, s6, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	beq	t1, zero, .main.bb102
.main.bb26:
	addiw	s3, s5, -1
	slliw	t1, s2, 2
	la	t2, get2
	addiw	s2, s2, 2
	add	s5, t2, t1
	li	t1, 32
	sw	s6, 4(s5)
	sw	t1, 0(s5)
	bne	s3, zero, .main.bb86
.main.bb85:
	li	t1, 0
.main.bb84:
	beq	t1, zero, .main.bb27
.main.bb29:
	mv	s5, s3	# phi-copy
	mv	s3, s6	# phi-copy
	j	.main.bb25
.main.bb86:
	li	t1, 1
	j	.main.bb84
.main.bb28:
.main.bb27:
	addiw	s3, s3, 1
	la	t2, chas
	slliw	t1, s3, 2
	add	t1, t2, t1
	li	t2, 45
	sw	t2, 0(t1)
.main.bb24:
	li	t1, 42
	beq	s4, t1, .main.bb32
	j	.main.bb103
.main.bb30:
.main.bb32:
	slliw	t1, s3, 2
	la	t2, chas
	add	t1, t2, t1
	lw	s5, 0(t1)
	li	t2, 47
	li	t1, 42
	subw	t2, s5, t2
	subw	t1, s5, t1
	sltiu	t2, t2, 1
	sltiu	t1, t1, 1
	or	t1, t1, t2
	li	t2, 37
	subw	t2, s5, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	li	t2, 94
	subw	t2, s5, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	beq	t1, zero, .main.bb104
.main.bb33:
	slliw	t1, s2, 2
	la	t2, get2
	add	s6, t2, t1
	addiw	s3, s3, -1
	li	t1, 32
	sw	s5, 4(s6)
	sw	t1, 0(s6)
	addiw	s2, s2, 2
	bne	s3, zero, .main.bb89
.main.bb88:
	li	t1, 0
.main.bb87:
	beq	t1, zero, .main.bb34
.main.bb36:
	mv	s6, s5	# phi-copy
	j	.main.bb32
.main.bb89:
	li	t1, 1
	j	.main.bb87
.main.bb35:
.main.bb34:
	addiw	s3, s3, 1
	la	t2, chas
	slliw	t1, s3, 2
	add	t1, t2, t1
	li	t2, 42
	sw	t2, 0(t1)
.main.bb31:
	li	t1, 47
	beq	s4, t1, .main.bb39
	j	.main.bb105
.main.bb37:
.main.bb39:
	slliw	t1, s3, 2
	la	t2, chas
	add	t1, t2, t1
	lw	s6, 0(t1)
	li	t2, 47
	li	t1, 42
	subw	t2, s6, t2
	subw	t1, s6, t1
	sltiu	t2, t2, 1
	sltiu	t1, t1, 1
	or	t1, t1, t2
	li	t2, 37
	subw	t2, s6, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	li	t2, 94
	subw	t2, s6, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	beq	t1, zero, .main.bb106
.main.bb40:
	slliw	t1, s2, 2
	la	t2, get2
	add	s5, t2, t1
	addiw	s3, s3, -1
	li	t1, 32
	sw	s6, 4(s5)
	sw	t1, 0(s5)
	addiw	s2, s2, 2
	bne	s3, zero, .main.bb92
.main.bb91:
	li	t1, 0
.main.bb90:
	beq	t1, zero, .main.bb41
.main.bb43:
	mv	s5, s6	# phi-copy
	j	.main.bb39
.main.bb92:
	li	t1, 1
	j	.main.bb90
.main.bb42:
.main.bb41:
	addiw	s3, s3, 1
	la	t2, chas
	slliw	t1, s3, 2
	add	t1, t2, t1
	li	t2, 47
	sw	t2, 0(t1)
.main.bb38:
	li	t1, 37
	beq	s4, t1, .main.bb46
	j	.main.bb107
.main.bb44:
.main.bb46:
	slliw	t1, s3, 2
	la	t2, chas
	add	t1, t2, t1
	lw	s4, 0(t1)
	li	t2, 47
	li	t1, 42
	subw	t2, s4, t2
	subw	t1, s4, t1
	sltiu	t2, t2, 1
	sltiu	t1, t1, 1
	or	t1, t1, t2
	li	t2, 37
	subw	t2, s4, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	li	t2, 94
	subw	t2, s4, t2
	sltiu	t2, t2, 1
	or	t1, t1, t2
	beq	t1, zero, .main.bb108
.main.bb47:
	slliw	t1, s2, 2
	la	t2, get2
	add	s5, t2, t1
	addiw	s3, s3, -1
	li	t1, 32
	sw	s4, 4(s5)
	sw	t1, 0(s5)
	addiw	s2, s2, 2
	bne	s3, zero, .main.bb95
.main.bb94:
	li	t1, 0
.main.bb93:
	beq	t1, zero, .main.bb48
.main.bb50:
	mv	s6, s4	# phi-copy
	j	.main.bb46
.main.bb95:
	li	t1, 1
	j	.main.bb93
.main.bb49:
.main.bb48:
	addiw	s3, s3, 1
	la	t2, chas
	slliw	t1, s3, 2
	add	t1, t2, t1
	li	t2, 37
	sw	t2, 0(t1)
.main.bb45:
	slliw	t1, s2, 2
	la	t2, get2
	add	t1, t2, t1
	li	t2, 32
	sw	t2, 0(t1)
	addiw	t1, s2, 1
.main.bb6:
	addiw	s1, s1, 1
	mv	s2, t1	# phi-copy
	mv	t1, s3	# phi-copy
	mv	s3, s4	# phi-copy
	blt	s1, t0, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	s1, t1	# phi-copy
.main.bb51:
	bge	zero, s1, .main.bb53
.main.bb52:
	slliw	t1, s1, 2
	la	t2, chas
	add	t1, t2, t1
	addiw	t0, s1, -1
	lw	t1, 0(t1)
	slliw	t2, s2, 2
	la	t3, get2
	add	s1, t3, t2
	li	t2, 32
	sw	t1, 4(s1)
	sw	t2, 0(s1)
	addiw	s2, s2, 2
	mv	s1, t0	# phi-copy
	blt	zero, s1, .main.bb52	# rotated-loop-branch
.main.bb53:
	slliw	t0, s2, 2
	la	t1, get2
	add	t0, t1, t0
	li	t1, 64
	sw	t1, 0(t0)
	li	s5, 0
	li	s4, 1
.main.bb54:
	slliw	t0, s4, 2
	la	t1, get2
	add	s6, t1, t0
	lw	s7, 0(s6)
	li	t0, 64
	beq	s7, t0, .main.bb56
.main.bb55:
	li	t0, 43
	subw	t0, s7, t0
	sltiu	s8, t0, 1
	li	t0, 45
	subw	t0, s7, t0
	sltiu	t0, t0, 1
	or	t0, s8, t0
	li	t1, 42
	subw	t1, s7, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	li	t1, 47
	subw	t1, s7, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	li	t1, 37
	subw	t1, s7, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	li	t1, 94
	subw	t1, s7, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	beq	t0, zero, .main.bb58
.main.bb57:
	addiw	s7, s5, -1
	slliw	t0, s5, 2
	la	t1, ints
	add	s5, t1, t0
	lw	s9, 0(s5)
	lw	s5, -4(s5)
	beqz	s8, .main.bb109
.main.bb60:
	addw	t0, s9, s5
.main.bb61:
	lw	s6, 0(s6)
	li	t1, 45
	beq	s6, t1, .main.bb62
.main.bb110:
.main.bb63:
	li	t1, 42
	beq	s6, t1, .main.bb64
.main.bb111:
.main.bb65:
	li	t1, 47
	beq	s6, t1, .main.bb66
.main.bb112:
.main.bb67:
	li	t1, 37
	beq	s6, t1, .main.bb68
.main.bb113:
.main.bb69:
	li	t1, 94
	beq	s6, t1, .main.bb70
.main.bb114:
.main.bb71:
	slliw	t1, s7, 2
	la	t2, ints
	add	t1, t2, t1
	sw	t0, 0(t1)
	mv	t0, s2	# phi-copy
	mv	t1, s4	# phi-copy
	mv	s2, s7	# phi-copy
.main.bb59:
	addiw	s4, t1, 1
	mv	s5, s2	# phi-copy
	mv	s2, t0	# phi-copy
	j	.main.bb54
.main.bb100:
	mv	s5, t1	# phi-copy
	mv	s3, t2	# phi-copy
	j	.main.bb20
.main.bb102:
	mv	s6, s3	# phi-copy
	mv	s3, s5	# phi-copy
	j	.main.bb27
.main.bb104:
	mv	s5, s6	# phi-copy
	j	.main.bb34
.main.bb106:
	mv	s6, s5	# phi-copy
	j	.main.bb41
.main.bb108:
	mv	s4, s6	# phi-copy
	j	.main.bb48
.main.bb58:
	li	t0, 32
	beq	s7, t0, .main.bb115
.main.bb72:
	addiw	s2, s5, 1
	la	t2, ints
	slliw	t1, s2, 2
	addiw	t0, s7, -48
	add	t1, t2, t1
	sw	t0, 0(t1)
	li	t0, 1
	li	t1, 32
.main.bb74:
	addw	s5, s4, t0
	la	t3, get2
	slliw	t2, s5, 2
	add	t2, t3, t2
	lw	s6, 0(t2)
	beq	s6, t1, .main.bb76
.main.bb75:
	slliw	t3, s2, 2
	la	a2, ints
	add	s5, a2, t3
	addiw	t2, s6, -48
	lw	t3, 0(s5)
	addiw	t0, t0, 1
	slliw	a2, t3, 3
	slliw	t3, t3, 1
	addw	t3, a2, t3
	addw	t2, t3, t2
	sw	t2, 0(s5)
	j	.main.bb74
.main.bb76:
	addiw	t1, s5, -1
	j	.main.bb59
.main.bb73:
	j	.main.bb59
.main.bb9:
	addiw	s5, s5, 1
	la	t2, chas
	slliw	t1, s5, 2
	add	t1, t2, t1
	li	t2, 94
	sw	t2, 0(t1)
	j	.main.bb10
.main.bb11:
	slliw	t2, s5, 2
	la	t3, chas
	add	t2, t3, t2
	addiw	t1, s5, -1
	lw	t2, 0(t2)
	li	t3, 40
.main.bb13:
	beq	t2, t3, .main.bb12
.main.bb14:
	slliw	a2, s2, 2
	la	a3, get2
	add	s3, a3, a2
	li	a2, 32
	sw	t2, 4(s3)
	sw	a2, 0(s3)
	la	a3, chas
	addiw	s2, s2, 2
	addiw	a2, t1, -1
	slliw	t1, t1, 2
	add	t1, a3, t1
	lw	t2, 0(t1)
	mv	t1, a2	# phi-copy
	bne	t2, t3, .main.bb14	# rotated-loop-branch
	j	.main.bb12	# rotated-loop-exit
.main.bb15:
	j	.main.bb12
.main.bb62:
	subw	t0, s5, s9
	j	.main.bb63
.main.bb64:
	mulw	t0, s9, s5
	j	.main.bb65
.main.bb66:
	divw	t0, s5, s9
	j	.main.bb67
.main.bb68:
	remw	t0, s5, s9
	j	.main.bb69
.main.bb70:
	li	t0, 1
.main.bb78:
	beq	s9, zero, .main.bb71
.main.bb79:
	mulw	t0, t0, s5
	addiw	s9, s9, -1
	bne	s9, zero, .main.bb79	# rotated-loop-branch
	j	.main.bb71	# rotated-loop-exit
.main.bb80:
	j	.main.bb71
.main.bb99:
	mv	s5, t1	# phi-copy
	mv	s3, t2	# phi-copy
	j	.main.bb17
.main.bb101:
	mv	s6, s3	# phi-copy
	mv	s3, s5	# phi-copy
	j	.main.bb24
.main.bb103:
	mv	s5, s6	# phi-copy
	j	.main.bb31
.main.bb105:
	mv	s6, s5	# phi-copy
	j	.main.bb38
.main.bb107:
	mv	s4, s6	# phi-copy
	j	.main.bb45
.main.bb4:
	slliw	t2, s2, 2
	la	t3, get2
	add	t2, t3, t2
	sw	s5, 0(t2)
	addiw	t2, s2, 1
	mv	s4, s3	# phi-copy
	mv	s3, t1	# phi-copy
	mv	t1, t2	# phi-copy
	j	.main.bb6
.main.bb96:
	mv	s5, t1	# phi-copy
	j	.main.bb8
.main.bb109:
	li	t0, 0
	j	.main.bb61
.main.bb115:
	mv	t0, s2	# phi-copy
	mv	t1, s4	# phi-copy
	mv	s2, s5	# phi-copy
	j	.main.bb59
.main.bb56:
	la	t0, ints
	lw	a0, 4(t0)
	call	putint
	la	t0, c
	sw	s3, 0(t0)
	la	t0, ii
	sw	s2, 0(t0)
	la	t0, i
	sw	s4, 0(t0)
	la	t0, chat
	sw	s1, 0(t0)
	la	t0, intt
	sw	s5, 0(t0)
	li	a0, 0
	ld	s9, 80(sp)
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 96
	ret
	.size main, .-main
