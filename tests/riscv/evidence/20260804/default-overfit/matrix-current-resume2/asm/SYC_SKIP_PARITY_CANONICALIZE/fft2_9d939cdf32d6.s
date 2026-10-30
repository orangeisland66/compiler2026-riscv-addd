
  .section .rodata
  .globl mod
  .align 2
  .type  mod, @object
  .size  mod, 4
mod:
  .word 998244353

  .bss
  .globl d
  .align 2
  .type  d, @object
  .size  d, 4
d:
  .zero 4

  .section .rodata
  .globl maxlen
  .align 2
  .type  maxlen, @object
  .size  maxlen, 4
maxlen:
  .word 2097152

  .bss
  .globl temp
  .align 2
  .type  temp, @object
  .size  temp, 8388608
temp:
  .zero 8388608

  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 8388608
a:
  .zero 8388608

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 8388608
b:
  .zero 8388608

  .bss
  .globl c
  .align 2
  .type  c, @object
  .size  c, 8388608
c:
  .zero 8388608

  .text

  .globl multiply
  .type  multiply, @function
multiply:
.multiply.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	mv	s1, a0
	mv	t0, a1
	beq	t0, zero, .multiply.bb2
.multiply.bb3:
	li	t1, 1
	beq	t0, t1, .multiply.bb4
.multiply.bb5:
	srliw	s2, t0, 31	# const-div.pow2.bias-fold
	addw	s3, t0, s2
	sraiw	a1, s3, 1
	mv	a0, s1	# call-arg-copy
	call	multiply
	mv	t0, a0
	addw	t0, t0, t0
	li	t1, 288737297
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 26	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244353
	mulw	t1, t1, t2	# const-rem.mul
	subw	t0, t0, t1	# const-rem.sub
	andi	t1, s3, 1
	subw	t1, t1, s2
	li	t2, 1
	beq	t1, t2, .multiply.bb6
.multiply.bb7:
.multiply.bb1:
	mv	a0, t0
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.multiply.bb4:
	li	t0, 288737297
	mul	t0, s1, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 26	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 998244353
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s1, t0	# const-rem.sub
	j	.multiply.bb1
.multiply.bb6:
	addw	t0, t0, s1
	li	t1, 288737297
	mul	t1, t0, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 26	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244353
	mulw	t1, t1, t2	# const-rem.mul
	subw	t0, t0, t1	# const-rem.sub
	j	.multiply.bb1
.multiply.bb2:
	li	t0, 0
	j	.multiply.bb1
	.size multiply, .-multiply

  .globl power
  .type  power, @function
power:
.power.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	mv	s1, a0
	mv	t0, a1
	beq	t0, zero, .power.bb2
.power.bb3:
	srliw	s2, t0, 31	# const-div.pow2.bias-fold
	addw	s3, t0, s2
	sraiw	a1, s3, 1
	mv	a0, s1	# call-arg-copy
	call	power
	mv	t0, a0
	mv	a0, t0	# call-arg-copy
	mv	a1, t0	# call-arg-copy
	call	multiply
	mv	t0, a0
	andi	t1, s3, 1
	subw	t1, t1, s2
	li	t2, 1
	beq	t1, t2, .power.bb4
.power.bb5:
.power.bb1:
	mv	a0, t0
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.power.bb4:
	mv	a0, t0	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	call	multiply
	mv	t0, a0
	j	.power.bb1
.power.bb2:
	li	t0, 1
	j	.power.bb1
	.size power, .-power

  .globl fft
  .type  fft, @function
fft:
.fft.bb0:
	addi	sp, sp, -128
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	sd	s6, 80(sp)
	sd	s7, 88(sp)
	sd	s8, 96(sp)
	sd	s9, 104(sp)
	sd	s10, 112(sp)
	sd	s11, 120(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	mv	s4, a3
	li	t0, 1
	bne	s3, t0, .fft.bb3
.fft.bb2:
	li	t0, 1
.fft.bb1:
	mv	a0, t0
	ld	s11, 120(sp)
	ld	s10, 112(sp)
	ld	s9, 104(sp)
	ld	s8, 96(sp)
	ld	s7, 88(sp)
	ld	s6, 80(sp)
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 128
	ret
.fft.bb5:
	srliw	s7, t0, 31	# const-div.pow2.bias-fold
	addw	s8, t0, s7
	andi	t2, s8, 1
	subw	t2, t2, s7
	bne	t2, zero, .fft.bb8
.fft.bb7:
	sraiw	t3, s8, 1
	slliw	t3, t3, 2
	add	t3, s6, t3
	lw	t2, 0(t1)
	sw	t2, 0(t3)
.fft.bb9:
	addiw	t0, t0, 1
	addi	t1, t1, 4	# loop.addr.bump
	blt	t0, s3, .fft.bb5	# rotated-loop-branch
.fft.bb6:
	slliw	t1, s2, 2
	la	t0, temp
	add	t1, s1, t1
	li	t2, 0
.fft.bb13:
	bge	t2, s3, .fft.bb15
.fft.bb14:
	lw	t3, 0(t0)
	addiw	t2, t2, 1
	sw	t3, 0(t1)
	addi	t0, t0, 4
	addi	t1, t1, 4
	blt	t2, s3, .fft.bb14	# rotated-loop-branch
.fft.bb15:
	mv	a0, s4	# call-arg-copy
	mv	a1, s4	# call-arg-copy
	call	multiply
	mv	s3, a0
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s5	# call-arg-copy
	mv	a3, s3	# call-arg-copy
	call	fft
	addw	a1, s2, s5
	mv	a0, s1	# call-arg-copy
	mv	a2, s5	# call-arg-copy
	mv	a3, s3	# call-arg-copy
	call	fft
	li	s6, 1
	li	s3, 0
.fft.bb10:
	bge	s3, s5, .fft.bb12
.fft.bb11:
	addw	s7, s2, s3
	slliw	t0, s7, 2
	add	s8, s1, t0
	lw	s9, 0(s8)
	addw	t0, s7, s5
	slliw	t0, t0, 2
	add	s7, s1, t0
	lw	a1, 0(s7)
	mv	a0, s6	# call-arg-copy
	call	multiply
	mv	s10, a0
	addw	s11, s9, s10
	li	t0, 288737297
	mul	t0, s11, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 26	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 998244353
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s11, t0	# const-rem.sub
	sw	t0, 0(s8)
	subw	t0, s9, s10
	addw	s8, t0, t1
	li	t0, 288737297
	mul	t0, s8, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 26	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 998244353
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s8, t0	# const-rem.sub
	sw	t0, 0(s7)
	mv	a0, s6	# call-arg-copy
	mv	a1, s4	# call-arg-copy
	call	multiply
	mv	t0, a0
	addiw	s3, s3, 1
	mv	s6, t0	# phi-copy
	blt	s3, s5, .fft.bb11	# rotated-loop-branch
.fft.bb12:
	li	t0, 0
	j	.fft.bb1
.fft.bb8:
	sraiw	t3, s8, 1
	addw	t3, s5, t3
	slliw	t3, t3, 2
	lw	t2, 0(t1)
	add	t3, s6, t3
	sw	t2, 0(t3)
	j	.fft.bb9
.fft.bb3:
	la	s6, temp	# loop.addr.hoisted
	srliw	t0, s3, 31	# const-div.pow2.bias-fold
	addw	t0, s3, t0
	sraiw	s5, t0, 1
	li	t0, 0
	addw	t1, t0, s2	# loop.addr.init
	slliw	t1, t1, 2	# loop.addr.init
	add	t1, s1, t1	# loop.addr.init
.fft.bb4:
	bge	t0, s3, .fft.bb6
	j	.fft.bb5
	.size fft, .-fft

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -96
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	sd	s6, 80(sp)
	la	t5, a
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s1, a0
	la	t5, b
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s2, a0
	li	t5, 60
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	li	t0, 1
	la	t1, d
	sw	t0, 0(t1)
	addw	t0, s1, s2
	addiw	s1, t0, -1
	la	s2, d	# loop.addr.hoisted
.main.bb1:
	lw	s3, 0(s2)
	bge	s3, s1, .main.bb3
.main.bb2:
	slliw	t0, s3, 1
	sw	t0, 0(s2)
	j	.main.bb1
.main.bb3:
	li	t0, 998244352
	divw	s2, t0, s3
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	power.spec.arg0.i32_p5
	mv	s4, a0
	la	t5, a
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	mv	t5, s3
	sd	t5, 16(sp)
	mv	t5, s4
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	fft
	la	t5, b
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	mv	t5, s3
	sd	t5, 16(sp)
	mv	t5, s4
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	fft
	li	s4, 0
.main.bb4:
	bge	s4, s3, .main.bb6
.main.bb5:
	slliw	s5, s4, 2
	la	t0, a
	add	s6, t0, s5
	lw	t5, 0(s6)
	la	t1, b
	add	t1, t1, s5
	lw	t1, 0(t1)
	sd	t5, 0(sp)
	mv	t5, t1
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	multiply
	mv	t0, a0
	sw	t0, 0(s6)
	addiw	s4, s4, 1
	blt	s4, s3, .main.bb5	# rotated-loop-branch
.main.bb6:
	li	t0, 998244352
	subw	t5, t0, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	power.spec.arg0.i32_p5
	mv	t0, a0
	la	t5, a
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	mv	t5, s3
	sd	t5, 16(sp)
	mv	t5, t0
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	fft
	la	t0, d
	lw	s2, 0(t0)
	mv	t5, s2
	sd	t5, 0(sp)
	li	t5, 998244351
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	power
	mv	s3, a0
	li	s4, 0
.main.bb7:
	bge	s4, s2, .main.bb9
.main.bb8:
	slliw	t0, s4, 2
	la	t1, a
	add	s5, t1, t0
	lw	t5, 0(s5)
	sd	t5, 0(sp)
	mv	t5, s3
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	multiply
	mv	t0, a0
	sw	t0, 0(s5)
	addiw	s4, s4, 1
	blt	s4, s2, .main.bb8	# rotated-loop-branch
.main.bb9:
	li	a0, 79
	call	_sysy_stoptime
	mv	t5, s1
	sd	t5, 0(sp)
	la	t5, a
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	putarray
	li	a0, 0
	ld	s6, 80(sp)
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 96
	ret
	.size main, .-main

  .globl power.spec.arg0.i32_p5
  .type  power.spec.arg0.i32_p5, @function
power.spec.arg0.i32_p5:
.power.spec.arg0.i32_p5.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	mv	t0, a0
	beq	t0, zero, .power.spec.arg0.i32_p5.bb2
.power.spec.arg0.i32_p5.bb3:
	srliw	s1, t0, 31	# const-div.pow2.bias-fold
	addw	s2, t0, s1
	sraiw	a0, s2, 1
	call	power.spec.arg0.i32_p5
	mv	t0, a0
	mv	a0, t0	# call-arg-copy
	mv	a1, t0	# call-arg-copy
	call	multiply
	mv	t0, a0
	andi	t1, s2, 1
	subw	t1, t1, s1
	li	t2, 1
	beq	t1, t2, .power.spec.arg0.i32_p5.bb4
.power.spec.arg0.i32_p5.bb5:
.power.spec.arg0.i32_p5.bb1:
	mv	a0, t0
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.power.spec.arg0.i32_p5.bb4:
	mv	a0, t0	# call-arg-copy
	li	a1, 5
	call	multiply
	mv	t0, a0
	j	.power.spec.arg0.i32_p5.bb1
.power.spec.arg0.i32_p5.bb2:
	li	t0, 1
	j	.power.spec.arg0.i32_p5.bb1
	.size power.spec.arg0.i32_p5, .-power.spec.arg0.i32_p5
