
  .bss
  .globl TOKEN_NUM
  .align 2
  .type  TOKEN_NUM, @object
  .size  TOKEN_NUM, 4
TOKEN_NUM:
  .zero 4

  .section .rodata
  .globl TOKEN_OTHER
  .align 2
  .type  TOKEN_OTHER, @object
  .size  TOKEN_OTHER, 4
TOKEN_OTHER:
  .word 1

  .data
  .globl last_char
  .align 2
  .type  last_char, @object
  .size  last_char, 4
last_char:
  .word 32

  .bss
  .globl num
  .align 2
  .type  num, @object
  .size  num, 4
num:
  .zero 4

  .bss
  .globl other
  .align 2
  .type  other, @object
  .size  other, 4
other:
  .zero 4

  .bss
  .globl cur_token
  .align 2
  .type  cur_token, @object
  .size  cur_token, 4
cur_token:
  .zero 4

  .text

  .globl next_char
  .type  next_char, @function
next_char:
.next_char.bb0:
	addi	sp, sp, -16
	sd	ra, 0(sp)
	call	getch
	mv	t0, a0
	la	t1, last_char
	sw	t0, 0(t1)
	mv	a0, t0
	ld	ra, 0(sp)
	addi	sp, sp, 16
	ret
	.size next_char, .-next_char

  .globl next_token
  .type  next_token, @function
next_token:
.next_token.bb0:
	addi	sp, sp, -16
	sd	ra, 0(sp)
	sd	s1, 8(sp)
.next_token.bb1:
	la	t0, last_char
	li	t1, 10
	lw	s1, 0(t0)
	li	t0, 32
	subw	t1, s1, t1
	subw	t0, s1, t0
	sltiu	t1, t1, 1
	sltiu	t0, t0, 1
	or	t0, t0, t1
.next_token.bb10:
	beq	t0, zero, .next_token.bb3
.next_token.bb2:
	call	next_char
	j	.next_token.bb1
.next_token.bb3:
	li	t0, 48
	li	t1, 57
	slt	t0, s1, t0
	slt	t1, t1, s1
	xori	t0, t0, 1
	xori	t1, t1, 1
	and	t0, t0, t1
.next_token.bb11:
	beq	t0, zero, .next_token.bb5
.next_token.bb4:
	addiw	t0, s1, -48
	la	t1, num
	sw	t0, 0(t1)
.next_token.bb7:
	call	next_char
	mv	s1, a0
	li	t0, 48
	slt	t0, s1, t0
	xori	t0, t0, 1
	li	t1, 57
	slt	t1, t1, s1
	xori	t1, t1, 1
	and	t0, t0, t1
.next_token.bb12:
	beq	t0, zero, .next_token.bb9
.next_token.bb8:
	la	s1, num
	lw	t0, 0(s1)
	slliw	t1, t0, 3
	slliw	t0, t0, 1
	addw	t0, t1, t0
	la	t1, last_char
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addiw	t0, t0, -48
	sw	t0, 0(s1)
	j	.next_token.bb7
.next_token.bb9:
	li	t0, 0
	la	t1, cur_token
	sw	t0, 0(t1)
.next_token.bb6:
	la	t0, cur_token
	lw	a0, 0(t0)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 16
	ret
.next_token.bb5:
	la	t0, other
	sw	s1, 0(t0)
	call	next_char
	li	t0, 1
	la	t1, cur_token
	sw	t0, 0(t1)
	j	.next_token.bb6
	.size next_token, .-next_token

  .globl panic
  .type  panic, @function
panic:
.panic.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 112
	call	putch
	li	a0, 97
	call	putch
	li	a0, 110
	call	putch
	li	a0, 105
	call	putch
	li	a0, 99
	call	putch
	li	a0, 33
	call	putch
	li	a0, 10
	call	putch
	li	a0, -1
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size panic, .-panic

  .globl eval
  .type  eval, @function
eval:
.eval.bb0:
	li	t6, -2160
	add	sp, sp, t6
	li	t6, 2072
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 2080
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 2088
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 2096
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 2104
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 2112
	add	t6, sp, t6
	sd	s5, 0(t6)
	li	t6, 2120
	add	t6, sp, t6
	sd	s6, 0(t6)
	li	t6, 2128
	add	t6, sp, t6
	sd	s7, 0(t6)
	li	t6, 2136
	add	t6, sp, t6
	sd	s8, 0(t6)
	li	t6, 2144
	add	t6, sp, t6
	sd	s9, 0(t6)
	addi	a0, sp, 24
	li	a1, 0
	li	a2, 1024
	call	memset
	addi	a0, sp, 1048
	li	a1, 0
	li	a2, 1024
	call	memset
	la	t0, cur_token
	lw	t0, 0(t0)
	addi	s1, sp, 24
	addi	s2, sp, 1048
	beq	t0, zero, .eval.bb3
.eval.bb2:
	call	panic
	mv	t0, a0
.eval.bb1:
	mv	a0, t0
	li	t6, 2144
	add	t6, sp, t6
	ld	s9, 0(t6)
	li	t6, 2136
	add	t6, sp, t6
	ld	s8, 0(t6)
	li	t6, 2128
	add	t6, sp, t6
	ld	s7, 0(t6)
	li	t6, 2120
	add	t6, sp, t6
	ld	s6, 0(t6)
	li	t6, 2112
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 2104
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 2096
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 2088
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 2080
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 2072
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 2160
	add	sp, sp, t6
	ret
.eval.bb29:
	sw	s5, 0(s1)
	sw	t0, -4(s7)
.eval.bb9:
	lw	s5, 0(s2)
	li	t0, 0
	subw	t0, s5, t0
	sltu	s6, zero, t0
	beqz	s6, .eval.bb51
.eval.bb13:
	slliw	t0, s5, 2
	li	t1, 45
	add	t0, s2, t0
	lw	s5, 0(t0)
	li	t0, 43
	subw	t1, s5, t1
	subw	t0, s5, t0
	sltiu	t1, t1, 1
	sltiu	t0, t0, 1
	or	t0, t0, t1
	bne	t0, zero, .eval.bb25
.eval.bb26:
	li	t0, 42
	li	t1, 47
	subw	t0, s5, t0
	subw	t1, s5, t1
	sltiu	t0, t0, 1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	li	t1, 37
	subw	t1, s5, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	bne	t0, zero, .eval.bb27
.eval.bb28:
	li	t0, 0
.eval.bb24:
	slt	t0, t0, s4
	xori	t0, t0, 1
.eval.bb12:
	beq	t0, zero, .eval.bb11
.eval.bb10:
	lw	s5, 0(s2)
	slliw	t0, s5, 2
	add	t0, s2, t0
	lw	s6, 0(t0)
	addiw	t0, s5, -1
	sw	t0, 0(s2)
	lw	s5, 0(s1)
	slliw	t0, s5, 2
	add	s7, s1, t0
	addiw	s5, s5, -1
	lw	s8, 0(s7)
	lw	s9, -4(s7)
	addiw	t0, s5, -1
	sw	t0, 0(s1)
	li	t0, 43
	beq	s6, t0, .eval.bb30
.eval.bb31:
	li	t0, 45
	beq	s6, t0, .eval.bb32
.eval.bb33:
	li	t0, 42
	beq	s6, t0, .eval.bb34
.eval.bb35:
	li	t0, 47
	beq	s6, t0, .eval.bb36
.eval.bb37:
	li	t0, 37
	beq	s6, t0, .eval.bb38
.eval.bb39:
	li	t0, 0
	j	.eval.bb29
.eval.bb51:
	mv	t0, s6	# phi-copy
	j	.eval.bb12
.eval.bb25:
	li	t0, 10
	j	.eval.bb24
.eval.bb27:
	li	t0, 20
	j	.eval.bb24
.eval.bb30:
	addw	t0, s9, s8
	j	.eval.bb29
.eval.bb32:
	subw	t0, s9, s8
	j	.eval.bb29
.eval.bb34:
	mulw	t0, s9, s8
	j	.eval.bb29
.eval.bb36:
	divw	t0, s9, s8
	j	.eval.bb29
.eval.bb38:
	remw	t0, s9, s8
	j	.eval.bb29
.eval.bb11:
	lw	t0, 0(s2)
	addiw	t0, t0, 1
	sw	t0, 0(s2)
	slliw	t0, t0, 2
	add	t0, s2, t0
	sw	s3, 0(t0)
	la	t0, cur_token
	lw	t0, 0(t0)
	bne	t0, zero, .eval.bb14
.eval.bb15:
	la	t0, num
	lw	t0, 0(t0)
	lw	t1, 0(s1)
	addiw	t1, t1, 1
	sw	t1, 0(s1)
	slliw	t1, t1, 2
	add	t1, s1, t1
	sw	t0, 0(t1)
	call	next_token
.eval.bb4:
	la	t0, cur_token
	li	t1, 1
	lw	t0, 0(t0)
	bne	t0, t1, .eval.bb6
.eval.bb5:
	la	t0, other
	li	t1, 45
	lw	s3, 0(t0)
	li	t0, 43
	subw	t1, s3, t1
	subw	t0, s3, t0
	sltiu	t1, t1, 1
	sltiu	t0, t0, 1
	or	t0, t0, t1
	bne	t0, zero, .eval.bb20
.eval.bb21:
	li	t0, 42
	li	t1, 47
	subw	t0, s3, t0
	subw	t1, s3, t1
	sltiu	t0, t0, 1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	li	t1, 37
	subw	t1, s3, t1
	sltiu	t1, t1, 1
	or	t0, t0, t1
	bne	t0, zero, .eval.bb22
.eval.bb23:
	li	s4, 0
.eval.bb19:
	beq	s4, zero, .eval.bb6
.eval.bb8:
	call	next_token
	j	.eval.bb9
.eval.bb20:
	li	s4, 10
	j	.eval.bb19
.eval.bb22:
	li	s4, 20
	j	.eval.bb19
.eval.bb40:
	sw	s3, 0(s1)
	sw	t0, -4(s5)
.eval.bb16:
	lw	s3, 0(s2)
	beq	s3, zero, .eval.bb18
.eval.bb17:
	slliw	t0, s3, 2
	add	t0, s2, t0
	lw	s4, 0(t0)
	addiw	t0, s3, -1
	sw	t0, 0(s2)
	lw	s3, 0(s1)
	slliw	t0, s3, 2
	add	s5, s1, t0
	addiw	s3, s3, -1
	lw	s6, 0(s5)
	lw	s7, -4(s5)
	addiw	t0, s3, -1
	sw	t0, 0(s1)
	li	t0, 43
	beq	s4, t0, .eval.bb41
.eval.bb42:
	li	t0, 45
	beq	s4, t0, .eval.bb43
.eval.bb44:
	li	t0, 42
	beq	s4, t0, .eval.bb45
.eval.bb46:
	li	t0, 47
	beq	s4, t0, .eval.bb47
.eval.bb48:
	li	t0, 37
	beq	s4, t0, .eval.bb49
.eval.bb50:
	li	t0, 0
	j	.eval.bb40
.eval.bb41:
	addw	t0, s7, s6
	j	.eval.bb40
.eval.bb43:
	subw	t0, s7, s6
	j	.eval.bb40
.eval.bb45:
	mulw	t0, s7, s6
	j	.eval.bb40
.eval.bb47:
	divw	t0, s7, s6
	j	.eval.bb40
.eval.bb49:
	remw	t0, s7, s6
	j	.eval.bb40
.eval.bb6:
	call	next_token
	j	.eval.bb16
.eval.bb7:
	j	.eval.bb6
.eval.bb18:
	lw	t0, 0(s1)
	slliw	t0, t0, 2
	add	t0, s1, t0
	lw	t0, 0(t0)
	j	.eval.bb1
.eval.bb14:
	call	panic
	mv	t0, a0
	j	.eval.bb1
.eval.bb3:
	la	t0, num
	lw	t0, 0(t0)
	lw	t1, 0(s1)
	addiw	t1, t1, 1
	sw	t1, 0(s1)
	slliw	t1, t1, 2
	add	t1, s1, t1
	sw	t0, 0(t1)
	call	next_token
	j	.eval.bb4
	.size eval, .-eval

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	call	getint
	mv	s1, a0
	call	getch
	call	next_token
.main.bb1:
	beq	s1, zero, .main.bb3
.main.bb2:
	call	eval
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s1, s1, -1
	bne	s1, zero, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
