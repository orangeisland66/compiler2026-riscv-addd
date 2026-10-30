
  .bss
  .globl data
  .align 2
  .type  data, @object
  .size  data, 128
data:
  .zero 128

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	la	t1, data
	li	t0, 1
	sw	t0, 0(t1)
	li	t0, 4
	sw	t0, 4(t1)
	li	t0, 7
	sw	t0, 8(t1)
	li	t0, 10
	sw	t0, 12(t1)
	li	t0, 13
	sw	t0, 16(t1)
	li	t0, 16
	sw	t0, 20(t1)
	li	t0, 19
	sw	t0, 24(t1)
	li	t0, 22
	sw	t0, 28(t1)
	li	t0, 25
	sw	t0, 32(t1)
	li	t0, 28
	sw	t0, 36(t1)
	li	t0, 31
	sw	t0, 40(t1)
	li	t0, 34
	sw	t0, 44(t1)
	li	t0, 37
	sw	t0, 48(t1)
	li	t0, 40
	sw	t0, 52(t1)
	li	t0, 43
	sw	t0, 56(t1)
	li	t0, 46
	sw	t0, 60(t1)
	li	t0, 49
	sw	t0, 64(t1)
	li	t0, 52
	sw	t0, 68(t1)
	li	t0, 55
	sw	t0, 72(t1)
	li	t0, 58
	sw	t0, 76(t1)
	li	t0, 61
	sw	t0, 80(t1)
	li	t0, 64
	sw	t0, 84(t1)
	li	t0, 67
	sw	t0, 88(t1)
	li	t0, 70
	sw	t0, 92(t1)
	li	t0, 73
	sw	t0, 96(t1)
	li	t0, 76
	sw	t0, 100(t1)
	li	t0, 79
	sw	t0, 104(t1)
	li	t0, 82
	sw	t0, 108(t1)
	li	t0, 85
	sw	t0, 112(t1)
	li	t0, 88
	sw	t0, 116(t1)
	li	t0, 91
	sw	t0, 120(t1)
	li	t0, 94
	sw	t0, 124(t1)
	li	t0, 1
	li	t1, 0
.main.bb1:
	li	t2, 8
	bge	t0, t2, .main.bb3
.main.bb12:
	addiw	s1, t0, 3
	addiw	s2, t0, 6
	slt	t2, t0, s1
	slt	t3, s1, s2
	li	a2, 8
	and	t2, t2, t3
	slt	a2, s1, a2
	and	t2, t2, a2
	bnez	t2, .main.bb13
.main.bb2:
	slliw	t0, t0, 2
	la	t2, data
	add	t0, t2, t0
	lw	t0, 0(t0)
	addw	t1, t1, t0
	mv	t0, s1	# phi-copy
	j	.main.bb1
.main.bb13:
	slliw	t0, t0, 2
	la	t2, data
	add	s1, t2, t0
	lw	t0, 0(s1)
	addw	t0, t1, t0
	lw	t1, 12(s1)
	addw	t1, t0, t1
	mv	t0, s2	# phi-copy
	j	.main.bb1
.main.bb4:
	li	t2, 10
	bge	t0, t2, .main.bb6
.main.bb14:
	addiw	s1, t0, 4
	addiw	s2, t0, 8
	slt	t2, t0, s1
	slt	t3, s1, s2
	li	a2, 10
	and	t2, t2, t3
	slt	a2, s1, a2
	and	t2, t2, a2
	bnez	t2, .main.bb15
.main.bb5:
	slliw	t0, t0, 2
	la	t2, data
	add	t0, t2, t0
	lw	t0, 0(t0)
	addw	t1, t1, t0
	mv	t0, s1	# phi-copy
	j	.main.bb4
.main.bb15:
	slliw	t0, t0, 2
	la	t2, data
	add	s1, t2, t0
	lw	t0, 0(s1)
	addw	t0, t1, t0
	lw	t1, 16(s1)
	addw	t1, t0, t1
	mv	t0, s2	# phi-copy
	j	.main.bb4
.main.bb9:
	li	t2, 1
	bge	t0, t2, .main.bb11
.main.bb17:
	addiw	s1, t0, -1
	addiw	s2, t0, -2
	slt	t2, t0, s1
	slt	t3, s1, s2
	and	t2, t2, t3
	bnez	t2, .main.bb18
.main.bb10:
	slliw	t0, t0, 2
	la	t2, data
	add	t0, t2, t0
	lw	t0, 0(t0)
	addw	t1, t1, t0
	mv	t0, s1	# phi-copy
	j	.main.bb9
.main.bb18:
	slliw	t0, t0, 2
	la	t2, data
	add	s1, t2, t0
	lw	t0, 0(s1)
	addw	t0, t1, t0
	lw	t1, -4(s1)
	addw	t1, t0, t1
	mv	t0, s2	# phi-copy
	j	.main.bb9
.main.bb3:
	mv	t5, t1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 2
	li	t1, 0
	j	.main.bb4
.main.bb6:
	mv	t5, t1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 10
	li	t1, 0
	li	t2, 1
	la	t3, data	# loop.addr.hoisted
.main.bb7:
	bge	t0, t2, .main.bb8
.main.bb16:
	slliw	a2, t0, 2
	add	a2, t3, a2
	lw	a2, 0(a2)
	addw	t1, t1, a2
	blt	t0, t2, .main.bb16	# rotated-loop-branch
.main.bb8:
	mv	t5, t1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 10
	li	t1, 0
	j	.main.bb9
.main.bb11:
	mv	a0, t1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	la	t0, data
	lw	t1, 120(t0)
	lw	t0, 124(t0)
	addw	t5, t1, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
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
