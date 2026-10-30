
  .data
  .globl a1
  .align 2
  .type  a1, @object
  .size  a1, 4
a1:
  .word 1

  .data
  .globl a2
  .align 2
  .type  a2, @object
  .size  a2, 4
a2:
  .word 2

  .data
  .globl a3
  .align 2
  .type  a3, @object
  .size  a3, 4
a3:
  .word 3

  .data
  .globl a4
  .align 2
  .type  a4, @object
  .size  a4, 4
a4:
  .word 4

  .data
  .globl a5
  .align 2
  .type  a5, @object
  .size  a5, 4
a5:
  .word 5

  .data
  .globl a6
  .align 2
  .type  a6, @object
  .size  a6, 4
a6:
  .word 6

  .data
  .globl a7
  .align 2
  .type  a7, @object
  .size  a7, 4
a7:
  .word 7

  .data
  .globl a8
  .align 2
  .type  a8, @object
  .size  a8, 4
a8:
  .word 8

  .data
  .globl a9
  .align 2
  .type  a9, @object
  .size  a9, 4
a9:
  .word 9

  .data
  .globl a10
  .align 2
  .type  a10, @object
  .size  a10, 4
a10:
  .word 10

  .data
  .globl a11
  .align 2
  .type  a11, @object
  .size  a11, 4
a11:
  .word 11

  .data
  .globl a12
  .align 2
  .type  a12, @object
  .size  a12, 4
a12:
  .word 12

  .data
  .globl a13
  .align 2
  .type  a13, @object
  .size  a13, 4
a13:
  .word 13

  .data
  .globl a14
  .align 2
  .type  a14, @object
  .size  a14, 4
a14:
  .word 14

  .data
  .globl a15
  .align 2
  .type  a15, @object
  .size  a15, 4
a15:
  .word 15

  .data
  .globl a16
  .align 2
  .type  a16, @object
  .size  a16, 4
a16:
  .word 16

  .data
  .globl a17
  .align 2
  .type  a17, @object
  .size  a17, 4
a17:
  .word 1

  .data
  .globl a18
  .align 2
  .type  a18, @object
  .size  a18, 4
a18:
  .word 2

  .data
  .globl a19
  .align 2
  .type  a19, @object
  .size  a19, 4
a19:
  .word 3

  .data
  .globl a20
  .align 2
  .type  a20, @object
  .size  a20, 4
a20:
  .word 4

  .data
  .globl a21
  .align 2
  .type  a21, @object
  .size  a21, 4
a21:
  .word 5

  .data
  .globl a22
  .align 2
  .type  a22, @object
  .size  a22, 4
a22:
  .word 6

  .data
  .globl a23
  .align 2
  .type  a23, @object
  .size  a23, 4
a23:
  .word 7

  .data
  .globl a24
  .align 2
  .type  a24, @object
  .size  a24, 4
a24:
  .word 8

  .data
  .globl a25
  .align 2
  .type  a25, @object
  .size  a25, 4
a25:
  .word 9

  .data
  .globl a26
  .align 2
  .type  a26, @object
  .size  a26, 4
a26:
  .word 10

  .data
  .globl a27
  .align 2
  .type  a27, @object
  .size  a27, 4
a27:
  .word 11

  .data
  .globl a28
  .align 2
  .type  a28, @object
  .size  a28, 4
a28:
  .word 12

  .data
  .globl a29
  .align 2
  .type  a29, @object
  .size  a29, 4
a29:
  .word 13

  .data
  .globl a30
  .align 2
  .type  a30, @object
  .size  a30, 4
a30:
  .word 14

  .data
  .globl a31
  .align 2
  .type  a31, @object
  .size  a31, 4
a31:
  .word 15

  .data
  .globl a32
  .align 2
  .type  a32, @object
  .size  a32, 4
a32:
  .word 16

  .text

  .globl func
  .type  func, @function
func:
.func.bb0:
	addi	sp, sp, -240
	sd	ra, 184(sp)
	sd	s1, 192(sp)
	sd	s2, 200(sp)
	sd	s3, 208(sp)
	sd	s4, 216(sp)
	sd	s5, 224(sp)
	mv	s1, a0
	mv	s2, a1
	call	getint
	mv	s3, a0
	call	getint
	mv	s4, a0
	call	getint
	mv	s5, a0
	call	getint
	mv	t0, a0
	addiw	t1, s3, 1
	addiw	t1, t1, 1
	addiw	t2, s4, 2
	addiw	t2, t2, 2
	addiw	t3, s5, 3
	addiw	t3, t3, 3
	addiw	a2, t0, 4
	addiw	a2, a2, 4
	addiw	a3, t1, 1
	addiw	a3, a3, 5
	addiw	a4, t2, 2
	addiw	t5, a4, 6
	sd	t5, 144(sp)
	addiw	a5, t3, 3
	addiw	t5, a5, 7
	sd	t5, 0(sp)
	addiw	a6, a2, 4
	addiw	t5, a6, 8
	sd	t5, 8(sp)
	addiw	a7, a3, 1
	addiw	t5, a7, 9
	sd	t5, 16(sp)
	ld	t5, 144(sp)
	addiw	t4, t5, 2
	addiw	t5, t4, 10
	sd	t5, 24(sp)
	ld	t5, 0(sp)
	addiw	a5, t5, 3
	addiw	t5, a5, 11
	sd	t5, 32(sp)
	ld	t5, 8(sp)
	addiw	a6, t5, 4
	addiw	t5, a6, 12
	sd	t5, 40(sp)
	ld	t5, 16(sp)
	addiw	a7, t5, 1
	addiw	t5, a7, 13
	sd	t5, 48(sp)
	ld	t5, 24(sp)
	addiw	t4, t5, 2
	addiw	t5, t4, 14
	sd	t5, 56(sp)
	ld	t5, 32(sp)
	addiw	a5, t5, 3
	addiw	t5, a5, 15
	sd	t5, 64(sp)
	ld	t5, 40(sp)
	addiw	a6, t5, 4
	addiw	t5, a6, 16
	sd	t5, 72(sp)
	ld	t5, 48(sp)
	addiw	a7, t5, 1
	addiw	t5, a7, 1
	sd	t5, 80(sp)
	ld	t5, 56(sp)
	addiw	t4, t5, 2
	addiw	t5, t4, 2
	sd	t5, 88(sp)
	ld	t5, 64(sp)
	addiw	a5, t5, 3
	addiw	t5, a5, 3
	sd	t5, 96(sp)
	ld	t5, 72(sp)
	addiw	a6, t5, 4
	addiw	t5, a6, 4
	sd	t5, 104(sp)
	ld	t5, 80(sp)
	addiw	a7, t5, 1
	addiw	t5, a7, 5
	sd	t5, 112(sp)
	ld	t5, 88(sp)
	addiw	t4, t5, 2
	addiw	t5, t4, 6
	sd	t5, 120(sp)
	ld	t5, 96(sp)
	addiw	a5, t5, 3
	addiw	t5, a5, 7
	sd	t5, 128(sp)
	ld	t5, 104(sp)
	addiw	a6, t5, 4
	addiw	t5, a6, 8
	sd	t5, 136(sp)
	ld	t5, 112(sp)
	addiw	a7, t5, 1
	addiw	t5, a7, 9
	sd	t5, 152(sp)
	ld	t5, 120(sp)
	addiw	t4, t5, 2
	addiw	t4, t4, 10
	ld	t5, 128(sp)
	addiw	a5, t5, 3
	addiw	a5, a5, 11
	ld	t5, 136(sp)
	addiw	a6, t5, 4
	addiw	a6, a6, 12
	subw	a4, s1, s2
	addiw	a4, a4, 10
	ld	t5, 152(sp)
	addiw	a7, t5, 1
	addiw	t5, a7, 13
	sd	t5, 160(sp)
	addiw	a7, t4, 2
	addiw	t5, a7, 14
	sd	t5, 168(sp)
	addiw	a7, a5, 3
	addiw	t5, a7, 15
	sd	t5, 176(sp)
	addiw	a7, a6, 4
	addiw	a7, a7, 16
	addw	a4, a4, s3
	addw	a4, a4, s4
	addw	a4, a4, s5
	addw	t0, a4, t0
	subw	t0, t0, t1
	subw	t0, t0, t2
	subw	t0, t0, t3
	subw	t0, t0, a2
	addw	t0, t0, a3
	ld	t5, 144(sp)
	addw	t0, t0, t5
	ld	t5, 0(sp)
	addw	t0, t0, t5
	ld	t5, 8(sp)
	addw	t0, t0, t5
	ld	t5, 16(sp)
	subw	t0, t0, t5
	ld	t5, 24(sp)
	subw	t0, t0, t5
	ld	t5, 32(sp)
	subw	t0, t0, t5
	ld	t5, 40(sp)
	subw	t0, t0, t5
	ld	t5, 48(sp)
	addw	t0, t0, t5
	ld	t5, 56(sp)
	addw	t0, t0, t5
	ld	t5, 64(sp)
	addw	t0, t0, t5
	ld	t5, 72(sp)
	addw	t0, t0, t5
	ld	t5, 80(sp)
	subw	t0, t0, t5
	ld	t5, 88(sp)
	subw	t0, t0, t5
	ld	t5, 96(sp)
	subw	t0, t0, t5
	ld	t5, 104(sp)
	subw	t0, t0, t5
	ld	t5, 112(sp)
	addw	t0, t0, t5
	ld	t5, 120(sp)
	addw	t0, t0, t5
	ld	t5, 128(sp)
	addw	t0, t0, t5
	ld	t5, 136(sp)
	addw	t0, t0, t5
	ld	t5, 152(sp)
	subw	t0, t0, t5
	subw	t0, t0, t4
	subw	t0, t0, a5
	subw	t0, t0, a6
	ld	t5, 160(sp)
	addw	t0, t0, t5
	ld	t5, 168(sp)
	addw	t0, t0, t5
	ld	t5, 176(sp)
	addw	t0, t0, t5
	addw	t0, t0, a7
	addiw	t0, t0, 1
	addiw	t0, t0, -2
	addiw	t0, t0, 3
	addiw	t0, t0, -4
	addiw	t0, t0, 5
	addiw	t0, t0, -6
	addiw	t0, t0, 7
	addiw	t0, t0, -8
	addiw	t0, t0, 9
	addiw	t0, t0, -10
	addiw	t0, t0, 11
	addiw	t0, t0, -12
	addiw	t0, t0, 13
	addiw	t0, t0, -14
	addiw	t0, t0, 15
	addiw	t0, t0, -16
	addiw	t0, t0, 1
	addiw	t0, t0, -2
	addiw	t0, t0, 3
	addiw	t0, t0, -4
	addiw	t0, t0, 5
	addiw	t0, t0, -6
	addiw	t0, t0, 7
	addiw	t0, t0, -8
	addiw	t0, t0, 9
	addiw	t0, t0, -10
	addiw	t0, t0, 11
	addiw	t0, t0, -12
	addiw	t0, t0, 13
	addiw	t0, t0, -14
	addiw	t0, t0, 15
	addiw	a0, t0, -16
	ld	s5, 224(sp)
	ld	s4, 216(sp)
	ld	s3, 208(sp)
	ld	s2, 200(sp)
	ld	s1, 192(sp)
	ld	ra, 184(sp)
	addi	sp, sp, 240
	ret
	.size func, .-func

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	call	getint
	mv	t0, a0
	addiw	t1, t0, 18
	mv	t5, t0
	sd	t5, 0(sp)
	mv	t5, t1
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	func
	mv	s1, a0
	mv	a0, s1
	call	putint
	mv	a0, s1
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
