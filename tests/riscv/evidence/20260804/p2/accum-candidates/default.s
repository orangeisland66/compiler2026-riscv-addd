
  .text

  .globl array_acc_candidate
  .type  array_acc_candidate, @function
array_acc_candidate:
.array_acc_candidate.bb0:
	addi	sp, sp, -288
	sd	s1, 256(sp)
	sd	s2, 264(sp)
	sd	s3, 272(sp)
	mv	t0, a0
	addi	s1, sp, 0
	addi	s2, sp, 0
	li	t1, 0
	li	t2, 64
	mv	s3, s1	# phi-copy
.array_acc_candidate.bb1:
	bge	t1, t2, .array_acc_candidate.bb3
.array_acc_candidate.bb2:
	sw	t1, 0(s3)
	addi	s3, s3, 4
	addiw	t1, t1, 1
	blt	t1, t2, .array_acc_candidate.bb2	# rotated-loop-branch
.array_acc_candidate.bb3:
	li	t1, 0
.array_acc_candidate.bb4:
	bge	t1, t0, .array_acc_candidate.bb6
.array_acc_candidate.bb5:
	lw	t2, 0(s2)
	addiw	t1, t1, 1
	lw	t3, 0(s1)
	addw	t2, t2, t3
	sw	t2, 0(s2)
	addi	s1, s1, 4
	blt	t1, t0, .array_acc_candidate.bb5	# rotated-loop-branch
.array_acc_candidate.bb6:
	lw	a0, 0(s2)
	ld	s3, 272(sp)
	ld	s2, 264(sp)
	ld	s1, 256(sp)
	addi	sp, sp, 288
	ret
	.size array_acc_candidate, .-array_acc_candidate

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
	li	t1, 7
	li	t2, 64
.main.bb1:
	bge	t0, t2, .main.bb3
.main.bb2:
	slliw	t3, t0, 1
	addw	t3, t3, t0
	addw	t1, t1, t3
	addiw	t0, t0, 1
	blt	t0, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	mv	a0, t1
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
