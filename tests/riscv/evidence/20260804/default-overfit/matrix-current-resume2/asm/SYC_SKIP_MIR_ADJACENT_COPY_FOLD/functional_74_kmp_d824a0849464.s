
  .text

  .globl KMP
  .type  KMP, @function
KMP:
.KMP.bb0:
	li	t6, -16416
	add	sp, sp, t6
	li	t6, 16384
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 16392
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 16400
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 16408
	add	t6, sp, t6
	sd	s4, 0(t6)
	mv	s1, a0
	mv	t0, a1
	li	t1, -1
	addi	s2, sp, 0
	sw	t1, 0(s2)
	li	t1, 0
	li	t2, -1
.KMP.bb12:
	slliw	t3, t1, 2
	add	t3, s1, t3
	lw	s3, 0(t3)
	beq	s3, zero, .KMP.bb14
.KMP.bb13:
	slliw	s4, t2, 2
	li	t3, -1
	add	a2, s1, s4
	subw	t3, t2, t3
	lw	a2, 0(a2)
	sltiu	t3, t3, 1
	subw	a2, s3, a2
	sltiu	a2, a2, 1
	or	t3, t3, a2
	beq	t3, zero, .KMP.bb16
.KMP.bb15:
	addiw	s4, t1, 1
	addiw	t2, t2, 1
	slliw	t1, s4, 2
	add	t1, s2, t1
	sw	t2, 0(t1)
	mv	t1, s4	# phi-copy
	j	.KMP.bb12
.KMP.bb17:
	j	.KMP.bb12
.KMP.bb16:
	add	t2, s2, s4
	lw	t2, 0(t2)
	j	.KMP.bb12
.KMP.bb14:
	li	t1, 0
	li	t2, 0
.KMP.bb2:
	slliw	t3, t1, 2
	add	t3, t0, t3
	lw	s3, 0(t3)
	beq	s3, zero, .KMP.bb4
.KMP.bb3:
	slliw	s4, t2, 2
	add	t3, s1, s4
	lw	t3, 0(t3)
	beq	t3, s3, .KMP.bb5
.KMP.bb6:
	add	t2, s2, s4
	lw	s3, 0(t2)
	li	t2, -1
	bne	s3, t2, .KMP.bb18
.KMP.bb10:
	addiw	t1, t1, 1
	li	t2, 0
	j	.KMP.bb2
.KMP.bb11:
	j	.KMP.bb2
.KMP.bb7:
	j	.KMP.bb2
.KMP.bb5:
	addiw	s3, t2, 1
	addiw	s4, t1, 1
	slliw	t1, s3, 2
	add	t1, s1, t1
	lw	t1, 0(t1)
	beq	t1, zero, .KMP.bb8
.KMP.bb9:
	mv	t2, s3	# phi-copy
	mv	t1, s4	# phi-copy
	j	.KMP.bb2
.KMP.bb18:
	mv	t2, s3	# phi-copy
	j	.KMP.bb2
.KMP.bb4:
	li	t0, -1
.KMP.bb1:
	mv	a0, t0
	li	t6, 16408
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 16400
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 16392
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 16384
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 16416
	add	sp, sp, t6
	ret
.KMP.bb8:
	mv	t0, s4	# phi-copy
	j	.KMP.bb1
	.size KMP, .-KMP

  .globl read_str
  .type  read_str, @function
read_str:
.read_str.bb0:
	addi	sp, sp, -48
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	sd	s3, 24(sp)
	sd	s4, 32(sp)
	sd	s5, 40(sp)
	mv	s1, a0
	li	s3, 0
	li	s2, 10
.read_str.bb1:
	call	getch
	mv	s4, a0
	slliw	t0, s3, 2
	add	s5, s1, t0
	sw	s4, 0(s5)
	beq	s4, s2, .read_str.bb2
.read_str.bb3:
	addiw	t0, s3, 1
	mv	s3, t0	# phi-copy
	j	.read_str.bb1
.read_str.bb2:
	li	t0, 0
	sw	t0, 0(s5)
	mv	a0, s3
	ld	s5, 40(sp)
	ld	s4, 32(sp)
	ld	s3, 24(sp)
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 48
	ret
	.size read_str, .-read_str

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t6, -32800
	add	sp, sp, t6
	li	t6, 32784
	add	t6, sp, t6
	sd	ra, 0(t6)
	addi	t5, sp, 16
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	read_str
	li	t5, 16400
	add	t5, sp, t5
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	read_str
	addi	t5, sp, 16
	sd	t5, 0(sp)
	li	t5, 16400
	add	t5, sp, t5
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	KMP
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	li	t6, 32784
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 32800
	add	sp, sp, t6
	ret
	.size main, .-main
