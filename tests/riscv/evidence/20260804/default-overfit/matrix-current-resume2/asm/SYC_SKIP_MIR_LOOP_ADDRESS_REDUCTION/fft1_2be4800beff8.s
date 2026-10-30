
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

  .globl power
  .type  power, @function
power:
.power.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	mv	s1, a0
	mv	s2, a1
	beq	s2, zero, .power.bb2
.power.bb3:
	srliw	t0, s2, 31	# const-div.pow2.bias-fold
	addw	t0, s2, t0
	sraiw	a1, t0, 1
	mv	a0, s1	# call-arg-copy
	call	power
	mv	t0, a0
	li	t1, 0
	slt	t1, t0, t1
	xori	t1, t1, 1
	li	t2, 998244353
	slt	t2, t0, t2
	and	t2, t1, t2
	and	t1, t2, t1
	beqz	t1, .power.bb7
.power.bb6:
	mul	t0, t0, t0	# modmul.wide.mul
	li	t1, 998244353
	rem	t0, t0, t1	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
.power.bb8:
	andi	t1, s2, 1
	beq	t1, zero, .power.bb1
.power.bb4:
	li	t1, 0
	li	t2, 0
	slt	t1, t0, t1
	li	t3, 998244353
	xori	t1, t1, 1
	slt	t2, s1, t2
	slt	t3, t0, t3
	xori	t2, t2, 1
	and	t1, t1, t3
	and	t1, t1, t2
	beqz	t1, .power.bb10
.power.bb9:
	mul	t0, t0, s1	# modmul.wide.mul
	li	t1, 998244353
	rem	t0, t0, t1	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
	j	.power.bb1
.power.bb11:
.power.bb1:
	mv	a0, t0
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.power.bb5:
	j	.power.bb1
.power.bb7:
	mv	a0, t0	# call-arg-copy
	mv	a1, t0	# call-arg-copy
	call	multiply.modmul.fallback
	mv	t0, a0
	j	.power.bb8
.power.bb10:
	mv	a0, t0	# call-arg-copy
	mv	a1, s1	# call-arg-copy
	call	multiply.modmul.fallback
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
	addi	sp, sp, -144
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	sd	s2, 56(sp)
	sd	s3, 64(sp)
	sd	s4, 72(sp)
	sd	s5, 80(sp)
	sd	s6, 88(sp)
	sd	s7, 96(sp)
	sd	s8, 104(sp)
	sd	s9, 112(sp)
	sd	s10, 120(sp)
	sd	s11, 128(sp)
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
	ld	s11, 128(sp)
	ld	s10, 120(sp)
	ld	s9, 112(sp)
	ld	s8, 104(sp)
	ld	s7, 96(sp)
	ld	s6, 88(sp)
	ld	s5, 80(sp)
	ld	s4, 72(sp)
	ld	s3, 64(sp)
	ld	s2, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 144
	ret
.fft.bb13:
	bge	t2, s3, .fft.bb15
.fft.bb25:
	addiw	t3, t2, 3
	blt	t3, s3, .fft.bb26
.fft.bb14:
	lw	t3, 0(t0)
	addiw	t2, t2, 1
	sw	t3, 0(t1)
	addi	t0, t0, 4
	addi	t1, t1, 4
	j	.fft.bb13
.fft.bb26:
	lw	t3, 0(t0)
	addi	a2, t1, 4
	addi	a2, a2, 4
	addi	a2, a2, 4
	sw	t3, 0(t1)
	lw	a3, 4(t0)
	addi	t3, t0, 4
	addi	t3, t3, 4
	addi	t3, t3, 4
	sw	a3, 4(t1)
	lw	a3, 8(t0)
	sw	a3, 8(t1)
	lw	t0, 12(t0)
	sw	t0, 12(t1)
	addiw	t2, t2, 4
	addi	t0, t3, 4
	addi	t1, a2, 4
	j	.fft.bb13
.fft.bb15:
	li	t0, 0
	slt	t0, s4, t0
	xori	t5, t0, 1
	sd	t5, 32(sp)
	li	t0, 998244353
	slt	t0, s4, t0
	and	t0, t5, t0
	and	t0, t0, t5
	beqz	t0, .fft.bb20
.fft.bb19:
	mul	t1, s4, s4	# modmul.wide.mul
	li	t0, 998244353
	rem	t0, t1, t0	# modmul.wide.rem
	li	t1, 0
	addw	s6, t0, t1	# modmul.narrow
.fft.bb21:
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s5	# call-arg-copy
	mv	a3, s6	# call-arg-copy
	call	fft
	addw	a1, s2, s5
	mv	a0, s1	# call-arg-copy
	mv	a2, s5	# call-arg-copy
	mv	a3, s6	# call-arg-copy
	call	fft
	li	s7, 1
	li	s6, 0
.fft.bb10:
	bge	s6, s5, .fft.bb12
.fft.bb11:
	addw	s8, s2, s6
	li	t1, 0
	slliw	t0, s8, 2
	li	t2, 998244353
	add	s9, s1, t0
	slt	t2, s7, t2
	lw	s10, 0(s9)
	addw	t0, s8, s5
	slliw	t0, t0, 2
	add	s8, s1, t0
	lw	s11, 0(s8)
	li	t0, 0
	slt	t0, s7, t0
	xori	t0, t0, 1
	slt	t1, s11, t1
	and	s3, t0, t2
	xori	t1, t1, 1
	and	t0, s3, t1
	beqz	t0, .fft.bb17
.fft.bb16:
	mul	t1, s7, s11	# modmul.wide.mul
	li	t0, 998244353
	rem	t0, t1, t0	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
.fft.bb18:
	addw	s11, s10, t0
	li	t1, 288737297
	mul	t1, s11, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 26	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244353
	mulw	t1, t1, t2	# const-rem.mul
	subw	t1, s11, t1	# const-rem.sub
	sw	t1, 0(s9)
	subw	t0, s10, t0
	li	t1, 998244353
	addw	s9, t0, t1
	li	t0, 288737297
	mul	t0, s9, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 26	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 998244353
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s9, t0	# const-rem.sub
	sw	t0, 0(s8)
	ld	t5, 32(sp)
	and	t0, s3, t5
	beqz	t0, .fft.bb23
.fft.bb22:
	mul	t1, s7, s4	# modmul.wide.mul
	li	t0, 998244353
	rem	t0, t1, t0	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
.fft.bb24:
	addiw	s6, s6, 1
	mv	s7, t0	# phi-copy
	blt	s6, s5, .fft.bb11	# rotated-loop-branch
.fft.bb12:
	li	t0, 0
	j	.fft.bb1
.fft.bb17:
	mv	a0, s7	# call-arg-copy
	mv	a1, s11	# call-arg-copy
	call	multiply.modmul.fallback
	mv	t0, a0
	j	.fft.bb18
.fft.bb23:
	mv	a0, s7	# call-arg-copy
	mv	a1, s4	# call-arg-copy
	call	multiply.modmul.fallback
	mv	t0, a0
	j	.fft.bb24
.fft.bb5:
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	s7, t0, t1
	andi	t1, t0, 1
	bne	t1, zero, .fft.bb8
.fft.bb7:
	addw	t1, t0, s2
	sraiw	t2, s7, 1
	slliw	t1, t1, 2
	slliw	t2, t2, 2
	add	t1, s1, t1
	add	t2, s6, t2
	lw	t1, 0(t1)
	sw	t1, 0(t2)
.fft.bb9:
	addiw	t0, t0, 1
	blt	t0, s3, .fft.bb5	# rotated-loop-branch
.fft.bb6:
	slliw	t1, s2, 2
	la	t0, temp
	add	t1, s1, t1
	li	t2, 0
	j	.fft.bb13
.fft.bb8:
	addw	t1, t0, s2
	sraiw	t2, s7, 1
	slliw	t1, t1, 2
	addw	t2, s5, t2
	add	t1, s1, t1
	slliw	t2, t2, 2
	lw	t1, 0(t1)
	add	t2, s6, t2
	sw	t1, 0(t2)
	j	.fft.bb9
.fft.bb20:
	mv	a0, s4	# call-arg-copy
	mv	a1, s4	# call-arg-copy
	call	multiply.modmul.fallback
	mv	t0, a0
	mv	s6, t0	# phi-copy
	j	.fft.bb21
.fft.bb3:
	la	s6, temp	# loop.addr.hoisted
	srliw	t0, s3, 31	# const-div.pow2.bias-fold
	addw	t0, s3, t0
	sraiw	s5, t0, 1
	li	t0, 0
.fft.bb4:
	bge	t0, s3, .fft.bb6
	j	.fft.bb5
	.size fft, .-fft

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -112
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	sd	s6, 80(sp)
	sd	s7, 88(sp)
	sd	s8, 96(sp)
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
.main.bb5:
	lw	s7, 0(s5)
	slliw	t0, s6, 2
	la	t1, b
	li	t2, 998244353
	add	t0, t1, t0
	lw	s8, 0(t0)
	slt	t2, s7, t2
	li	t1, 0
	li	t0, 0
	slt	t1, s8, t1
	slt	t0, s7, t0
	xori	t1, t1, 1
	xori	t0, t0, 1
	and	t0, t0, t2
	and	t0, t0, t1
	beqz	t0, .main.bb11
.main.bb10:
	mul	t1, s7, s8	# modmul.wide.mul
	li	t0, 998244353
	rem	t0, t1, t0	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
.main.bb12:
	sw	t0, 0(s5)
	addi	s5, s5, 4
	addiw	s6, s6, 1
	blt	s6, s3, .main.bb5	# rotated-loop-branch
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
	li	t0, 0
	slt	t0, s3, t0
	xori	s5, t0, 1
	li	s6, 0
.main.bb7:
	bge	s6, s2, .main.bb9
.main.bb8:
	lw	s7, 0(s4)
	li	t0, 0
	li	t1, 998244353
	slt	t0, s7, t0
	slt	t1, s7, t1
	xori	t0, t0, 1
	and	t0, t0, t1
	and	t0, t0, s5
	beqz	t0, .main.bb14
.main.bb13:
	mul	t1, s7, s3	# modmul.wide.mul
	li	t0, 998244353
	rem	t0, t1, t0	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
.main.bb15:
	sw	t0, 0(s4)
	addi	s4, s4, 4
	addiw	s6, s6, 1
	blt	s6, s2, .main.bb8	# rotated-loop-branch
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
	ld	s8, 96(sp)
	ld	s7, 88(sp)
	ld	s6, 80(sp)
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 112
	ret
.main.bb11:
	mv	t5, s7
	sd	t5, 0(sp)
	mv	t5, s8
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	multiply.modmul.fallback
	mv	t0, a0
	j	.main.bb12
.main.bb14:
	mv	t5, s7
	sd	t5, 0(sp)
	mv	t5, s3
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	multiply.modmul.fallback
	mv	t0, a0
	j	.main.bb15
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
	la	s4, a
	li	s6, 0
	mv	s5, s4	# phi-copy
.main.bb4:
	bge	s6, s3, .main.bb6
	j	.main.bb5
	.size main, .-main

  .globl multiply.modmul.fallback
  .type  multiply.modmul.fallback, @function
multiply.modmul.fallback:
.multiply.modmul.fallback.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	mv	s1, a0
	mv	s2, a1
	beq	s2, zero, .multiply.modmul.fallback.bb2
.multiply.modmul.fallback.bb3:
	li	t0, 1
	beq	s2, t0, .multiply.modmul.fallback.bb4
.multiply.modmul.fallback.bb5:
	srliw	t0, s2, 31	# const-div.pow2.bias-fold
	addw	t0, s2, t0
	sraiw	a1, t0, 1
	mv	a0, s1	# call-arg-copy
	call	multiply.modmul.fallback
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
	andi	t1, s2, 1
	bne	t1, zero, .multiply.modmul.fallback.bb6
.multiply.modmul.fallback.bb7:
.multiply.modmul.fallback.bb1:
	mv	a0, t0
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.multiply.modmul.fallback.bb4:
	li	t0, 288737297
	mul	t0, s1, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 26	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 998244353
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, s1, t0	# const-rem.sub
	j	.multiply.modmul.fallback.bb1
.multiply.modmul.fallback.bb6:
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
	j	.multiply.modmul.fallback.bb1
.multiply.modmul.fallback.bb2:
	li	t0, 0
	j	.multiply.modmul.fallback.bb1
	.size multiply.modmul.fallback, .-multiply.modmul.fallback

  .globl power.spec.arg0.i32_p5
  .type  power.spec.arg0.i32_p5, @function
power.spec.arg0.i32_p5:
.power.spec.arg0.i32_p5.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	mv	s1, a0
	beq	s1, zero, .power.spec.arg0.i32_p5.bb2
.power.spec.arg0.i32_p5.bb3:
	srliw	t0, s1, 31	# const-div.pow2.bias-fold
	addw	t0, s1, t0
	sraiw	a0, t0, 1
	call	power.spec.arg0.i32_p5
	mv	t0, a0
	li	t1, 0
	slt	t1, t0, t1
	xori	t1, t1, 1
	li	t2, 998244353
	slt	t2, t0, t2
	and	t2, t1, t2
	and	t1, t2, t1
	beqz	t1, .power.spec.arg0.i32_p5.bb7
.power.spec.arg0.i32_p5.bb6:
	mul	t0, t0, t0	# modmul.wide.mul
	li	t1, 998244353
	rem	t0, t0, t1	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
.power.spec.arg0.i32_p5.bb8:
	andi	t1, s1, 1
	beq	t1, zero, .power.spec.arg0.i32_p5.bb1
.power.spec.arg0.i32_p5.bb4:
	li	t1, 0
	li	t2, 998244353
	slt	t1, t0, t1
	slt	t2, t0, t2
	xori	t1, t1, 1
	and	t1, t1, t2
	beqz	t1, .power.spec.arg0.i32_p5.bb10
.power.spec.arg0.i32_p5.bb9:
	li	t1, 5
	li	t2, 998244353
	mul	t0, t0, t1	# modmul.wide.mul
	rem	t0, t0, t2	# modmul.wide.rem
	li	t1, 0
	addw	t0, t0, t1	# modmul.narrow
	j	.power.spec.arg0.i32_p5.bb1
.power.spec.arg0.i32_p5.bb11:
.power.spec.arg0.i32_p5.bb1:
	mv	a0, t0
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
.power.spec.arg0.i32_p5.bb5:
	j	.power.spec.arg0.i32_p5.bb1
.power.spec.arg0.i32_p5.bb7:
	mv	a0, t0	# call-arg-copy
	mv	a1, t0	# call-arg-copy
	call	multiply.modmul.fallback
	mv	t0, a0
	j	.power.spec.arg0.i32_p5.bb8
.power.spec.arg0.i32_p5.bb10:
	mv	a0, t0	# call-arg-copy
	li	a1, 5
	call	multiply.modmul.fallback
	mv	t0, a0
	j	.power.spec.arg0.i32_p5.bb1
.power.spec.arg0.i32_p5.bb2:
	li	t0, 1
	j	.power.spec.arg0.i32_p5.bb1
	.size power.spec.arg0.i32_p5, .-power.spec.arg0.i32_p5
