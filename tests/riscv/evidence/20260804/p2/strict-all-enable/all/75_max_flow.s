
  .section .rodata
  .globl INF
  .align 2
  .type  INF, @object
  .size  INF, 4
INF:
  .word 1879048192

  .bss
  .globl size
  .align 2
  .type  size, @object
  .size  size, 40
size:
  .zero 40

  .bss
  .globl to
  .align 2
  .type  to, @object
  .size  to, 400
to:
  .zero 400

  .bss
  .globl cap
  .align 2
  .type  cap, @object
  .size  cap, 400
cap:
  .zero 400

  .bss
  .globl rev
  .align 2
  .type  rev, @object
  .size  rev, 400
rev:
  .zero 400

  .bss
  .globl used
  .align 2
  .type  used, @object
  .size  used, 40
used:
  .zero 40

  .text

  .globl dfs
  .type  dfs, @function
dfs:
.dfs.bb0:
	addi	sp, sp, -112
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	sd	s3, 48(sp)
	sd	s4, 56(sp)
	sd	s5, 64(sp)
	sd	s6, 72(sp)
	sd	s7, 80(sp)
	sd	s8, 88(sp)
	sd	s9, 96(sp)
	sd	s10, 104(sp)
	mv	t0, a0
	mv	s1, a1
	mv	s2, a2
	bne	t0, s1, .dfs.bb3
.dfs.bb2:
	mv	t0, s2	# phi-copy
.dfs.bb1:
	mv	a0, t0
	ld	s10, 104(sp)
	ld	s9, 96(sp)
	ld	s8, 88(sp)
	ld	s7, 80(sp)
	ld	s6, 72(sp)
	ld	s5, 64(sp)
	ld	s4, 56(sp)
	ld	s3, 48(sp)
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 112
	ret
.dfs.bb4:
	lw	t0, 0(s3)
	bge	s6, t0, .dfs.bb6
.dfs.bb5:
	slliw	t0, s6, 2
	la	t1, used
	addw	s7, s4, t0
	lw	t0, 0(s5)
	slliw	t0, t0, 2
	add	t0, t1, t0
	lw	t0, 0(t0)
	bne	t0, zero, .dfs.bb7
.dfs.bb8:
	la	t0, cap
	add	s8, t0, s7
	lw	s9, 0(s8)
	bge	zero, s9, .dfs.bb9
.dfs.bb10:
	blt	s2, s9, .dfs.bb11
.dfs.bb12:
.dfs.bb13:
	lw	s10, 0(s5)
	mv	a0, s10	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	mv	a2, s9	# call-arg-copy
	call	dfs
	mv	s9, a0
	blt	zero, s9, .dfs.bb14
.dfs.bb15:
	addiw	s6, s6, 1
	addi	s5, s5, 4
	j	.dfs.bb4
.dfs.bb7:
	addiw	s6, s6, 1
	addi	s5, s5, 4
	j	.dfs.bb4
.dfs.bb6:
	li	t0, 0
	j	.dfs.bb1
.dfs.bb9:
	addiw	s6, s6, 1
	addi	s5, s5, 4
	j	.dfs.bb4
.dfs.bb11:
	mv	s9, s2	# phi-copy
	j	.dfs.bb13
.dfs.bb14:
	lw	t0, 0(s8)
	slliw	t1, s10, 3
	subw	t0, t0, s9
	sw	t0, 0(s8)
	slliw	t0, s10, 5
	addw	t0, t0, t1
	la	t1, rev
	add	t1, t1, s7
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	addw	t0, t0, t1
	la	t1, cap
	add	t0, t1, t0
	lw	t1, 0(t0)
	addw	t1, t1, s9
	sw	t1, 0(t0)
	mv	t0, s9	# phi-copy
	j	.dfs.bb1
.dfs.bb3:
	slliw	t1, t0, 2
	la	t2, used
	add	t2, t2, t1
	li	t3, 1
	sw	t3, 0(t2)
	la	t2, size
	add	s3, t2, t1
	slliw	t1, t0, 5
	slliw	t0, t0, 3
	addw	s4, t1, t0
	la	t0, to
	li	s6, 0
	add	s5, t0, s4
	j	.dfs.bb4
	.size dfs, .-dfs

  .globl max_flow
  .type  max_flow, @function
max_flow:
.max_flow.bb0:
	addi	sp, sp, -64
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	sd	s3, 48(sp)
	sd	s4, 56(sp)
	mv	s1, a0
	mv	s2, a1
	li	s3, 0
.max_flow.bb1:
	li	t0, 0
	la	s4, used
	sw	t0, 0(s4)
	sw	t0, 4(s4)
	sw	t0, 8(s4)
	sw	t0, 12(s4)
	sw	t0, 16(s4)
	sw	t0, 20(s4)
	sw	t0, 24(s4)
	sw	t0, 28(s4)
	sw	t0, 32(s4)
	sw	t0, 36(s4)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	li	a2, 1879048192
	call	dfs
	mv	t0, a0
	beq	t0, zero, .max_flow.bb2
.max_flow.bb3:
	addw	s3, s3, t0
	j	.max_flow.bb1
.max_flow.bb2:
	mv	a0, s3
	ld	s4, 56(sp)
	ld	s3, 48(sp)
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 64
	ret
	.size max_flow, .-max_flow

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -112
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
	call	getint
	mv	s1, a0
	call	getint
	mv	t0, a0
	li	t1, 0
	la	t2, size
	sw	t1, 0(t2)
	sw	t1, 4(t2)
	sw	t1, 8(t2)
	sw	t1, 12(t2)
	sw	t1, 16(t2)
	sw	t1, 20(t2)
	sw	t1, 24(t2)
	sw	t1, 28(t2)
	sw	t1, 32(t2)
	sw	t1, 36(t2)
	mv	s2, t0	# phi-copy
.main.bb1:
	bge	zero, s2, .main.bb3
.main.bb2:
	call	getint
	mv	s3, a0
	call	getint
	mv	s4, a0
	call	getint
	mv	t0, a0
	slliw	t1, s3, 5
	slliw	t2, s3, 3
	addw	t1, t1, t2
	slliw	t2, s3, 2
	la	s5, size
	add	s6, s5, t2
	lw	s7, 0(s6)
	slliw	t2, s7, 2
	addw	s8, t1, t2
	la	s9, to
	add	t1, s9, s8
	sw	s4, 0(t1)
	la	s10, cap
	add	t1, s10, s8
	sw	t0, 0(t1)
	slliw	t0, s4, 2
	add	s5, s5, t0
	lw	s11, 0(s5)
	la	t0, rev
	add	t1, t0, s8
	sw	s11, 0(t1)
	slliw	t1, s4, 5
	slliw	t2, s4, 3
	addw	t1, t1, t2
	slliw	t2, s11, 2
	addw	s4, t1, t2
	add	t1, s9, s4
	sw	s3, 0(t1)
	add	t1, s10, s4
	li	t2, 0
	sw	t2, 0(t1)
	add	t0, t0, s4
	sw	s7, 0(t0)
	addiw	t0, s7, 1
	sw	t0, 0(s6)
	lw	t0, 0(s5)
	addiw	t0, t0, 1
	sw	t0, 0(s5)
	addiw	s2, s2, -1
	blt	zero, s2, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t5, 1
	sd	t5, 0(sp)
	mv	t5, s1
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	max_flow
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
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
	addi	sp, sp, 112
	ret
	.size main, .-main
