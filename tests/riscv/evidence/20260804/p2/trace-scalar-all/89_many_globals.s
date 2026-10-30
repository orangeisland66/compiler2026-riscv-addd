
  .bss
  .globl a0
  .align 2
  .type  a0, @object
  .size  a0, 4
a0:
  .zero 4

  .bss
  .globl a1
  .align 2
  .type  a1, @object
  .size  a1, 4
a1:
  .zero 4

  .bss
  .globl a2
  .align 2
  .type  a2, @object
  .size  a2, 4
a2:
  .zero 4

  .bss
  .globl a3
  .align 2
  .type  a3, @object
  .size  a3, 4
a3:
  .zero 4

  .bss
  .globl a4
  .align 2
  .type  a4, @object
  .size  a4, 4
a4:
  .zero 4

  .bss
  .globl a5
  .align 2
  .type  a5, @object
  .size  a5, 4
a5:
  .zero 4

  .bss
  .globl a6
  .align 2
  .type  a6, @object
  .size  a6, 4
a6:
  .zero 4

  .bss
  .globl a7
  .align 2
  .type  a7, @object
  .size  a7, 4
a7:
  .zero 4

  .bss
  .globl a8
  .align 2
  .type  a8, @object
  .size  a8, 4
a8:
  .zero 4

  .bss
  .globl a9
  .align 2
  .type  a9, @object
  .size  a9, 4
a9:
  .zero 4

  .bss
  .globl a10
  .align 2
  .type  a10, @object
  .size  a10, 4
a10:
  .zero 4

  .bss
  .globl a11
  .align 2
  .type  a11, @object
  .size  a11, 4
a11:
  .zero 4

  .bss
  .globl a12
  .align 2
  .type  a12, @object
  .size  a12, 4
a12:
  .zero 4

  .bss
  .globl a13
  .align 2
  .type  a13, @object
  .size  a13, 4
a13:
  .zero 4

  .bss
  .globl a14
  .align 2
  .type  a14, @object
  .size  a14, 4
a14:
  .zero 4

  .bss
  .globl a15
  .align 2
  .type  a15, @object
  .size  a15, 4
a15:
  .zero 4

  .bss
  .globl a16
  .align 2
  .type  a16, @object
  .size  a16, 4
a16:
  .zero 4

  .bss
  .globl a17
  .align 2
  .type  a17, @object
  .size  a17, 4
a17:
  .zero 4

  .bss
  .globl a18
  .align 2
  .type  a18, @object
  .size  a18, 4
a18:
  .zero 4

  .bss
  .globl a19
  .align 2
  .type  a19, @object
  .size  a19, 4
a19:
  .zero 4

  .bss
  .globl a20
  .align 2
  .type  a20, @object
  .size  a20, 4
a20:
  .zero 4

  .bss
  .globl a21
  .align 2
  .type  a21, @object
  .size  a21, 4
a21:
  .zero 4

  .bss
  .globl a22
  .align 2
  .type  a22, @object
  .size  a22, 4
a22:
  .zero 4

  .bss
  .globl a23
  .align 2
  .type  a23, @object
  .size  a23, 4
a23:
  .zero 4

  .bss
  .globl a24
  .align 2
  .type  a24, @object
  .size  a24, 4
a24:
  .zero 4

  .bss
  .globl a25
  .align 2
  .type  a25, @object
  .size  a25, 4
a25:
  .zero 4

  .bss
  .globl a26
  .align 2
  .type  a26, @object
  .size  a26, 4
a26:
  .zero 4

  .bss
  .globl a27
  .align 2
  .type  a27, @object
  .size  a27, 4
a27:
  .zero 4

  .bss
  .globl a28
  .align 2
  .type  a28, @object
  .size  a28, 4
a28:
  .zero 4

  .bss
  .globl a29
  .align 2
  .type  a29, @object
  .size  a29, 4
a29:
  .zero 4

  .bss
  .globl a30
  .align 2
  .type  a30, @object
  .size  a30, 4
a30:
  .zero 4

  .bss
  .globl a31
  .align 2
  .type  a31, @object
  .size  a31, 4
a31:
  .zero 4

  .bss
  .globl a32
  .align 2
  .type  a32, @object
  .size  a32, 4
a32:
  .zero 4

  .bss
  .globl a33
  .align 2
  .type  a33, @object
  .size  a33, 4
a33:
  .zero 4

  .bss
  .globl a34
  .align 2
  .type  a34, @object
  .size  a34, 4
a34:
  .zero 4

  .bss
  .globl a35
  .align 2
  .type  a35, @object
  .size  a35, 4
a35:
  .zero 4

  .bss
  .globl a36
  .align 2
  .type  a36, @object
  .size  a36, 4
a36:
  .zero 4

  .bss
  .globl a37
  .align 2
  .type  a37, @object
  .size  a37, 4
a37:
  .zero 4

  .bss
  .globl a38
  .align 2
  .type  a38, @object
  .size  a38, 4
a38:
  .zero 4

  .bss
  .globl a39
  .align 2
  .type  a39, @object
  .size  a39, 4
a39:
  .zero 4

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	t0, 1
	la	t1, a1
	sw	t0, 0(t1)
	li	t0, 2
	la	t1, a2
	sw	t0, 0(t1)
	li	t0, 3
	la	t1, a3
	sw	t0, 0(t1)
	li	t0, 4
	la	t1, a4
	sw	t0, 0(t1)
	li	t0, 5
	la	t1, a5
	sw	t0, 0(t1)
	li	t0, 6
	la	t1, a6
	sw	t0, 0(t1)
	li	t0, 7
	la	t1, a7
	sw	t0, 0(t1)
	li	t0, 8
	la	t1, a8
	sw	t0, 0(t1)
	li	t0, 9
	la	t1, a9
	sw	t0, 0(t1)
	li	t0, 0
	la	t1, a10
	sw	t0, 0(t1)
	li	t0, 1
	la	t1, a11
	sw	t0, 0(t1)
	li	t0, 2
	la	t1, a12
	sw	t0, 0(t1)
	li	t0, 3
	la	t1, a13
	sw	t0, 0(t1)
	li	t0, 4
	la	t1, a14
	sw	t0, 0(t1)
	li	t0, 5
	la	t1, a15
	sw	t0, 0(t1)
	li	t0, 6
	la	t1, a16
	sw	t0, 0(t1)
	li	t0, 7
	la	t1, a17
	sw	t0, 0(t1)
	li	t0, 8
	la	t1, a18
	sw	t0, 0(t1)
	li	t0, 9
	la	t1, a19
	sw	t0, 0(t1)
	li	t0, 0
	la	t1, a20
	sw	t0, 0(t1)
	li	t0, 1
	la	t1, a21
	sw	t0, 0(t1)
	li	t0, 2
	la	t1, a22
	sw	t0, 0(t1)
	li	t0, 3
	la	t1, a23
	sw	t0, 0(t1)
	li	t0, 4
	la	t1, a24
	sw	t0, 0(t1)
	li	t0, 5
	la	t1, a25
	sw	t0, 0(t1)
	li	t0, 6
	la	t1, a26
	sw	t0, 0(t1)
	li	t0, 7
	la	t1, a27
	sw	t0, 0(t1)
	li	t0, 8
	la	t1, a28
	sw	t0, 0(t1)
	li	t0, 9
	la	t1, a29
	sw	t0, 0(t1)
	li	t0, 0
	la	t1, a30
	sw	t0, 0(t1)
	li	t0, 1
	la	t1, a31
	sw	t0, 0(t1)
	li	t0, 4
	la	t1, a32
	sw	t0, 0(t1)
	li	t0, 5
	la	t1, a33
	sw	t0, 0(t1)
	li	t0, 6
	la	t1, a34
	sw	t0, 0(t1)
	li	t0, 7
	la	t1, a35
	sw	t0, 0(t1)
	li	t0, 8
	la	t1, a36
	sw	t0, 0(t1)
	li	t0, 9
	la	t1, a37
	sw	t0, 0(t1)
	li	t0, 0
	la	t1, a38
	sw	t0, 0(t1)
	li	t0, 1
	la	t1, a39
	sw	t0, 0(t1)
	li	a0, 28
	call	putint
	li	t5, 22
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 118
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t0, 118
	la	t1, a0
	sw	t0, 0(t1)
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
