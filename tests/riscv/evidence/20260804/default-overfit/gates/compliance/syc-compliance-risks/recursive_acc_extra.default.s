
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -64
	sd	ra, 40(sp)
	sd	s1, 48(sp)
	sd	s2, 56(sp)
	li	t0, 3
	addi	t1, sp, 24
	sw	t0, 0(t1)
	li	t0, 4
	sw	t0, 4(t1)
	li	t0, 5
	sw	t0, 8(t1)
	li	t0, 6
	sw	t0, 12(t1)
	call	getint
	mv	s1, a0
	call	getint
	mv	s2, a0
	mv	t5, s1
	sd	t5, 0(sp)
	addi	t5, sp, 24
	sd	t5, 8(sp)
	li	t5, 17
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	__norm_walk_state
	mv	t0, a0
	li	t1, 7
	subw	t1, t0, t1
	sltiu	t1, t1, 1
	subw	t2, zero, t0
	addiw	t2, t2, -1
	addw	t2, t2, s2
	li	t3, 1
	subw	t1, t3, t1
	li	t3, 0
	subw	t1, t3, t1
	xor	t2, t0, t2
	and	t1, t2, t1
	xor	t0, t0, t1
.main.bb1:
	mv	a0, t0
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s2, 56(sp)
	ld	s1, 48(sp)
	ld	ra, 40(sp)
	addi	sp, sp, 64
	ret
	.size main, .-main

  .globl __norm_walk_state
  .type  __norm_walk_state, @function
__norm_walk_state:
.__norm_walk_state.bb0:
	addi	sp, sp, -32
	sd	ra, 24(sp)
	mv	t0, a0
	mv	t1, a1
	mv	t2, a2
	beq	t0, zero, .__norm_walk_state.bb2
.__norm_walk_state.bb3:
	blt	t0, zero, .__norm_walk_state.bb4
.__norm_walk_state.bb5:
	addiw	a0, t0, -1
	mv	a1, t1	# call-arg-copy
	mv	a2, t2	# call-arg-copy
	call	__norm_walk_state
	mv	t0, a0
	li	t1, 7
	beq	t0, t1, .__norm_walk_state.bb1
.__norm_walk_state.bb7:
	addiw	t0, t0, -1
.__norm_walk_state.bb1:
	mv	a0, t0
	ld	ra, 24(sp)
	addi	sp, sp, 32
	ret
.__norm_walk_state.bb4:
	li	t0, 7
	j	.__norm_walk_state.bb1
.__norm_walk_state.bb6:
	j	.__norm_walk_state.bb1
.__norm_walk_state.bb2:
	li	t0, -1
	j	.__norm_walk_state.bb1
	.size __norm_walk_state, .-__norm_walk_state
