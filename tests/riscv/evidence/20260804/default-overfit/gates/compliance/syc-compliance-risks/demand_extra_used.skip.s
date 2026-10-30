
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
	addi	sp, sp, -64
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	mv	s1, a0
	addi	t0, sp, 0
	li	t1, 4
	addi	t2, sp, 0
	sw	t1, 0(t2)
	li	t1, 6
	sw	t1, 4(t2)
	li	t1, 3
	sw	t1, 8(t2)
	li	t1, 0
.run_demand_extra.bb15:
	addiw	s2, t1, 8
	bge	t1, s1, .run_demand_extra.bb3
.run_demand_extra.bb16:
	bge	s2, s1, .run_demand_extra.bb17
.run_demand_extra.bb18:
.run_demand_extra.bb19:
	slliw	t2, t1, 2
	la	t3, storage
	add	t2, t3, t2
.run_demand_extra.bb1:
	bge	t1, s2, .run_demand_extra.bb15
.run_demand_extra.bb2:
	slliw	t3, t1, 1
	addw	t3, t3, t1
	addiw	t3, t3, 1
	sw	t3, 0(t2)
	li	t3, -1840700269
	mul	t3, t1, t3	# const-div.fullmul
	srai	t3, t3, 32	# const-div.mulhi
	addw	t3, t3, t1	# const-div.fixup
	sraiw	t3, t3, 2	# const-div.shift
	srliw	a2, t3, 31	# const-div.sign
	addw	t3, t3, a2	# const-div.round
	li	a2, 7
	mulw	t3, t3, a2	# const-rem.mul
	subw	t3, t1, t3	# const-rem.sub
	li	a2, 2
	beq	t3, a2, .run_demand_extra.bb4
.run_demand_extra.bb5:
	addiw	t1, t1, 1
	addi	t2, t2, 4
	j	.run_demand_extra.bb1
.run_demand_extra.bb4:
	addiw	t3, t1, -11
	sw	t3, 0(t2)
	j	.run_demand_extra.bb5
.run_demand_extra.bb12:
	slt	a5, t2, a3
	slt	a4, a3, s2
	xori	a5, a5, 1
	and	a4, a4, a5
	beqz	a4, .run_demand_extra.bb13
.run_demand_extra.bb14:
	lw	a4, 0(t3)
	addiw	a3, a3, 1
	sw	a4, 0(a2)
	addw	a4, a4, s3
	add	a2, a2, t1
	sw	a4, 0(t3)
	addi	t3, t3, 4
	j	.run_demand_extra.bb12
.run_demand_extra.bb13:
	addiw	t2, t2, 1
.run_demand_extra.bb9:
	bge	t2, s4, .run_demand_extra.bb11
.run_demand_extra.bb10:
	mulw	t3, t2, s2
	la	s5, storage
	slliw	a2, t2, 2
	li	a3, 0
	add	a2, s5, a2
	slliw	t3, t3, 2
	add	t3, s5, t3
	j	.run_demand_extra.bb12
.run_demand_extra.bb11:
	addi	t0, t0, 4
	mv	t1, s3	# phi-copy
.run_demand_extra.bb6:
	li	t2, 3
	bge	t1, t2, .run_demand_extra.bb8
.run_demand_extra.bb7:
	lw	s2, 0(t0)
	addiw	s3, t1, 1
	li	t2, 0
	divw	s4, s1, s2
	slliw	t1, s4, 2
	j	.run_demand_extra.bb9
.run_demand_extra.bb20:
	j	.run_demand_extra.bb15
.run_demand_extra.bb17:
	mv	s2, s1	# phi-copy
	j	.run_demand_extra.bb19
.run_demand_extra.bb3:
	li	t1, 0
	j	.run_demand_extra.bb6
.run_demand_extra.bb8:
	la	t0, storage
	lw	t1, 0(t0)
	slliw	t2, t1, 1
	addw	t1, t2, t1
	addiw	t1, t1, 5
	lw	t2, 4(t0)
	slliw	t2, t2, 2
	addw	t1, t1, t2
	addiw	t1, t1, -1
	lw	t0, 8(t0)
	slliw	t2, t0, 2
	addw	t0, t2, t0
	addw	t0, t1, t0
	addiw	a0, t0, -2
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	addi	sp, sp, 64
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
