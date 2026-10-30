
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
.pseudo_md5.bb47:
	slliw	a0, a6, 1
	srliw	s8, a6, 31	# const-div.pow2.bias-fold
	addiw	t4, t4, 1
	addw	a6, a6, s8
	andi	a6, a6, 1
	subw	a6, a6, s8
	addw	a6, a0, a6
	blt	t4, a7, .pseudo_md5.bb47	# rotated-loop-branch
.pseudo_md5.bb48:
	addw	a6, a6, s2
	addiw	a5, a5, 1
	addi	s9, s9, 4
	addi	s10, s10, 4
	mv	s8, s7	# phi-copy
	mv	s7, s6	# phi-copy
	mv	s6, s2	# phi-copy
	mv	s2, a6	# phi-copy
.pseudo_md5.bb34:
	li	a6, 64
	bge	a5, a6, .pseudo_md5.bb36
.pseudo_md5.bb35:
	li	a6, 16
	blt	a5, a6, .pseudo_md5.bb37
.pseudo_md5.bb38:
	li	a6, 32
	blt	a5, a6, .pseudo_md5.bb40
.pseudo_md5.bb41:
	li	a6, 48
	bge	a5, a6, .pseudo_md5.bb44
.pseudo_md5.bb43:
	slliw	a7, a5, 1
	addw	a6, s6, s7
	addw	a7, a7, a5
	subw	a6, s2, a6
	addiw	s11, a7, 5
	subw	a6, zero, a6
	sraiw	a7, s11, 31
	andi	a7, a7, 15
	addw	t4, s11, a7
	andi	t4, t4, 15
	subw	a7, t4, a7
	j	.pseudo_md5.bb39
.pseudo_md5.bb45:
	j	.pseudo_md5.bb39
.pseudo_md5.bb42:
.pseudo_md5.bb39:
	lw	t4, 0(s9)
	addw	a6, s8, a6
	slliw	a7, a7, 2
	addw	a6, a6, t4
	addi	t4, sp, 0
	add	a7, t4, a7
	lw	a7, 0(a7)
	li	t4, 1
	addw	a6, a6, a7
	lw	a7, 0(s10)
.pseudo_md5.bb46:
	bge	t4, a7, .pseudo_md5.bb48
	j	.pseudo_md5.bb47
.pseudo_md5.bb37:
	li	a6, 0
	mv	a7, a5	# phi-copy
	j	.pseudo_md5.bb39
.pseudo_md5.bb40:
	slliw	a6, a5, 2
	addw	a6, a6, a5
	addiw	s11, a6, 1
	sraiw	a6, s11, 31
	andi	a6, a6, 15
	addw	a7, s11, a6
	andi	a7, a7, 15
	subw	a7, a7, a6
	li	a6, 0
	j	.pseudo_md5.bb39
.pseudo_md5.bb44:
	slliw	a7, a5, 3
	subw	a6, zero, s6
	subw	s11, a7, a5
	sraiw	a7, s11, 31
	andi	a7, a7, 15
	addw	t4, s11, a7
	andi	t4, t4, 15
	subw	a7, t4, a7
	j	.pseudo_md5.bb39
.pseudo_md5.bb36:
	addw	a4, a4, s8
	addw	a3, a3, s2
	addw	a2, a2, s6
	addw	t3, t3, s7
	addiw	t2, t2, 64
	bge	t2, t0, .pseudo_md5.bb30	# rotated-loop-branch
.pseudo_md5.bb29:
	slliw	a5, t2, 2
	li	a6, 0
	add	a5, s1, a5
	li	a7, 16
	mv	s2, t1	# phi-copy
.pseudo_md5.bb31:
	bge	a6, a7, .pseudo_md5.bb33
.pseudo_md5.bb32:
	lw	t4, 0(a5)
	addiw	a6, a6, 1
	sw	t4, 0(s2)
	addi	a5, a5, 4
	addi	s2, s2, 4
	blt	a6, a7, .pseudo_md5.bb32	# rotated-loop-branch
.pseudo_md5.bb33:
	li	a5, 0
	mv	s2, a3	# phi-copy
	mv	s6, a2	# phi-copy
	mv	s7, t3	# phi-copy
	mv	s8, a4	# phi-copy
	mv	s9, s4	# phi-copy
	mv	s10, s5	# phi-copy
	j	.pseudo_md5.bb34
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
	li	t3, 4
	sw	t2, 0(t0)
	addi	t0, t0, 4
	li	t2, 1
.pseudo_md5.bb25:
	bge	t2, t3, .pseudo_md5.bb27
.pseudo_md5.bb26:
	li	a2, 0
	addiw	t2, t2, 1
	sw	a2, 0(t0)
	addi	t0, t0, 4
	blt	t2, t3, .pseudo_md5.bb26	# rotated-loop-branch
.pseudo_md5.bb27:
	addiw	t0, t1, 4
	li	t2, 0
	addi	t1, sp, 0
	li	t3, 271733878
	li	a2, -1732584194
	li	a3, -271733879
	li	a4, 1732584193
.pseudo_md5.bb28:
	blt	t2, t0, .pseudo_md5.bb29
.pseudo_md5.bb30:
	sw	a4, 0(s3)
	sw	a3, 4(s3)
	sw	a2, 8(s3)
	sw	t3, 12(s3)
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
	mv	t0, a0
	la	t1, state
	sw	t0, 0(t1)
	call	getint
	mv	s1, a0
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
	la	s2, buffer
.main.bb1:
	bge	zero, s1, .main.bb3
.main.bb2:
	li	t0, 0
	li	t1, 32000
	mv	s3, s2	# phi-copy
.main.bb4:
	bge	t0, t1, .main.bb6
.main.bb5:
	la	s4, state
	addiw	t0, t0, 1
	lw	t2, 0(s4)
	slliw	t3, t2, 13
	addw	s5, t2, t3
	sraiw	t2, s5, 31
	li	t3, 131071
	and	t2, t2, t3
	addw	t2, s5, t2
	sraiw	t2, t2, 17
	addw	t2, s5, t2
	slliw	t3, t2, 5
	addw	s5, t2, t3
	sw	s5, 0(s4)
	sraiw	t2, s5, 31
	andi	s4, t2, 255
	addw	t2, s5, s4
	andi	t2, t2, 255
	subw	t2, t2, s4
	sw	t2, 0(s3)
	addi	s3, s3, 4
	blt	t0, t1, .main.bb5	# rotated-loop-branch
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
.main.bb10:
	addiw	s3, t0, 4
	li	t1, 5
	bge	t0, t1, .main.bb9
.main.bb11:
	li	t1, 5
	bge	s3, t1, .main.bb12
.main.bb13:
	mv	t1, s3	# phi-copy
.main.bb14:
	slliw	s3, t0, 2
	addi	t2, sp, 48
	addi	t3, sp, 24
	add	t2, t2, s3
	add	t3, t3, s3
.main.bb7:
	bge	t0, t1, .main.bb10
.main.bb8:
	lw	a2, 0(t2)
	addiw	t0, t0, 1
	lw	a3, 0(t3)
	addw	a2, a2, a3
	addi	t3, t3, 4
	subw	a2, zero, a2
	sw	a2, 0(t2)
	addi	t2, t2, 4
	blt	t0, t1, .main.bb8	# rotated-loop-branch
	j	.main.bb10	# rotated-loop-exit
.main.bb15:
	j	.main.bb10
.main.bb12:
	li	t1, 5
	j	.main.bb14
.main.bb9:
	addiw	s1, s1, -1
	blt	zero, s1, .main.bb2	# rotated-loop-branch
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
