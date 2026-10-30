
  .bss
  .globl arr1
  .align 2
  .type  arr1, @object
  .size  arr1, 57600
arr1:
  .zero 57600

  .bss
  .globl arr2
  .align 2
  .type  arr2, @object
  .size  arr2, 107520
arr2:
  .zero 107520

  .text

  .globl loop1
  .type  loop1, @function
loop1:
.loop1.bb0:
	addi	sp, sp, -80
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	mv	s1, a0
	mv	s2, a1
	li	t0, 0
.loop1.bb1:
	slt	t1, t0, s1
	slt	t2, t0, s2
	and	t1, t1, t2
	beq	t1, zero, .loop1.bb3
.loop1.bb2:
	li	t1, 5760
	mulw	t5, t0, t1
	sd	t5, 0(sp)
	li	t2, 0
.loop1.bb4:
	li	t3, 2
	bge	t2, t3, .loop1.bb6
.loop1.bb5:
	addw	t5, t0, t2
	sd	t5, 8(sp)
	li	a2, 2880
	mulw	a2, t2, a2
	ld	t5, 0(sp)
	addw	t5, t5, a2
	sd	t5, 16(sp)
	li	a3, 0
.loop1.bb7:
	li	a4, 3
	bge	a3, a4, .loop1.bb9
.loop1.bb8:
	ld	t5, 8(sp)
	addw	t5, t5, a3
	sd	t5, 24(sp)
	slliw	a5, a3, 10
	ld	t5, 16(sp)
	addw	a5, t5, a5
	slliw	a6, a3, 6
	subw	t5, a5, a6
	sd	t5, 32(sp)
	li	a6, 0
.loop1.bb10:
	li	a7, 4
	bge	a6, a7, .loop1.bb12
.loop1.bb11:
	ld	t5, 24(sp)
	addw	a7, t5, a6
	slliw	t4, a6, 8
	ld	t5, 32(sp)
	addw	t4, t5, t4
	slliw	a0, a6, 4
	subw	t4, t4, a0
	li	a0, 0
.loop1.bb13:
	li	a1, 5
	bge	a0, a1, .loop1.bb15
.loop1.bb14:
	slliw	a1, a0, 5
	slliw	t1, a0, 4
	addw	a1, t4, a1
	addw	s3, a7, a0
	addw	t1, a1, t1
	li	a1, 0
.loop1.bb16:
	li	t3, 6
	bge	a1, t3, .loop1.bb18
.loop1.bb17:
	slliw	a2, a1, 3
	la	a4, arr1
	addw	a2, t1, a2
	addw	t3, s3, a1
	add	a2, a4, a2
	li	s4, 2
	li	a4, 0
.loop1.bb19:
	bge	a4, s4, .loop1.bb21
.loop1.bb20:
	addw	a5, t3, a4
	addw	a5, a5, s1
	addiw	a4, a4, 1
	addw	a5, a5, s2
	sw	a5, 0(a2)
	addi	a2, a2, 4
	blt	a4, s4, .loop1.bb20	# rotated-loop-branch
.loop1.bb21:
	addiw	a1, a1, 1
	j	.loop1.bb16
.loop1.bb18:
	addiw	a0, a0, 1
	j	.loop1.bb13
.loop1.bb15:
	addiw	a6, a6, 1
	j	.loop1.bb10
.loop1.bb12:
	addiw	a3, a3, 1
	j	.loop1.bb7
.loop1.bb9:
	addiw	t2, t2, 1
	j	.loop1.bb4
.loop1.bb6:
	addiw	t0, t0, 1
	j	.loop1.bb1
.loop1.bb3:
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	addi	sp, sp, 80
	ret
	.size loop1, .-loop1

  .globl loop2
  .type  loop2, @function
loop2:
.loop2.bb0:
	addi	sp, sp, -32
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	li	t0, 0
.loop2.bb1:
	li	t1, 10
	bge	t0, t1, .loop2.bb3
.loop2.bb2:
	li	t1, 10752
	mulw	t5, t0, t1
	sd	t5, 0(sp)
	li	t2, 0
.loop2.bb4:
	li	t3, 2
	bge	t2, t3, .loop2.bb6
.loop2.bb5:
	addw	t3, t0, t2
	li	a2, 5376
	mulw	a2, t2, a2
	ld	t5, 0(sp)
	addw	t5, t5, a2
	sd	t5, 8(sp)
	li	a3, 0
.loop2.bb7:
	li	a4, 3
	bge	a3, a4, .loop2.bb9
.loop2.bb8:
	slliw	a4, a3, 11
	ld	t5, 8(sp)
	addw	a4, t5, a4
	slliw	a5, a3, 8
	subw	a4, a4, a5
	li	a5, 0
.loop2.bb10:
	li	a6, 2
	bge	a5, a6, .loop2.bb12
.loop2.bb11:
	slliw	a7, a5, 10
	slliw	t4, a5, 7
	addw	a7, a4, a7
	addw	a6, t3, a5
	subw	a7, a7, t4
	li	t4, 0
.loop2.bb13:
	li	a0, 4
	bge	t4, a0, .loop2.bb15
.loop2.bb14:
	slliw	a0, t4, 8
	slliw	a1, t4, 5
	addw	a0, a7, a0
	subw	s1, a0, a1
	li	a0, 0
.loop2.bb16:
	li	a1, 8
	bge	a0, a1, .loop2.bb18
.loop2.bb17:
	slliw	a1, a0, 5
	slliw	t1, a0, 2
	addw	a1, s1, a1
	li	s2, 7
	subw	t1, a1, t1
	la	a1, arr2
	add	t1, a1, t1
	li	a1, 0
.loop2.bb19:
	bge	a1, s2, .loop2.bb21
.loop2.bb20:
	addw	a2, a6, a1
	sw	a2, 0(t1)
	addiw	a1, a1, 1
	addi	t1, t1, 4
	blt	a1, s2, .loop2.bb20	# rotated-loop-branch
.loop2.bb21:
	addiw	a0, a0, 1
	j	.loop2.bb16
.loop2.bb18:
	addiw	t4, t4, 1
	j	.loop2.bb13
.loop2.bb15:
	addiw	a5, a5, 1
	j	.loop2.bb10
.loop2.bb12:
	addiw	a3, a3, 1
	j	.loop2.bb7
.loop2.bb9:
	addiw	t2, t2, 1
	j	.loop2.bb4
.loop2.bb6:
	addiw	t0, t0, 1
	j	.loop2.bb1
.loop2.bb3:
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	addi	sp, sp, 32
	ret
	.size loop2, .-loop2

  .globl loop3
  .type  loop3, @function
loop3:
.loop3.bb0:
	addi	sp, sp, -160
	sd	s1, 136(sp)
	sd	s2, 144(sp)
	mv	t5, a0
	sd	t5, 16(sp)
	mv	t5, a1
	sd	t5, 40(sp)
	mv	t5, a2
	sd	t5, 64(sp)
	mv	t5, a3
	sd	t5, 88(sp)
	mv	t5, a4
	sd	t5, 128(sp)
	mv	a3, a5
	mv	a4, a6
	li	a5, 0
	li	a6, 0
	sd	a6, 112(sp)
.loop3.bb1:
	li	a7, 10
	ld	t5, 112(sp)
	bge	t5, a7, .loop3.bb36
.loop3.bb2:
	li	a7, 5760
	ld	t5, 112(sp)
	mulw	t5, t5, a7
	sd	t5, 0(sp)
	li	t4, 10752
	ld	t5, 112(sp)
	mulw	t5, t5, t4
	sd	t5, 8(sp)
	li	a0, 0
.loop3.bb4:
	li	a1, 100
	bge	a0, a1, .loop3.bb37
.loop3.bb5:
	li	a1, 2880
	mulw	a1, a0, a1
	ld	t5, 0(sp)
	addw	t5, t5, a1
	sd	t5, 24(sp)
	li	a7, 5376
	mulw	a7, a0, a7
	ld	t5, 8(sp)
	addw	t5, t5, a7
	sd	t5, 32(sp)
	li	t4, 0
.loop3.bb7:
	li	t0, 1000
	bge	t4, t0, .loop3.bb38
.loop3.bb8:
	slliw	t0, t4, 10
	ld	t5, 24(sp)
	addw	t0, t5, t0
	slliw	a1, t4, 6
	subw	t5, t0, a1
	sd	t5, 48(sp)
	slliw	a1, t4, 11
	ld	t5, 32(sp)
	addw	a1, t5, a1
	slliw	a7, t4, 8
	subw	t5, a1, a7
	sd	t5, 56(sp)
	li	a1, 0
.loop3.bb10:
	li	t1, 10000
	bge	a1, t1, .loop3.bb39
.loop3.bb11:
	slliw	t1, a1, 8
	ld	t5, 48(sp)
	addw	t1, t5, t1
	slliw	t0, a1, 4
	subw	t5, t1, t0
	sd	t5, 72(sp)
	slliw	t1, a1, 10
	ld	t5, 56(sp)
	addw	t1, t5, t1
	slliw	a7, a1, 7
	subw	t5, t1, a7
	sd	t5, 80(sp)
	li	a7, 0
.loop3.bb13:
	li	t2, 100000
	bge	a7, t2, .loop3.bb40
.loop3.bb14:
	slliw	s1, a7, 5
	ld	t5, 72(sp)
	addw	t2, t5, s1
	slliw	t0, a7, 4
	addw	t5, t2, t0
	sd	t5, 96(sp)
	slliw	t2, a7, 8
	ld	t5, 80(sp)
	addw	t2, t5, t2
	subw	t5, t2, s1
	sd	t5, 104(sp)
	li	t1, 0
.loop3.bb16:
	li	t3, 1000000
	bge	t1, t3, .loop3.bb41
.loop3.bb17:
	slliw	t3, t1, 3
	ld	t5, 96(sp)
	addw	t3, t5, t3
	la	t0, arr1
	add	t0, t0, t3
	slliw	t3, t1, 5
	ld	t5, 104(sp)
	addw	t3, t5, t3
	slliw	t2, t1, 2
	subw	t2, t3, t2
	la	t3, arr2
	add	t2, t3, t2
	li	t3, 0
	li	t5, 10000000
	sd	t5, 120(sp)
.loop3.bb19:
	ld	t5, 120(sp)
	bge	t3, t5, .loop3.bb42
.loop3.bb20:
	li	a6, -1603384363
	li	a2, 817
	mul	a6, a5, a6	# const-div.fullmul
	addiw	s2, t3, 1
	srai	a6, a6, 32	# const-div.mulhi
	addw	a6, a6, a5	# const-div.fixup
	sraiw	a6, a6, 9	# const-div.shift
	srliw	s1, a6, 31	# const-div.sign
	addw	a6, a6, s1	# const-div.round
	mulw	a2, a6, a2	# const-rem.mul
	subw	a2, a5, a2	# const-rem.sub
	lw	a5, 0(t0)
	addw	a2, a2, a5
	lw	a5, 0(t2)
	addw	s1, a2, a5
	bge	s2, a4, .loop3.bb21
.loop3.bb23:
	addi	t0, t0, 4
	addi	t2, t2, 4
	mv	a5, s1	# phi-copy
	mv	t3, s2	# phi-copy
	j	.loop3.bb19
.loop3.bb42:
	mv	s1, a5	# phi-copy
.loop3.bb21:
	addiw	s2, t1, 1
	bge	s2, a3, .loop3.bb18
.loop3.bb25:
	mv	t1, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb16
.loop3.bb22:
	j	.loop3.bb21
.loop3.bb24:
.loop3.bb18:
	addiw	s2, a7, 1
	ld	t5, 128(sp)
	bge	s2, t5, .loop3.bb15
.loop3.bb27:
	mv	a7, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb13
.loop3.bb41:
	mv	s1, a5	# phi-copy
	j	.loop3.bb18
.loop3.bb26:
.loop3.bb15:
	addiw	s2, a1, 1
	ld	t5, 88(sp)
	bge	s2, t5, .loop3.bb12
.loop3.bb29:
	mv	a1, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb10
.loop3.bb40:
	mv	s1, a5	# phi-copy
	j	.loop3.bb15
.loop3.bb28:
.loop3.bb12:
	addiw	s2, t4, 1
	ld	t5, 64(sp)
	bge	s2, t5, .loop3.bb9
.loop3.bb31:
	mv	t4, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb7
.loop3.bb39:
	mv	s1, a5	# phi-copy
	j	.loop3.bb12
.loop3.bb30:
.loop3.bb9:
	addiw	s2, a0, 1
	ld	t5, 40(sp)
	bge	s2, t5, .loop3.bb6
.loop3.bb33:
	mv	a0, s2	# phi-copy
	mv	a5, s1	# phi-copy
	j	.loop3.bb4
.loop3.bb38:
	mv	s1, a5	# phi-copy
	j	.loop3.bb9
.loop3.bb32:
.loop3.bb6:
	ld	t5, 112(sp)
	addiw	s2, t5, 1
	ld	t5, 16(sp)
	bge	s2, t5, .loop3.bb3
.loop3.bb35:
	sd	s2, 112(sp)
	mv	a5, s1	# phi-copy
	j	.loop3.bb1
.loop3.bb37:
	mv	s1, a5	# phi-copy
	j	.loop3.bb6
.loop3.bb34:
.loop3.bb3:
	mv	a0, s1
	ld	s2, 144(sp)
	ld	s1, 136(sp)
	addi	sp, sp, 160
	ret
.loop3.bb36:
	mv	s1, a5	# phi-copy
	j	.loop3.bb3
	.size loop3, .-loop3

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -144
	sd	ra, 56(sp)
	sd	s1, 64(sp)
	sd	s2, 72(sp)
	sd	s3, 80(sp)
	sd	s4, 88(sp)
	sd	s5, 96(sp)
	sd	s6, 104(sp)
	sd	s7, 112(sp)
	sd	s8, 120(sp)
	sd	s9, 128(sp)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	call	getint
	mv	s3, a0
	call	getint
	mv	s4, a0
	call	getint
	mv	s5, a0
	call	getint
	mv	s6, a0
	call	getint
	mv	s7, a0
	call	getint
	mv	s8, a0
	call	getint
	mv	s9, a0
	mv	t5, s1
	sd	t5, 0(sp)
	mv	t5, s2
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	loop1
	call	loop2
	mv	t5, s3
	sd	t5, 0(sp)
	mv	t5, s4
	sd	t5, 8(sp)
	mv	t5, s5
	sd	t5, 16(sp)
	mv	t5, s6
	sd	t5, 24(sp)
	mv	t5, s7
	sd	t5, 32(sp)
	mv	t5, s8
	sd	t5, 40(sp)
	mv	t5, s9
	sd	t5, 48(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	ld	a5, 40(sp)
	ld	a6, 48(sp)
	call	loop3
	mv	t0, a0
	mv	a0, t0
	ld	s9, 128(sp)
	ld	s8, 120(sp)
	ld	s7, 112(sp)
	ld	s6, 104(sp)
	ld	s5, 96(sp)
	ld	s4, 88(sp)
	ld	s3, 80(sp)
	ld	s2, 72(sp)
	ld	s1, 64(sp)
	ld	ra, 56(sp)
	addi	sp, sp, 144
	ret
	.size main, .-main
