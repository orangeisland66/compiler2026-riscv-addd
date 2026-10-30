
  .section .rodata
  .globl N
  .align 2
  .type  N, @object
  .size  N, 4
N:
  .word 100

  .section .rodata
  .globl M
  .align 2
  .type  M, @object
  .size  M, 4
M:
  .word 20

  .bss
  .globl array
  .align 2
  .type  array, @object
  .size  array, 8000
array:
  .zero 8000

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 0
.main.bb1:
	li	t1, 20
	bge	t0, t1, .main.bb3
.main.bb2:
	li	t1, 400
	la	t2, array
	mulw	t1, t0, t1
	li	t3, 100
	add	t1, t2, t1
	li	t2, 0
.main.bb4:
	bge	t2, t3, .main.bb6
.main.bb5:
	sw	t2, 0(t1)
	addiw	t2, t2, 1
	addi	t1, t1, 4
	blt	t2, t3, .main.bb5	# rotated-loop-branch
.main.bb6:
	addiw	t0, t0, 1
	j	.main.bb1
.main.bb3:
	li	t1, 7692
	la	t0, array
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 7200
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 6800
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 6400
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 6000
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 5600
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 5200
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 4800
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 4400
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 4000
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 3600
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 3200
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 2800
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	li	t2, 2400
	addw	t1, t1, t2
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	addiw	t1, t1, 2000
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	addiw	t1, t1, 1600
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	addiw	t1, t1, 1200
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	addiw	t1, t1, 800
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	addiw	t1, t1, 400
	add	t1, t0, t1
	lw	t1, 0(t1)
	slliw	t1, t1, 2
	add	t1, t0, t1
	lw	t1, 0(t1)
	li	t2, 7672
	add	t2, t0, t2
	lw	t2, 0(t2)
	li	t3, 400
	mulw	t2, t2, t3
	addiw	t2, t2, 68
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 64
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 60
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 56
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 52
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 48
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 44
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 40
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 36
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 32
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 28
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 24
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 20
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 16
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 12
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 8
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 4
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	add	t2, t0, t2
	lw	t2, 0(t2)
	mulw	t2, t2, t3
	addiw	t2, t2, 224
	add	t0, t0, t2
	lw	t0, 0(t0)
	addw	a0, t1, t0
	call	putint
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
