
  .section .rodata
  .globl TAPE_LEN
  .align 2
  .type  TAPE_LEN, @object
  .size  TAPE_LEN, 4
TAPE_LEN:
  .word 65536

  .section .rodata
  .globl BUFFER_LEN
  .align 2
  .type  BUFFER_LEN, @object
  .size  BUFFER_LEN, 4
BUFFER_LEN:
  .word 32768

  .bss
  .globl tape
  .align 2
  .type  tape, @object
  .size  tape, 262144
tape:
  .zero 262144

  .bss
  .globl program
  .align 2
  .type  program, @object
  .size  program, 131072
program:
  .zero 131072

  .bss
  .globl ptr11
  .align 2
  .type  ptr11, @object
  .size  ptr11, 4
ptr11:
  .zero 4

  .text

  .globl read_program
  .type  read_program, @function
read_program:
.read_program.bb0:
	addi	sp, sp, -32
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	sd	s2, 16(sp)
	call	getint
	mv	s1, a0
	li	s2, 0
.read_program.bb1:
	bge	s2, s1, .read_program.bb3
.read_program.bb2:
	call	getch
	mv	t0, a0
	slliw	t1, s2, 2
	la	t2, program
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s2, s2, 1
	blt	s2, s1, .read_program.bb2	# rotated-loop-branch
.read_program.bb3:
	slliw	t0, s2, 2
	la	t1, program
	add	t0, t1, t0
	li	t1, 0
	sw	t1, 0(t0)
	ld	s2, 16(sp)
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 32
	ret
	.size read_program, .-read_program

  .globl interpret
  .type  interpret, @function
interpret:
.interpret.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	mv	s1, a0
	la	t0, ptr11
	lw	t0, 0(t0)
	li	s2, 0
	mv	s3, t0	# phi-copy
.interpret.bb1:
	slliw	t0, s2, 2
	add	t0, s1, t0
	lw	s4, 0(t0)
	beq	s4, zero, .interpret.bb3
.interpret.bb2:
	li	t0, 62
	beq	s4, t0, .interpret.bb4
.interpret.bb5:
	li	t0, 60
	beq	s4, t0, .interpret.bb7
.interpret.bb8:
	li	t0, 43
	beq	s4, t0, .interpret.bb10
.interpret.bb11:
	li	t0, 45
	beq	s4, t0, .interpret.bb13
.interpret.bb14:
	li	t0, 46
	beq	s4, t0, .interpret.bb16
.interpret.bb17:
	li	t0, 44
	beq	s4, t0, .interpret.bb19
.interpret.bb20:
	slliw	t1, s3, 2
	la	t2, tape
	add	t1, t2, t1
	li	t0, 93
	lw	t1, 0(t1)
	subw	t0, s4, t0
	li	t2, 0
	sltiu	t0, t0, 1
	subw	t1, t1, t2
	sltu	t1, zero, t1
	and	t0, t0, t1
	beq	t0, zero, .interpret.bb6
.interpret.bb22:
	addiw	t1, s2, -1	# loop.addr.init
	li	t0, 1
	slliw	t1, t1, 2	# loop.addr.init
	add	t1, s1, t1	# loop.addr.init
.interpret.bb24:
	bge	zero, t0, .interpret.bb6
.interpret.bb25:
	lw	s4, 0(t1)
	addiw	s2, s2, -1
	li	t2, 91
	beq	s4, t2, .interpret.bb27
.interpret.bb28:
	li	t2, 93
	bne	s4, t2, .interpret.bb29
.interpret.bb30:
	addiw	t0, t0, 1
	j	.interpret.bb29
.interpret.bb31:
.interpret.bb29:
	addi	t1, t1, -4	# loop.addr.bump
	j	.interpret.bb24
.interpret.bb27:
	addiw	t0, t0, -1
	j	.interpret.bb29
.interpret.bb33:
	j	.interpret.bb29
.interpret.bb26:
	j	.interpret.bb6
.interpret.bb23:
	j	.interpret.bb6
.interpret.bb21:
	j	.interpret.bb6
.interpret.bb18:
	j	.interpret.bb6
.interpret.bb15:
	j	.interpret.bb6
.interpret.bb12:
	j	.interpret.bb6
.interpret.bb9:
.interpret.bb6:
	addiw	s2, s2, 1
	j	.interpret.bb1
.interpret.bb4:
	addiw	s3, s3, 1
	j	.interpret.bb6
.interpret.bb7:
	addiw	s3, s3, -1
	j	.interpret.bb6
.interpret.bb10:
	slliw	t0, s3, 2
	la	t1, tape
	add	s4, t1, t0
	lw	t0, 0(s4)
	addiw	t0, t0, 1
	sw	t0, 0(s4)
	j	.interpret.bb6
.interpret.bb13:
	slliw	t0, s3, 2
	la	t1, tape
	add	s4, t1, t0
	lw	t0, 0(s4)
	addiw	t0, t0, -1
	sw	t0, 0(s4)
	j	.interpret.bb6
.interpret.bb16:
	slliw	t0, s3, 2
	la	t1, tape
	add	t0, t1, t0
	lw	a0, 0(t0)
	call	putch
	j	.interpret.bb6
.interpret.bb19:
	call	getch
	mv	t0, a0
	slliw	t1, s3, 2
	la	t2, tape
	add	t1, t2, t1
	sw	t0, 0(t1)
	j	.interpret.bb6
.interpret.bb32:
	j	.interpret.bb6
.interpret.bb3:
	la	t0, ptr11
	sw	s3, 0(t0)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size interpret, .-interpret

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	call	read_program
	la	t5, program
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	interpret
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
