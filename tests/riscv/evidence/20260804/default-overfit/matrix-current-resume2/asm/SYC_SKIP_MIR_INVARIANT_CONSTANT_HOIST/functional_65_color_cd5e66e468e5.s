
  .section .rodata
  .globl maxn
  .align 2
  .type  maxn, @object
  .size  maxn, 4
maxn:
  .word 18

  .section .rodata
  .globl mod
  .align 2
  .type  mod, @object
  .size  mod, 4
mod:
  .word 1000000007

  .bss
  .globl dp
  .align 2
  .type  dp, @object
  .size  dp, 52907904
dp:
  .zero 52907904

  .bss
  .globl list
  .align 2
  .type  list, @object
  .size  list, 800
list:
  .zero 800

  .bss
  .globl cns
  .align 2
  .type  cns, @object
  .size  cns, 80
cns:
  .zero 80

  .text

  .globl dfs
  .type  dfs, @function
dfs:
.dfs.bb0:
	addi	sp, sp, -128
	sd	ra, 48(sp)
	sd	s1, 56(sp)
	sd	s2, 64(sp)
	sd	s3, 72(sp)
	sd	s4, 80(sp)
	sd	s5, 88(sp)
	sd	s6, 96(sp)
	sd	s7, 104(sp)
	sd	s8, 112(sp)
	sd	s9, 120(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	mv	s5, a4
	mv	s6, a5
	li	t0, 2939328
	mulw	t0, s1, t0
	li	t1, 163296
	mulw	t1, s2, t1
	addw	t0, t0, t1
	li	t1, 9072
	mulw	t1, s3, t1
	addw	t0, t0, t1
	slliw	t1, s4, 9
	slliw	t2, s4, 3
	subw	t1, t1, t2
	addw	t0, t0, t1
	slliw	t1, s5, 5
	slliw	t2, s5, 2
	subw	t1, t1, t2
	addw	t0, t0, t1
	slliw	t1, s6, 2
	addw	t0, t0, t1
	la	t1, dp
	add	s7, t1, t0
	lw	t0, 0(s7)
	li	t1, -1
	bne	t0, t1, .dfs.bb1
.dfs.bb3:
	addw	t0, s1, s2
	addw	t0, t0, s3
	addw	t0, t0, s4
	addw	t0, t0, s5
	beq	t0, zero, .dfs.bb4
.dfs.bb5:
	bne	s1, zero, .dfs.bb6
.dfs.bb20:
	li	s8, 0
.dfs.bb7:
	bne	s2, zero, .dfs.bb8
.dfs.bb21:
.dfs.bb9:
	bne	s3, zero, .dfs.bb10
.dfs.bb22:
.dfs.bb11:
	bne	s4, zero, .dfs.bb12
.dfs.bb23:
.dfs.bb13:
	bne	s5, zero, .dfs.bb14
.dfs.bb24:
	mv	t0, s8	# phi-copy
.dfs.bb15:
	li	t1, 1152921497
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 28	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 1000000007
	mulw	t1, t1, t2	# const-rem.mul
	subw	t0, t0, t1	# const-rem.sub
	sw	t0, 0(s7)
.dfs.bb1:
	mv	a0, t0
	ld	s9, 120(sp)
	ld	s8, 112(sp)
	ld	s7, 104(sp)
	ld	s6, 96(sp)
	ld	s5, 88(sp)
	ld	s4, 80(sp)
	ld	s3, 72(sp)
	ld	s2, 64(sp)
	ld	s1, 56(sp)
	ld	ra, 48(sp)
	addi	sp, sp, 128
	ret
.dfs.bb8:
	li	t0, 3
	subw	t0, s6, t0
	sltiu	t0, t0, 1
.dfs.bb17:
	subw	s9, s2, t0
	addiw	a0, s1, 1
	addiw	a1, s2, -1
	mv	a2, s3	# call-arg-copy
	mv	a3, s4	# call-arg-copy
	mv	a4, s5	# call-arg-copy
	li	a5, 2
	call	dfs
	mv	t0, a0
	mulw	t0, s9, t0
	addw	t0, s8, t0
	li	t1, 1152921497
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 28	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 1000000007
	mulw	t1, t1, t2	# const-rem.mul
	subw	s8, t0, t1	# const-rem.sub
	j	.dfs.bb9
.dfs.bb10:
	li	t0, 4
	subw	t0, s6, t0
	sltiu	t0, t0, 1
.dfs.bb18:
	subw	s9, s3, t0
	addiw	a1, s2, 1
	addiw	a2, s3, -1
	mv	a0, s1	# call-arg-copy
	mv	a3, s4	# call-arg-copy
	mv	a4, s5	# call-arg-copy
	li	a5, 3
	call	dfs
	mv	t0, a0
	mulw	t0, s9, t0
	addw	t0, s8, t0
	li	t1, 1152921497
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 28	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 1000000007
	mulw	t1, t1, t2	# const-rem.mul
	subw	s8, t0, t1	# const-rem.sub
	j	.dfs.bb11
.dfs.bb12:
	li	t0, 5
	subw	t0, s6, t0
	sltiu	t0, t0, 1
.dfs.bb19:
	subw	s6, s4, t0
	addiw	a2, s3, 1
	addiw	a3, s4, -1
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a4, s5	# call-arg-copy
	li	a5, 4
	call	dfs
	mv	t0, a0
	mulw	t0, s6, t0
	addw	t0, s8, t0
	li	t1, 1152921497
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 28	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 1000000007
	mulw	t1, t1, t2	# const-rem.mul
	subw	s8, t0, t1	# const-rem.sub
	j	.dfs.bb13
.dfs.bb14:
	addiw	a3, s4, 1
	addiw	a4, s5, -1
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a5, 5
	call	dfs
	mv	t0, a0
	mulw	t0, s5, t0
	addw	t0, s8, t0
	li	t1, 1152921497
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 28	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 1000000007
	mulw	t1, t1, t2	# const-rem.mul
	subw	t0, t0, t1	# const-rem.sub
	j	.dfs.bb15
.dfs.bb4:
	li	t0, 1
	j	.dfs.bb1
.dfs.bb6:
	li	t0, 2
	subw	t0, s6, t0
	sltiu	t0, t0, 1
.dfs.bb16:
	subw	s8, s1, t0
	addiw	a0, s1, -1
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	mv	a3, s4	# call-arg-copy
	mv	a4, s5	# call-arg-copy
	li	a5, 1
	call	dfs
	mv	t0, a0
	mulw	t0, s8, t0
	li	t1, 1152921497
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 28	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 1000000007
	mulw	t1, t1, t2	# const-rem.mul
	subw	s8, t0, t1	# const-rem.sub
	j	.dfs.bb7
.dfs.bb2:
	j	.dfs.bb1
	.size dfs, .-dfs

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 48(sp)
	sd	s1, 56(sp)
	sd	s2, 64(sp)
	sd	s3, 72(sp)
	call	getint
	mv	s1, a0
	li	t0, 0
.main.bb1:
	li	t1, 18
	bge	t0, t1, .main.bb3
.main.bb2:
	li	t1, 2939328
	li	t2, 0
	mulw	t1, t0, t1
.main.bb4:
	li	t3, 18
	bge	t2, t3, .main.bb6
.main.bb5:
	li	t3, 163296
	li	a2, 0
	mulw	t3, t2, t3
	addw	t3, t1, t3
.main.bb7:
	li	a3, 18
	bge	a2, a3, .main.bb9
.main.bb8:
	li	a3, 9072
	li	a4, 0
	mulw	a3, a2, a3
	addw	a3, t3, a3
.main.bb10:
	li	a5, 18
	bge	a4, a5, .main.bb12
.main.bb11:
	slliw	a5, a4, 9
	slliw	a6, a4, 3
	addw	a5, a3, a5
	subw	a5, a5, a6
	li	a6, 0
.main.bb13:
	li	a7, 18
	bge	a6, a7, .main.bb15
.main.bb14:
	slliw	a7, a6, 5
	slliw	t4, a6, 2
	addw	a7, a5, a7
	subw	a7, a7, t4
	la	t4, dp
	add	a7, t4, a7
	li	t4, 0
.main.bb16:
	li	a0, 7
	bge	t4, a0, .main.bb18
.main.bb17:
	li	a0, -1
	addiw	t4, t4, 1
	sw	a0, 0(a7)
	addi	a7, a7, 4
	j	.main.bb16
.main.bb18:
	addiw	a6, a6, 1
	j	.main.bb13
.main.bb15:
	addiw	a4, a4, 1
	j	.main.bb10
.main.bb12:
	addiw	a2, a2, 1
	j	.main.bb7
.main.bb9:
	addiw	t2, t2, 1
	j	.main.bb4
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	li	s2, 0
.main.bb19:
	bge	s2, s1, .main.bb21
.main.bb20:
	call	getint
	mv	t0, a0
	slliw	t0, t0, 2
	la	t1, cns
	add	s3, t1, t0
	lw	t0, 0(s3)
	addiw	t0, t0, 1
	sw	t0, 0(s3)
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb20	# rotated-loop-branch
.main.bb21:
	la	t0, cns
	lw	t5, 4(t0)
	lw	t2, 8(t0)
	lw	t3, 12(t0)
	lw	a2, 16(t0)
	lw	t0, 20(t0)
	sd	t5, 0(sp)
	mv	t5, t2
	sd	t5, 8(sp)
	mv	t5, t3
	sd	t5, 16(sp)
	mv	t5, a2
	sd	t5, 24(sp)
	mv	t5, t0
	sd	t5, 32(sp)
	li	t5, 0
	sd	t5, 40(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	ld	a5, 40(sp)
	call	dfs
	mv	s1, a0
	mv	a0, s1
	call	putint
	mv	a0, s1
	ld	s3, 72(sp)
	ld	s2, 64(sp)
	ld	s1, 56(sp)
	ld	ra, 48(sp)
	addi	sp, sp, 80
	ret
	.size main, .-main
