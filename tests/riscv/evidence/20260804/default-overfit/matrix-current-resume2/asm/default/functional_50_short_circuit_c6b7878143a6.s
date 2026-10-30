
  .bss
  .globl g
  .align 2
  .type  g, @object
  .size  g, 4
g:
  .zero 4

  .text

  .globl func
  .type  func, @function
func:
.func.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	mv	t0, a0
	la	t1, g
	lw	t2, 0(t1)
	addw	s1, t2, t0
	sw	s1, 0(t1)
	mv	a0, s1
	call	putint
	mv	a0, s1
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size func, .-func

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	call	getint
	mv	t0, a0
	li	t1, 10
	slt	t1, t1, t0
	beqz	t1, .main.bb26
.main.bb2:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	func
	mv	t0, a0
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
.main.bb1:
	bne	t0, zero, .main.bb5
	j	.main.bb5
.main.bb3:
.main.bb5:
	call	getint
	mv	t0, a0
	li	t1, 11
	slt	t1, t1, t0
	beqz	t1, .main.bb27
.main.bb7:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	func
	mv	t0, a0
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
.main.bb6:
	bne	t0, zero, .main.bb10
	j	.main.bb10
.main.bb8:
.main.bb10:
	call	getint
	mv	t0, a0
	li	t1, 99
	slt	t1, t1, t0
	xori	t1, t1, 1
	beqz	t1, .main.bb12
.main.bb28:
	mv	t0, t1	# phi-copy
.main.bb11:
	bne	t0, zero, .main.bb15
	j	.main.bb15
.main.bb13:
.main.bb15:
	call	getint
	mv	t0, a0
	li	t1, 100
	slt	t1, t1, t0
	xori	t1, t1, 1
	beqz	t1, .main.bb17
.main.bb29:
	mv	t0, t1	# phi-copy
.main.bb16:
	bne	t0, zero, .main.bb20
	j	.main.bb20
.main.bb18:
.main.bb20:
	li	t5, 99
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	func
	mv	t0, a0
	li	t1, 0
	subw	t0, t0, t1
	sltiu	t0, t0, 1
	beqz	t0, .main.bb21
.main.bb22:
	li	t5, 100
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	func
	mv	t0, a0
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
.main.bb21:
	bne	t0, zero, .main.bb25
	j	.main.bb25
.main.bb23:
.main.bb25:
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.main.bb4:
	j	.main.bb5
.main.bb9:
	j	.main.bb10
.main.bb12:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	func
	mv	t0, a0
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
	j	.main.bb11
.main.bb14:
	j	.main.bb15
.main.bb17:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	func
	mv	t0, a0
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
	j	.main.bb16
.main.bb19:
	j	.main.bb20
.main.bb24:
	j	.main.bb25
.main.bb27:
	mv	t0, t1	# phi-copy
	j	.main.bb6
.main.bb30:
	j	.main.bb21
.main.bb26:
	mv	t0, t1	# phi-copy
	j	.main.bb1
	.size main, .-main
