
  .text

  .globl foo
  .type  foo, @function
foo:
.foo.bb0:
	li	a0, 74
	ret
	.size foo, .-foo

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	call	foo
	mv	t0, a0
	addiw	t1, t0, 30
	addiw	t0, t0, 41
	addw	t0, t1, t0
	addiw	a0, t0, 35
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
