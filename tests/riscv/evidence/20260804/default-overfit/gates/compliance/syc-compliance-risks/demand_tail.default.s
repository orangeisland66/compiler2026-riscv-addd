
  .bss
  .globl storage
  .align 2
  .type  storage, @object
  .size  storage, 512
storage:
  .zero 512

  .text

  .globl run_demand_tail
  .type  run_demand_tail, @function
run_demand_tail:
.run_demand_tail.bb0:
	addi	sp, sp, -32
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	mv	s1, a0
	li	t0, 0
.run_demand_tail.bb6:
	addiw	s2, t0, 8
	bge	t0, s1, .run_demand_tail.bb3
.run_demand_tail.bb7:
	bge	s2, s1, .run_demand_tail.bb8
.run_demand_tail.bb9:
.run_demand_tail.bb10:
	slliw	t1, t0, 2
	la	t2, storage
	add	t1, t2, t1
.run_demand_tail.bb1:
	bge	t0, s2, .run_demand_tail.bb6
.run_demand_tail.bb2:
	slliw	t2, t0, 1
	addw	t2, t2, t0
	addiw	t2, t2, 1
	sw	t2, 0(t1)
	li	t2, -1840700269
	mul	t2, t0, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	addw	t2, t2, t0	# const-div.fixup
	sraiw	t2, t2, 2	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	li	t3, 7
	mulw	t2, t2, t3	# const-rem.mul
	subw	t2, t0, t2	# const-rem.sub
	li	t3, 2
	beq	t2, t3, .run_demand_tail.bb4
.run_demand_tail.bb5:
	addiw	t0, t0, 1
	addi	t1, t1, 4
	j	.run_demand_tail.bb1
.run_demand_tail.bb4:
	addiw	t2, t0, -11
	sw	t2, 0(t1)
	j	.run_demand_tail.bb5
.run_demand_tail.bb11:
	j	.run_demand_tail.bb6
.run_demand_tail.bb8:
	mv	s2, s1	# phi-copy
	j	.run_demand_tail.bb10
.run_demand_tail.bb15:
	li	a3, 4
	slt	a4, t1, a2
	slt	a3, a2, a3
	xori	a4, a4, 1
	and	a3, a3, a4
	beqz	a3, .run_demand_tail.bb16
.run_demand_tail.bb17:
	lw	a3, 0(t2)
	addiw	a2, a2, 1
	sw	a3, 0(t3)
	addi	t2, t2, 4
	add	t3, t3, t0
	j	.run_demand_tail.bb15
.run_demand_tail.bb16:
	addiw	t1, t1, 1
	bge	t1, s2, .run_demand_tail.bb14	# rotated-loop-branch
.run_demand_tail.bb13:
	slliw	s3, t1, 2
	la	t3, storage
	slliw	t2, s3, 2
	li	a2, 0
	add	t2, t3, t2
	add	t3, t3, s3
	j	.run_demand_tail.bb15
.run_demand_tail.bb21:
	li	a3, 6
	slt	a4, t1, a2
	slt	a3, a2, a3
	xori	a4, a4, 1
	and	a3, a3, a4
	beqz	a3, .run_demand_tail.bb22
.run_demand_tail.bb23:
	lw	a3, 0(t2)
	addiw	a2, a2, 1
	sw	a3, 0(t3)
	addi	t2, t2, 4
	add	t3, t3, t0
	j	.run_demand_tail.bb21
.run_demand_tail.bb22:
	addiw	t1, t1, 1
	bge	t1, s2, .run_demand_tail.bb20	# rotated-loop-branch
.run_demand_tail.bb19:
	slliw	s3, t1, 2
	slliw	t2, t1, 1
	addw	t2, s3, t2
	la	t3, storage
	slliw	t2, t2, 2
	li	a2, 0
	add	t2, t3, t2
	add	t3, t3, s3
	j	.run_demand_tail.bb21
.run_demand_tail.bb27:
	li	a3, 3
	slt	a4, t1, a2
	slt	a3, a2, a3
	xori	a4, a4, 1
	and	a3, a3, a4
	beqz	a3, .run_demand_tail.bb28
.run_demand_tail.bb29:
	lw	a3, 0(t2)
	addiw	a2, a2, 1
	sw	a3, 0(t3)
	addi	t2, t2, 4
	add	t3, t3, t0
	j	.run_demand_tail.bb27
.run_demand_tail.bb28:
	addiw	t1, t1, 1
	bge	t1, s2, .run_demand_tail.bb26	# rotated-loop-branch
.run_demand_tail.bb25:
	slliw	t2, t1, 1
	la	s3, storage
	addw	t2, t2, t1
	slliw	t3, t1, 2
	slliw	t2, t2, 2
	add	t3, s3, t3
	add	t2, s3, t2
	li	a2, 0
	j	.run_demand_tail.bb27
.run_demand_tail.bb14:
	la	t0, storage
	sw	s1, 0(t0)
	li	t0, 715827883
	mul	t0, s1, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	srliw	t1, t0, 31	# const-div.sign
	addw	s2, t0, t1	# const-div.round
	slliw	t0, s2, 2
	li	t1, 0
.run_demand_tail.bb18:
	blt	t1, s2, .run_demand_tail.bb19
.run_demand_tail.bb20:
	la	t0, storage
	sw	s1, 0(t0)
	li	t0, 1431655766
	mul	t0, s1, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	srliw	t1, t0, 31	# const-div.sign
	addw	s2, t0, t1	# const-div.round
	slliw	t0, s2, 2
	li	t1, 0
.run_demand_tail.bb24:
	blt	t1, s2, .run_demand_tail.bb25
.run_demand_tail.bb26:
	la	t0, storage
	sw	s1, 0(t0)
	slliw	t1, s1, 1
	addw	t1, t1, s1
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
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 32
	ret
.run_demand_tail.bb3:
	sraiw	t0, s1, 31
	li	t1, 0
	andi	t0, t0, 3
	addw	t0, s1, t0
	sraiw	s2, t0, 2
	slliw	t0, s2, 2
.run_demand_tail.bb12:
	bge	t1, s2, .run_demand_tail.bb14
	j	.run_demand_tail.bb13
	.size run_demand_tail, .-run_demand_tail

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t5, 24
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	run_demand_tail
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
