
  .bss
  .globl values
  .align 2
  .type  values, @object
  .size  values, 64
values:
  .zero 64

  .text

  .globl visit_values
  .type  visit_values, @function
visit_values:
.visit_values.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	mv	t0, a0
	mv	t1, a1
	bge	zero, t1, .visit_values.bb1
.visit_values.bb3:
	addiw	t2, t1, -1
	slliw	t3, t2, 2
	add	t3, t0, t3
	sw	t1, 0(t3)
	blt	zero, t2, .visit_values.bb4
.visit_values.bb5:
.visit_values.bb1:
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
.visit_values.bb4:
	mv	a0, t0	# call-arg-copy
	mv	a1, t2	# call-arg-copy
	call	visit_values
	j	.visit_values.bb1
.visit_values.bb2:
	j	.visit_values.bb1
	.size visit_values, .-visit_values

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	la	t5, values
	sd	t5, 0(sp)
	li	t5, 8
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	visit_values
	la	s1, values
	lw	t5, 0(s1)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
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
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
