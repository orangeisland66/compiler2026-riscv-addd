
  .bss
  .globl values
  .align 2
  .type  values, @object
  .size  values, 64
values:
  .zero 64

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	la	t0, values
	li	t1, 8
.main.bb1:
	bge	zero, t1, .main.bb2
.main.bb3:
	addiw	s1, t1, -1
	slliw	t2, s1, 2
	add	t2, t0, t2
	sw	t1, 0(t2)
	mv	t1, s1	# phi-copy
	blt	zero, t1, .main.bb3	# rotated-loop-branch
.main.bb2:
	la	s1, values
	lw	a0, 0(s1)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	lw	t5, 28(s1)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
