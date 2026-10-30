
  .text

  .globl loop
  .type  loop, @function
loop:
.loop.bb0:
	addi	sp, sp, -32
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	sd	s4, 24(sp)
	mv	t0, a0
	mv	s1, a1
	mv	s2, a2
	li	t1, 0
.loop.bb1:
	bge	t0, s1, .loop.bb3
.loop.bb7:
	addw	s3, t0, s2
	addw	s4, s3, s2
	slt	t2, t0, s3
	slt	t3, s3, s4
	slt	a2, s3, s1
	and	t2, t2, t3
	and	t2, t2, a2
	bnez	t2, .loop.bb8
.loop.bb2:
	li	t2, 5
	bge	t0, t2, .loop.bb6
.loop.bb5:
	addiw	t0, t0, 1
.loop.bb4:
	addw	t1, t1, t0
	mv	t0, s3	# phi-copy
	j	.loop.bb1
.loop.bb8:
	li	t2, 5
	bge	t0, t2, .loop.bb11
.loop.bb10:
	addiw	t0, t0, 1
.loop.bb9:
	addw	t0, t1, t0
	li	t1, 5
	bge	s3, t1, .loop.bb14
.loop.bb13:
	addiw	t1, s3, 1
.loop.bb12:
	addw	t1, t0, t1
	mv	t0, s4	# phi-copy
	j	.loop.bb1
.loop.bb14:
	li	t1, 10
	subw	t1, t1, s3
	j	.loop.bb12
.loop.bb6:
	li	t2, 10
	subw	t0, t2, t0
	j	.loop.bb4
.loop.bb11:
	li	t2, 10
	subw	t0, t2, t0
	j	.loop.bb9
.loop.bb3:
	mv	a0, t1
	ld	s4, 24(sp)
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 32
	ret
	.size loop, .-loop

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 24(sp)
	li	t5, 1
	sd	t5, 0(sp)
	li	t5, 8
	sd	t5, 8(sp)
	li	t5, 3
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	loop
	mv	t0, a0
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 2
	sd	t5, 0(sp)
	li	t5, 10
	sd	t5, 8(sp)
	li	t5, 4
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	loop
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	loop.spec.arg0.i32_p10.spec.arg0.i32_p1
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, -1
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	loop.spec.arg0.i32_p10.spec.arg0.i32_p1
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 2147483646
	sd	t5, 0(sp)
	li	t5, 2147483647
	sd	t5, 8(sp)
	li	t5, 1
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	loop
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
	ld	ra, 24(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main

  .globl loop.spec.arg0.i32_p10.spec.arg0.i32_p1
  .type  loop.spec.arg0.i32_p10.spec.arg0.i32_p1, @function
loop.spec.arg0.i32_p10.spec.arg0.i32_p1:
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb0:
	addi	sp, sp, -32
	sd	s1, 0(sp)
	sd	s2, 8(sp)
	sd	s3, 16(sp)
	mv	s1, a0
	li	t0, 10
	li	t1, 0
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb1:
	li	t2, 1
	bge	t0, t2, .loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb3
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb6:
	addw	s2, t0, s1
	li	a2, 1
	addw	s3, s2, s1
	slt	t2, t0, s2
	slt	t3, s2, s3
	slt	a2, s2, a2
	and	t2, t2, t3
	and	t2, t2, a2
	bnez	t2, .loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb9
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb2:
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb5:
	addiw	t0, t0, 1
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb4:
	addw	t1, t1, t0
	mv	t0, s2	# phi-copy
	j	.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb1
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb7:
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb9:
	addiw	t0, t0, 1
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb8:
	addw	t0, t1, t0
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb11:
	addiw	t1, s2, 1
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb10:
	addw	t1, t0, t1
	mv	t0, s3	# phi-copy
	j	.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb1
.loop.spec.arg0.i32_p10.spec.arg0.i32_p1.bb3:
	mv	a0, t1
	ld	s3, 16(sp)
	ld	s2, 8(sp)
	ld	s1, 0(sp)
	addi	sp, sp, 32
	ret
	.size loop.spec.arg0.i32_p10.spec.arg0.i32_p1, .-loop.spec.arg0.i32_p10.spec.arg0.i32_p1
