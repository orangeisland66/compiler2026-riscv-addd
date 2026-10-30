
  .text

  .globl emit_start
  .type  emit_start, @function
emit_start:
.emit_start.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 2
	call	_sysy_starttime
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size emit_start, .-emit_start

  .globl emit_int
  .type  emit_int, @function
emit_int:
.emit_int.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 7
	call	putint
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size emit_int, .-emit_int

  .globl emit_float
  .type  emit_float, @function
emit_float:
.emit_float.bb0:
	addi	sp, sp, -32
	sd	ra, 16(sp)
	li	t5, 1065353216
	fmv.w.x	fa0, t5
	call	putfloat
	ld	ra, 16(sp)
	addi	sp, sp, 32
	ret
	.size emit_float, .-emit_float

  .globl emit_char
  .type  emit_char, @function
emit_char:
.emit_char.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 10
	call	putch
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size emit_char, .-emit_char

  .globl emit_stop
  .type  emit_stop, @function
emit_stop:
.emit_stop.bb0:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	li	a0, 18
	call	_sysy_stoptime
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
	.size emit_stop, .-emit_stop

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 32(sp)
	call	getint
	mv	t5, a0
	sd	t5, 0(sp)
	beq	t5, zero, .main.bb2
.main.bb3:
	li	t6, 1
	ld	t5, 0(sp)
	beq	t5, t6, .main.bb4
.main.bb5:
	li	t6, 2
	ld	t5, 0(sp)
	beq	t5, t6, .main.bb6
.main.bb7:
	li	t6, 3
	ld	t5, 0(sp)
	beq	t5, t6, .main.bb8
.main.bb9:
	call	emit_stop
.main.bb1:
	li	a0, 0
	ld	ra, 32(sp)
	addi	sp, sp, 48
	ret
.main.bb4:
	call	emit_int
	j	.main.bb1
.main.bb6:
	call	emit_float
	j	.main.bb1
.main.bb8:
	call	emit_char
	j	.main.bb1
.main.bb2:
	call	emit_start
	j	.main.bb1
	.size main, .-main
