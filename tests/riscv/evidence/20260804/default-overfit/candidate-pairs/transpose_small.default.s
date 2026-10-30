
  .bss
  .globl materialized
  .align 2
  .type  materialized, @object
  .size  materialized, 1024
materialized:
  .zero 1024

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	li	t0, 0
.main.bb1:
	li	t1, 256
	bge	t0, t1, .main.bb3
.main.bb10:
	addiw	s1, t0, 3
	li	t1, 256
	blt	s1, t1, .main.bb11
.main.bb2:
	slliw	t1, t0, 2
	la	t2, materialized
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb11:
	slliw	t1, t0, 2
	la	t2, materialized
	add	s2, t2, t1
	addiw	t1, t0, 1
	sw	t0, 0(s2)
	sw	t1, 4(s2)
	addiw	t1, t0, 2
	sw	s1, 12(s2)
	sw	t1, 8(s2)
	addiw	t0, t0, 4
	j	.main.bb1
.main.bb7:
	li	t3, 16
	slt	a2, t0, s2
	slt	t3, s2, t3
	xori	a2, a2, 1
	and	t3, t3, a2
	beqz	t3, .main.bb8
.main.bb9:
	lw	t3, 0(t1)
	sw	t3, 0(t2)
	addiw	s2, s2, 1
	addi	t1, t1, 4
	addi	t2, t2, 64	# loop.addr.bump
	j	.main.bb7
.main.bb8:
	addiw	t0, t0, 1
.main.bb4:
	li	t1, 16
	bge	t0, t1, .main.bb6
.main.bb5:
	li	s2, 0
	slliw	t1, t0, 6
	slliw	t2, s2, 4	# loop.addr.init
	add	t1, s1, t1
	addw	t2, t2, t0	# loop.addr.init
	slliw	t2, t2, 2	# loop.addr.init
	add	t2, s1, t2	# loop.addr.init
	j	.main.bb7
.main.bb3:
	li	t0, 0
	la	s1, materialized	# loop.addr.hoisted
	j	.main.bb4
.main.bb6:
	la	t0, materialized
	lw	a0, 68(t0)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
