
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 4
a:
  .zero 4

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 4
b:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 0(sp)
	sd	s1, 8(sp)
	call	getint
	mv	s1, a0
	la	t0, a
	sw	s1, 0(t0)
	call	getint
	mv	t0, a0
	la	t1, b
	sw	t0, 0(t1)
	subw	t0, s1, t0
	sltiu	t0, t0, 1
	li	t1, 3
	subw	t1, s1, t1
	sltu	t1, zero, t1
	and	t0, t0, t1
.main.bb1:
	mv	a0, t0
	ld	s1, 8(sp)
	ld	ra, 0(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
