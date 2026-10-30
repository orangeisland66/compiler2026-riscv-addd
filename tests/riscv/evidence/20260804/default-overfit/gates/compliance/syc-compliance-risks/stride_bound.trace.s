
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
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	li	s1, 0
	li	t1, 0
	li	t2, 0
	li	t3, 5
.main.bb1:
	bge	t1, t3, .main.bb3
.main.bb2:
	slliw	a2, t1, 1
	addiw	t0, s1, 1
	addw	a2, a2, t1
	addiw	t3, t3, -10
	addiw	a2, a2, 1
	addiw	t1, t1, 2
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
	mv	s1, t0	# phi-copy
	blt	t1, t3, .main.bb2	# rotated-loop-branch
.main.bb3:
	addw	a0, t2, s1
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	la	t0, state
	sw	s1, 0(t0)
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
