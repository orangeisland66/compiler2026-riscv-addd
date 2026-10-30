
  .text

  .globl reverse
  .type  reverse, @function
reverse:
.reverse.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	mv	s1, a0
	li	t0, 1
	bge	t0, s1, .reverse.bb1
.reverse.bb2:
	call	getint
	mv	s2, a0
	addiw	a0, s1, -1
	call	reverse
	mv	a0, s2
	call	putint
.reverse.bb3:
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.reverse.bb1:
	call	getint
	mv	t0, a0
	mv	a0, t0
	call	putint
	j	.reverse.bb3
	.size reverse, .-reverse

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 200
	call	reverse
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
