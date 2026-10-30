
  .bss
  .globl storage
  .align 2
  .type  storage, @object
  .size  storage, 512
storage:
  .zero 512

  .text

  .globl run_reentry
  .type  run_reentry, @function
run_reentry:
.run_reentry.bb0:
	addi	sp, sp, -48
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
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
.run_reentry.bb1:
	bge	t1, s1, .run_reentry.bb3
.run_reentry.bb13:
	addiw	s2, t1, 3
	blt	s2, s1, .run_reentry.bb14
.run_reentry.bb2:
	slliw	t2, t1, 1
	slliw	t3, t1, 2
	addw	t2, t2, t1
	la	a2, storage
	addiw	t2, t2, 1
	add	t3, a2, t3
	sw	t2, 0(t3)
	addiw	t1, t1, 1
	j	.run_reentry.bb1
.run_reentry.bb10:
	slt	a6, t3, a4
	slt	a5, a4, s2
	xori	a6, a6, 1
	and	a5, a5, a6
	beqz	a5, .run_reentry.bb11
.run_reentry.bb12:
	lw	a5, 0(a2)
	addiw	a4, a4, 1
	sw	a5, 0(a3)
	addi	a2, a2, 4
	add	a3, a3, t2
	j	.run_reentry.bb10
.run_reentry.bb11:
	addiw	t3, t3, 1
.run_reentry.bb7:
	bge	t3, s3, .run_reentry.bb9
.run_reentry.bb8:
	mulw	a2, t3, s2
	la	s4, storage
	slliw	a3, t3, 2
	li	a4, 0
	add	a3, s4, a3
	slliw	a2, a2, 2
	add	a2, s4, a2
	j	.run_reentry.bb10
.run_reentry.bb9:
	addi	t0, t0, 4
.run_reentry.bb4:
	li	t2, 3
	bge	t1, t2, .run_reentry.bb6
.run_reentry.bb5:
	lw	s2, 0(t0)
	addiw	t1, t1, 1
	li	t3, 0
	divw	s3, s1, s2
	slliw	t2, s3, 2
	j	.run_reentry.bb7
.run_reentry.bb14:
	slliw	t2, t1, 1
	slliw	t3, t1, 2
	addw	t2, t2, t1
	la	a2, storage
	addiw	t2, t2, 1
	add	s3, a2, t3
	sw	t2, 0(s3)
	addiw	t2, t1, 1
	slliw	t3, t2, 1
	addw	t2, t3, t2
	addiw	t2, t2, 1
	sw	t2, 4(s3)
	addiw	t2, t1, 2
	slliw	a2, t2, 1
	addiw	t1, t1, 4
	addw	t2, a2, t2
	addiw	t2, t2, 1
	sw	t2, 8(s3)
	slliw	t2, s2, 1
	addw	t2, t2, s2
	addiw	t2, t2, 1
	sw	t2, 12(s3)
	j	.run_reentry.bb1
.run_reentry.bb3:
	li	t1, 0
	j	.run_reentry.bb4
.run_reentry.bb6:
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
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	addi	sp, sp, 48
	ret
	.size run_reentry, .-run_reentry

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t5, 24
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	run_reentry
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	run_reentry
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
