
  .section .rodata
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .word 10000

  .text

  .globl long_array
  .type  long_array, @function
long_array:
.long_array.bb0:
	li	t6, -120096
	add	sp, sp, t6
	li	t6, 120008
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 120016
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 120024
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 120032
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 120040
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 120048
	add	t6, sp, t6
	sd	s5, 0(t6)
	li	t6, 120056
	add	t6, sp, t6
	sd	s6, 0(t6)
	li	t6, 120064
	add	t6, sp, t6
	sd	s7, 0(t6)
	li	t6, 120072
	add	t6, sp, t6
	sd	s8, 0(t6)
	li	t6, 120080
	add	t6, sp, t6
	sd	s9, 0(t6)
	mv	s1, a0
	addi	s2, sp, 8
	li	s3, 40008
	add	s3, sp, s3
	addi	s4, sp, 8
	li	s5, 80008
	add	s5, sp, s5
	li	t0, 0
	li	t1, 10000
	mv	s6, s2	# phi-copy
.long_array.bb1:
	bge	t0, t1, .long_array.bb3
.long_array.bb2:
	mulw	s7, t0, t0
	li	t2, 1717986919
	mul	t2, s7, t2	# const-div.fullmul
	addiw	t0, t0, 1
	srai	t2, t2, 32	# const-div.mulhi
	sraiw	t2, t2, 2	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	li	t3, 10
	mulw	t2, t2, t3	# const-rem.mul
	subw	t2, s7, t2	# const-rem.sub
	sw	t2, 0(s6)
	addi	s6, s6, 4
	blt	t0, t1, .long_array.bb2	# rotated-loop-branch
.long_array.bb3:
	li	s6, 40008
	add	s6, sp, s6
	li	t0, 0
	li	t1, 10000
	mv	s7, s2	# phi-copy
	mv	s8, s6	# phi-copy
.long_array.bb4:
	bge	t0, t1, .long_array.bb6
.long_array.bb5:
	lw	t2, 0(s7)
	addiw	t0, t0, 1
	mulw	s9, t2, t2
	li	t2, 1717986919
	mul	t2, s9, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	sraiw	t2, t2, 2	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	li	t3, 10
	mulw	t2, t2, t3	# const-rem.mul
	subw	t2, s9, t2	# const-rem.sub
	addi	t3, s8, 4
	sw	t2, 0(s8)
	addi	s7, s7, 4
	mv	s8, t3	# phi-copy
	blt	t0, t1, .long_array.bb5	# rotated-loop-branch
.long_array.bb6:
	li	s7, 80008
	add	s7, sp, s7
	li	t0, 0
	li	t1, 10000
	mv	s8, s7	# phi-copy
.long_array.bb7:
	bge	t0, t1, .long_array.bb9
.long_array.bb8:
	lw	t2, 0(s6)
	addiw	t0, t0, 1
	addi	a2, s2, 4
	mulw	s9, t2, t2
	li	t2, 1374389535
	mul	t2, s9, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	sraiw	t2, t2, 5	# const-div.shift
	srliw	t3, t2, 31	# const-div.sign
	addw	t2, t2, t3	# const-div.round
	li	t3, 100
	mulw	t2, t2, t3	# const-rem.mul
	lw	t3, 0(s2)
	subw	t2, s9, t2	# const-rem.sub
	addw	t2, t2, t3
	sw	t2, 0(s8)
	addi	s6, s6, 4
	addi	s8, s8, 4
	mv	s2, a2	# phi-copy
	blt	t0, t1, .long_array.bb8	# rotated-loop-branch
.long_array.bb9:
	addi	t0, sp, 8
	li	t1, 20000
	add	s2, t0, t1
	li	t0, 0
	li	s6, 0
.long_array.bb10:
	li	t1, 10000
	bge	s6, t1, .long_array.bb12
.long_array.bb11:
	li	t1, 10
	blt	s6, t1, .long_array.bb13
.long_array.bb14:
	li	t1, 20
	blt	s6, t1, .long_array.bb16
.long_array.bb17:
	li	t1, 30
	bge	s6, t1, .long_array.bb23
.long_array.bb22:
	slliw	t1, s6, 2
	li	t3, 5000
	add	t2, s3, t1
	li	a2, 10000
	lw	t2, 0(t2)
	add	t1, s4, t1
	lw	t1, 0(t1)
	mv	s8, t0	# phi-copy
.long_array.bb25:
	bge	t3, a2, .long_array.bb27
.long_array.bb26:
	li	t0, 2233
	bge	t0, t3, .long_array.bb29
.long_array.bb28:
	slliw	a3, t3, 2
	addw	t0, s8, t2
	add	a3, s4, a3
	addiw	t3, t3, 1
	lw	a3, 0(a3)
	subw	t0, t0, a3
.long_array.bb30:
	mv	s8, t0	# phi-copy
	j	.long_array.bb25
.long_array.bb29:
	slliw	a3, t3, 2
	addw	t0, s8, t1
	add	a3, s5, a3
	addiw	t3, t3, 2
	lw	a3, 0(a3)
	addw	s8, t0, a3
	li	t0, 329861735
	mul	t0, s8, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 10	# const-div.shift
	srliw	a3, t0, 31	# const-div.sign
	addw	t0, t0, a3	# const-div.round
	li	a3, 13333
	mulw	t0, t0, a3	# const-rem.mul
	subw	t0, s8, t0	# const-rem.sub
	j	.long_array.bb30
.long_array.bb27:
	mv	a0, s8
	call	putint
	mv	t0, s8	# phi-copy
	j	.long_array.bb15
.long_array.bb24:
	j	.long_array.bb15
.long_array.bb18:
.long_array.bb15:
	addiw	s6, s6, 1
	addi	s7, s7, 4
	j	.long_array.bb10
.long_array.bb13:
	lw	t1, 0(s7)
	addw	s8, t0, t1
	li	t0, 824839931
	mul	t0, s8, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 8	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 1333
	mulw	t0, t0, t1	# const-rem.mul
	subw	s8, s8, t0	# const-rem.sub
	mv	a0, s8
	call	putint
	mv	t0, s8	# phi-copy
	j	.long_array.bb15
.long_array.bb16:
	lw	t1, 0(s7)
	li	t2, 5000
	li	t3, 10000
	mv	s8, t0	# phi-copy
	mv	s9, s2	# phi-copy
.long_array.bb19:
	bge	t2, t3, .long_array.bb21
.long_array.bb20:
	lw	a2, 0(s9)
	addw	t0, s8, t1
	addiw	t2, t2, 1
	subw	s8, t0, a2
	addi	s9, s9, 4
	blt	t2, t3, .long_array.bb20	# rotated-loop-branch
.long_array.bb21:
	mv	a0, s8
	call	putint
	mv	t0, s8	# phi-copy
	j	.long_array.bb15
.long_array.bb23:
	lw	t1, 0(s7)
	mulw	t1, t1, s1
	addw	s8, t0, t1
	li	t0, 1407543789
	mul	t0, s8, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 15	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 99988
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s8, t0	# const-rem.sub
	j	.long_array.bb15
.long_array.bb12:
	mv	a0, t0
	li	t6, 120080
	add	t6, sp, t6
	ld	s9, 0(t6)
	li	t6, 120072
	add	t6, sp, t6
	ld	s8, 0(t6)
	li	t6, 120064
	add	t6, sp, t6
	ld	s7, 0(t6)
	li	t6, 120056
	add	t6, sp, t6
	ld	s6, 0(t6)
	li	t6, 120048
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 120040
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 120032
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 120024
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 120016
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 120008
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 120096
	add	sp, sp, t6
	ret
	.size long_array, .-long_array

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t5, 9
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	long_array
	mv	t0, a0
	mv	a0, t0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
