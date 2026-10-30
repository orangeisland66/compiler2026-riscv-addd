
  .bss
  .globl state
  .align 2
  .type  state, @object
  .size  state, 4
state:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	li	t0, 0
	li	t1, 1
	li	t2, 0
.main.bb1:
	li	t3, 8
	bge	t1, t3, .main.bb3
.main.bb12:
	addiw	s1, t1, 3
	addiw	s2, t1, 6
	slt	t3, t1, s1
	slt	a2, s1, s2
	li	a3, 8
	and	t3, t3, a2
	slt	a3, s1, a3
	and	t3, t3, a3
	bnez	t3, .main.bb13
.main.bb2:
	slliw	t3, t1, 1
	addiw	t0, t0, 1
	addw	t1, t3, t1
	addiw	t1, t1, 1
	addw	s2, t2, t1
	li	t1, -1985070077
	mul	t1, s2, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s2	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	subw	t2, s2, t1	# const-rem.sub
	mv	t1, s1	# phi-copy
	j	.main.bb1
.main.bb13:
	slliw	t3, t1, 1
	addiw	t0, t0, 2
	addw	t1, t3, t1
	addiw	t1, t1, 1
	addw	s3, t2, t1
	li	t1, -1985070077
	mul	t1, s3, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s3	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	slliw	t2, s1, 1
	subw	t1, s3, t1	# const-rem.sub
	addw	t2, t2, s1
	addiw	t2, t2, 1
	addw	s1, t1, t2
	li	t1, -1985070077
	mul	t1, s1, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s1	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	subw	t2, s1, t1	# const-rem.sub
	mv	t1, s2	# phi-copy
	j	.main.bb1
.main.bb4:
	li	t3, 10
	bge	t1, t3, .main.bb6
.main.bb14:
	addiw	s1, t1, 4
	addiw	s2, t1, 8
	slt	t3, t1, s1
	slt	a2, s1, s2
	li	a3, 10
	and	t3, t3, a2
	slt	a3, s1, a3
	and	t3, t3, a3
	bnez	t3, .main.bb15
.main.bb5:
	slliw	t3, t1, 1
	addiw	t0, t0, 1
	addw	t1, t3, t1
	addiw	t1, t1, 1
	addw	s2, t2, t1
	li	t1, -1985070077
	mul	t1, s2, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s2	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	subw	t2, s2, t1	# const-rem.sub
	mv	t1, s1	# phi-copy
	j	.main.bb4
.main.bb15:
	slliw	t3, t1, 1
	addiw	t0, t0, 2
	addw	t1, t3, t1
	addiw	t1, t1, 1
	addw	s3, t2, t1
	li	t1, -1985070077
	mul	t1, s3, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s3	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	slliw	t2, s1, 1
	subw	t1, s3, t1	# const-rem.sub
	addw	t2, t2, s1
	addiw	t2, t2, 1
	addw	s1, t1, t2
	li	t1, -1985070077
	mul	t1, s1, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s1	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	subw	t2, s1, t1	# const-rem.sub
	mv	t1, s2	# phi-copy
	j	.main.bb4
.main.bb9:
	li	t3, 1
	bge	t1, t3, .main.bb11
.main.bb17:
	addiw	s1, t1, -1
	addiw	s2, t1, -2
	slt	t3, t1, s1
	slt	a2, s1, s2
	and	t3, t3, a2
	bnez	t3, .main.bb18
.main.bb10:
	slliw	t3, t1, 1
	addiw	t0, t0, 1
	addw	t1, t3, t1
	addiw	t1, t1, 1
	addw	s2, t2, t1
	li	t1, -1985070077
	mul	t1, s2, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s2	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	subw	t2, s2, t1	# const-rem.sub
	mv	t1, s1	# phi-copy
	j	.main.bb9
.main.bb18:
	slliw	t3, t1, 1
	addiw	t0, t0, 2
	addw	t1, t3, t1
	addiw	t1, t1, 1
	addw	s3, t2, t1
	li	t1, -1985070077
	mul	t1, s3, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s3	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	slliw	t2, s1, 1
	subw	t1, s3, t1	# const-rem.sub
	addw	t2, t2, s1
	addiw	t2, t2, 1
	addw	s1, t1, t2
	li	t1, -1985070077
	mul	t1, s1, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	addw	t1, t1, s1	# const-div.fixup
	sraiw	t1, t1, 29	# const-div.shift
	srliw	t2, t1, 31	# const-div.sign
	addw	t1, t1, t2	# const-div.round
	li	t2, 998244853
	mulw	t1, t1, t2	# const-rem.mul
	subw	t2, s1, t1	# const-rem.sub
	mv	t1, s2	# phi-copy
	j	.main.bb9
.main.bb3:
	addw	t5, t2, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 0
	li	t1, 2
	li	t2, 0
	j	.main.bb4
.main.bb6:
	addw	t5, t2, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 0
	li	t1, 10
	li	t2, 0
	li	t3, 1
.main.bb7:
	bge	t1, t3, .main.bb8
.main.bb16:
	slliw	a2, t1, 1
	addiw	t0, t0, 1
	addw	a2, a2, t1
	addiw	a2, a2, 1
	addw	s1, t2, a2
	li	t2, -1985070077
	mul	t2, s1, t2	# const-div.fullmul
	srai	t2, t2, 32	# const-div.mulhi
	addw	t2, t2, s1	# const-div.fixup
	sraiw	t2, t2, 29	# const-div.shift
	srliw	a2, t2, 31	# const-div.sign
	addw	t2, t2, a2	# const-div.round
	li	a2, 998244853
	mulw	t2, t2, a2	# const-rem.mul
	subw	t2, s1, t2	# const-rem.sub
	blt	t1, t3, .main.bb16	# rotated-loop-branch
.main.bb8:
	addw	t5, t2, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 0
	li	t1, 10
	li	t2, 0
	j	.main.bb9
.main.bb11:
	addw	a0, t2, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 150993938
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 1
	la	t1, state
	sw	t0, 0(t1)
	li	a0, 0
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
