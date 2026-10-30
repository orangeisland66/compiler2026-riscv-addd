
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -432
	sd	ra, 400(sp)
	sd	s1, 408(sp)
	sd	s2, 416(sp)
	addi	s1, sp, 0
	li	s2, 0
.main.bb1:
	call	getint
	mv	t0, a0
	beq	t0, zero, .main.bb3
.main.bb2:
	call	getint
	mv	t0, a0
	slliw	t1, s2, 2
	add	t1, s1, t1
	sw	t0, 0(t1)
	addiw	s2, s2, 1
	j	.main.bb1
.main.bb3:
	slliw	t0, s2, 2
	li	t1, 0
	addiw	t0, t0, -4
	add	t0, s1, t0
.main.bb4:
	beq	s2, zero, .main.bb6
.main.bb5:
	lw	t3, 0(t0)
	addiw	s2, s2, -1
	addw	t1, t1, t3
	addi	t0, t0, -4
	bne	s2, zero, .main.bb5	# rotated-loop-branch
.main.bb6:
	li	t0, 1739733589
	mul	t0, t1, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 5	# const-div.shift
	srliw	t2, t0, 31	# const-div.sign
	addw	t0, t0, t2	# const-div.round
	li	t2, 79
	mulw	t0, t0, t2	# const-rem.mul
	subw	a0, t1, t0	# const-rem.sub
	ld	s2, 416(sp)
	ld	s1, 408(sp)
	ld	ra, 400(sp)
	addi	sp, sp, 432
	ret
	.size main, .-main
