
  .text

  .globl exgcd
  .type  exgcd, @function
exgcd:
.exgcd.bb0:
	addi	sp, sp, -80
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	bne	s2, zero, .exgcd.bb3
.exgcd.bb2:
	li	t0, 1
	sw	t0, 0(s3)
	li	t0, 0
	sw	t0, 0(s4)
	mv	t0, s1	# phi-copy
.exgcd.bb1:
	mv	a0, t0
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 80
	ret
.exgcd.bb3:
	remw	a1, s1, s2
	mv	a0, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	mv	a3, s4	# call-arg-copy
	call	exgcd
	mv	t0, a0
	lw	t1, 0(s3)
	lw	t2, 0(s4)
	sw	t2, 0(s3)
	divw	t3, s1, s2
	mulw	t2, t3, t2
	subw	t1, t1, t2
	sw	t1, 0(s4)
	j	.exgcd.bb1
	.size exgcd, .-exgcd

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	addi	t5, sp, 32
	sw	zero, 0(t5)	# memset-zero
	li	t0, 1
	addi	s1, sp, 32
	sw	t0, 0(s1)
	addi	t5, sp, 36
	sw	zero, 0(t5)	# memset-zero
	addi	t1, sp, 36
	sw	t0, 0(t1)
	li	t5, 7
	sd	t5, 0(sp)
	li	t5, 15
	sd	t5, 8(sp)
	addi	t5, sp, 32
	sd	t5, 16(sp)
	addi	t5, sp, 36
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	exgcd
	lw	t0, 0(s1)
	li	t1, -2004318071
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, t0	# const-div.fixup
	sraiw	t1, t1, 3	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 15
	mulw	t1, t1, t2	# const-rem.mul
	subw	t0, t0, t1	# const-rem.sub
	addiw	t0, t0, 15
	li	t1, -2004318071
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, t0	# const-div.fixup
	sraiw	t1, t1, 3	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 15
	mulw	t1, t1, t2	# const-rem.mul
	subw	t5, t0, t1	# const-rem.sub
	sw	t5, 0(s1)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	a0, 0
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main
