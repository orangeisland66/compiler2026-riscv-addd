
  .bss
  .globl counter
  .align 2
  .type  counter, @object
  .size  counter, 4
counter:
  .zero 4

  .bss
  .globl scratch
  .align 2
  .type  scratch, @object
  .size  scratch, 64
scratch:
  .zero 64

  .text

  .globl advance_state
  .type  advance_state, @function
advance_state:
.advance_state.bb0:
	la	t0, counter
	lw	t0, 0(t0)
.advance_state.bb1:
	li	t1, 3
.advance_state.bb3:
	bge	zero, t1, .advance_state.bb5
.advance_state.bb4:
	addiw	t0, t0, 1
	addiw	t1, t1, -1
	blt	zero, t1, .advance_state.bb4	# rotated-loop-branch
.advance_state.bb5:
	bge	t0, zero, .advance_state.bb2
.advance_state.bb6:
	j	.advance_state.bb1
.advance_state.bb7:
.advance_state.bb2:
	la	t1, counter
	sw	t0, 0(t1)
	mv	a0, t0
	ret
	.size advance_state, .-advance_state

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	li	s3, 0
	li	s2, 0
	li	s1, 6
.main.bb1:
	bge	zero, s1, .main.bb3
.main.bb2:
	call	advance_state
	addiw	t0, s1, -1
	la	t1, counter
	lw	s1, 0(t1)
	subw	t1, s1, s2
	sltiu	t1, t1, 1
	and	t1, s3, t1
	bnez	t1, .main.bb3
.main.bb4:
	li	s3, 1
	mv	s2, s1	# phi-copy
	mv	s1, t0	# phi-copy
	blt	zero, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	la	t0, counter
	lw	a0, 0(t0)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size main, .-main
