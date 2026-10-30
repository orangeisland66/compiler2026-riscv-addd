
  .bss
  .globl interchange_input
  .align 2
  .type  interchange_input, @object
  .size  interchange_input, 1024
interchange_input:
  .zero 1024

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	la	t0, interchange_input
	li	t1, 0
	li	t2, 256
.main.bb1:
	bge	t1, t2, .main.bb3
.main.bb2:
	addiw	t1, t1, 1
	sw	t1, 0(t0)
	addi	t0, t0, 4
	blt	t1, t2, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t0, 0
	li	t1, 0
.main.bb4:
	li	t2, 4
	bge	t1, t2, .main.bb6
.main.bb5:
	slliw	t2, t1, 8
	la	t3, interchange_input
	add	t2, t3, t2
	li	a2, 64
	li	t3, 0
.main.bb7:
	bge	t3, a2, .main.bb9
.main.bb8:
	lw	a3, 0(t2)
	addiw	t3, t3, 1
	addw	t0, t0, a3
	addi	t2, t2, 4
	blt	t3, a2, .main.bb8	# rotated-loop-branch
.main.bb9:
	addiw	t1, t1, 1
	j	.main.bb4
.main.bb6:
	mv	a0, t0
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
