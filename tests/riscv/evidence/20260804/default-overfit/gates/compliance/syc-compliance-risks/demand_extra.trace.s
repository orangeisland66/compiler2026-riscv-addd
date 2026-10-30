
  .bss
  .globl storage
  .align 2
  .type  storage, @object
  .size  storage, 512
storage:
  .zero 512

  .text

  .globl run_demand_extra
  .type  run_demand_extra, @function
run_demand_extra:
.run_demand_extra.bb0:
	addi	sp, sp, -96
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	sd	s9, 80(sp)
	mv	s1, a0
	li	t0, 4
	addi	t1, sp, 0
	sw	t0, 0(t1)
	li	t0, 6
	sw	t0, 4(t1)
	addi	t0, sp, 0
	addi	t0, t0, 8
	li	t2, 3
	sw	t2, 8(t1)
	li	t1, 0
	li	t2, 5
.run_demand_extra.bb1:
	li	t3, 3
	bge	t1, t3, .run_demand_extra.bb3
.run_demand_extra.bb2:
	li	t3, 2
	mv	s2, t1	# phi-copy
	mv	s3, t0	# phi-copy
.run_demand_extra.bb4:
	blt	t3, zero, .run_demand_extra.bb6
.run_demand_extra.bb5:
	lw	s4, 0(s3)
	divw	s5, s1, s4
	bge	zero, s5, .run_demand_extra.bb8
.run_demand_extra.bb7:
	li	a2, 1
	mv	s6, s5	# phi-copy
	mv	s7, s4	# phi-copy
.run_demand_extra.bb9:
	beq	a2, zero, .run_demand_extra.bb8
.run_demand_extra.bb10:
	remw	s8, s2, s5
	divw	s9, s2, s5
	slt	a4, s8, s9
	subw	a5, s8, s6
	slt	a3, s9, s4
	xori	a4, a4, 1
	sltiu	a5, a5, 1
	slt	a6, s9, s7
	and	a3, a3, a4
	and	a5, a5, a6
	slt	a4, s8, s6
	or	a4, a4, a5
	and	a3, a3, a4
	beq	a3, zero, .run_demand_extra.bb13
.run_demand_extra.bb12:
	mulw	a3, s8, s4
	addw	s2, a3, s9
	mv	s6, s8	# phi-copy
	mv	s7, s9	# phi-copy
	j	.run_demand_extra.bb9
.run_demand_extra.bb14:
	j	.run_demand_extra.bb9
.run_demand_extra.bb13:
	li	a2, 0
	j	.run_demand_extra.bb9
.run_demand_extra.bb11:
.run_demand_extra.bb8:
	addiw	t3, t3, -1
	addi	s3, s3, -4
	j	.run_demand_extra.bb4
.run_demand_extra.bb20:
	j	.run_demand_extra.bb8
.run_demand_extra.bb6:
	bge	s2, s1, .run_demand_extra.bb16
.run_demand_extra.bb15:
	li	a2, -1840700269
	slliw	t3, s2, 1
	mul	a2, s2, a2	# const-div.fullmul
	addw	t3, t3, s2
	addiw	t3, t3, 1
	srai	a2, a2, 32	# const-div.mulhi
	addw	a2, a2, s2	# const-div.fixup
	sraiw	a2, a2, 2	# const-div.shift
	srliw	a3, a2, 31	# const-div.sign
	addw	a2, a2, a3	# const-div.round
	li	a3, 7
	mulw	a2, a2, a3	# const-rem.mul
	subw	a2, s2, a2	# const-rem.sub
	li	a3, 2
	bne	a2, a3, .run_demand_extra.bb17
.run_demand_extra.bb18:
	addiw	t3, s2, -11
	j	.run_demand_extra.bb17
.run_demand_extra.bb19:
.run_demand_extra.bb17:
	addiw	a2, t1, 3
	mulw	t3, a2, t3
	addw	t2, t2, t3
	subw	t2, t2, t1
	addiw	t1, t1, 1
	j	.run_demand_extra.bb1
.run_demand_extra.bb16:
	slliw	t3, s2, 2
	la	a2, storage
	add	t3, a2, t3
	lw	t3, 0(t3)
	j	.run_demand_extra.bb17
.run_demand_extra.bb21:
	j	.run_demand_extra.bb17
.run_demand_extra.bb3:
	mv	a0, t2
	ld	s9, 80(sp)
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	addi	sp, sp, 96
	ret
	.size run_demand_extra, .-run_demand_extra

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t5, 24
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	run_demand_extra
	mv	t0, a0
	mv	a0, t0
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
