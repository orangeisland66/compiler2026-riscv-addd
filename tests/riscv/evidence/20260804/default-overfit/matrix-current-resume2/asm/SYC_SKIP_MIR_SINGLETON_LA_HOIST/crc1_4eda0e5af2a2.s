
  .bss
  .globl crc32table
  .align 2
  .type  crc32table, @object
  .size  crc32table, 1024
crc32table:
  .zero 1024

  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 400080
a:
  .zero 400080

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	la	t5, a
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	getarray
	mv	s1, a0
	la	t0, crc32table
	li	t1, 0
	li	t2, 256
.main.bb1:
	bge	t1, t2, .main.bb3
.main.bb2:
	li	t3, 19260817
	addw	t3, t1, t3
	sw	t3, 0(t0)
	addiw	t1, t1, 1
	addi	t0, t0, 4
	blt	t1, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t5, 68
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	_sysy_starttime
	bne	s1, zero, .main.bb4
.main.bb5:
	li	a0, 73
	call	_sysy_stoptime
	addi	t0, sp, 8
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
.main.bb7:
	lw	a3, 0(t0)
	andi	a2, t2, 255
	addiw	t1, t1, 1
	xor	a2, a2, a3
	addi	t0, t0, 4
	sraiw	a3, t2, 31
	slliw	a2, a2, 2
	andi	a3, a3, 255
	addw	t2, t2, a3
	la	a3, crc32table
	sraiw	t2, t2, 8
	add	a2, a3, a2
	lw	a2, 0(a2)
	xor	t2, t2, a2
	blt	t1, t3, .main.bb7	# rotated-loop-branch
.main.bb8:
	li	t0, 935287351
	mul	t0, t2, t0	# const-div.fullmul
	srai	t0, t0, 32	# const-div.mulhi
	sraiw	t0, t0, 22	# const-div.shift
	srliw	t1, t0, 31	# const-div.sign
	addw	t0, t0, t1	# const-div.round
	li	t1, 19260817
	mulw	t0, t0, t1	# const-rem.mul
	subw	t0, t2, t0	# const-rem.sub
	addi	t1, sp, 8
	sw	t0, 0(t1)
	j	.main.bb5
.main.bb4:
	la	t0, a
	li	t1, 0
	li	t2, 0
	li	t3, 1
.main.bb6:
	bge	t1, t3, .main.bb8
	j	.main.bb7
	.size main, .-main
