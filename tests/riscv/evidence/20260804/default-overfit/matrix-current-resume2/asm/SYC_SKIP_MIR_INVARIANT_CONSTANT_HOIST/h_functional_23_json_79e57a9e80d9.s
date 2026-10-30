
  .bss
  .globl pos
  .align 2
  .type  pos, @object
  .size  pos, 4
pos:
  .zero 4

  .bss
  .globl buffer
  .align 2
  .type  buffer, @object
  .size  buffer, 200000000
buffer:
  .zero 200000000

  .text

  .globl skip_space
  .type  skip_space, @function
skip_space:
.skip_space.bb0:
	addi	sp, sp, -16
	sd	s1, 0(sp)
	mv	t0, a0
	mv	t1, a1
	la	t2, pos
	lw	t2, 0(t2)
.skip_space.bb1:
	bge	t2, t1, .skip_space.bb2
.skip_space.bb4:
	slliw	t3, t2, 2
	add	t3, t0, t3
	lw	s1, 0(t3)
	li	t3, 32
	beq	s1, t3, .skip_space.bb5
.skip_space.bb6:
	li	t3, 9
	beq	s1, t3, .skip_space.bb8
.skip_space.bb9:
	li	t3, 10
	beq	s1, t3, .skip_space.bb11
.skip_space.bb12:
	li	t3, 13
	bne	s1, t3, .skip_space.bb2
.skip_space.bb14:
	addiw	t2, t2, 1
	j	.skip_space.bb7
.skip_space.bb13:
	j	.skip_space.bb7
.skip_space.bb10:
.skip_space.bb7:
	blt	t2, t1, .skip_space.bb4	# rotated-loop-branch
	j	.skip_space.bb2	# rotated-loop-exit
.skip_space.bb3:
.skip_space.bb2:
	la	t0, pos
	sw	t2, 0(t0)
	ld	s1, 0(sp)
	addi	sp, sp, 16
	ret
.skip_space.bb5:
	addiw	t2, t2, 1
	j	.skip_space.bb7
.skip_space.bb8:
	addiw	t2, t2, 1
	j	.skip_space.bb7
.skip_space.bb11:
	addiw	t2, t2, 1
	j	.skip_space.bb7
.skip_space.bb15:
	j	.skip_space.bb2
	.size skip_space, .-skip_space

  .globl detect_item
  .type  detect_item, @function
detect_item:
.detect_item.bb0:
	addi	sp, sp, -80
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	sd	s3, 48(sp)
	sd	s4, 56(sp)
	sd	s5, 64(sp)
	mv	s1, a0
	mv	s2, a1
	mv	s3, a2
	la	t0, pos
	lw	t0, 0(t0)
	bge	t0, s3, .detect_item.bb2
.detect_item.bb3:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	beq	s1, zero, .detect_item.bb4
.detect_item.bb5:
	li	t0, 1
	bne	s1, t0, .detect_item.bb25
.detect_item.bb24:
	la	t0, pos
	li	t2, 43
	lw	t0, 0(t0)
	slliw	t1, t0, 2
	add	t1, s2, t1
	lw	t1, 0(t1)
	beq	t1, t2, .detect_item.bb27
.detect_item.bb28:
	li	t2, 45
	beq	t1, t2, .detect_item.bb30
	j	.detect_item.bb29
.detect_item.bb31:
.detect_item.bb29:
	la	t0, pos
	lw	t0, 0(t0)
	bge	t0, s3, .detect_item.bb32
.detect_item.bb33:
	slliw	t0, t0, 2
	li	t1, 48
	add	t0, s2, t0
	lw	t0, 0(t0)
	blt	t0, t1, .detect_item.bb177
.detect_item.bb176:
	li	t1, 57
	blt	t1, t0, .detect_item.bb179
.detect_item.bb178:
	li	t0, 1
.detect_item.bb175:
	bne	t0, zero, .detect_item.bb35
.detect_item.bb34:
	li	t0, 0
.detect_item.bb1:
	mv	a0, t0
	ld	s5, 64(sp)
	ld	s4, 56(sp)
	ld	s3, 48(sp)
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 80
	ret
.detect_item.bb180:
	li	t1, 1
	bne	t0, t1, .detect_item.bb38
.detect_item.bb40:
	addiw	t0, s4, 1
	sw	t0, 0(s1)
.detect_item.bb36:
	lw	s4, 0(s1)
	bge	s4, s3, .detect_item.bb38
.detect_item.bb37:
	slliw	t0, s4, 2
	add	t0, s2, t0
	lw	s5, 0(t0)
	li	t0, 48
	blt	s5, t0, .detect_item.bb182
.detect_item.bb181:
	li	t0, 57
	blt	t0, s5, .detect_item.bb184
.detect_item.bb183:
	li	t0, 1
	j	.detect_item.bb180
.detect_item.bb182:
	li	t0, 0
	j	.detect_item.bb180
.detect_item.bb184:
	li	t0, 0
	j	.detect_item.bb180
.detect_item.bb38:
	la	t0, pos
	lw	t0, 0(t0)
	blt	t0, s3, .detect_item.bb41
.detect_item.bb42:
	la	t0, pos
	lw	t0, 0(t0)
	blt	t0, s3, .detect_item.bb50
	j	.detect_item.bb26
.detect_item.bb51:
.detect_item.bb26:
	li	t0, 1
	j	.detect_item.bb1
.detect_item.bb39:
	j	.detect_item.bb38
.detect_item.bb185:
	li	t1, 1
	bne	t0, t1, .detect_item.bb42
.detect_item.bb49:
	addiw	t0, s4, 1
	sw	t0, 0(s1)
.detect_item.bb45:
	lw	s4, 0(s1)
	bge	s4, s3, .detect_item.bb42
.detect_item.bb46:
	slliw	t0, s4, 2
	add	t0, s2, t0
	lw	s5, 0(t0)
	li	t0, 48
	blt	s5, t0, .detect_item.bb187
.detect_item.bb186:
	li	t0, 57
	blt	t0, s5, .detect_item.bb189
.detect_item.bb188:
	li	t0, 1
	j	.detect_item.bb185
.detect_item.bb187:
	li	t0, 0
	j	.detect_item.bb185
.detect_item.bb189:
	li	t0, 0
	j	.detect_item.bb185
.detect_item.bb47:
	j	.detect_item.bb42
.detect_item.bb44:
	j	.detect_item.bb42
.detect_item.bb48:
	j	.detect_item.bb42
.detect_item.bb190:
	li	t1, 1
	bne	t0, t1, .detect_item.bb26
.detect_item.bb66:
	addiw	t0, s4, 1
	sw	t0, 0(s1)
.detect_item.bb62:
	lw	s4, 0(s1)
	bge	s4, s3, .detect_item.bb26
.detect_item.bb63:
	slliw	t0, s4, 2
	add	t0, s2, t0
	lw	s5, 0(t0)
	li	t0, 48
	blt	s5, t0, .detect_item.bb192
.detect_item.bb191:
	li	t0, 57
	blt	t0, s5, .detect_item.bb194
.detect_item.bb193:
	li	t0, 1
	j	.detect_item.bb190
.detect_item.bb192:
	li	t0, 0
	j	.detect_item.bb190
.detect_item.bb194:
	li	t0, 0
	j	.detect_item.bb190
.detect_item.bb64:
	j	.detect_item.bb26
.detect_item.bb53:
	j	.detect_item.bb26
.detect_item.bb65:
	j	.detect_item.bb26
.detect_item.bb41:
	slliw	t1, t0, 2
	li	t2, 46
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb42
.detect_item.bb43:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	la	s1, pos	# loop.addr.hoisted
	j	.detect_item.bb45
.detect_item.bb50:
	slliw	t1, t0, 2
	li	t2, 101
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb26
.detect_item.bb52:
	addiw	t1, t0, 1
	la	t2, pos
	sw	t1, 0(t2)
	blt	t1, s3, .detect_item.bb54
.detect_item.bb55:
	la	t0, pos
	lw	t0, 0(t0)
	blt	t0, s3, .detect_item.bb58
.detect_item.bb59:
	la	s1, pos	# loop.addr.hoisted
	j	.detect_item.bb62
.detect_item.bb58:
	slliw	t1, t0, 2
	li	t2, 45
	add	t1, s2, t1
	lw	t1, 0(t1)
	beq	t1, t2, .detect_item.bb60
	j	.detect_item.bb59
.detect_item.bb61:
	j	.detect_item.bb59
.detect_item.bb70:
	lw	s4, 0(s1)
	bge	s4, s3, .detect_item.bb72
.detect_item.bb71:
	slliw	t0, s4, 2
	add	t0, s2, t0
	lw	s5, 0(t0)
	li	t0, 34
	beq	s5, t0, .detect_item.bb72
.detect_item.bb74:
	li	t0, 92
	beq	s5, t0, .detect_item.bb75
.detect_item.bb76:
	addiw	t0, s4, 1
	sw	t0, 0(s1)
	j	.detect_item.bb70
.detect_item.bb77:
	j	.detect_item.bb70
.detect_item.bb75:
	addiw	t0, s4, 2
	sw	t0, 0(s1)
	j	.detect_item.bb70
.detect_item.bb72:
	la	t0, pos
	lw	t0, 0(t0)
	bge	t0, s3, .detect_item.bb78
.detect_item.bb79:
	slliw	t1, t0, 2
	li	t2, 34
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb80
.detect_item.bb81:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb26
.detect_item.bb69:
	j	.detect_item.bb26
.detect_item.bb73:
	j	.detect_item.bb72
.detect_item.bb91:
	la	t0, pos
	li	t1, 44
	lw	s1, 0(t0)
	slliw	t0, s1, 2
	add	t0, s2, t0
	lw	t0, 0(t0)
	bne	t0, t1, .detect_item.bb93
.detect_item.bb92:
	addiw	t0, s1, 1
	la	t1, pos
	sw	t0, 0(t1)
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 0
	call	detect_item
	mv	t0, a0
	beq	t0, zero, .detect_item.bb94
.detect_item.bb95:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	j	.detect_item.bb91
.detect_item.bb93:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	la	t0, pos
	lw	t0, 0(t0)
	bge	t0, s3, .detect_item.bb96
.detect_item.bb97:
	slliw	t1, t0, 2
	li	t2, 93
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb98
.detect_item.bb99:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb26
.detect_item.bb84:
	j	.detect_item.bb26
.detect_item.bb94:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb115:
	la	t0, pos
	li	t1, 44
	lw	s1, 0(t0)
	slliw	t0, s1, 2
	add	t0, s2, t0
	lw	t0, 0(t0)
	bne	t0, t1, .detect_item.bb117
.detect_item.bb116:
	addiw	t0, s1, 1
	la	t1, pos
	sw	t0, 0(t1)
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 2
	call	detect_item
	mv	t0, a0
	beq	t0, zero, .detect_item.bb118
.detect_item.bb119:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	la	t0, pos
	lw	s1, 0(t0)
	bge	s1, s3, .detect_item.bb120
.detect_item.bb121:
	slliw	t0, s1, 2
	li	t1, 58
	add	t0, s2, t0
	lw	t0, 0(t0)
	bne	t0, t1, .detect_item.bb122
.detect_item.bb123:
	addiw	t0, s1, 1
	la	t1, pos
	sw	t0, 0(t1)
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 0
	call	detect_item
	mv	t0, a0
	beq	t0, zero, .detect_item.bb124
.detect_item.bb125:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	j	.detect_item.bb115
.detect_item.bb117:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	la	t0, pos
	lw	t0, 0(t0)
	bge	t0, s3, .detect_item.bb126
.detect_item.bb127:
	slliw	t1, t0, 2
	li	t2, 125
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb128
.detect_item.bb129:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb26
.detect_item.bb102:
	j	.detect_item.bb26
.detect_item.bb118:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb120:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb122:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb124:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb4:
	la	t0, pos
	li	t1, 123
	lw	t0, 0(t0)
	slliw	t0, t0, 2
	add	t0, s2, t0
	lw	t0, 0(t0)
	beq	t0, t1, .detect_item.bb6
.detect_item.bb7:
	li	t1, 91
	beq	t0, t1, .detect_item.bb8
.detect_item.bb9:
	li	t1, 34
	beq	t0, t1, .detect_item.bb10
.detect_item.bb11:
	li	t1, 48
	blt	t0, t1, .detect_item.bb172
.detect_item.bb171:
	li	t1, 57
	blt	t1, t0, .detect_item.bb174
.detect_item.bb173:
	li	t1, 1
.detect_item.bb170:
	li	t2, 1
	beq	t1, t2, .detect_item.bb12
.detect_item.bb13:
	li	t1, 43
	beq	t0, t1, .detect_item.bb14
.detect_item.bb15:
	li	t1, 45
	beq	t0, t1, .detect_item.bb16
.detect_item.bb17:
	li	t1, 116
	beq	t0, t1, .detect_item.bb18
.detect_item.bb19:
	li	t1, 102
	beq	t0, t1, .detect_item.bb20
.detect_item.bb21:
	li	t1, 110
	beq	t0, t1, .detect_item.bb22
.detect_item.bb23:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb12:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 1
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb25:
	li	t0, 2
	beq	s1, t0, .detect_item.bb67
.detect_item.bb68:
	li	t0, 3
	bne	s1, t0, .detect_item.bb83
.detect_item.bb82:
	la	s1, pos
	lw	t0, 0(s1)
	addiw	t0, t0, 1
	sw	t0, 0(s1)
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	lw	t0, 0(s1)
	blt	t0, s3, .detect_item.bb85
.detect_item.bb86:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 0
	call	detect_item
	mv	t0, a0
	bne	t0, zero, .detect_item.bb90
.detect_item.bb89:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb35:
	la	s1, pos	# loop.addr.hoisted
	j	.detect_item.bb36
.detect_item.bb54:
	slliw	t1, t1, 2
	li	t2, 43
	add	t1, s2, t1
	lw	t1, 0(t1)
	beq	t1, t2, .detect_item.bb56
	j	.detect_item.bb55
.detect_item.bb57:
	j	.detect_item.bb55
.detect_item.bb83:
	li	t0, 4
	bne	s1, t0, .detect_item.bb101
.detect_item.bb100:
	la	s1, pos
	lw	t0, 0(s1)
	addiw	t0, t0, 1
	sw	t0, 0(s1)
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	lw	t0, 0(s1)
	blt	t0, s3, .detect_item.bb103
.detect_item.bb104:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 2
	call	detect_item
	mv	t0, a0
	beq	t0, zero, .detect_item.bb107
.detect_item.bb108:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	la	t0, pos
	lw	t0, 0(t0)
	bge	t0, s3, .detect_item.bb109
.detect_item.bb110:
	slliw	t1, t0, 2
	li	t2, 58
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb111
.detect_item.bb112:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 0
	call	detect_item
	mv	t0, a0
	beq	t0, zero, .detect_item.bb113
.detect_item.bb114:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	j	.detect_item.bb115
.detect_item.bb85:
	slliw	t1, t0, 2
	li	t2, 93
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb86
.detect_item.bb87:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	li	t0, 1
	j	.detect_item.bb1
.detect_item.bb101:
	li	t0, 5
	bne	s1, t0, .detect_item.bb131
.detect_item.bb130:
	la	t0, pos
	lw	t0, 0(t0)
	addiw	t1, t0, 3
	bge	t1, s3, .detect_item.bb133
.detect_item.bb134:
	slliw	t2, t0, 2
	li	t3, 116
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb135
.detect_item.bb136:
	addiw	t2, t0, 1
	li	t3, 114
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb137
.detect_item.bb138:
	addiw	t2, t0, 2
	li	t3, 117
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb139
.detect_item.bb140:
	slliw	t1, t1, 2
	li	t2, 101
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb141
.detect_item.bb142:
	addiw	t0, t0, 4
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb26
.detect_item.bb132:
	j	.detect_item.bb26
.detect_item.bb103:
	slliw	t1, t0, 2
	li	t2, 125
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb104
.detect_item.bb105:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	li	t0, 1
	j	.detect_item.bb1
.detect_item.bb131:
	li	t0, 6
	bne	s1, t0, .detect_item.bb144
.detect_item.bb143:
	la	t0, pos
	lw	t0, 0(t0)
	addiw	t1, t0, 4
	bge	t1, s3, .detect_item.bb146
.detect_item.bb147:
	slliw	t2, t0, 2
	li	t3, 102
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb148
.detect_item.bb149:
	addiw	t2, t0, 1
	li	t3, 97
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb150
.detect_item.bb151:
	addiw	t2, t0, 2
	li	t3, 108
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb152
.detect_item.bb153:
	addiw	t2, t0, 3
	li	t3, 115
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb154
.detect_item.bb155:
	slliw	t1, t1, 2
	li	t2, 101
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb156
.detect_item.bb157:
	addiw	t0, t0, 5
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb26
.detect_item.bb145:
	j	.detect_item.bb26
.detect_item.bb144:
	li	t0, 7
	bne	s1, t0, .detect_item.bb159
.detect_item.bb158:
	la	t0, pos
	lw	t0, 0(t0)
	addiw	t1, t0, 3
	bge	t1, s3, .detect_item.bb160
.detect_item.bb161:
	slliw	t2, t0, 2
	li	t3, 110
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb162
.detect_item.bb163:
	addiw	t2, t0, 1
	li	t3, 117
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb164
.detect_item.bb165:
	addiw	t2, t0, 2
	li	t3, 108
	slliw	t2, t2, 2
	add	t2, s2, t2
	lw	t2, 0(t2)
	bne	t2, t3, .detect_item.bb166
.detect_item.bb167:
	slliw	t1, t1, 2
	li	t2, 108
	add	t1, s2, t1
	lw	t1, 0(t1)
	bne	t1, t2, .detect_item.bb168
.detect_item.bb169:
	addiw	t0, t0, 4
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb26
.detect_item.bb32:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb78:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb90:
	mv	a0, s2	# call-arg-copy
	mv	a1, s3	# call-arg-copy
	call	skip_space
	j	.detect_item.bb91
.detect_item.bb107:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb6:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 4
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb8:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 3
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb10:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 2
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb14:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 1
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb16:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 1
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb18:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 5
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb20:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 6
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb22:
	mv	a1, s2	# call-arg-copy
	mv	a2, s3	# call-arg-copy
	li	a0, 7
	call	detect_item
	mv	t0, a0
	j	.detect_item.bb1
.detect_item.bb27:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb29
.detect_item.bb30:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb29
.detect_item.bb56:
	addiw	t0, t0, 2
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb55
.detect_item.bb60:
	addiw	t0, t0, 1
	la	t1, pos
	sw	t0, 0(t1)
	j	.detect_item.bb59
.detect_item.bb67:
	la	t0, pos
	la	s1, pos	# loop.addr.hoisted
	lw	t1, 0(t0)
	addiw	t1, t1, 1
	sw	t1, 0(t0)
	j	.detect_item.bb70
.detect_item.bb80:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb88:
	j	.detect_item.bb86
.detect_item.bb96:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb98:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb106:
	j	.detect_item.bb104
.detect_item.bb109:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb111:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb113:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb126:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb128:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb133:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb135:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb137:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb139:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb141:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb146:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb148:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb150:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb152:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb154:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb156:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb159:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb160:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb162:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb164:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb166:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb168:
	li	t0, 0
	j	.detect_item.bb1
.detect_item.bb172:
	li	t1, 0
	j	.detect_item.bb170
.detect_item.bb174:
	li	t1, 0
	j	.detect_item.bb170
.detect_item.bb177:
	li	t0, 0
	j	.detect_item.bb175
.detect_item.bb179:
	li	t0, 0
	j	.detect_item.bb175
.detect_item.bb2:
	li	t0, 0
	j	.detect_item.bb1
	.size detect_item, .-detect_item

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 24(sp)
	sd	s1, 32(sp)
	sd	s2, 40(sp)
	call	getch
	mv	t0, a0
	li	s1, 0
.main.bb2:
	li	t1, 35
	beq	t0, t1, .main.bb4
.main.bb3:
	slliw	t1, s1, 2
	la	t2, buffer
	add	t1, t2, t1
	sw	t0, 0(t1)
	addiw	s1, s1, 1
	call	getch
	mv	t0, a0
	j	.main.bb2
.main.bb4:
	la	t5, buffer
	sd	t5, 0(sp)
	mv	t5, s1
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	skip_space
	li	t5, 0
	sd	t5, 0(sp)
	la	t5, buffer
	sd	t5, 8(sp)
	mv	t5, s1
	sd	t5, 16(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	call	detect_item
	mv	s2, a0
	la	t5, buffer
	sd	t5, 0(sp)
	mv	t5, s1
	sd	t5, 8(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	call	skip_space
	bne	s2, zero, .main.bb5
.main.bb6:
	li	a0, 110
	call	putch
	li	t5, 111
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 116
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 111
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 107
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 1
.main.bb1:
	mv	a0, t0
	ld	s2, 40(sp)
	ld	s1, 32(sp)
	ld	ra, 24(sp)
	addi	sp, sp, 48
	ret
.main.bb5:
	li	a0, 111
	call	putch
	li	t5, 107
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t0, 0
	j	.main.bb1
	.size main, .-main
