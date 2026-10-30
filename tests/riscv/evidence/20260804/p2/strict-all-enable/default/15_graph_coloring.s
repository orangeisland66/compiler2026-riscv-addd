
  .section .rodata
  .globl V
  .align 2
  .type  V, @object
  .size  V, 4
V:
  .word 4

  .section .rodata
  .globl space
  .align 2
  .type  space, @object
  .size  space, 4
space:
  .word 32

  .section .rodata
  .globl LF
  .align 2
  .type  LF, @object
  .size  LF, 4
LF:
  .word 10

  .text

  .globl printSolution
  .type  printSolution, @function
printSolution:
.printSolution.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	mv	s1, a0
	li	s3, 0
	li	s2, 4
.printSolution.bb1:
	bge	s3, s2, .printSolution.bb3
.printSolution.bb2:
	slliw	t0, s3, 2
	add	t0, s1, t0
	lw	a0, 0(t0)
	call	putint
	li	a0, 32
	call	putch
	addiw	s3, s3, 1
	blt	s3, s2, .printSolution.bb2	# rotated-loop-branch
.printSolution.bb3:
	li	a0, 10
	call	putch
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
	.size printSolution, .-printSolution

  .globl printMessage
  .type  printMessage, @function
printMessage:
.printMessage.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 78
	call	putch
	li	a0, 111
	call	putch
	li	a0, 116
	call	putch
	li	a0, 32
	call	putch
	li	a0, 101
	call	putch
	li	a0, 120
	call	putch
	li	a0, 105
	call	putch
	li	a0, 115
	call	putch
	li	a0, 116
	call	putch
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size printMessage, .-printMessage

  .globl graphColoring
  .type  graphColoring, @function
graphColoring:
.graphColoring.bb0:
	addi	sp, sp, -96
	sd	ra, 32(sp)
	sd	s1, 40(sp)
	sd	s2, 48(sp)
	sd	s3, 56(sp)
	sd	s4, 64(sp)
	sd	s5, 72(sp)
	sd	s6, 80(sp)
	mv	s1, a0
	mv	s2, a1
	mv	t0, a2
	mv	s3, a3
	li	t1, 4
	bne	t0, t1, .graphColoring.bb3
.graphColoring.bb2:
	mv	t0, s3
	li	t1, 0
.graphColoring.bb12:
	li	t2, 4
	bge	t1, t2, .graphColoring.bb14
.graphColoring.bb13:
	lw	t2, 0(t0)
	addiw	s2, t1, 1
	slliw	t1, t1, 4
	slliw	s4, s2, 2
	addw	t1, t1, s4
	add	t3, s3, s4
	add	t1, s1, t1
	li	a2, 4
	mv	s4, s2	# phi-copy
.graphColoring.bb15:
	bge	s4, a2, .graphColoring.bb17
.graphColoring.bb16:
	lw	a3, 0(t1)
	li	a4, 0
	subw	a3, a3, a4
	lw	a4, 0(t3)
	sltu	a3, zero, a3
	subw	a4, a4, t2
	sltiu	a4, a4, 1
	and	a3, a3, a4
	bne	a3, zero, .graphColoring.bb18
.graphColoring.bb19:
	addiw	s4, s4, 1
	addi	t1, t1, 4
	addi	t3, t3, 4
	j	.graphColoring.bb15
.graphColoring.bb17:
	addi	t0, t0, 4
	mv	t1, s2	# phi-copy
	j	.graphColoring.bb12
.graphColoring.bb18:
	li	t0, 0
.graphColoring.bb11:
	beq	t0, zero, .graphColoring.bb5
.graphColoring.bb4:
	mv	a0, s3
	call	printSolution
	li	t0, 1
.graphColoring.bb1:
	mv	a0, t0
	ld	s6, 80(sp)
	ld	s5, 72(sp)
	ld	s4, 64(sp)
	ld	s3, 56(sp)
	ld	s2, 48(sp)
	ld	s1, 40(sp)
	ld	ra, 32(sp)
	addi	sp, sp, 96
	ret
.graphColoring.bb14:
	li	t0, 1
	j	.graphColoring.bb11
.graphColoring.bb7:
	sw	s6, 0(s4)
	mv	a0, s1	# call-arg-copy
	mv	a1, s2	# call-arg-copy
	mv	a2, s5	# call-arg-copy
	mv	a3, s3	# call-arg-copy
	call	graphColoring
	mv	t0, a0
	bne	t0, zero, .graphColoring.bb9
.graphColoring.bb10:
	li	t0, 0
	sw	t0, 0(s4)
	addiw	s6, s6, 1
	bge	s2, s6, .graphColoring.bb7	# rotated-loop-branch
.graphColoring.bb8:
	li	t0, 0
	j	.graphColoring.bb1
.graphColoring.bb9:
	li	t0, 1
	j	.graphColoring.bb1
.graphColoring.bb5:
	li	t0, 0
	j	.graphColoring.bb1
.graphColoring.bb3:
	slliw	t1, t0, 2
	addiw	s5, t0, 1
	add	s4, s3, t1
	li	s6, 1
.graphColoring.bb6:
	blt	s2, s6, .graphColoring.bb8
	j	.graphColoring.bb7
	.size graphColoring, .-graphColoring

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -128
	sd	ra, 112(sp)
	addi	t5, sp, 48
	sd	zero, 0(t5)	# memset-zero
	sd	zero, 8(t5)	# memset-zero
	sd	zero, 16(t5)	# memset-zero
	sd	zero, 24(t5)	# memset-zero
	sd	zero, 32(t5)	# memset-zero
	sd	zero, 40(t5)	# memset-zero
	sd	zero, 48(t5)	# memset-zero
	sd	zero, 56(t5)	# memset-zero
	li	t0, 0
	addi	t1, sp, 48
	sw	t0, 0(t1)
	li	t0, 1
	sw	t0, 4(t1)
	sw	t0, 8(t1)
	sw	t0, 12(t1)
	sw	t0, 16(t1)
	li	t0, 0
	sw	t0, 20(t1)
	li	t0, 1
	sw	t0, 24(t1)
	li	t0, 0
	sw	t0, 28(t1)
	li	t0, 1
	sw	t0, 32(t1)
	sw	t0, 36(t1)
	li	t0, 0
	sw	t0, 40(t1)
	li	t0, 1
	sw	t0, 44(t1)
	sw	t0, 48(t1)
	li	t0, 0
	sw	t0, 52(t1)
	li	t0, 1
	sw	t0, 56(t1)
	li	t0, 0
	sw	t0, 60(t1)
	addi	t1, sp, 32
	sw	t0, 0(t1)
	sw	t0, 4(t1)
	sw	t0, 8(t1)
	sw	t0, 12(t1)
	addi	t5, sp, 48
	sd	t5, 0(sp)
	li	t5, 3
	sd	t5, 8(sp)
	li	t5, 0
	sd	t5, 16(sp)
	addi	t5, sp, 32
	sd	t5, 24(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	call	graphColoring
	mv	t0, a0
	beq	t0, zero, .main.bb1
.main.bb2:
	li	a0, 0
	ld	ra, 112(sp)
	addi	sp, sp, 128
	ret
.main.bb1:
	call	printMessage
	j	.main.bb2
	.size main, .-main
