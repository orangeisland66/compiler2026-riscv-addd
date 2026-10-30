
  .section .rodata
  .globl base
  .align 2
  .type  base, @object
  .size  base, 4
base:
  .word 16

  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 120000040
a:
  .zero 120000040

  .bss
  .globl ans
  .align 2
  .type  ans, @object
  .size  ans, 4
ans:
  .zero 4

  .text

  .globl getMaxNum
  .type  getMaxNum, @function
getMaxNum:
.getMaxNum.bb0:
	addi	sp, sp, -32
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	mv	t0, a0
	mv	s1, a1
	addi	t1, sp, 0
	sw	t0, 0(t1)
	li	t0, 0
	addi	t1, sp, 8
	sw	t0, 0(t1)
	addi	t1, sp, 12
	sw	t0, 0(t1)
.getMaxNum.bb2:
	addi	t0, sp, 12
	addi	t1, sp, 0
	lw	t0, 0(t0)
	lw	t1, 0(t1)
	slt	t0, t0, t1
	beq	t0, zero, .getMaxNum.bb4
.getMaxNum.bb3:
	addi	t0, sp, 12
	li	t1, 4
	lw	t0, 0(t0)
	mulw	t0, t0, t1
	addi	t1, sp, 8
	lw	t1, 0(t1)
	add	t0, s1, t0
	lw	t0, 0(t0)
	slt	t0, t1, t0
	bne	t0, zero, .getMaxNum.bb5
.getMaxNum.bb6:
	addi	s2, sp, 12
	lw	t0, 0(s2)
	addiw	t0, t0, 1
	sw	t0, 0(s2)
	j	.getMaxNum.bb2
.getMaxNum.bb5:
	addi	t0, sp, 12
	li	t1, 4
	lw	t0, 0(t0)
	mulw	t0, t0, t1
	addi	t1, sp, 8
	add	t0, s1, t0
	lw	t0, 0(t0)
	sw	t0, 0(t1)
	j	.getMaxNum.bb6
.getMaxNum.bb4:
	addi	t0, sp, 8
	addi	t1, sp, 4
	lw	t0, 0(t0)
	sw	t0, 0(t1)
.getMaxNum.bb1:
	addi	t0, sp, 4
	lw	a0, 0(t0)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	addi	sp, sp, 32
	ret
	.size getMaxNum, .-getMaxNum

  .globl getNumPos
  .type  getNumPos, @function
getNumPos:
.getNumPos.bb0:
	addi	sp, sp, -32
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	mv	t0, a0
	mv	t1, a1
	addi	t2, sp, 0
	sw	t0, 0(t2)
	addi	t0, sp, 4
	sw	t1, 0(t0)
	li	t0, 0
	addi	t1, sp, 12
	sw	t0, 0(t1)
.getNumPos.bb2:
	addi	t0, sp, 12
	addi	t1, sp, 4
	lw	t0, 0(t0)
	lw	t1, 0(t1)
	slt	t0, t0, t1
	beq	t0, zero, .getNumPos.bb4
.getNumPos.bb3:
	addi	s1, sp, 0
	lw	s2, 0(s1)
	sraiw	t0, s2, 31	# const-div.pow2.sign
	andi	t0, t0, 15	# const-div.pow2.bias
	addw	t0, s2, t0	# const-div.pow2.adjust
	sraiw	t0, t0, 4	# const-div.pow2.shift
	sw	t0, 0(s1)
	addi	s1, sp, 12
	lw	t0, 0(s1)
	addiw	t0, t0, 1
	sw	t0, 0(s1)
	j	.getNumPos.bb2
.getNumPos.bb4:
	addi	t0, sp, 0
	lw	t0, 0(t0)
	sraiw	t1, t0, 31	# const-div.pow2.sign
	andi	t1, t1, 15	# const-div.pow2.bias
	addw	t1, t0, t1	# const-div.pow2.adjust
	sraiw	t1, t1, 4	# const-div.pow2.shift
	slliw	t1, t1, 4	# const-rem.pow2.product
	subw	t0, t0, t1	# const-rem.pow2.sub
	addi	t1, sp, 8
	sw	t0, 0(t1)
.getNumPos.bb1:
	addi	t0, sp, 8
	lw	a0, 0(t0)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	addi	sp, sp, 32
	ret
	.size getNumPos, .-getNumPos

  .globl radixSort
  .type  radixSort, @function
radixSort:
.radixSort.bb0:
	addi	sp, sp, -304
	sd	ra, 256(sp)
	sd	s1, 264(sp)
	sd	s2, 272(sp)
	sd	s3, 280(sp)
	sd	s4, 288(sp)
	sd	s5, 296(sp)
	mv	t0, a0
	mv	s1, a1
	mv	t1, a2
	mv	t2, a3
	addi	t3, sp, 32
	sw	t0, 0(t3)
	addi	t0, sp, 36
	sw	t1, 0(t0)
	addi	t1, sp, 40
	sw	t2, 0(t1)
	addi	t5, sp, 64
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	addi	t5, sp, 128
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	addi	t5, sp, 192
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	lw	t2, 0(t3)
	li	t3, 1
	subw	t3, zero, t3
	subw	t2, t2, t3
	sltiu	t2, t2, 1
	lw	t0, 0(t0)
	addiw	t0, t0, 1
	lw	t1, 0(t1)
	slt	t0, t0, t1
	xori	t0, t0, 1
	or	t0, t2, t0
	bne	t0, zero, .radixSort.bb1
	j	.radixSort.bb3
.radixSort.bb2:
.radixSort.bb1:
	ld	s5, 296(sp)
	ld	s4, 288(sp)
	ld	s3, 280(sp)
	ld	s2, 272(sp)
	ld	s1, 264(sp)
	ld	ra, 256(sp)
	addi	sp, sp, 304
	ret
.radixSort.bb16:
	addi	t0, sp, 52
	lw	a0, 0(t0)
	addi	t1, sp, 32
	lw	a1, 0(t1)
	call	getNumPos
	mv	t0, a0
	addi	t1, sp, 44
	lw	t1, 0(t1)
	subw	t0, t0, t1
	sltu	t0, zero, t0
	beq	t0, zero, .radixSort.bb18
.radixSort.bb17:
	addi	s2, sp, 52
	lw	t0, 0(s2)
	addi	s3, sp, 56
	sw	t0, 0(s3)
	lw	a0, 0(s3)
	addi	s4, sp, 32
	lw	a1, 0(s4)
	call	getNumPos
	mv	t0, a0
	li	t1, 4
	mulw	t0, t0, t1
	addi	s5, sp, 64
	add	t0, s5, t0
	lw	t0, 0(t0)
	mulw	t0, t0, t1
	add	t0, s1, t0
	lw	t0, 0(t0)
	sw	t0, 0(s2)
	lw	s2, 0(s3)
	lw	a0, 0(s3)
	lw	a1, 0(s4)
	call	getNumPos
	mv	t0, a0
	li	t1, 4
	mulw	t0, t0, t1
	add	t0, s5, t0
	lw	t0, 0(t0)
	mulw	t0, t0, t1
	add	t0, s1, t0
	sw	s2, 0(t0)
	lw	a0, 0(s3)
	lw	t1, 0(s4)
	mv	a1, t1	# call-arg-copy
	call	getNumPos
	mv	t0, a0
	li	t1, 4
	mulw	t0, t0, t1
	add	t0, s5, t0
	lw	t0, 0(t0)
	addiw	s2, t0, 1
	lw	t0, 0(s3)
	lw	t1, 0(s4)
	mv	a0, t0	# call-arg-copy
	mv	a1, t1	# call-arg-copy
	call	getNumPos
	mv	t0, a0
	li	t1, 4
	mulw	t0, t0, t1
	add	t0, s5, t0
	sw	s2, 0(t0)
	j	.radixSort.bb16
.radixSort.bb18:
	addi	s2, sp, 44
	li	t2, 4
	lw	t1, 0(s2)
	addi	s3, sp, 64
	addi	t0, sp, 52
	lw	t0, 0(t0)
	mulw	t1, t1, t2
	add	t1, s3, t1
	lw	t1, 0(t1)
	mulw	t1, t1, t2
	add	t1, s1, t1
	sw	t0, 0(t1)
	lw	t0, 0(s2)
	li	t1, 4
	mulw	t0, t0, t1
	lw	t1, 0(s2)
	add	t0, s3, t0
	lw	t0, 0(t0)
	mulw	t1, t1, t2
	addiw	t0, t0, 1
	add	t1, s3, t1
	sw	t0, 0(t1)
.radixSort.bb13:
	addi	s2, sp, 44
	li	t1, 4
	lw	t0, 0(s2)
	li	t2, 4
	mulw	t0, t0, t1
	addi	t1, sp, 64
	add	t0, t1, t0
	lw	t1, 0(s2)
	lw	t0, 0(t0)
	mulw	t1, t1, t2
	addi	t2, sp, 128
	add	t1, t2, t1
	lw	t1, 0(t1)
	slt	t0, t0, t1
	beq	t0, zero, .radixSort.bb15
.radixSort.bb14:
	addi	t0, sp, 44
	li	t1, 4
	lw	t0, 0(t0)
	mulw	t0, t0, t1
	addi	t1, sp, 64
	add	t0, t1, t0
	lw	t0, 0(t0)
	li	t1, 4
	mulw	t0, t0, t1
	addi	t1, sp, 52
	add	t0, s1, t0
	lw	t0, 0(t0)
	sw	t0, 0(t1)
	j	.radixSort.bb16
.radixSort.bb15:
	addi	s2, sp, 44
	lw	t0, 0(s2)
	addiw	t0, t0, 1
	sw	t0, 0(s2)
.radixSort.bb10:
	addi	t0, sp, 44
	li	t1, 16
	lw	t0, 0(t0)
	slt	t0, t0, t1
	beq	t0, zero, .radixSort.bb12
.radixSort.bb11:
	j	.radixSort.bb13
.radixSort.bb12:
	addi	t0, sp, 36
	li	t3, 0
	lw	t1, 0(t0)
	addi	t2, sp, 48
	li	a2, 4
	mulw	t3, t3, a2
	sw	t1, 0(t2)
	lw	t1, 0(t0)
	addi	a2, sp, 64
	add	t3, a2, t3
	sw	t1, 0(t3)
	lw	t0, 0(t0)
	li	t1, 0
	li	t3, 4
	mulw	t1, t1, t3
	addi	t3, sp, 192
	add	t1, t3, t1
	lw	t1, 0(t1)
	li	t3, 4
	addw	t0, t0, t1
	li	t1, 0
	mulw	t1, t1, t3
	addi	t3, sp, 128
	add	t1, t3, t1
	sw	t0, 0(t1)
	li	t0, 0
	sw	t0, 0(t2)
.radixSort.bb19:
	addi	t0, sp, 48
	li	t1, 16
	lw	t0, 0(t0)
	slt	t0, t0, t1
	beq	t0, zero, .radixSort.bb1
.radixSort.bb20:
	addi	t0, sp, 48
	li	t1, 0
	lw	t0, 0(t0)
	slt	t0, t1, t0
	bne	t0, zero, .radixSort.bb22
.radixSort.bb23:
	addi	t0, sp, 32
	lw	t0, 0(t0)
	addiw	t0, t0, -1
	addi	s2, sp, 48
	lw	t1, 0(s2)
	li	t2, 4
	mulw	t1, t1, t2
	addi	t2, sp, 64
	add	t1, t2, t1
	lw	t1, 0(t1)
	lw	t2, 0(s2)
	li	t3, 4
	mulw	t2, t2, t3
	addi	t3, sp, 128
	add	t2, t3, t2
	lw	a3, 0(t2)
	mv	a0, t0	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	mv	a2, t1	# call-arg-copy
	call	radixSort
	lw	t0, 0(s2)
	addiw	t0, t0, 1
	sw	t0, 0(s2)
	j	.radixSort.bb19
.radixSort.bb22:
	addi	s2, sp, 48
	li	t1, 4
	lw	t0, 0(s2)
	addi	s3, sp, 128
	li	t2, 4
	addi	s4, sp, 64
	addiw	t0, t0, -1
	mulw	t0, t0, t1
	lw	t1, 0(s2)
	add	t0, s3, t0
	lw	t0, 0(t0)
	mulw	t1, t1, t2
	add	t1, s4, t1
	sw	t0, 0(t1)
	lw	t0, 0(s2)
	li	t1, 4
	mulw	t0, t0, t1
	lw	t1, 0(s2)
	add	t0, s4, t0
	lw	t0, 0(t0)
	mulw	t1, t1, t2
	addi	t2, sp, 192
	add	t1, t2, t1
	lw	t1, 0(t1)
	li	t2, 4
	addw	t0, t0, t1
	lw	t1, 0(s2)
	mulw	t1, t1, t2
	add	t1, s3, t1
	sw	t0, 0(t1)
	j	.radixSort.bb23
.radixSort.bb21:
	j	.radixSort.bb1
.radixSort.bb4:
	addi	t0, sp, 44
	addi	t1, sp, 40
	lw	t0, 0(t0)
	lw	t1, 0(t1)
	slt	t0, t0, t1
	beq	t0, zero, .radixSort.bb6
.radixSort.bb5:
	addi	s2, sp, 44
	lw	t0, 0(s2)
	li	t1, 4
	mulw	t0, t0, t1
	add	t0, s1, t0
	lw	a0, 0(t0)
	addi	s3, sp, 32
	lw	a1, 0(s3)
	call	getNumPos
	mv	t0, a0
	li	t1, 4
	mulw	t0, t0, t1
	addi	s4, sp, 192
	add	t0, s4, t0
	lw	t0, 0(t0)
	addiw	s5, t0, 1
	lw	t0, 0(s2)
	mulw	t0, t0, t1
	add	t0, s1, t0
	lw	a0, 0(t0)
	lw	a1, 0(s3)
	call	getNumPos
	mv	t0, a0
	li	t1, 4
	mulw	t0, t0, t1
	add	t0, s4, t0
	sw	s5, 0(t0)
	lw	t0, 0(s2)
	addiw	t0, t0, 1
	sw	t0, 0(s2)
	j	.radixSort.bb4
.radixSort.bb6:
	li	t2, 0
	li	t3, 4
	addi	t0, sp, 36
	lw	t1, 0(t0)
	mulw	t2, t2, t3
	addi	t3, sp, 64
	add	t2, t3, t2
	sw	t1, 0(t2)
	lw	t0, 0(t0)
	li	t1, 0
	li	t2, 4
	mulw	t1, t1, t2
	addi	t2, sp, 192
	add	t1, t2, t1
	lw	t1, 0(t1)
	li	t2, 4
	addw	t0, t0, t1
	li	t1, 0
	mulw	t1, t1, t2
	addi	t2, sp, 128
	add	t1, t2, t1
	sw	t0, 0(t1)
	li	t0, 1
	addi	t1, sp, 44
	sw	t0, 0(t1)
.radixSort.bb7:
	addi	t0, sp, 44
	li	t1, 16
	lw	t0, 0(t0)
	slt	t0, t0, t1
	beq	t0, zero, .radixSort.bb9
.radixSort.bb8:
	addi	s2, sp, 44
	li	t1, 4
	lw	t0, 0(s2)
	addi	s3, sp, 128
	li	t2, 4
	addi	s4, sp, 64
	addiw	t0, t0, -1
	mulw	t0, t0, t1
	lw	t1, 0(s2)
	add	t0, s3, t0
	lw	t0, 0(t0)
	mulw	t1, t1, t2
	add	t1, s4, t1
	sw	t0, 0(t1)
	lw	t0, 0(s2)
	li	t1, 4
	mulw	t0, t0, t1
	lw	t1, 0(s2)
	add	t0, s4, t0
	lw	t0, 0(t0)
	mulw	t1, t1, t2
	addi	t2, sp, 192
	add	t1, t2, t1
	lw	t1, 0(t1)
	li	t2, 4
	addw	t0, t0, t1
	lw	t1, 0(s2)
	mulw	t1, t1, t2
	add	t1, s3, t1
	sw	t0, 0(t1)
	lw	t0, 0(s2)
	addiw	t0, t0, 1
	sw	t0, 0(s2)
	j	.radixSort.bb7
.radixSort.bb9:
	li	t0, 0
	addi	t1, sp, 44
	sw	t0, 0(t1)
	j	.radixSort.bb10
.radixSort.bb3:
	addi	t0, sp, 36
	addi	t1, sp, 44
	lw	t0, 0(t0)
	sw	t0, 0(t1)
	j	.radixSort.bb4
	.size radixSort, .-radixSort

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -80
	sd	ra, 48(sp)
	sd	s1, 56(sp)
	sd	s2, 64(sp)
	la	t5, a
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	t0, a0
	addi	s1, sp, 36
	sw	t0, 0(s1)
	li	t5, 90
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	lw	t0, 0(s1)
	li	t5, 8
	sd	t5, 0(sp)
	la	t5, a
	sd	t5, 8(sp)
	li	t5, 0
	sd	t5, 16(sp)
	mv	t5, t0
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	radixSort
	li	t0, 0
	addi	t1, sp, 40
	sw	t0, 0(t1)
.main.bb2:
	addi	t0, sp, 40
	addi	t1, sp, 36
	lw	t0, 0(t0)
	lw	t1, 0(t1)
	slt	t0, t0, t1
	beq	t0, zero, .main.bb4
.main.bb3:
	addi	s2, sp, 40
	li	t3, 4
	lw	t2, 0(s2)
	la	s1, ans
	lw	t1, 0(s2)
	lw	t0, 0(s1)
	mulw	t2, t2, t3
	la	t3, a
	add	t2, t3, t2
	lw	t3, 0(s2)
	lw	t2, 0(t2)
	addiw	t3, t3, 2
	remw	t2, t2, t3
	mulw	t1, t1, t2
	addw	t0, t0, t1
	sw	t0, 0(s1)
	lw	t0, 0(s2)
	addiw	t0, t0, 1
	sw	t0, 0(s2)
	j	.main.bb2
.main.bb4:
	la	t0, ans
	li	t1, 0
	lw	t0, 0(t0)
	slt	t0, t0, t1
	bne	t0, zero, .main.bb5
.main.bb6:
	li	a0, 102
	call	_sysy_stoptime
	la	t0, ans
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 0
	addi	t1, sp, 32
	sw	t0, 0(t1)
.main.bb1:
	addi	t0, sp, 32
	lw	a0, 0(t0)
	ld	s2, 64(sp)
	ld	s1, 56(sp)
	ld	ra, 48(sp)
	addi	sp, sp, 80
	ret
.main.bb5:
	la	t0, ans
	lw	t1, 0(t0)
	subw	t1, zero, t1
	sw	t1, 0(t0)
	j	.main.bb6
	.size main, .-main
