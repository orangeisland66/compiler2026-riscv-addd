
  .section .rodata
  .globl MODULUS
  .align 2
  .type  MODULUS, @object
  .size  MODULUS, 4
MODULUS:
  .word 1009

  .text

  .globl multiply_mod_candidate
  .type  multiply_mod_candidate, @function
multiply_mod_candidate:
.multiply_mod_candidate.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	mv	s1, a0
	mv	s2, a1
	beq	s2, zero, .multiply_mod_candidate.bb2
.multiply_mod_candidate.bb3:
	li	t0, 1
	beq	s2, t0, .multiply_mod_candidate.bb4
.multiply_mod_candidate.bb5:
	srliw	t0, s2, 31	# const-div.pow2.bias-fold
	addw	t0, s2, t0
	sraiw	a1, t0, 1
	mv	a0, s1	# call-arg-copy
	call	multiply_mod_candidate
	mv	t0, a0
	addw	t0, t0, t0
	li	t1, -2115558717
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, t0	# const-div.fixup
	sraiw	t1, t1, 9	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 1009
	mulw	t1, t1, t2	# const-rem.mul
	subw	t0, t0, t1	# const-rem.sub
	andi	t1, s2, 1
	bne	t1, zero, .multiply_mod_candidate.bb6
.multiply_mod_candidate.bb7:
.multiply_mod_candidate.bb1:
	mv	a0, t0
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.multiply_mod_candidate.bb4:
	li	t0, -2115558717
	mul	t0, s1, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	addw	t0, t0, s1	# const-div.fixup
	sraiw	t0, t0, 9	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 1009
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s1, t0	# const-rem.sub
	j	.multiply_mod_candidate.bb1
.multiply_mod_candidate.bb6:
	addw	t0, t0, s1
	li	t1, -2115558717
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, t0	# const-div.fixup
	sraiw	t1, t1, 9	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 1009
	mulw	t1, t1, t2	# const-rem.mul
	subw	t0, t0, t1	# const-rem.sub
	j	.multiply_mod_candidate.bb1
.multiply_mod_candidate.bb2:
	li	t0, 0
	j	.multiply_mod_candidate.bb1
	.size multiply_mod_candidate, .-multiply_mod_candidate

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	li	t5, 777
	sd	t5, 0(sp)
	li	t5, 321
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	multiply_mod_candidate
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
