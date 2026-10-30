
  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 16384
a:
  .zero 16384

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	t6, -32816
	add	sp, sp, t6
	li	t6, 32792
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 32800
	add	t6, sp, t6
	sd	s1, 0(t6)
	addi	t5, sp, 24
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	li	t5, 16384
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	memset
	li	t0, 1
	addi	t1, sp, 24
	li	t2, 4096
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 8192
	li	t0, 2
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 8196
	li	t0, 3
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 12288
	li	t0, 4
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 12292
	li	t0, 5
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 12296
	li	t0, 6
	add	t1, t1, t2
	sw	t0, 0(t1)
	li	t5, 16408
	add	t5, sp, t5
	sd	t5, 0(sp)
	li	t5, 0
	sd	t5, 8(sp)
	li	t5, 16384
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	memset
	li	t0, 1
	li	s1, 16408
	add	s1, sp, s1
	sw	t0, 0(s1)
	li	t0, 2
	sw	t0, 4(s1)
	li	t0, 3
	sw	t0, 16(s1)
	li	t0, 4
	sw	t0, 20(s1)
	li	t0, 4000
	la	t1, a
	sw	t0, 20(t1)
	li	t2, 16000
	li	t0, 3
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t2, 16380
	li	t0, 7
	add	t2, t1, t2
	sw	t0, 0(t2)
	li	t0, 8864
	add	t0, t1, t0
	lw	t0, 0(t0)
	addiw	t0, t0, 9
	sw	t0, 28(s1)
	lw	t0, 20(t1)
	slliw	t0, t0, 2
	add	t0, t1, t0
	lw	t5, 0(t0)
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	lw	a0, 32(s1)
	li	t6, 32800
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 32792
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 32816
	add	sp, sp, t6
	ret
	.size main, .-main
