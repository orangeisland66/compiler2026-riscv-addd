
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
	addi	sp, sp, -16
	sd	ra, 8(sp)
	la	t0, data
	li	t1, 0
	li	t2, 32
.main.bb1:
	bge	t1, t2, .main.bb3
.main.bb2:
	slliw	t3, t1, 1
	addw	t3, t3, t1
	addiw	t3, t3, 1
	addiw	t1, t1, 1
	sw	t3, 0(t0)
	addi	t0, t0, 4
	blt	t1, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 1
	li	t1, 0
	li	t2, 8
	la	t3, data	# loop.addr.hoisted
.main.bb4:
	bge	t0, t2, .main.bb6
.main.bb5:
	slliw	a2, t0, 2
	add	a2, t3, a2
	addiw	t0, t0, 3
	lw	a2, 0(a2)
	addw	t1, t1, a2
	blt	t0, t2, .main.bb5	# rotated-loop-branch
.main.bb6:
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
	li	t2, 10
	la	t3, data	# loop.addr.hoisted
.main.bb7:
	bge	t0, t2, .main.bb9
.main.bb8:
	slliw	a2, t0, 2
	add	a2, t3, a2
	addiw	t0, t0, 4
	lw	a2, 0(a2)
	addw	t1, t1, a2
	blt	t0, t2, .main.bb8	# rotated-loop-branch
.main.bb9:
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
.main.bb10:
	bge	t0, t2, .main.bb12
.main.bb11:
	slliw	a2, t0, 2
	add	a2, t3, a2
	lw	a2, 0(a2)
	addw	t1, t1, a2
	blt	t0, t2, .main.bb11	# rotated-loop-branch
.main.bb12:
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
.main.bb13:
	bge	t0, t2, .main.bb15
.main.bb14:
	slliw	a2, t0, 2
	add	a2, t3, a2
	addiw	t0, t0, -1
	lw	a2, 0(a2)
	addw	t1, t1, a2
	blt	t0, t2, .main.bb14	# rotated-loop-branch
.main.bb15:
	mv	t5, t1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 30
	li	t1, 0
	li	t2, 32
	la	t3, data	# loop.addr.hoisted
.main.bb16:
	bge	t0, t2, .main.bb18
.main.bb17:
	slliw	a2, t0, 2
	add	a2, t3, a2
	addiw	t0, t0, 1
	lw	a2, 0(a2)
	addw	t1, t1, a2
	blt	t0, t2, .main.bb17	# rotated-loop-branch
.main.bb18:
	mv	a0, t1
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
