
  .section .rodata
  .globl mod
  .align 2
  .type  mod, @object
  .size  mod, 4
mod:
  .word 998244853

  .text

  .globl loop_test
  .type  loop_test, @function
loop_test:
.loop_test.bb0:
	addi	sp, sp, -16
	sd	s1, 0(sp)
	mv	t0, a0
	mv	t1, a1
	mv	t2, a2
	li	t3, 0
.loop_test.bb1:
	bge	t0, t1, .loop_test.bb3
.loop_test.bb2:
	li	a2, 2147483647
	subw	s1, a2, t0
	blt	t0, s1, .loop_test.bb4
	j	.loop_test.bb6
.loop_test.bb5:
.loop_test.bb4:
	slliw	a2, s1, 1
	li	a3, 274877907
	addw	a2, a2, s1
	addw	t0, t0, t2
	mul	a2, a2, a3	# const-div.fullmul
	srai	a2, a2, 32	# const-div.mulhi
	sraiw	a2, a2, 6	# const-div.shift
	srliw	a3, a2, 31	# const-div.sign
	addw	a2, a2, a3	# const-div.round
	li	a3, 1001
	mulw	a2, a2, a3
	addw	s1, s1, a2
	li	a2, 462120917
	mul	a2, s1, a2	# const-div.fullmul
	srai	a2, a2, 32	# const-div.mulhi
	sraiw	a2, a2, 21	# const-div.shift
	srliw	a3, a2, 31	# const-div.sign
	addw	a2, a2, a3	# const-div.round
	li	a3, 19491001
	mulw	a2, a2, a3	# const-rem.mul
	subw	a2, s1, a2	# const-rem.sub
	addw	t3, t3, a2
	addiw	s1, t3, 1
	li	t3, -1985070077
	mul	t3, s1, t3	# const-div.fullmul
	srai	t3, t3, 32	# const-div.mulhi
	addw	t3, t3, s1	# const-div.fixup
	sraiw	t3, t3, 29	# const-div.shift
	srliw	a2, t3, 31	# const-div.sign
	addw	t3, t3, a2	# const-div.round
	li	a2, 998244853
	mulw	t3, t3, a2	# const-rem.mul
	subw	t3, s1, t3	# const-rem.sub
	blt	t0, t1, .loop_test.bb2	# rotated-loop-branch
.loop_test.bb3:
	mv	a0, t3
	ld	s1, 0(sp)
	addi	sp, sp, 16
	ret
.loop_test.bb6:
	mv	s1, t0	# phi-copy
	j	.loop_test.bb4
	.size loop_test, .-loop_test

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	sd	s3, 48(sp)
	call	getint
	mv	s1, a0
	li	t5, 33
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
.main.bb1:
	beq	s1, zero, .main.bb3
.main.bb2:
	call	getint
	mv	s2, a0
	call	getint
	mv	s3, a0
	call	getint
	mv	t0, a0
	mv	t5, s2
	sd	t5, 0(sp)
	mv	t5, s3
	sd	t5, 8(sp)
	mv	t5, t0
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	loop_test
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
	li	a0, 42
	call	_sysy_stoptime
	li	a0, 0
	ld	s3, 48(sp)
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main
