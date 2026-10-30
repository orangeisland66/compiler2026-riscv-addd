
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
	addi	sp, sp, -208
	sd	ra, 112(sp)
	sd	s1, 120(sp)
	sd	s2, 128(sp)
	sd	s3, 136(sp)
	sd	s4, 144(sp)
	sd	s5, 152(sp)
	sd	s6, 160(sp)
	sd	s7, 168(sp)
	sd	s8, 176(sp)
	sd	s9, 184(sp)
	sd	s10, 192(sp)
	sd	s11, 200(sp)
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
	addiw	a4, a4, 6
	addiw	a5, t3, 3
	addiw	a5, a5, 7
	addiw	a6, a2, 4
	addiw	a6, a6, 8
	addiw	a7, a3, 1
	addiw	a7, a7, 9
	addiw	t4, a4, 2
	addiw	t4, t4, 10
	addiw	a0, a5, 3
	addiw	s6, a0, 11
	addiw	a0, a6, 4
	addiw	s7, a0, 12
	addiw	a0, a7, 1
	addiw	t5, a0, 13
	sd	t5, 88(sp)
	addiw	a0, t4, 2
	addiw	t5, a0, 14
	sd	t5, 80(sp)
	addiw	a0, s6, 3
	addiw	t5, a0, 15
	sd	t5, 0(sp)
	addiw	a0, s7, 4
	addiw	t5, a0, 16
	sd	t5, 8(sp)
	ld	t5, 88(sp)
	addiw	a0, t5, 1
	addiw	t5, a0, 1
	sd	t5, 16(sp)
	ld	t5, 80(sp)
	addiw	a1, t5, 2
	addiw	t5, a1, 2
	sd	t5, 24(sp)
	ld	t5, 0(sp)
	addiw	s10, t5, 3
	addiw	t5, s10, 3
	sd	t5, 32(sp)
	ld	t5, 8(sp)
	addiw	s11, t5, 4
	addiw	t5, s11, 4
	sd	t5, 40(sp)
	ld	t5, 16(sp)
	addiw	a0, t5, 1
	addiw	t5, a0, 5
	sd	t5, 48(sp)
	ld	t5, 24(sp)
	addiw	a1, t5, 2
	addiw	t5, a1, 6
	sd	t5, 56(sp)
	ld	t5, 32(sp)
	addiw	s10, t5, 3
	addiw	t5, s10, 7
	sd	t5, 64(sp)
	ld	t5, 40(sp)
	addiw	s11, t5, 4
	addiw	t5, s11, 8
	sd	t5, 72(sp)
	ld	t5, 48(sp)
	addiw	a0, t5, 1
	addiw	t5, a0, 9
	sd	t5, 96(sp)
	ld	t5, 56(sp)
	addiw	a1, t5, 2
	addiw	a1, a1, 10
	ld	t5, 64(sp)
	addiw	s10, t5, 3
	addiw	s10, s10, 11
	ld	t5, 72(sp)
	addiw	s11, t5, 4
	addiw	s11, s11, 12
	subw	s9, s1, s2
	addiw	s8, s9, 10
	ld	t5, 96(sp)
	addiw	a0, t5, 1
	addiw	s1, a0, 13
	addiw	a0, a1, 2
	addiw	s2, a0, 14
	addiw	a0, s10, 3
	addiw	s9, a0, 15
	addiw	a0, s11, 4
	addiw	t5, a0, 16
	sd	t5, 104(sp)
	addw	a0, s8, s3
	addw	a0, a0, s4
	addw	a0, a0, s5
	addw	t0, a0, t0
	subw	t0, t0, t1
	subw	t0, t0, t2
	subw	t0, t0, t3
	subw	t0, t0, a2
	addw	t0, t0, a3
	addw	t0, t0, a4
	addw	t0, t0, a5
	addw	t0, t0, a6
	subw	t0, t0, a7
	subw	t0, t0, t4
	subw	t0, t0, s6
	subw	t0, t0, s7
	ld	t5, 88(sp)
	addw	t0, t0, t5
	ld	t5, 80(sp)
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
	ld	t5, 96(sp)
	subw	t0, t0, t5
	subw	t0, t0, a1
	subw	t0, t0, s10
	subw	t0, t0, s11
	addw	t0, t0, s1
	addw	t0, t0, s2
	addw	t0, t0, s9
	ld	t5, 104(sp)
	addw	t0, t0, t5
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
	ld	s11, 200(sp)
	ld	s10, 192(sp)
	ld	s9, 184(sp)
	ld	s8, 176(sp)
	ld	s7, 168(sp)
	ld	s6, 160(sp)
	ld	s5, 152(sp)
	ld	s4, 144(sp)
	ld	s3, 136(sp)
	ld	s2, 128(sp)
	ld	s1, 120(sp)
	ld	ra, 112(sp)
	addi	sp, sp, 208
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
