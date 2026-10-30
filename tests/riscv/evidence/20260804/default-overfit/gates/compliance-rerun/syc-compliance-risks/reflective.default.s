
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
.main.bb1:
	li	t0, 999877
	slliw	t0, t0, 1
	li	t1, 999877
	addw	t0, t0, t1
	li	t1, 274877907
	mul	t0, t0, t1	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 6	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 1001
	mulw	t0, t0, t1
	li	t1, 999877
	addw	t0, t0, t1
	li	t1, 462120917
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 21	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 19491001
	mulw	t1, t1, t2	# const-rem.mul
	subw	a0, t0, t1	# const-rem.sub
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
