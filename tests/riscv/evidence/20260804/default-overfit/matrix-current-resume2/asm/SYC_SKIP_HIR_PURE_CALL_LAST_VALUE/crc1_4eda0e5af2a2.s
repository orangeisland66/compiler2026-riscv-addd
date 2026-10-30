
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
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
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
	la	t0, a
.main.bb4:
	beq	s1, zero, .main.bb6
.main.bb5:
	li	t1, 0
	li	t2, 0
	la	t3, crc32table	# loop.addr.hoisted
	mv	s2, t0	# phi-copy
.main.bb7:
	bge	t1, s1, .main.bb9
.main.bb8:
	lw	a3, 0(s2)
	andi	a2, t2, 255
	addiw	t1, t1, 1
	xor	a2, a2, a3
	slliw	a2, a2, 2
	sraiw	a3, t2, 31
	add	a2, t3, a2
	andi	a3, a3, 255
	lw	a2, 0(a2)
	addw	t2, t2, a3
	sraiw	t2, t2, 8
	xor	t2, t2, a2
	addi	s2, s2, 4
	blt	t1, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	li	t1, 935287351
	mul	t1, t2, t1	# const-div.fullmul
	srai	t1, t1, 32	# const-div.mulhi
	sraiw	t1, t1, 22	# const-div.shift
	srliw	t3, t1, 31	# const-div.sign
	addw	t1, t1, t3	# const-div.round
	li	t3, 19260817
	mulw	t1, t1, t3	# const-rem.mul
	subw	t1, t2, t1	# const-rem.sub
	addi	t2, sp, 8
	sw	t1, 0(t2)
	addiw	s1, s1, -1
	bne	s1, zero, .main.bb5	# rotated-loop-branch
.main.bb6:
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
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
