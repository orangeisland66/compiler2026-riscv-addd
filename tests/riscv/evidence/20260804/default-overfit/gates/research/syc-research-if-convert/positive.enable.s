
  .bss
  .globl left
  .align 2
  .type  left, @object
  .size  left, 4096
left:
  .zero 4096

  .bss
  .globl right
  .align 2
  .type  right, @object
  .size  right, 4096
right:
  .zero 4096

  .bss
  .globl output
  .align 2
  .type  output, @object
  .size  output, 4096
output:
  .zero 4096

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -112
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	sd	s9, 80(sp)
	sd	s10, 88(sp)
	sd	s11, 96(sp)
	li	t0, 0
.main.bb1:
	li	t1, 32
	bge	t0, t1, .main.bb3
.main.bb2:
	slliw	s1, t0, 7
	li	t1, 0
	li	t2, 32
	la	s2, left	# loop.addr.hoisted
	la	s3, right	# loop.addr.hoisted
.main.bb16:
	addiw	s4, t1, 8
	bge	t1, t2, .main.bb6
.main.bb17:
	li	t3, 32
	bge	s4, t3, .main.bb18
.main.bb19:
	mv	t3, s4	# phi-copy
	j	.main.bb4
.main.bb20:
.main.bb4:
	bge	t1, t3, .main.bb16
.main.bb22:
	addiw	s4, t1, 3
	blt	s4, t3, .main.bb23
.main.bb5:
	slliw	a3, t1, 2
	addw	a2, t0, t1
	addw	s4, s1, a3
	addiw	a2, a2, 1
	add	a3, s2, s4
	sw	a2, 0(a3)
	subw	a2, t0, t1
	add	a3, s3, s4
	sw	a2, 0(a3)
	addiw	t1, t1, 1
	j	.main.bb4
.main.bb23:
	slliw	a3, t1, 2
	addw	a2, t0, t1
	addw	s5, s1, a3
	addiw	a2, a2, 1
	add	s6, s2, s5
	addiw	s7, t1, 1
	sw	a2, 0(s6)
	add	s5, s3, s5
	subw	a2, t0, t1
	sw	a2, 0(s5)
	addw	a2, t0, s7
	addiw	a2, a2, 1
	sw	a2, 4(s6)
	subw	a2, t0, s7
	sw	a2, 4(s5)
	addiw	s7, t1, 2
	addw	a2, t0, s7
	addiw	t1, t1, 4
	addiw	a2, a2, 1
	sw	a2, 8(s6)
	subw	a2, t0, s7
	sw	a2, 8(s5)
	addw	a2, t0, s4
	addiw	a2, a2, 1
	sw	a2, 12(s6)
	subw	a2, t0, s4
	sw	a2, 12(s5)
	j	.main.bb4
.main.bb21:
	j	.main.bb16
.main.bb13:
	li	a3, 32
	bge	a2, a3, .main.bb15
.main.bb32:
	addiw	a3, a2, 3
	li	a4, 32
	blt	a3, a4, .main.bb33
.main.bb14:
	slliw	a3, a2, 2
	li	a4, 0
	addw	a3, s4, a3
	add	a3, s1, a3
	lw	s7, 0(a3)
	andi	a3, s7, 1
	sltiu	s8, a3, 1
	slliw	a3, a2, 7
	mv	a5, s8	# sel.mask
	addw	a3, a3, s6
	subw	a5, zero, a5	# sel.negmask
	add	s9, s2, a3
	xori	a6, a5, -1	# sel.invmask
	lw	a3, 0(s9)
	and	a4, a4, a6	# sel.false
	addiw	a2, a2, 1
	mulw	a3, s7, a3
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	mv	a5, s8	# sel.mask
	addw	t3, t3, a3
	subw	a5, zero, a5	# sel.negmask
	li	a4, 0
	lw	a3, 4(s9)
	xori	a6, a5, -1	# sel.invmask
	and	a4, a4, a6	# sel.false
	mulw	a3, s7, a3
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb13
.main.bb33:
	slliw	a3, a2, 2
	li	a4, 0
	addw	a3, s4, a3
	add	s7, s1, a3
	lw	s8, 0(s7)
	andi	a3, s8, 1
	sltiu	s9, a3, 1
	slliw	a3, a2, 7
	mv	a5, s9	# sel.mask
	addw	a3, a3, s6
	subw	a5, zero, a5	# sel.negmask
	add	s10, s2, a3
	xori	a6, a5, -1	# sel.invmask
	lw	a3, 0(s10)
	and	a4, a4, a6	# sel.false
	mv	a6, s9	# sel.mask
	addiw	a2, a2, 4
	subw	a6, zero, a6	# sel.negmask
	mulw	a3, s8, a3
	xori	a7, a6, -1	# sel.invmask
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	li	a5, 0
	lw	a4, 4(s10)
	and	a5, a5, a7	# sel.false
	addw	t3, t3, a3
	mulw	a4, s8, a4
	lw	s8, 4(s7)
	and	a4, a4, a6	# sel.true
	or	a4, a4, a5
	li	a6, 0
	addw	t2, t2, a4
	andi	a5, s8, 1
	sltiu	s9, a5, 1
	lw	a5, 128(s10)
	mv	a7, s9	# sel.mask
	subw	a7, zero, a7	# sel.negmask
	xori	t4, a7, -1	# sel.invmask
	mulw	a5, s8, a5
	and	a6, a6, t4	# sel.false
	lw	a3, 388(s10)
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	mv	a7, s9	# sel.mask
	addw	t3, t3, a5
	subw	a7, zero, a7	# sel.negmask
	lw	a5, 132(s10)
	xori	t4, a7, -1	# sel.invmask
	li	a6, 0
	and	a6, a6, t4	# sel.false
	mulw	a5, s8, a5
	lw	s8, 8(s7)
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	addw	t2, t2, a5
	li	a6, 0
	lw	s7, 12(s7)
	andi	a5, s8, 1
	sltiu	s9, a5, 1
	lw	a5, 256(s10)
	mv	a7, s9	# sel.mask
	subw	a7, zero, a7	# sel.negmask
	andi	a4, s7, 1
	xori	t4, a7, -1	# sel.invmask
	mulw	a3, s7, a3
	mulw	a5, s8, a5
	and	a6, a6, t4	# sel.false
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	mv	a7, s9	# sel.mask
	addw	t3, t3, a5
	subw	a7, zero, a7	# sel.negmask
	lw	a5, 260(s10)
	xori	t4, a7, -1	# sel.invmask
	li	a6, 0
	and	a6, a6, t4	# sel.false
	mulw	a5, s8, a5
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	addw	t2, t2, a5
	li	a5, 0
	sltiu	s8, a4, 1
	lw	a4, 384(s10)
	mv	a6, s8	# sel.mask
	subw	a6, zero, a6	# sel.negmask
	xori	a7, a6, -1	# sel.invmask
	mulw	a4, s7, a4
	and	a5, a5, a7	# sel.false
	and	a4, a4, a6	# sel.true
	or	a4, a4, a5
	mv	a5, s8	# sel.mask
	addw	t3, t3, a4
	subw	a5, zero, a5	# sel.negmask
	li	a4, 0
	xori	a6, a5, -1	# sel.invmask
	and	a3, a3, a5	# sel.true
	and	a4, a4, a6	# sel.false
	or	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb13
.main.bb15:
	addw	a2, s4, s6
	add	s6, s3, a2
	addiw	a2, s6, 4
	sw	t2, 0(a2)
	sw	t3, 0(s6)
	mv	t2, s5	# phi-copy
.main.bb10:
	li	t3, 32
	bge	t2, t3, .main.bb12
.main.bb24:
	addiw	s5, t2, 2
	addiw	s6, t2, 4
	slt	t3, t2, s5
	slt	a2, s5, s6
	li	a3, 32
	and	t3, t3, a2
	slt	a3, s5, a3
	and	t3, t3, a3
	bnez	t3, .main.bb25
.main.bb11:
	slliw	s6, t2, 2
	li	t3, 0
	li	t2, 0
	li	a2, 0
	j	.main.bb13
.main.bb26:
	li	a3, 32
	bge	a2, a3, .main.bb28
.main.bb34:
	addiw	a3, a2, 3
	li	a4, 32
	blt	a3, a4, .main.bb35
.main.bb27:
	slliw	a3, a2, 2
	li	a4, 0
	addw	a3, s4, a3
	add	a3, s1, a3
	lw	s8, 0(a3)
	andi	a3, s8, 1
	sltiu	s9, a3, 1
	slliw	a3, a2, 7
	mv	a5, s9	# sel.mask
	addw	a3, a3, s7
	subw	a5, zero, a5	# sel.negmask
	add	s10, s2, a3
	xori	a6, a5, -1	# sel.invmask
	lw	a3, 0(s10)
	and	a4, a4, a6	# sel.false
	addiw	a2, a2, 1
	mulw	a3, s8, a3
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	mv	a5, s9	# sel.mask
	addw	t3, t3, a3
	subw	a5, zero, a5	# sel.negmask
	li	a4, 0
	lw	a3, 4(s10)
	xori	a6, a5, -1	# sel.invmask
	and	a4, a4, a6	# sel.false
	mulw	a3, s8, a3
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb26
.main.bb35:
	slliw	a3, a2, 2
	li	a4, 0
	addw	a3, s4, a3
	add	s8, s1, a3
	lw	s9, 0(s8)
	andi	a3, s9, 1
	sltiu	s10, a3, 1
	slliw	a3, a2, 7
	mv	a5, s10	# sel.mask
	addw	a3, a3, s7
	subw	a5, zero, a5	# sel.negmask
	add	s11, s2, a3
	xori	a6, a5, -1	# sel.invmask
	lw	a3, 0(s11)
	and	a4, a4, a6	# sel.false
	mv	a6, s10	# sel.mask
	addiw	a2, a2, 4
	subw	a6, zero, a6	# sel.negmask
	mulw	a3, s9, a3
	xori	a7, a6, -1	# sel.invmask
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	li	a5, 0
	lw	a4, 4(s11)
	and	a5, a5, a7	# sel.false
	addw	t3, t3, a3
	mulw	a4, s9, a4
	lw	s9, 4(s8)
	and	a4, a4, a6	# sel.true
	or	a4, a4, a5
	li	a6, 0
	addw	t2, t2, a4
	andi	a5, s9, 1
	sltiu	s10, a5, 1
	lw	a5, 128(s11)
	mv	a7, s10	# sel.mask
	subw	a7, zero, a7	# sel.negmask
	xori	t4, a7, -1	# sel.invmask
	mulw	a5, s9, a5
	and	a6, a6, t4	# sel.false
	lw	a3, 388(s11)
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	mv	a7, s10	# sel.mask
	addw	t3, t3, a5
	subw	a7, zero, a7	# sel.negmask
	lw	a5, 132(s11)
	xori	t4, a7, -1	# sel.invmask
	li	a6, 0
	and	a6, a6, t4	# sel.false
	mulw	a5, s9, a5
	lw	s9, 8(s8)
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	addw	t2, t2, a5
	li	a6, 0
	lw	s8, 12(s8)
	andi	a5, s9, 1
	sltiu	s10, a5, 1
	lw	a5, 256(s11)
	mv	a7, s10	# sel.mask
	subw	a7, zero, a7	# sel.negmask
	andi	a4, s8, 1
	xori	t4, a7, -1	# sel.invmask
	mulw	a3, s8, a3
	mulw	a5, s9, a5
	and	a6, a6, t4	# sel.false
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	mv	a7, s10	# sel.mask
	addw	t3, t3, a5
	subw	a7, zero, a7	# sel.negmask
	lw	a5, 260(s11)
	xori	t4, a7, -1	# sel.invmask
	li	a6, 0
	and	a6, a6, t4	# sel.false
	mulw	a5, s9, a5
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	addw	t2, t2, a5
	li	a5, 0
	sltiu	s9, a4, 1
	lw	a4, 384(s11)
	mv	a6, s9	# sel.mask
	subw	a6, zero, a6	# sel.negmask
	xori	a7, a6, -1	# sel.invmask
	mulw	a4, s8, a4
	and	a5, a5, a7	# sel.false
	and	a4, a4, a6	# sel.true
	or	a4, a4, a5
	mv	a5, s9	# sel.mask
	addw	t3, t3, a4
	subw	a5, zero, a5	# sel.negmask
	li	a4, 0
	xori	a6, a5, -1	# sel.invmask
	and	a3, a3, a5	# sel.true
	and	a4, a4, a6	# sel.false
	or	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb26
.main.bb28:
	addw	a2, s4, s7
	slliw	s5, s5, 2
	add	s7, s3, a2
	addiw	a2, s7, 4
	sw	t2, 0(a2)
	sw	t3, 0(s7)
	li	t2, 0
	li	t3, 0
	li	a2, 0
.main.bb29:
	li	a3, 32
	bge	a2, a3, .main.bb31
.main.bb36:
	addiw	a3, a2, 3
	li	a4, 32
	blt	a3, a4, .main.bb37
.main.bb30:
	slliw	a3, a2, 2
	li	a4, 0
	addw	a3, s4, a3
	add	a3, s1, a3
	lw	s7, 0(a3)
	andi	a3, s7, 1
	sltiu	s8, a3, 1
	slliw	a3, a2, 7
	mv	a5, s8	# sel.mask
	addw	a3, a3, s5
	subw	a5, zero, a5	# sel.negmask
	add	s9, s2, a3
	xori	a6, a5, -1	# sel.invmask
	lw	a3, 0(s9)
	and	a4, a4, a6	# sel.false
	addiw	a2, a2, 1
	mulw	a3, s7, a3
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	mv	a5, s8	# sel.mask
	addw	t3, t3, a3
	subw	a5, zero, a5	# sel.negmask
	li	a4, 0
	lw	a3, 4(s9)
	xori	a6, a5, -1	# sel.invmask
	and	a4, a4, a6	# sel.false
	mulw	a3, s7, a3
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb29
.main.bb37:
	slliw	a3, a2, 2
	li	a4, 0
	addw	a3, s4, a3
	add	s7, s1, a3
	lw	s8, 0(s7)
	andi	a3, s8, 1
	sltiu	s9, a3, 1
	slliw	a3, a2, 7
	mv	a5, s9	# sel.mask
	addw	a3, a3, s5
	subw	a5, zero, a5	# sel.negmask
	add	s10, s2, a3
	xori	a6, a5, -1	# sel.invmask
	lw	a3, 0(s10)
	and	a4, a4, a6	# sel.false
	mv	a6, s9	# sel.mask
	addiw	a2, a2, 4
	subw	a6, zero, a6	# sel.negmask
	mulw	a3, s8, a3
	xori	a7, a6, -1	# sel.invmask
	and	a3, a3, a5	# sel.true
	or	a3, a3, a4
	li	a5, 0
	lw	a4, 4(s10)
	and	a5, a5, a7	# sel.false
	addw	t3, t3, a3
	mulw	a4, s8, a4
	lw	s8, 4(s7)
	and	a4, a4, a6	# sel.true
	or	a4, a4, a5
	li	a6, 0
	addw	t2, t2, a4
	andi	a5, s8, 1
	sltiu	s9, a5, 1
	lw	a5, 128(s10)
	mv	a7, s9	# sel.mask
	subw	a7, zero, a7	# sel.negmask
	xori	t4, a7, -1	# sel.invmask
	mulw	a5, s8, a5
	and	a6, a6, t4	# sel.false
	lw	a3, 388(s10)
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	mv	a7, s9	# sel.mask
	addw	t3, t3, a5
	subw	a7, zero, a7	# sel.negmask
	lw	a5, 132(s10)
	xori	t4, a7, -1	# sel.invmask
	li	a6, 0
	and	a6, a6, t4	# sel.false
	mulw	a5, s8, a5
	lw	s8, 8(s7)
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	addw	t2, t2, a5
	li	a6, 0
	lw	s7, 12(s7)
	andi	a5, s8, 1
	sltiu	s9, a5, 1
	lw	a5, 256(s10)
	mv	a7, s9	# sel.mask
	subw	a7, zero, a7	# sel.negmask
	andi	a4, s7, 1
	xori	t4, a7, -1	# sel.invmask
	mulw	a3, s7, a3
	mulw	a5, s8, a5
	and	a6, a6, t4	# sel.false
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	mv	a7, s9	# sel.mask
	addw	t3, t3, a5
	subw	a7, zero, a7	# sel.negmask
	lw	a5, 260(s10)
	xori	t4, a7, -1	# sel.invmask
	li	a6, 0
	and	a6, a6, t4	# sel.false
	mulw	a5, s8, a5
	and	a5, a5, a7	# sel.true
	or	a5, a5, a6
	addw	t2, t2, a5
	li	a5, 0
	sltiu	s8, a4, 1
	lw	a4, 384(s10)
	mv	a6, s8	# sel.mask
	subw	a6, zero, a6	# sel.negmask
	xori	a7, a6, -1	# sel.invmask
	mulw	a4, s7, a4
	and	a5, a5, a7	# sel.false
	and	a4, a4, a6	# sel.true
	or	a4, a4, a5
	mv	a5, s8	# sel.mask
	addw	t3, t3, a4
	subw	a5, zero, a5	# sel.negmask
	li	a4, 0
	xori	a6, a5, -1	# sel.invmask
	and	a3, a3, a5	# sel.true
	and	a4, a4, a6	# sel.false
	or	a3, a3, a4
	addw	t2, t2, a3
	j	.main.bb29
.main.bb31:
	addw	a2, s4, s5
	add	s5, s3, a2
	addiw	a2, s5, 4
	sw	t2, 0(a2)
	sw	t3, 0(s5)
	mv	t2, s6	# phi-copy
	j	.main.bb10
.main.bb25:
	slliw	s7, t2, 2
	li	t3, 0
	li	t2, 0
	li	a2, 0
	j	.main.bb26
.main.bb18:
	li	t3, 32
	j	.main.bb4
.main.bb12:
	addiw	t0, t0, 1
	bge	t0, t1, .main.bb9	# rotated-loop-branch
.main.bb8:
	slliw	s4, t0, 7
	li	t2, 0
	j	.main.bb10
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	li	t0, 0
	li	t1, 32
	la	s1, left	# loop.addr.hoisted
	la	s2, right	# loop.addr.hoisted
	la	s3, output	# loop.addr.hoisted
.main.bb7:
	blt	t0, t1, .main.bb8
.main.bb9:
	la	t0, output
	lw	a0, 404(t0)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s11, 96(sp)
	ld	s10, 88(sp)
	ld	s9, 80(sp)
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 112
	ret
	.size main, .-main
