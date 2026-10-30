
  .data
  .globl state
  .align 2
  .type  state, @object
  .size  state, 4
state:
  .word 19260817

  .bss
  .globl buffer
  .align 2
  .type  buffer, @object
  .size  buffer, 131072
buffer:
  .zero 131072

  .text

  .globl pseudo_md5
  .type  pseudo_md5, @function
pseudo_md5:
.pseudo_md5.bb0:
	addi	sp, sp, -672
	sd	s1, 576(sp)
	sd	s2, 584(sp)
	sd	s3, 592(sp)
	sd	s4, 600(sp)
	sd	s5, 608(sp)
	sd	s6, 616(sp)
	sd	s7, 624(sp)
	sd	s8, 632(sp)
	sd	s9, 640(sp)
	sd	s10, 648(sp)
	sd	s11, 656(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	addi	s4, sp, 64
	addi	s5, sp, 320
	addi	s6, sp, 320
	addi	s7, sp, 64
	li	t0, 0
	li	t1, 64
	slliw	t2, t0, 2	# loop.addr.init
	add	a2, s6, t2	# loop.addr.init
	slliw	t3, t0, 2	# loop.addr.init
	add	a3, s7, t3	# loop.addr.init
	mv	s8, s4	# phi-copy
	mv	s9, s4	# phi-copy
	mv	s10, s4	# phi-copy
	mv	s11, s4	# phi-copy
	mv	t2, s5	# phi-copy
	mv	t3, s4	# phi-copy
.pseudo_md5.bb1:
	bge	t0, t1, .pseudo_md5.bb3
.pseudo_md5.bb2:
	li	a4, 16
	bge	t0, a4, .pseudo_md5.bb5
.pseudo_md5.bb4:
	sraiw	a4, t0, 31
	andi	a4, a4, 3
	addw	a5, t0, a4
	andi	a5, a5, 3
	subw	a4, a5, a4
	beq	a4, zero, .pseudo_md5.bb7
.pseudo_md5.bb8:
	li	a5, 1
	beq	a4, a5, .pseudo_md5.bb10
.pseudo_md5.bb11:
	li	a5, 2
	bne	a4, a5, .pseudo_md5.bb14
.pseudo_md5.bb13:
	li	a4, 69234907
	sw	a4, 0(s10)
	j	.pseudo_md5.bb9
.pseudo_md5.bb15:
	j	.pseudo_md5.bb9
.pseudo_md5.bb12:
.pseudo_md5.bb9:
	li	a4, 7
	sw	a4, 0(t2)
.pseudo_md5.bb6:
	addiw	t0, t0, 1
	addi	s8, s8, 4
	addi	s9, s9, 4
	addi	s10, s10, 4
	addi	a7, s11, 4
	addi	t2, t2, 4
	addi	t3, t3, 4
	addi	a2, a2, 4	# loop.addr.bump
	addi	a3, a3, 4	# loop.addr.bump
	mv	s11, a7	# phi-copy
	blt	t0, t1, .pseudo_md5.bb2	# rotated-loop-branch
.pseudo_md5.bb3:
	slliw	t0, s2, 2
	li	t1, 128
	add	t0, s1, t0
	li	t2, 56
	sw	t1, 0(t0)
	addiw	t1, s2, 1
	addi	t0, t0, 4
.pseudo_md5.bb22:
	sraiw	t3, t1, 31
	andi	s6, t3, 63
	addw	t3, t1, s6
	andi	t3, t3, 63
	subw	t3, t3, s6
	beq	t3, t2, .pseudo_md5.bb24
.pseudo_md5.bb23:
	li	t3, 0
	addiw	t1, t1, 1
	sw	t3, 0(t0)
	addi	t0, t0, 4
	j	.pseudo_md5.bb22
.pseudo_md5.bb40:
	bge	a7, s8, .pseudo_md5.bb42
.pseudo_md5.bb43:
	addiw	t4, a7, 3
	blt	t4, s8, .pseudo_md5.bb44
.pseudo_md5.bb41:
	sraiw	a0, a6, 31
	slliw	t4, a6, 1
	andi	s11, a0, 1
	addiw	a7, a7, 1
	addw	a6, a6, s11
	andi	a6, a6, 1
	subw	a6, a6, s11
	addw	a6, t4, a6
	j	.pseudo_md5.bb40
.pseudo_md5.bb44:
	sraiw	a0, a6, 31
	slliw	t4, a6, 1
	andi	s11, a0, 1
	addiw	a7, a7, 4
	addw	a6, a6, s11
	andi	a6, a6, 1
	subw	a6, a6, s11
	addw	s11, t4, a6
	sraiw	t4, s11, 31
	slliw	a6, s11, 1
	andi	t4, t4, 1
	addw	a0, s11, t4
	andi	a0, a0, 1
	subw	t4, a0, t4
	addw	s11, a6, t4
	sraiw	t4, s11, 31
	slliw	a6, s11, 1
	andi	t4, t4, 1
	addw	a0, s11, t4
	andi	a0, a0, 1
	subw	t4, a0, t4
	addw	s11, a6, t4
	sraiw	t4, s11, 31
	slliw	a6, s11, 1
	andi	t4, t4, 1
	addw	a0, s11, t4
	andi	a0, a0, 1
	subw	t4, a0, t4
	addw	a6, a6, t4
	j	.pseudo_md5.bb40
.pseudo_md5.bb42:
	addw	a6, a6, s2
	addiw	a4, a4, 1
	addi	s9, s9, 4
	addi	s10, s10, 4
	mv	s8, s7	# phi-copy
	mv	s7, s6	# phi-copy
	mv	s6, s2	# phi-copy
	mv	s2, a6	# phi-copy
.pseudo_md5.bb28:
	bge	a4, a5, .pseudo_md5.bb30
.pseudo_md5.bb29:
	li	a6, 16
	blt	a4, a6, .pseudo_md5.bb31
.pseudo_md5.bb32:
	li	a6, 32
	blt	a4, a6, .pseudo_md5.bb34
.pseudo_md5.bb35:
	li	a6, 48
	bge	a4, a6, .pseudo_md5.bb38
.pseudo_md5.bb37:
	slliw	a7, a4, 1
	addw	a6, s6, s7
	addw	a7, a7, a4
	subw	a6, s2, a6
	addiw	s11, a7, 5
	subw	a6, zero, a6
	sraiw	a7, s11, 31
	andi	a7, a7, 15
	addw	t4, s11, a7
	andi	t4, t4, 15
	subw	a7, t4, a7
	j	.pseudo_md5.bb33
.pseudo_md5.bb39:
	j	.pseudo_md5.bb33
.pseudo_md5.bb36:
.pseudo_md5.bb33:
	lw	t4, 0(s9)
	addw	a6, s8, a6
	lw	s8, 0(s10)
	slliw	a7, a7, 2
	addw	a6, a6, t4
	addi	t4, sp, 0
	add	a7, t4, a7
	lw	a7, 0(a7)
	addw	a6, a6, a7
	li	a7, 1
	j	.pseudo_md5.bb40
.pseudo_md5.bb31:
	li	a6, 0
	mv	a7, a4	# phi-copy
	j	.pseudo_md5.bb33
.pseudo_md5.bb34:
	slliw	a6, a4, 2
	addw	a6, a6, a4
	addiw	s11, a6, 1
	sraiw	a6, s11, 31
	andi	a6, a6, 15
	addw	a7, s11, a6
	andi	a7, a7, 15
	subw	a7, a7, a6
	li	a6, 0
	j	.pseudo_md5.bb33
.pseudo_md5.bb38:
	slliw	a7, a4, 3
	subw	a6, zero, s6
	subw	s11, a7, a4
	sraiw	a7, s11, 31
	andi	a7, a7, 15
	addw	t4, s11, a7
	andi	t4, t4, 15
	subw	a7, t4, a7
	j	.pseudo_md5.bb33
.pseudo_md5.bb30:
	addw	a3, a3, s8
	addw	a2, a2, s2
	addw	t3, t3, s6
	addw	t2, t2, s7
	addiw	t1, t1, 64
	bge	t1, t0, .pseudo_md5.bb27	# rotated-loop-branch
.pseudo_md5.bb26:
	slliw	a4, t1, 2
	addi	s6, sp, 0
	add	s2, s1, a4
	lw	a4, 0(s2)
	sw	a4, 0(s6)
	lw	a5, 4(s2)
	sw	a5, 4(s6)
	lw	a5, 8(s2)
	sw	a5, 8(s6)
	lw	a5, 12(s2)
	sw	a5, 12(s6)
	lw	a5, 16(s2)
	sw	a5, 16(s6)
	lw	a5, 20(s2)
	sw	a5, 20(s6)
	lw	a5, 24(s2)
	sw	a5, 24(s6)
	lw	a5, 28(s2)
	sw	a5, 28(s6)
	lw	a5, 32(s2)
	sw	a5, 32(s6)
	lw	a5, 36(s2)
	sw	a5, 36(s6)
	lw	a5, 40(s2)
	sw	a5, 40(s6)
	lw	a5, 44(s2)
	sw	a5, 44(s6)
	lw	a5, 48(s2)
	sw	a5, 48(s6)
	lw	a5, 52(s2)
	sw	a5, 52(s6)
	lw	a5, 56(s2)
	sw	a5, 56(s6)
	lw	a4, 60(s2)
	li	a5, 64
	sw	a4, 60(s6)
	li	a4, 0
	mv	s2, a2	# phi-copy
	mv	s6, t3	# phi-copy
	mv	s7, t2	# phi-copy
	mv	s8, a3	# phi-copy
	mv	s9, s4	# phi-copy
	mv	s10, s5	# phi-copy
	j	.pseudo_md5.bb28
.pseudo_md5.bb5:
	li	a4, 32
	blt	t0, a4, .pseudo_md5.bb16
.pseudo_md5.bb17:
	li	a4, 48
	bge	t0, a4, .pseudo_md5.bb20
.pseudo_md5.bb19:
	li	a5, 228417826
	sw	a5, 0(a3)
	li	a4, 4
	sw	a4, 0(a2)
	j	.pseudo_md5.bb6
.pseudo_md5.bb21:
	j	.pseudo_md5.bb6
.pseudo_md5.bb18:
	j	.pseudo_md5.bb6
.pseudo_md5.bb7:
	li	a4, 124429432
	sw	a4, 0(s8)
	j	.pseudo_md5.bb9
.pseudo_md5.bb10:
	li	a4, 147306326
	sw	a4, 0(s9)
	j	.pseudo_md5.bb9
.pseudo_md5.bb14:
	li	a4, 29216494
	sw	a4, 0(s11)
	j	.pseudo_md5.bb9
.pseudo_md5.bb16:
	li	a5, 102638946
	sw	a5, 0(t3)
	li	a4, 5
	sw	a4, 0(a2)
	j	.pseudo_md5.bb6
.pseudo_md5.bb20:
	li	a5, 69804612
	sw	a5, 0(a3)
	li	a4, 6
	sw	a4, 0(a2)
	j	.pseudo_md5.bb6
.pseudo_md5.bb24:
	slliw	t0, t1, 2
	slliw	t2, s2, 3
	add	t0, s1, t0
	li	t3, 0
	sw	t2, 0(t0)
	li	a2, -271733879
	sw	t3, 4(t0)
	sw	t3, 8(t0)
	li	t2, 0
	li	t3, -1732584194
	sw	t2, 12(t0)
	li	a3, 1732584193
	addiw	t0, t1, 4
	li	t2, 271733878
	li	t1, 0
.pseudo_md5.bb25:
	blt	t1, t0, .pseudo_md5.bb26
.pseudo_md5.bb27:
	sw	a3, 0(s3)
	sw	a2, 4(s3)
	sw	t3, 8(s3)
	sw	t2, 12(s3)
	ld	s11, 656(sp)
	ld	s10, 648(sp)
	ld	s9, 640(sp)
	ld	s8, 632(sp)
	ld	s7, 624(sp)
	ld	s6, 616(sp)
	ld	s5, 608(sp)
	ld	s4, 600(sp)
	ld	s3, 592(sp)
	ld	s2, 584(sp)
	ld	s1, 576(sp)
	addi	sp, sp, 672
	ret
	.size pseudo_md5, .-pseudo_md5

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -128
	sd	ra, 72(sp)
	sd	s1, 80(sp)
	sd	s2, 88(sp)
	sd	s3, 96(sp)
	sd	s4, 104(sp)
	sd	s5, 112(sp)
	addi	t5, sp, 24
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sw	zero, 16(t5)	# memset-zero
	li	t0, 0
	addi	t1, sp, 24
	sw	t0, 0(t1)
	sw	t0, 4(t1)
	sw	t0, 8(t1)
	sw	t0, 12(t1)
	sw	t0, 16(t1)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	li	t5, 261
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 0
	addi	t1, sp, 48
	sw	t0, 0(t1)
	sw	t0, 4(t1)
	sw	t0, 8(t1)
	sw	t0, 12(t1)
	sw	t0, 16(t1)
	la	s3, buffer
.main.bb1:
	bge	zero, s2, .main.bb3
.main.bb2:
	li	t0, 0
	mv	s4, s1	# phi-copy
	mv	s1, s3	# phi-copy
.main.bb4:
	li	t1, 32000
	bge	t0, t1, .main.bb6
.main.bb18:
	addiw	t1, t0, 3
	li	t2, 32000
	blt	t1, t2, .main.bb19
.main.bb5:
	slliw	t1, s4, 13
	li	t2, 131071
	addw	s4, s4, t1
	addiw	t0, t0, 1
	sraiw	t1, s4, 31
	and	t1, t1, t2
	addw	t1, s4, t1
	sraiw	t1, t1, 17
	addw	t1, s4, t1
	slliw	t2, t1, 5
	addw	s4, t1, t2
	sraiw	t1, s4, 31
	andi	s5, t1, 255
	addw	t1, s4, s5
	andi	t1, t1, 255
	subw	t1, t1, s5
	sw	t1, 0(s1)
	addi	s1, s1, 4
	j	.main.bb4
.main.bb7:
	bge	t0, t2, .main.bb10
.main.bb16:
	addiw	t3, t0, 15
	blt	t3, t2, .main.bb17
.main.bb8:
	slliw	s1, t0, 2
	addi	t3, sp, 48
	addi	a2, sp, 24
	add	s5, t3, s1
	lw	t3, 0(s5)
	add	a2, a2, s1
	lw	a2, 0(a2)
	addiw	t0, t0, 1
	addw	t3, t3, a2
	subw	t3, zero, t3
	sw	t3, 0(s5)
	j	.main.bb7
.main.bb17:
	slliw	s1, t0, 2
	addi	t3, sp, 48
	add	s5, t3, s1
	addi	a2, sp, 24
	lw	t3, 0(s5)
	add	s1, a2, s1
	lw	a2, 0(s1)
	addiw	t0, t0, 16
	addw	t3, t3, a2
	lw	a2, 4(s5)
	subw	t3, zero, t3
	sw	t3, 0(s5)
	lw	a4, 4(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 4(s5)
	lw	a2, 8(s5)
	lw	a4, 8(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 8(s5)
	lw	a2, 12(s5)
	lw	a4, 12(s1)
	addw	a2, a2, a4
	lw	t3, 60(s5)
	subw	a2, zero, a2
	sw	a2, 12(s5)
	lw	a2, 16(s5)
	lw	a4, 16(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 16(s5)
	lw	a2, 20(s5)
	lw	a4, 20(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 20(s5)
	lw	a2, 24(s5)
	lw	a4, 24(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 24(s5)
	lw	a2, 28(s5)
	lw	a4, 28(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 28(s5)
	lw	a2, 32(s5)
	lw	a4, 32(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 32(s5)
	lw	a2, 36(s5)
	lw	a4, 36(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 36(s5)
	lw	a2, 40(s5)
	lw	a4, 40(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 40(s5)
	lw	a2, 44(s5)
	lw	a4, 44(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 44(s5)
	lw	a2, 48(s5)
	lw	a4, 48(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 48(s5)
	lw	a2, 52(s5)
	lw	a4, 52(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 52(s5)
	lw	a2, 56(s5)
	lw	a4, 56(s1)
	addw	a2, a2, a4
	subw	a2, zero, a2
	sw	a2, 56(s5)
	lw	a2, 60(s1)
	addw	t3, t3, a2
	subw	t3, zero, t3
	sw	t3, 60(s5)
	j	.main.bb7
.main.bb15:
.main.bb10:
	addiw	s1, t0, 4
	bge	t0, t1, .main.bb9
.main.bb11:
	li	t2, 5
	bge	s1, t2, .main.bb12
.main.bb13:
	mv	t2, s1	# phi-copy
	j	.main.bb7
.main.bb14:
	j	.main.bb7
.main.bb19:
	slliw	t1, s4, 13
	li	t2, 131071
	addw	s4, s4, t1
	li	t3, 131071
	sraiw	t1, s4, 31
	addiw	t0, t0, 4
	and	t1, t1, t2
	addw	t1, s4, t1
	sraiw	t1, t1, 17
	addw	t1, s4, t1
	slliw	t2, t1, 5
	addw	s4, t1, t2
	sraiw	t1, s4, 31
	slliw	t2, s4, 13
	andi	s5, t1, 255
	addw	t1, s4, s5
	addw	s4, s4, t2
	andi	t1, t1, 255
	sraiw	t2, s4, 31
	subw	t1, t1, s5
	and	t2, t2, t3
	sw	t1, 0(s1)
	addw	t2, s4, t2
	addi	t1, s1, 4
	sraiw	t2, t2, 17
	addi	t1, t1, 4
	addw	t2, s4, t2
	addi	t1, t1, 4
	slliw	t3, t2, 5
	addi	t1, t1, 4
	addw	s4, t2, t3
	sraiw	t2, s4, 31
	li	t3, 131071
	andi	s5, t2, 255
	addw	t2, s4, s5
	andi	t2, t2, 255
	subw	t2, t2, s5
	sw	t2, 4(s1)
	slliw	t2, s4, 13
	addw	s4, s4, t2
	sraiw	t2, s4, 31
	and	t2, t2, t3
	addw	t2, s4, t2
	sraiw	t2, t2, 17
	addw	t2, s4, t2
	slliw	t3, t2, 5
	addw	s4, t2, t3
	sraiw	t2, s4, 31
	li	t3, 131071
	andi	s5, t2, 255
	addw	t2, s4, s5
	andi	t2, t2, 255
	subw	t2, t2, s5
	sw	t2, 8(s1)
	slliw	t2, s4, 13
	addw	s4, s4, t2
	sraiw	t2, s4, 31
	and	t2, t2, t3
	addw	t2, s4, t2
	sraiw	t2, t2, 17
	addw	t2, s4, t2
	slliw	t3, t2, 5
	addw	s4, t2, t3
	sraiw	t2, s4, 31
	andi	s5, t2, 255
	addw	t2, s4, s5
	andi	t2, t2, 255
	subw	t2, t2, s5
	sw	t2, 12(s1)
	mv	s1, t1	# phi-copy
	j	.main.bb4
.main.bb12:
	li	t2, 5
	j	.main.bb7
.main.bb6:
	la	t5, buffer
	sd	t5, 0(sp)
	li	t5, 32000
	sd	t5, 8(sp)
	addi	t5, sp, 24
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	pseudo_md5
	li	t0, 0
	li	t1, 5
	j	.main.bb10
.main.bb9:
	addiw	s2, s2, -1
	mv	s1, s4	# phi-copy
	blt	zero, s2, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 284
	call	_sysy_stoptime
	li	t5, 5
	sd	t5, 0(sp)
	addi	t5, sp, 48
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	la	t0, state
	sw	s1, 0(t0)
	li	a0, 0
	ld	s5, 112(sp)
	ld	s4, 104(sp)
	ld	s3, 96(sp)
	ld	s2, 88(sp)
	ld	s1, 80(sp)
	ld	ra, 72(sp)
	addi	sp, sp, 128
	ret
	.size main, .-main
