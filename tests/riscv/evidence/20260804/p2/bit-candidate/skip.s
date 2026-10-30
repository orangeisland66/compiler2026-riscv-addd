
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 32
	li	t1, 1
	li	t2, 0
	li	t3, 11
	li	a2, 13
.main.bb1:
	beq	t0, zero, .main.bb3
.main.bb2:
	andi	a3, a2, 1
	li	a5, 0
	andi	a4, t3, 1
	and	a3, a3, a4
	addw	a4, t2, t1
	subw	a3, a5, a3
	xor	a4, t2, a4
	and	a3, a4, a3
	xor	t2, t2, a3
.main.bb4:
	slliw	t1, t1, 1
	srliw	a3, a2, 31	# const-div.pow2.bias-fold
	addiw	t0, t0, -1
	addw	a2, a2, a3
	sraiw	a2, a2, 1
	srliw	a3, t3, 31	# const-div.pow2.bias-fold
	addw	t3, t3, a3
	sraiw	t3, t3, 1
	bne	t0, zero, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, t2
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
