
  .section .rodata
  .globl maxN
  .align 2
  .type  maxN, @object
  .size  maxN, 4
maxN:
  .word 30

  .section .rodata
  .globl maxM
  .align 2
  .type  maxM, @object
  .size  maxM, 4
maxM:
  .word 600

  .bss
  .globl store
  .align 2
  .type  store, @object
  .size  store, 120
store:
  .zero 120

  .bss
  .globl n
  .align 2
  .type  n, @object
  .size  n, 4
n:
  .zero 4

  .bss
  .globl m
  .align 2
  .type  m, @object
  .size  m, 4
m:
  .zero 4

  .bss
  .globl graph
  .align 2
  .type  graph, @object
  .size  graph, 3600
graph:
  .zero 3600

  .text

  .globl maxCliques
  .type  maxCliques, @function
maxCliques:
.maxCliques.bb0:
	addi	sp, sp, -96
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	sd	s7, 72(sp)
	sd	s8, 80(sp)
	mv	s1, a1
	slliw	t0, s1, 2
	la	t1, store
	add	s2, t1, t0
	addiw	s3, s1, 1
	la	t0, store
	addi	s4, t0, 4
	li	s5, 1
	li	t1, 0
.maxCliques.bb1:
	la	t0, n
	lw	t0, 0(t0)
	blt	t0, s5, .maxCliques.bb3
.maxCliques.bb2:
	sw	s5, 0(s2)
	li	t0, 1
	mv	s6, s4	# phi-copy
.maxCliques.bb11:
	bge	t0, s3, .maxCliques.bb13
.maxCliques.bb12:
	lw	s8, 0(s6)
	addiw	s7, t0, 1
	slliw	t0, s7, 2
	la	t2, store
	add	t0, t2, t0
	slliw	t2, s8, 7
	slliw	t3, s8, 3
	subw	t2, t2, t3
	la	t3, graph	# loop.addr.hoisted
	mv	s8, s7	# phi-copy
.maxCliques.bb14:
	bge	s8, s3, .maxCliques.bb16
.maxCliques.bb15:
	lw	a2, 0(t0)
	slliw	a2, a2, 2
	addw	a2, t2, a2
	add	a2, t3, a2
	lw	a2, 0(a2)
	beq	a2, zero, .maxCliques.bb17
.maxCliques.bb18:
	addiw	s8, s8, 1
	addi	t0, t0, 4
	blt	s8, s3, .maxCliques.bb15	# rotated-loop-branch
.maxCliques.bb16:
	addi	s6, s6, 4
	mv	t0, s7	# phi-copy
	blt	t0, s3, .maxCliques.bb12	# rotated-loop-branch
.maxCliques.bb13:
	li	t0, 1
.maxCliques.bb10:
	beq	t0, zero, .maxCliques.bb19
.maxCliques.bb4:
	bge	t1, s1, .maxCliques.bb20
.maxCliques.bb6:
	mv	s6, s1	# phi-copy
.maxCliques.bb7:
	mv	a0, s5	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	maxCliques
	mv	s7, a0
	blt	s6, s7, .maxCliques.bb5
.maxCliques.bb21:
	mv	s7, s6	# phi-copy
	j	.maxCliques.bb5
.maxCliques.bb9:
.maxCliques.bb5:
	addiw	s5, s5, 1
	mv	t1, s7	# phi-copy
	j	.maxCliques.bb1
.maxCliques.bb17:
	li	t0, 0
	j	.maxCliques.bb10
.maxCliques.bb8:
	j	.maxCliques.bb5
.maxCliques.bb19:
	mv	s7, t1	# phi-copy
	j	.maxCliques.bb5
.maxCliques.bb20:
	mv	s6, t1	# phi-copy
	j	.maxCliques.bb7
.maxCliques.bb3:
	mv	a0, t1
	ld	s8, 80(sp)
	ld	s7, 72(sp)
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 96
	ret
	.size maxCliques, .-maxCliques

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t6, -4880
	add	sp, sp, t6
	li	t6, 4816
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 4824
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 4832
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 4840
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 4848
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 4856
	add	t6, sp, t6
	sd	s5, 0(t6)
	li	t6, 4864
	add	t6, sp, t6
	sd	s6, 0(t6)
	call	getint
	mv	t0, a0
	la	t1, n
	sw	t0, 0(t1)
	call	getint
	mv	s1, a0
	la	t0, m
	sw	s1, 0(t0)
	addi	s2, sp, 16
	li	s3, 0
.main.bb1:
	bge	s3, s1, .main.bb3
.main.bb2:
	call	getint
	mv	t0, a0
	slliw	t1, s3, 3
	add	s4, s2, t1
	sw	t0, 0(s4)
	call	getint
	mv	t0, a0
	sw	t0, 4(s4)
	addiw	s3, s3, 1
	blt	s3, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	la	t0, m
	lw	s1, 0(t0)
	li	t0, 0
.main.bb4:
	bge	t0, s1, .main.bb6
.main.bb7:
	addiw	t1, t0, 3
	blt	t1, s1, .main.bb8
.main.bb5:
	slliw	t1, t0, 3
	la	s5, graph
	add	s3, s2, t1
	addiw	t0, t0, 1
	lw	s4, 0(s3)
	slliw	t1, s4, 7
	slliw	t2, s4, 3
	subw	t1, t1, t2
	lw	s3, 4(s3)
	slliw	t2, s3, 2
	addw	t1, t1, t2
	add	t1, s5, t1
	li	t2, 1
	sw	t2, 0(t1)
	slliw	t1, s3, 7
	slliw	t2, s3, 3
	subw	t1, t1, t2
	slliw	t2, s4, 2
	addw	t1, t1, t2
	add	t1, s5, t1
	li	t2, 1
	sw	t2, 0(t1)
	j	.main.bb4
.main.bb8:
	slliw	t1, t0, 3
	la	s6, graph
	add	s3, s2, t1
	addiw	t0, t0, 1
	lw	s4, 0(s3)
	addiw	t0, t0, 1
	lw	s5, 4(s3)
	addiw	t0, t0, 1
	addiw	t0, t0, 1
	slliw	t1, s4, 7
	slliw	t2, s4, 3
	subw	t1, t1, t2
	slliw	t3, s5, 2
	addw	t1, t1, t3
	add	t1, s6, t1
	li	t3, 1
	sw	t3, 0(t1)
	slliw	t1, s5, 7
	slliw	t3, s5, 3
	subw	t1, t1, t3
	slliw	t3, s4, 2
	addw	t1, t1, t3
	add	t1, s6, t1
	li	t3, 1
	sw	t3, 0(t1)
	lw	s4, 8(s3)
	lw	s5, 12(s3)
	slliw	t2, s4, 7
	slliw	t3, s4, 3
	subw	t2, t2, t3
	slliw	t3, s5, 2
	addw	t2, t2, t3
	add	t2, s6, t2
	li	t3, 1
	sw	t3, 0(t2)
	slliw	t2, s5, 7
	slliw	t3, s5, 3
	subw	t2, t2, t3
	slliw	t3, s4, 2
	addw	t2, t2, t3
	add	t2, s6, t2
	li	t3, 1
	sw	t3, 0(t2)
	lw	s4, 16(s3)
	lw	s5, 20(s3)
	slliw	t2, s4, 7
	slliw	t3, s4, 3
	subw	t2, t2, t3
	slliw	t3, s5, 2
	addw	t2, t2, t3
	add	t2, s6, t2
	li	t3, 1
	sw	t3, 0(t2)
	slliw	t2, s5, 7
	slliw	t3, s5, 3
	subw	t2, t2, t3
	slliw	t3, s4, 2
	addw	t2, t2, t3
	add	t2, s6, t2
	li	t3, 1
	sw	t3, 0(t2)
	lw	s4, 24(s3)
	lw	s3, 28(s3)
	slliw	t2, s4, 7
	slliw	t3, s4, 3
	subw	t2, t2, t3
	slliw	t1, s3, 2
	addw	t1, t2, t1
	add	t1, s6, t1
	li	t2, 1
	sw	t2, 0(t1)
	slliw	t1, s3, 7
	slliw	t2, s3, 3
	subw	t1, t1, t2
	slliw	t2, s4, 2
	addw	t1, t1, t2
	add	t1, s6, t1
	li	t2, 1
	sw	t2, 0(t1)
	j	.main.bb4
.main.bb6:
	li	t5, 0
	sd	t5, 0(sp)
	li	t5, 1
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	maxCliques
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	a0, 0
	li	t6, 4864
	add	t6, sp, t6
	ld	s6, 0(t6)
	li	t6, 4856
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 4848
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 4840
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 4832
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 4824
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 4816
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 4880
	add	sp, sp, t6
	ret
	.size main, .-main
