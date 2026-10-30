
  .section .rodata
  .globl INF
  .align 2
  .type  INF, @object
  .size  INF, 4
INF:
  .word 65535

  .bss
  .globl e
  .align 2
  .type  e, @object
  .size  e, 1024
e:
  .zero 1024

  .bss
  .globl book
  .align 2
  .type  book, @object
  .size  book, 64
book:
  .zero 64

  .bss
  .globl dis
  .align 2
  .type  dis, @object
  .size  dis, 64
dis:
  .zero 64

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
  .globl v1
  .align 2
  .type  v1, @object
  .size  v1, 4
v1:
  .zero 4

  .bss
  .globl v2
  .align 2
  .type  v2, @object
  .size  v2, 4
v2:
  .zero 4

  .bss
  .globl w
  .align 2
  .type  w, @object
  .size  w, 4
w:
  .zero 4

  .text

  .globl Dijkstra
  .type  Dijkstra, @function
Dijkstra:
.Dijkstra.bb0:
	addi	sp, sp, -48
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	sd	s5, 32(sp)
	sd	s6, 40(sp)
	la	t0, n
	lw	s1, 0(t0)
	la	t0, dis
	addi	s2, t0, 4
	la	t0, book
	addi	s3, t0, 4
	li	t0, 1
	mv	s4, s2	# phi-copy
	mv	s5, s3	# phi-copy
.Dijkstra.bb1:
	blt	s1, t0, .Dijkstra.bb3
.Dijkstra.bb2:
	slliw	t1, t0, 2
	la	t2, e
	addiw	t1, t1, 64
	addiw	t0, t0, 1
	add	t1, t2, t1
	lw	t1, 0(t1)
	addi	t2, s5, 4
	sw	t1, 0(s4)
	li	t1, 0
	sw	t1, 0(s5)
	addi	s4, s4, 4
	mv	s5, t2	# phi-copy
	bge	s1, t0, .Dijkstra.bb2	# rotated-loop-branch
.Dijkstra.bb3:
	li	t0, 1
	la	t1, book
	sw	t0, 4(t1)
	addiw	t0, s1, -1
	li	t1, 1
.Dijkstra.bb4:
	blt	t0, t1, .Dijkstra.bb6
.Dijkstra.bb5:
	li	t2, 65535
	li	t3, 1
	li	a2, 0
	mv	s4, s2	# phi-copy
	mv	s5, s3	# phi-copy
.Dijkstra.bb7:
	blt	s1, t3, .Dijkstra.bb9
.Dijkstra.bb8:
	lw	a4, 0(s5)
	li	a5, 0
	lw	s6, 0(s4)
	subw	a4, a4, a5
	slt	a3, s6, t2
	sltiu	a4, a4, 1
	and	a3, a3, a4
	beq	a3, zero, .Dijkstra.bb19
.Dijkstra.bb10:
	mv	a2, t3	# phi-copy
.Dijkstra.bb11:
	addiw	t3, t3, 1
	addi	s5, s5, 4
	addi	s4, s4, 4
	mv	t2, s6	# phi-copy
	j	.Dijkstra.bb7
.Dijkstra.bb19:
	mv	s6, t2	# phi-copy
	j	.Dijkstra.bb11
.Dijkstra.bb9:
	slliw	s4, a2, 2
	la	t2, book
	add	t2, t2, s4
	li	t3, 1
	sw	t3, 0(t2)
	slliw	t2, a2, 6
	la	t3, dis
	add	t3, t3, s4
	li	a2, 1
	mv	s4, s2	# phi-copy
.Dijkstra.bb12:
	blt	s1, a2, .Dijkstra.bb14
.Dijkstra.bb13:
	slliw	a3, a2, 2
	la	a4, e
	addw	a3, t2, a3
	add	a3, a4, a3
	lw	s5, 0(a3)
	li	a3, 65535
	blt	s5, a3, .Dijkstra.bb15
.Dijkstra.bb16:
	addiw	a2, a2, 1
	addi	s4, s4, 4
	j	.Dijkstra.bb12
.Dijkstra.bb15:
	lw	a4, 0(t3)
	lw	a3, 0(s4)
	addw	s5, a4, s5
	blt	s5, a3, .Dijkstra.bb17
	j	.Dijkstra.bb16
.Dijkstra.bb18:
	j	.Dijkstra.bb16
.Dijkstra.bb17:
	sw	s5, 0(s4)
	j	.Dijkstra.bb16
.Dijkstra.bb14:
	addiw	t1, t1, 1
	bge	t0, t1, .Dijkstra.bb5	# rotated-loop-branch
.Dijkstra.bb6:
	ld	s6, 40(sp)
	ld	s5, 32(sp)
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 48
	ret
	.size Dijkstra, .-Dijkstra

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	call	getint
	mv	s1, a0
	la	t0, n
	sw	s1, 0(t0)
	call	getint
	mv	t0, a0
	la	t1, m
	sw	t0, 0(t1)
	li	t0, 1
.main.bb1:
	blt	s1, t0, .main.bb3
.main.bb2:
	slliw	t1, t0, 6
	la	t2, e
	addiw	t1, t1, 4
	add	t1, t2, t1
	li	t2, 1
.main.bb4:
	blt	s1, t2, .main.bb6
.main.bb5:
	bne	t0, t2, .main.bb8
.main.bb7:
	li	t3, 0
	sw	t3, 0(t1)
.main.bb9:
	addiw	t2, t2, 1
	addi	t1, t1, 4
	j	.main.bb4
.main.bb8:
	li	t3, 65535
	sw	t3, 0(t1)
	j	.main.bb9
.main.bb6:
	addiw	t0, t0, 1
	bge	s1, t0, .main.bb2	# rotated-loop-branch
.main.bb3:
	la	t0, m
	lw	s1, 0(t0)
	li	s2, 1
.main.bb10:
	blt	s1, s2, .main.bb12
.main.bb11:
	call	getint
	mv	s3, a0
	call	getint
	mv	s4, a0
	call	getint
	mv	t0, a0
	slliw	t1, s3, 6
	slliw	t2, s4, 2
	addw	t1, t1, t2
	la	t2, e
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s2, s2, 1
	bge	s1, s2, .main.bb11	# rotated-loop-branch
.main.bb12:
	call	Dijkstra
	la	t0, n
	lw	s1, 0(t0)
	li	s2, 1
.main.bb13:
	blt	s1, s2, .main.bb15
.main.bb14:
	slliw	t0, s2, 2
	la	t1, dis
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s2, s2, 1
	bge	s1, s2, .main.bb14	# rotated-loop-branch
.main.bb15:
	li	a0, 10
	call	putch
	li	a0, 0
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
