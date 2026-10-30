
  .section .rodata
  .globl ascii_0
  .align 2
  .type  ascii_0, @object
  .size  ascii_0, 4
ascii_0:
  .word 48

  .text

  .globl my_getint
  .type  my_getint, @function
my_getint:
.my_getint.bb0:
	addi	sp, sp, -32
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
.my_getint.bb1:
	call	getch
	mv	t0, a0
	addiw	s1, t0, -48
	li	t0, 0
	slt	t0, s1, t0
	li	t1, 9
	slt	t1, t1, s1
	or	t0, t0, t1
	beq	t0, zero, .my_getint.bb4
.my_getint.bb2:
	j	.my_getint.bb1
.my_getint.bb3:
.my_getint.bb4:
	call	getch
	mv	t0, a0
	addiw	s2, t0, -48
	li	t0, 0
	slt	t0, s2, t0
	xori	t0, t0, 1
	li	t1, 9
	slt	t1, t1, s2
	xori	t1, t1, 1
	and	t0, t0, t1
	beq	t0, zero, .my_getint.bb6
.my_getint.bb5:
	slliw	t0, s1, 3
	slliw	t1, s1, 1
	addw	t0, t0, t1
	addw	t0, t0, s2
	mv	s1, t0	# phi-copy
	j	.my_getint.bb4
.my_getint.bb6:
	mv	a0, s1
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 32
	ret
	.size my_getint, .-my_getint

  .globl my_putint
  .type  my_putint, @function
my_putint:
.my_putint.bb0:
	addi	sp, sp, -96
	sd	ra, 72(sp)
	sd	s1, 80(sp)
	mv	t0, a0
	addi	t1, sp, 8
	li	t2, 0
.my_putint.bb1:
	bge	zero, t0, .my_putint.bb4
.my_putint.bb2:
	li	t3, 1717986919
	addiw	t2, t2, 1
	mul	t3, t0, t3	# const-div.fullmul
	srai	t3, t3, 32	# const-div.mulhi
	sraiw	t3, t3, 2	# const-div.shift
	srliw	a2, t3, 31	# const-div.sign
	addw	t3, t3, a2	# const-div.round
	li	a2, 10
	mulw	t3, t3, a2	# const-rem.mul
	subw	t3, t0, t3	# const-rem.sub
	addiw	t3, t3, 48
	sw	t3, 0(t1)
	li	t3, 1717986919
	addi	t1, t1, 4
	mul	t0, t0, t3	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 2	# const-div.shift
	srliw	t3, t0, 31	# const-div.sign
	addw	t0, t0, t3	# const-div.round
	blt	zero, t0, .my_putint.bb2	# rotated-loop-branch
	j	.my_putint.bb4	# rotated-loop-exit
.my_putint.bb3:
.my_putint.bb4:
	bge	zero, t2, .my_putint.bb6
.my_putint.bb5:
	addiw	s1, t2, -1
	slliw	t0, s1, 2
	addi	t1, sp, 8
	add	t0, t1, t0
	lw	a0, 0(t0)
	call	putch
	mv	t2, s1	# phi-copy
	blt	zero, t2, .my_putint.bb5	# rotated-loop-branch
.my_putint.bb6:
	ld	s1, 80(sp)
	ld	ra, 72(sp)
	addi	sp, sp, 96
	ret
	.size my_putint, .-my_putint

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	call	my_getint
	mv	t0, a0
	mv	s1, t0	# phi-copy
.main.bb1:
	bge	zero, s1, .main.bb3
.main.bb2:
	call	my_getint
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	my_putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	t0, s1, -1
	mv	s1, t0	# phi-copy
	blt	zero, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
