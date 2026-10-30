
  .text

  .globl scalar_acc_candidate
  .type  scalar_acc_candidate, @function
scalar_acc_candidate:
.scalar_acc_candidate.bb0:
	mv	t0, a0
	li	t1, 0
	li	t2, 7
.scalar_acc_candidate.bb1:
	bge	t1, t0, .scalar_acc_candidate.bb3
.scalar_acc_candidate.bb2:
	slliw	t3, t1, 1
	addw	t3, t3, t1
	addw	t2, t2, t3
	addiw	t1, t1, 1
	blt	t1, t0, .scalar_acc_candidate.bb2	# rotated-loop-branch
.scalar_acc_candidate.bb3:
	mv	a0, t2
	ret
	.size scalar_acc_candidate, .-scalar_acc_candidate

  .globl array_acc_candidate
  .type  array_acc_candidate, @function
array_acc_candidate:
.array_acc_candidate.bb0:
	addi	sp, sp, -272
	sd	s1, 256(sp)
	mv	t0, a0
	addi	s1, sp, 0
	li	t1, 0
	li	t2, 64
.array_acc_candidate.bb1:
	bge	t1, t2, .array_acc_candidate.bb3
.array_acc_candidate.bb2:
	slliw	t3, t1, 2
	add	t3, s1, t3
	sw	t1, 0(t3)
	addiw	t1, t1, 1
	blt	t1, t2, .array_acc_candidate.bb2	# rotated-loop-branch
.array_acc_candidate.bb3:
	li	t1, 0
.array_acc_candidate.bb4:
	bge	t1, t0, .array_acc_candidate.bb6
.array_acc_candidate.bb5:
	slliw	t3, t1, 2
	lw	t2, 0(s1)
	add	t3, s1, t3
	lw	t3, 0(t3)
	addiw	t1, t1, 1
	addw	t2, t2, t3
	sw	t2, 0(s1)
	blt	t1, t0, .array_acc_candidate.bb5	# rotated-loop-branch
.array_acc_candidate.bb6:
	lw	a0, 0(s1)
	ld	s1, 256(sp)
	addi	sp, sp, 272
	ret
	.size array_acc_candidate, .-array_acc_candidate

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t5, 64
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	scalar_acc_candidate
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 16
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	array_acc_candidate
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
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
