
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

  .section .rodata
  .globl maxNode
  .align 2
  .type  maxNode, @object
  .size  maxNode, 4
maxNode:
  .word 10000

  .bss
  .globl value
  .align 2
  .type  value, @object
  .size  value, 40000
value:
  .zero 40000

  .bss
  .globl left_child
  .align 2
  .type  left_child, @object
  .size  left_child, 40000
left_child:
  .zero 40000

  .bss
  .globl right_child
  .align 2
  .type  right_child, @object
  .size  right_child, 40000
right_child:
  .zero 40000

  .bss
  .globl now
  .align 2
  .type  now, @object
  .size  now, 4
now:
  .zero 4

  .text

  .globl insert
  .type  insert, @function
insert:
.insert.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	mv	s1, a0
	mv	t0, a1
	li	t1, -1
	beq	s1, t1, .insert.bb2
.insert.bb3:
	slliw	t1, s1, 2
	la	t2, value
	add	t2, t2, t1
	lw	t2, 0(t2)
	blt	t2, t0, .insert.bb4
.insert.bb5:
	la	t2, left_child
	add	s2, t2, t1
	lw	a0, 0(s2)
	mv	a1, t0	# call-arg-copy
	call	insert
	mv	t0, a0
	sw	t0, 0(s2)
	j	.insert.bb1
.insert.bb6:
.insert.bb1:
	mv	a0, s1
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.insert.bb4:
	la	t2, right_child
	add	s2, t2, t1
	lw	a0, 0(s2)
	mv	a1, t0	# call-arg-copy
	call	insert
	mv	t0, a0
	sw	t0, 0(s2)
	j	.insert.bb1
.insert.bb2:
	la	t1, now
	la	a2, value
	lw	t2, 0(t1)
	slliw	t3, t2, 2
	add	a2, a2, t3
	sw	t0, 0(a2)
	la	t0, left_child
	li	a2, -1
	add	t0, t0, t3
	sw	a2, 0(t0)
	la	t0, right_child
	add	t0, t0, t3
	li	t3, -1
	sw	t3, 0(t0)
	addiw	t0, t2, 1
	sw	t0, 0(t1)
	mv	s1, t2	# phi-copy
	j	.insert.bb1
	.size insert, .-insert

  .globl delete
  .type  delete, @function
delete:
.delete.bb0:
	addi	sp, sp, -80
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	sd	s4, 48(sp)
	sd	s5, 56(sp)
	sd	s6, 64(sp)
	mv	s1, a0
	mv	t0, a1
	li	t1, -1
	beq	s1, t1, .delete.bb2
.delete.bb3:
	slliw	t1, s1, 2
	la	t2, value
	add	s2, t2, t1
	lw	t2, 0(s2)
	blt	t2, t0, .delete.bb4
.delete.bb5:
	blt	t0, t2, .delete.bb7
.delete.bb8:
	la	t0, left_child
	la	t3, right_child
	add	t0, t0, t1
	add	s3, t3, t1
	lw	t0, 0(t0)
	li	t2, -1
	lw	s4, 0(s3)
	li	t1, -1
	subw	t2, t0, t2
	subw	t1, s4, t1
	sltiu	t2, t2, 1
	sltiu	t1, t1, 1
	and	t3, t2, t1
	bne	t3, zero, .delete.bb10
.delete.bb11:
	or	t1, t2, t1
	beq	t1, zero, .delete.bb13
.delete.bb12:
	beqz	t2, .delete.bb1
.delete.bb14:
	mv	t0, s4	# phi-copy
.delete.bb1:
	mv	a0, t0
	ld	s6, 64(sp)
	ld	s5, 56(sp)
	ld	s4, 48(sp)
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 80
	ret
.delete.bb19:
	slliw	t2, s5, 2
	add	t2, t1, t2
	lw	s6, 0(t2)
	li	t2, -1
	beq	s6, t2, .delete.bb16
.delete.bb20:
	mv	s5, s6	# phi-copy
	bne	s5, t0, .delete.bb19	# rotated-loop-branch
.delete.bb18:
	li	s5, -1
.delete.bb16:
	slliw	t0, s5, 2
	la	t1, value
	add	t0, t1, t0
	lw	t0, 0(t0)
	sw	t0, 0(s2)
	mv	a0, s4	# call-arg-copy
	mv	a1, t0	# call-arg-copy
	call	delete
	mv	t0, a0
	sw	t0, 0(s3)
	j	.delete.bb6
.delete.bb9:
.delete.bb6:
	mv	t0, s1	# phi-copy
	j	.delete.bb1
.delete.bb21:
	j	.delete.bb16
.delete.bb4:
	la	t2, right_child
	add	s2, t2, t1
	lw	a0, 0(s2)
	mv	a1, t0	# call-arg-copy
	call	delete
	mv	t0, a0
	sw	t0, 0(s2)
	j	.delete.bb6
.delete.bb7:
	la	t2, left_child
	add	s2, t2, t1
	lw	a0, 0(s2)
	mv	a1, t0	# call-arg-copy
	call	delete
	mv	t0, a0
	sw	t0, 0(s2)
	j	.delete.bb6
.delete.bb10:
	li	t0, -1
	j	.delete.bb1
.delete.bb13:
	li	t0, -1
	la	t1, left_child	# loop.addr.hoisted
	mv	s5, s4	# phi-copy
.delete.bb17:
	bne	s5, t0, .delete.bb19
	j	.delete.bb18
.delete.bb15:
	j	.delete.bb1
.delete.bb2:
	li	t0, -1
	j	.delete.bb1
	.size delete, .-delete

  .globl inorder
  .type  inorder, @function
inorder:
.inorder.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	mv	t0, a0
	li	t1, -1
	beq	t0, t1, .inorder.bb2
.inorder.bb1:
	slliw	s1, t0, 2
	li	t1, -1
	la	t0, left_child
	add	t0, t0, s1
	lw	t0, 0(t0)
	bne	t0, t1, .inorder.bb3
.inorder.bb4:
	la	t0, value
	add	t0, t0, s1
	lw	a0, 0(t0)
	call	putint
	li	a0, 32
	call	putch
	la	t0, right_child
	add	t0, t0, s1
	lw	a0, 0(t0)
	call	inorder
.inorder.bb2:
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.inorder.bb3:
	mv	a0, t0
	call	inorder
	j	.inorder.bb4
	.size inorder, .-inorder

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 16(sp)
	sd	s1, 24(sp)
	sd	s2, 32(sp)
	sd	s3, 40(sp)
	li	t0, 0
	la	t1, now
	sw	t0, 0(t1)
	call	getint
	mv	s1, a0
	beq	s1, zero, .main.bb1
	j	.main.bb3
.main.bb2:
.main.bb1:
	li	a0, 0
	ld	s3, 40(sp)
	ld	s2, 32(sp)
	ld	s1, 24(sp)
	ld	ra, 16(sp)
	addi	sp, sp, 48
	ret
.main.bb5:
	call	getint
	mv	t0, a0
	li	t5, 0
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	insert
	addiw	s2, s2, 1
	blt	s2, s1, .main.bb5	# rotated-loop-branch
.main.bb6:
	li	t5, 0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	inorder
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	call	getint
	mv	s1, a0
	li	t0, 0
	li	s2, 0
	mv	s3, t0	# phi-copy
.main.bb7:
	bge	s2, s1, .main.bb9
.main.bb8:
	call	getint
	mv	t0, a0
	mv	t5, s3
	sd	t5, 0(sp)
	mv	t5, t0
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	delete
	mv	t0, a0
	addiw	s2, s2, 1
	mv	s3, t0	# phi-copy
	blt	s2, s1, .main.bb8	# rotated-loop-branch
.main.bb9:
	mv	a0, s3
	call	inorder
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	j	.main.bb1
.main.bb3:
	call	getint
	mv	t0, a0
	la	t1, value
	sw	t0, 0(t1)
	li	t0, -1
	la	t1, left_child
	sw	t0, 0(t1)
	la	t1, right_child
	sw	t0, 0(t1)
	li	t0, 1
	la	t1, now
	sw	t0, 0(t1)
	mv	s2, t0	# phi-copy
.main.bb4:
	bge	s2, s1, .main.bb6
	j	.main.bb5
	.size main, .-main
