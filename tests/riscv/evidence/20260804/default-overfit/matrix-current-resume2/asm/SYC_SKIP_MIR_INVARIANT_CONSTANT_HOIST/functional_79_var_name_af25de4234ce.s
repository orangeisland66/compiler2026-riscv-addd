
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -112
	sd	ra, 88(sp)
	sd	s1, 96(sp)
	sd	s2, 104(sp)
	addi	t5, sp, 8
	sd	zero, 0(t5)	# memset-zero
	li	t0, 1
	sd	zero, 8(t5)	# memset-zero
	addi	t1, sp, 8
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	sd	zero, 64(t5)	# memset-zero
	sd	zero, 72(t5)	# memset-zero
	sw	t0, 0(t1)
	li	s1, 2
	sw	s1, 4(t1)
	li	t1, 0
.main.bb1:
	li	t0, 20
	bge	s1, t0, .main.bb3
.main.bb2:
	slliw	t0, s1, 2
	addi	t2, sp, 8
	add	s2, t2, t0
	lw	t0, 0(s2)
	lw	t3, -4(s2)
	addw	t0, t0, t3
	lw	t2, -8(s2)
	addw	t5, t0, t2
	sw	t5, 0(s2)
	addw	s2, t1, t5
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s1, s1, 1
	mv	t1, s2	# phi-copy
	j	.main.bb1
.main.bb3:
	mv	a0, t1
	ld	s2, 104(sp)
	ld	s1, 96(sp)
	ld	ra, 88(sp)
	addi	sp, sp, 112
	ret
	.size main, .-main
