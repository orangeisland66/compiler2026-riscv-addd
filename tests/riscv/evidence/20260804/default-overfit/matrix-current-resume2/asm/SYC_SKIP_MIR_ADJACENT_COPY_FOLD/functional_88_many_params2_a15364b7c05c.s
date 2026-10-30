
  .text

  .globl func
  .type  func, @function
func:
.func.bb0:
	addi	sp, sp, -112
	sd	s0, 8(sp)
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	sd	s8, 80(sp)
	sd	s9, 88(sp)
	sd	s10, 96(sp)
	sd	s11, 104(sp)
	addi	s0, sp, 112
	mv	t0, a0
	mv	s1, a1
	mv	s2, a2
	mv	s3, a3
	mv	s4, a4
	mv	s5, a5
	mv	s6, a6
	mv	s7, a7
	lw	s8, 0(s0)
	li	t1, 236
	mulw	s9, t0, t1
	li	s11, 0
	li	s10, 10
.func.bb1:
	bge	s11, s10, .func.bb3
.func.bb2:
	slliw	t0, s11, 2
	addw	t0, s9, t0
	add	t0, s1, t0
	lw	a0, 0(t0)
	call	putint
	addiw	t0, s11, 1
	mv	s11, t0	# phi-copy
	blt	s11, s10, .func.bb2	# rotated-loop-branch
.func.bb3:
	li	a0, 10
	call	putch
	slliw	t0, s2, 2
	add	t0, s3, t0
	lw	a0, 0(t0)
	call	putint
	li	a0, 10
	call	putch
	slliw	t0, s8, 2
	add	t0, s6, t0
	li	t1, 10
.func.bb4:
	bge	s8, t1, .func.bb6
.func.bb5:
	li	t2, 128875
	mulw	s1, s7, t2
	li	t2, -1932965947
	mul	t2, s1, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	addw	t2, t2, s1	# const-div.fixup
	sraiw	t2, t2, 11	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	li	t3, 3724
	mulw	t2, t2, t3	# const-rem.mul
	subw	t2, s1, t2	# const-rem.sub
	addiw	s7, s7, 7
	sw	t2, 0(t0)
	addiw	s8, s8, 1
	addi	t0, t0, 4
	blt	s8, t1, .func.bb5	# rotated-loop-branch
.func.bb6:
	addw	a0, s4, s5
	ld	s11, 104(sp)
	ld	s10, 96(sp)
	ld	s9, 88(sp)
	ld	s8, 80(sp)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	ld	s0, 8(sp)
	addi	sp, sp, 112
	ret
	.size func, .-func

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t6, -28960
	add	sp, sp, t6
	li	t6, 28936
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 28944
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 28952
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t5, 12584
	add	t5, sp, t5
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	li	t5, 16348
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	memset
	addi	t5, sp, 72
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	li	t5, 12508
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	memset
	li	t0, 6
	li	t1, 12584
	add	t1, sp, t1
	li	t2, 4560
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 4568
	li	t0, 7
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 4572
	li	t0, 4
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 4584
	li	t0, 9
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 4600
	li	t0, 11
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t0, 1
	addi	s1, sp, 72
	sw	t0, 1420(s1)
	li	t0, 2
	sw	t0, 1424(s1)
	li	t0, 3
	sw	t0, 1428(s1)
	li	t0, 9
	sw	t0, 1452(s1)
	li	t0, 4560
	add	t0, t1, t0
	lw	t0, 0(t0)
	li	t2, 4568
	add	t1, t1, t2
	lw	t1, 0(t1)
	lw	t2, 1428(s1)
	lw	t3, 1416(s1)
	li	a2, 8040
	add	a2, s1, a2
	lw	a2, 0(a2)
	li	a3, 12108
	add	a3, s1, a3
	lw	a3, 0(a3)
	mv	t5, t0
	sd	t5, 0(sp)
	addi	t5, sp, 72
	sd	t5, 8(sp)
	mv	t5, t1
	sd	t5, 16(sp)
	li	t5, 12584
	add	t5, sp, t5
	li	t0, 4556
	add	t5, t5, t0
	sd	t5, 24(sp)
	mv	t5, t2
	sd	t5, 32(sp)
	mv	t5, t3
	sd	t5, 40(sp)
	addi	t5, sp, 72
	addi	t5, t5, 1416
	sd	t5, 48(sp)
	mv	t5, a2
	sd	t5, 56(sp)
	mv	t5, a3
	sd	t5, 64(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	ld	a5, 40(sp)
	ld	a6, 48(sp)
	ld	a7, 56(sp)
	ld	t5, 64(sp)
	sd	t5, 0(sp)
	call	func
	mv	t0, a0
	slliw	t1, t0, 1
	addw	t0, t1, t0
	mv	s2, t0	# phi-copy
.main.bb1:
	blt	s2, zero, .main.bb3
.main.bb2:
	slliw	t0, s2, 2
	addiw	t0, t0, 1416
	add	t0, s1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	t0, s2, -1
	mv	s2, t0	# phi-copy
	bge	s2, zero, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 10
	call	putch
	li	a0, 0
	li	t6, 28952
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 28944
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 28936
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 28960
	add	sp, sp, t6
	ret
	.size main, .-main
