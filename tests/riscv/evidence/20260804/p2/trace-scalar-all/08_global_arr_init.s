
  .bss
  .globl a0
  .align 2
  .type  a0, @object
  .size  a0, 12
a0:
  .zero 12

  .data
  .globl b0
  .align 2
  .type  b0, @object
  .size  b0, 16
b0:
  .word 0
  .word 1
  .word 0
  .word 0

  .data
  .globl c0
  .align 2
  .type  c0, @object
  .size  c0, 28
c0:
  .word 2
  .word 8
  .word 6
  .word 3
  .word 9
  .word 1
  .word 5

  .bss
  .globl d0
  .align 2
  .type  d0, @object
  .size  d0, 44
d0:
  .zero 44

  .data
  .globl e0
  .align 2
  .type  e0, @object
  .size  e0, 8
e0:
  .word 22
  .word 33

  .bss
  .globl f0
  .align 2
  .type  f0, @object
  .size  f0, 24
f0:
  .zero 24

  .data
  .globl g0
  .align 2
  .type  g0, @object
  .size  g0, 36
g0:
  .word 85
  .word 0
  .word 1
  .word 29
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0

  .bss
  .globl a
  .align 2
  .type  a, @object
  .size  a, 60
a:
  .zero 60

  .bss
  .globl b
  .align 2
  .type  b, @object
  .size  b, 60
b:
  .zero 60

  .data
  .globl c
  .align 2
  .type  c, @object
  .size  c, 60
c:
  .word 1
  .word 2
  .word 3
  .word 4
  .word 5
  .word 6
  .word 7
  .word 8
  .word 9
  .word 10
  .word 11
  .word 12
  .word 13
  .word 14
  .word 15

  .data
  .globl d
  .align 2
  .type  d, @object
  .size  d, 60
d:
  .word 1
  .word 2
  .word 3
  .word 4
  .word 5
  .word 6
  .word 7
  .word 8
  .word 9
  .word 10
  .word 11
  .word 12
  .word 13
  .word 14
  .word 15

  .data
  .globl e
  .align 2
  .type  e, @object
  .size  e, 60
e:
  .word 1
  .word 2
  .word 3
  .word 4
  .word 5
  .word 6
  .word 7
  .word 8
  .word 9
  .word 10
  .word 11
  .word 12
  .word 13
  .word 14
  .word 15

  .bss
  .globl f
  .align 2
  .type  f, @object
  .size  f, 20
f:
  .zero 20

  .data
  .globl g
  .align 2
  .type  g, @object
  .size  g, 60
g:
  .word 1
  .word 2
  .word 3
  .word 4
  .word 0
  .word 0
  .word 7
  .word 0
  .word 0
  .word 10
  .word 11
  .word 12
  .word 0
  .word 0
  .word 0

  .bss
  .globl h
  .align 2
  .type  h, @object
  .size  h, 12
h:
  .zero 12

  .data
  .globl i
  .align 2
  .type  i, @object
  .size  i, 96
i:
  .word 1
  .word 2
  .word 3
  .word 4
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 5
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	li	a0, 5
	ret
	.size main, .-main
