
  .bss
  .globl sum
  .align 2
  .type  sum, @object
  .size  sum, 4
sum:
  .zero 4

  .bss
  .globl array
  .align 2
  .type  array, @object
  .size  array, 80
array:
  .zero 80

  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	la	t0, sum
	lw	t0, 0(t0)
	la	t1, array
	li	t3, 20
	la	s1, array	# loop.addr.hoisted
	li	t2, 0
.main.bb1:
	bge	t2, t3, .main.bb3
.main.bb2:
	addiw	a2, t0, 1
	blt	zero, t2, .main.bb141
.main.bb140:
	li	a3, 0
.main.bb139:
	li	a4, 0
	subw	a3, a3, a4
	sltu	s2, zero, a3
	beqz	s2, .main.bb302
.main.bb23:
	addiw	t0, t0, 2
	li	a2, 1
	blt	a2, t2, .main.bb144
.main.bb143:
	li	a2, 0
.main.bb142:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb22:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb301
.main.bb24:
	addiw	t0, t0, 1
	li	a2, 2
	blt	a2, t2, .main.bb147
.main.bb146:
	li	a2, 0
.main.bb145:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb21:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb300
.main.bb25:
	addiw	t0, t0, 1
	li	a2, 3
	blt	a2, t2, .main.bb150
.main.bb149:
	li	a2, 0
.main.bb148:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb20:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb299
.main.bb26:
	addiw	t0, t0, 1
	li	a2, 4
	blt	a2, t2, .main.bb153
.main.bb152:
	li	a2, 0
.main.bb151:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb19:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb298
.main.bb27:
	addiw	t0, t0, 1
	li	a2, 5
	blt	a2, t2, .main.bb156
.main.bb155:
	li	a2, 0
.main.bb154:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb18:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb297
.main.bb28:
	addiw	t0, t0, 1
	li	a2, 6
	blt	a2, t2, .main.bb159
.main.bb158:
	li	a2, 0
.main.bb157:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb17:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb296
.main.bb29:
	addiw	t0, t0, 1
	li	a2, 7
	blt	a2, t2, .main.bb162
.main.bb161:
	li	a2, 0
.main.bb160:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb16:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb295
.main.bb30:
	addiw	t0, t0, 1
	li	a2, 8
	blt	a2, t2, .main.bb165
.main.bb164:
	li	a2, 0
.main.bb163:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb15:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb294
.main.bb31:
	addiw	t0, t0, 1
	li	a2, 9
	blt	a2, t2, .main.bb168
.main.bb167:
	li	a2, 0
.main.bb166:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb14:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb293
.main.bb32:
	addiw	t0, t0, 1
	li	a2, 10
	blt	a2, t2, .main.bb171
.main.bb170:
	li	a2, 0
.main.bb169:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb13:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb292
.main.bb33:
	addiw	t0, t0, 1
	li	a2, 11
	blt	a2, t2, .main.bb174
.main.bb173:
	li	a2, 0
.main.bb172:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb12:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb291
.main.bb34:
	addiw	t0, t0, 1
	li	a2, 12
	blt	a2, t2, .main.bb177
.main.bb176:
	li	a2, 0
.main.bb175:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb11:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb290
.main.bb35:
	addiw	t0, t0, 1
	li	a2, 13
	blt	a2, t2, .main.bb180
.main.bb179:
	li	a2, 0
.main.bb178:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb10:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb289
.main.bb36:
	addiw	t0, t0, 1
	li	a2, 14
	blt	a2, t2, .main.bb183
.main.bb182:
	li	a2, 0
.main.bb181:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb9:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb288
.main.bb37:
	addiw	t0, t0, 1
	li	a2, 15
	blt	a2, t2, .main.bb186
.main.bb185:
	li	a2, 0
.main.bb184:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb8:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb287
.main.bb38:
	addiw	t0, t0, 1
	li	a2, 16
	blt	a2, t2, .main.bb189
.main.bb188:
	li	a2, 0
.main.bb187:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb7:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb286
.main.bb39:
	addiw	t0, t0, 1
	li	a2, 17
	blt	a2, t2, .main.bb192
.main.bb191:
	li	a2, 0
.main.bb190:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb6:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb285
.main.bb40:
	addiw	t0, t0, 1
	li	a2, 18
	blt	a2, t2, .main.bb195
.main.bb194:
	li	a2, 0
.main.bb193:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb5:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb284
.main.bb41:
	addiw	t0, t0, 1
.main.bb196:
	li	a2, 0
.main.bb4:
	beq	a2, zero, .main.bb43
.main.bb43:
	addiw	t2, t2, 1
	blt	t2, t3, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	t2, 0
	li	t3, 20
	la	s1, array	# loop.addr.hoisted
.main.bb44:
	bge	t2, t3, .main.bb46
.main.bb45:
	addiw	a2, t0, 2
	blt	zero, t2, .main.bb199
.main.bb198:
	li	a3, 1
.main.bb197:
	li	a4, 0
	subw	a3, a3, a4
	sltu	s2, zero, a3
	bnez	s2, .main.bb321
.main.bb66:
	addiw	t0, t0, 4
	li	a2, 1
	blt	a2, t2, .main.bb202
.main.bb201:
	li	a2, 1
.main.bb200:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb65:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb320
.main.bb67:
	addiw	t0, t0, 2
	li	a2, 2
	blt	a2, t2, .main.bb205
.main.bb204:
	li	a2, 1
.main.bb203:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb64:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb319
.main.bb68:
	addiw	t0, t0, 2
	li	a2, 3
	blt	a2, t2, .main.bb208
.main.bb207:
	li	a2, 1
.main.bb206:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb63:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb318
.main.bb69:
	addiw	t0, t0, 2
	li	a2, 4
	blt	a2, t2, .main.bb211
.main.bb210:
	li	a2, 1
.main.bb209:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb62:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb317
.main.bb70:
	addiw	t0, t0, 2
	li	a2, 5
	blt	a2, t2, .main.bb214
.main.bb213:
	li	a2, 1
.main.bb212:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb61:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb316
.main.bb71:
	addiw	t0, t0, 2
	li	a2, 6
	blt	a2, t2, .main.bb217
.main.bb216:
	li	a2, 1
.main.bb215:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb60:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb315
.main.bb72:
	addiw	t0, t0, 2
	li	a2, 7
	blt	a2, t2, .main.bb220
.main.bb219:
	li	a2, 1
.main.bb218:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb59:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb314
.main.bb73:
	addiw	t0, t0, 2
	li	a2, 8
	blt	a2, t2, .main.bb223
.main.bb222:
	li	a2, 1
.main.bb221:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb58:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb313
.main.bb74:
	addiw	t0, t0, 2
	li	a2, 9
	blt	a2, t2, .main.bb226
.main.bb225:
	li	a2, 1
.main.bb224:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb57:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb312
.main.bb75:
	addiw	t0, t0, 2
	li	a2, 10
	blt	a2, t2, .main.bb229
.main.bb228:
	li	a2, 1
.main.bb227:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb56:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb311
.main.bb76:
	addiw	t0, t0, 2
	li	a2, 11
	blt	a2, t2, .main.bb232
.main.bb231:
	li	a2, 1
.main.bb230:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb55:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb310
.main.bb77:
	addiw	t0, t0, 2
	li	a2, 12
	blt	a2, t2, .main.bb235
.main.bb234:
	li	a2, 1
.main.bb233:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb54:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb309
.main.bb78:
	addiw	t0, t0, 2
	li	a2, 13
	blt	a2, t2, .main.bb238
.main.bb237:
	li	a2, 1
.main.bb236:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb53:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb308
.main.bb79:
	addiw	t0, t0, 2
	li	a2, 14
	blt	a2, t2, .main.bb241
.main.bb240:
	li	a2, 1
.main.bb239:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb52:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb307
.main.bb80:
	addiw	t0, t0, 2
	li	a2, 15
	blt	a2, t2, .main.bb244
.main.bb243:
	li	a2, 1
.main.bb242:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb51:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb306
.main.bb81:
	addiw	t0, t0, 2
	li	a2, 16
	blt	a2, t2, .main.bb247
.main.bb246:
	li	a2, 1
.main.bb245:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb50:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb305
.main.bb82:
	addiw	t0, t0, 2
	li	a2, 17
	blt	a2, t2, .main.bb250
.main.bb249:
	li	a2, 1
.main.bb248:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb49:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	bnez	s2, .main.bb304
.main.bb83:
	addiw	t0, t0, 2
	li	a2, 18
	blt	a2, t2, .main.bb253
.main.bb252:
	li	a2, 1
.main.bb251:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb48:
	li	a3, 0
	subw	a2, a2, a3
	sltu	s2, zero, a2
	beqz	s2, .main.bb84
.main.bb303:
	mv	a2, s2	# phi-copy
.main.bb47:
	beq	a2, zero, .main.bb86
.main.bb86:
	addiw	t2, t2, 1
	blt	t2, t3, .main.bb45	# rotated-loop-branch
.main.bb46:
	li	t2, 1
	la	s1, array	# loop.addr.hoisted
	addiw	t3, t2, -1	# loop.addr.init
	addiw	t3, t3, -1	# loop.addr.init
	slliw	t3, t3, 2	# loop.addr.init
	add	t3, s1, t3	# loop.addr.init
.main.bb87:
	li	a2, 20
	slt	s2, t2, a2
	beqz	s2, .main.bb322
.main.bb91:
	addiw	s2, t2, -1
	addiw	t0, t0, 1
	bge	s2, t2, .main.bb257
.main.bb258:
	li	a2, 1
	sw	a2, 0(t1)
	beq	s2, zero, .main.bb259
.main.bb260:
	lw	a2, 0(t3)
.main.bb256:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.main.bb90:
	beq	a2, zero, .main.bb89
.main.bb88:
	addiw	t2, t2, 1
	addi	t1, t1, 4
	addi	t3, t3, 4	# loop.addr.bump
	j	.main.bb87
.main.bb42:
	j	.main.bb43
.main.bb84:
	addiw	t0, t0, 2
.main.bb254:
	li	a2, 1
	j	.main.bb47
.main.bb85:
	j	.main.bb86
.main.bb141:
	li	a3, 1
	sw	a3, 0(s1)
	j	.main.bb139
.main.bb199:
	li	a3, 0
	sw	a3, 0(s1)
	j	.main.bb197
.main.bb284:
	mv	a2, s2	# phi-copy
	j	.main.bb4
.main.bb285:
	mv	a2, s2	# phi-copy
	j	.main.bb5
.main.bb286:
	mv	a2, s2	# phi-copy
	j	.main.bb6
.main.bb287:
	mv	a2, s2	# phi-copy
	j	.main.bb7
.main.bb288:
	mv	a2, s2	# phi-copy
	j	.main.bb8
.main.bb289:
	mv	a2, s2	# phi-copy
	j	.main.bb9
.main.bb290:
	mv	a2, s2	# phi-copy
	j	.main.bb10
.main.bb291:
	mv	a2, s2	# phi-copy
	j	.main.bb11
.main.bb292:
	mv	a2, s2	# phi-copy
	j	.main.bb12
.main.bb293:
	mv	a2, s2	# phi-copy
	j	.main.bb13
.main.bb294:
	mv	a2, s2	# phi-copy
	j	.main.bb14
.main.bb295:
	mv	a2, s2	# phi-copy
	j	.main.bb15
.main.bb296:
	mv	a2, s2	# phi-copy
	j	.main.bb16
.main.bb297:
	mv	a2, s2	# phi-copy
	j	.main.bb17
.main.bb298:
	mv	a2, s2	# phi-copy
	j	.main.bb18
.main.bb299:
	mv	a2, s2	# phi-copy
	j	.main.bb19
.main.bb300:
	mv	a2, s2	# phi-copy
	j	.main.bb20
.main.bb301:
	mv	a2, s2	# phi-copy
	j	.main.bb21
.main.bb302:
	mv	t0, a2	# phi-copy
	mv	a2, s2	# phi-copy
	j	.main.bb22
.main.bb304:
	mv	a2, s2	# phi-copy
	j	.main.bb48
.main.bb305:
	mv	a2, s2	# phi-copy
	j	.main.bb49
.main.bb306:
	mv	a2, s2	# phi-copy
	j	.main.bb50
.main.bb307:
	mv	a2, s2	# phi-copy
	j	.main.bb51
.main.bb308:
	mv	a2, s2	# phi-copy
	j	.main.bb52
.main.bb309:
	mv	a2, s2	# phi-copy
	j	.main.bb53
.main.bb310:
	mv	a2, s2	# phi-copy
	j	.main.bb54
.main.bb311:
	mv	a2, s2	# phi-copy
	j	.main.bb55
.main.bb312:
	mv	a2, s2	# phi-copy
	j	.main.bb56
.main.bb313:
	mv	a2, s2	# phi-copy
	j	.main.bb57
.main.bb314:
	mv	a2, s2	# phi-copy
	j	.main.bb58
.main.bb315:
	mv	a2, s2	# phi-copy
	j	.main.bb59
.main.bb316:
	mv	a2, s2	# phi-copy
	j	.main.bb60
.main.bb317:
	mv	a2, s2	# phi-copy
	j	.main.bb61
.main.bb318:
	mv	a2, s2	# phi-copy
	j	.main.bb62
.main.bb319:
	mv	a2, s2	# phi-copy
	j	.main.bb63
.main.bb320:
	mv	a2, s2	# phi-copy
	j	.main.bb64
.main.bb321:
	mv	t0, a2	# phi-copy
	mv	a2, s2	# phi-copy
	j	.main.bb65
.main.bb322:
	mv	a2, s2	# phi-copy
	j	.main.bb90
.main.bb144:
	li	a2, 1
	sw	a2, 4(s1)
	lw	a2, 0(s1)
	j	.main.bb142
.main.bb147:
	li	a2, 1
	sw	a2, 8(s1)
	lw	a2, 4(s1)
	j	.main.bb145
.main.bb150:
	li	a2, 1
	sw	a2, 12(s1)
	lw	a2, 8(s1)
	j	.main.bb148
.main.bb153:
	li	a2, 1
	sw	a2, 16(s1)
	lw	a2, 12(s1)
	j	.main.bb151
.main.bb156:
	li	a2, 1
	sw	a2, 20(s1)
	lw	a2, 16(s1)
	j	.main.bb154
.main.bb159:
	li	a2, 1
	sw	a2, 24(s1)
	lw	a2, 20(s1)
	j	.main.bb157
.main.bb162:
	li	a2, 1
	sw	a2, 28(s1)
	lw	a2, 24(s1)
	j	.main.bb160
.main.bb165:
	li	a2, 1
	sw	a2, 32(s1)
	lw	a2, 28(s1)
	j	.main.bb163
.main.bb168:
	li	a2, 1
	sw	a2, 36(s1)
	lw	a2, 32(s1)
	j	.main.bb166
.main.bb171:
	li	a2, 1
	sw	a2, 40(s1)
	lw	a2, 36(s1)
	j	.main.bb169
.main.bb174:
	li	a2, 1
	sw	a2, 44(s1)
	lw	a2, 40(s1)
	j	.main.bb172
.main.bb177:
	li	a2, 1
	sw	a2, 48(s1)
	lw	a2, 44(s1)
	j	.main.bb175
.main.bb180:
	li	a2, 1
	sw	a2, 52(s1)
	lw	a2, 48(s1)
	j	.main.bb178
.main.bb183:
	li	a2, 1
	sw	a2, 56(s1)
	lw	a2, 52(s1)
	j	.main.bb181
.main.bb186:
	li	a2, 1
	sw	a2, 60(s1)
	lw	a2, 56(s1)
	j	.main.bb184
.main.bb189:
	li	a2, 1
	sw	a2, 64(s1)
	lw	a2, 60(s1)
	j	.main.bb187
.main.bb192:
	li	a2, 1
	sw	a2, 68(s1)
	lw	a2, 64(s1)
	j	.main.bb190
.main.bb195:
	li	a2, 1
	sw	a2, 72(s1)
	lw	a2, 68(s1)
	j	.main.bb193
.main.bb202:
	li	a2, 0
	sw	a2, 4(s1)
	lw	a2, 0(s1)
	j	.main.bb200
.main.bb205:
	li	a2, 0
	sw	a2, 8(s1)
	lw	a2, 4(s1)
	j	.main.bb203
.main.bb208:
	li	a2, 0
	sw	a2, 12(s1)
	lw	a2, 8(s1)
	j	.main.bb206
.main.bb211:
	li	a2, 0
	sw	a2, 16(s1)
	lw	a2, 12(s1)
	j	.main.bb209
.main.bb214:
	li	a2, 0
	sw	a2, 20(s1)
	lw	a2, 16(s1)
	j	.main.bb212
.main.bb217:
	li	a2, 0
	sw	a2, 24(s1)
	lw	a2, 20(s1)
	j	.main.bb215
.main.bb220:
	li	a2, 0
	sw	a2, 28(s1)
	lw	a2, 24(s1)
	j	.main.bb218
.main.bb223:
	li	a2, 0
	sw	a2, 32(s1)
	lw	a2, 28(s1)
	j	.main.bb221
.main.bb226:
	li	a2, 0
	sw	a2, 36(s1)
	lw	a2, 32(s1)
	j	.main.bb224
.main.bb229:
	li	a2, 0
	sw	a2, 40(s1)
	lw	a2, 36(s1)
	j	.main.bb227
.main.bb232:
	li	a2, 0
	sw	a2, 44(s1)
	lw	a2, 40(s1)
	j	.main.bb230
.main.bb235:
	li	a2, 0
	sw	a2, 48(s1)
	lw	a2, 44(s1)
	j	.main.bb233
.main.bb238:
	li	a2, 0
	sw	a2, 52(s1)
	lw	a2, 48(s1)
	j	.main.bb236
.main.bb241:
	li	a2, 0
	sw	a2, 56(s1)
	lw	a2, 52(s1)
	j	.main.bb239
.main.bb244:
	li	a2, 0
	sw	a2, 60(s1)
	lw	a2, 56(s1)
	j	.main.bb242
.main.bb247:
	li	a2, 0
	sw	a2, 64(s1)
	lw	a2, 60(s1)
	j	.main.bb245
.main.bb250:
	li	a2, 0
	sw	a2, 68(s1)
	lw	a2, 64(s1)
	j	.main.bb248
.main.bb253:
	li	a2, 0
	sw	a2, 72(s1)
	lw	a2, 68(s1)
	j	.main.bb251
.main.bb257:
	li	a2, 0
	j	.main.bb256
.main.bb259:
	lw	a2, 0(s1)
	j	.main.bb256
.main.bb89:
	la	t2, array
	addiw	t1, t0, 3
	lw	t2, 0(t2)
.main.bb255:
	li	t3, 0
	subw	t3, t2, t3
	sltu	t3, zero, t3
	beqz	t3, .main.bb325
.main.bb95:
	la	t1, array
	addiw	t0, t0, 6
	lw	t1, 4(t1)
.main.bb261:
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.main.bb94:
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
	bnez	t1, .main.bb93
	j	.main.bb96
.main.bb324:
.main.bb93:
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
	bnez	t1, .main.bb92
	j	.main.bb97
.main.bb323:
.main.bb92:
	beq	t1, zero, .main.bb99
.main.bb99:
	la	t3, array
	addiw	t1, t0, 3
	lw	t3, 16(t3)
.main.bb264:
	li	a2, 0
	subw	a2, t3, a2
	sltiu	a2, a2, 1
	beqz	a2, .main.bb102
.main.bb327:
	mv	t0, t1	# phi-copy
	mv	t1, a2	# phi-copy
.main.bb101:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
	bnez	t1, .main.bb100
	j	.main.bb107
.main.bb326:
.main.bb100:
	beq	t1, zero, .main.bb109
.main.bb109:
	la	a2, array
	addiw	t1, t0, 3
	lw	a2, 36(a2)
.main.bb269:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
	beqz	a2, .main.bb334
.main.bb115:
	la	t1, array
	addiw	t0, t0, 6
	lw	t1, 40(t1)
.main.bb270:
	li	a2, 0
	subw	t1, t1, a2
	sltiu	t1, t1, 1
.main.bb114:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
	bnez	t1, .main.bb113
	j	.main.bb116
.main.bb333:
.main.bb113:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
	bnez	t1, .main.bb112
	j	.main.bb117
.main.bb332:
.main.bb112:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
	bnez	t1, .main.bb111
	j	.main.bb118
.main.bb331:
.main.bb111:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
	bnez	t1, .main.bb110
	j	.main.bb119
.main.bb330:
.main.bb110:
	beq	t1, zero, .main.bb123
.main.bb123:
	addiw	t1, t0, 3
.main.bb276:
	li	a2, 0
	subw	t2, t2, a2
	sltu	t2, zero, t2
	beqz	t2, .main.bb341
.main.bb130:
	la	t1, array
	addiw	t0, t0, 6
	lw	t1, 8(t1)
.main.bb277:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
.main.bb129:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	beqz	t1, .main.bb128
.main.bb131:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 12(t1)
.main.bb278:
	li	t2, 0
	subw	t1, t1, t2
	sltiu	t1, t1, 1
.main.bb128:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	beqz	t1, .main.bb127
.main.bb132:
	addiw	t0, t0, 3
.main.bb279:
	li	t1, 0
	subw	t1, t3, t1
	sltiu	t1, t1, 1
.main.bb127:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	bnez	t1, .main.bb126
	j	.main.bb133
.main.bb338:
.main.bb126:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	bnez	t1, .main.bb125
	j	.main.bb134
.main.bb337:
.main.bb125:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	beqz	t1, .main.bb137
.main.bb336:
	mv	s1, t0	# phi-copy
.main.bb124:
	li	t0, 0
	subw	t0, t1, t0
	sltu	t0, zero, t0
.main.bb138:
	addw	a0, s1, t0
	call	putint
	la	t0, sum
	sw	s1, 0(t0)
	li	a0, 0
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
.main.bb96:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 8(t1)
.main.bb262:
	li	t3, 0
	subw	t1, t1, t3
	sltiu	t1, t1, 1
	j	.main.bb93
.main.bb97:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 12(t1)
.main.bb263:
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
	j	.main.bb92
.main.bb98:
	j	.main.bb99
.main.bb107:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 32(t1)
.main.bb268:
	li	a2, 0
	subw	t1, t1, a2
	sltiu	t1, t1, 1
	j	.main.bb100
.main.bb108:
	j	.main.bb109
.main.bb116:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 44(t1)
.main.bb271:
	li	a2, 0
	subw	t1, t1, a2
	sltiu	t1, t1, 1
	j	.main.bb113
.main.bb117:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 48(t1)
.main.bb272:
	li	a2, 0
	subw	t1, t1, a2
	sltiu	t1, t1, 1
	j	.main.bb112
.main.bb118:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 52(t1)
.main.bb273:
	li	a2, 0
	subw	t1, t1, a2
	sltiu	t1, t1, 1
	j	.main.bb111
.main.bb119:
	la	a2, array
	addiw	t1, t0, 3
	lw	a2, 56(a2)
.main.bb274:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
	beqz	a2, .main.bb335
.main.bb121:
	la	t1, array
	addiw	t0, t0, 6
	lw	t1, 60(t1)
.main.bb275:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
.main.bb120:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
	j	.main.bb110
.main.bb122:
	j	.main.bb123
.main.bb133:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 20(t1)
.main.bb280:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	j	.main.bb126
.main.bb134:
	la	t2, array
	addiw	t1, t0, 3
	lw	t2, 24(t2)
.main.bb281:
	li	t3, 0
	subw	t2, t2, t3
	sltu	t2, zero, t2
	beqz	t2, .main.bb342
.main.bb136:
	la	t1, array
	addiw	t0, t0, 6
	lw	t1, 28(t1)
.main.bb282:
	li	t2, 0
	subw	t1, t1, t2
	sltiu	t1, t1, 1
.main.bb135:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	j	.main.bb125
.main.bb137:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 32(t1)
.main.bb283:
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	mv	s1, t0	# phi-copy
	j	.main.bb124
.main.bb339:
	j	.main.bb127
.main.bb340:
	j	.main.bb128
.main.bb102:
	la	a2, array
	addiw	t1, t0, 6
	lw	a2, 20(a2)
.main.bb265:
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
	beqz	a2, .main.bb329
.main.bb105:
	la	t1, array
	addiw	t0, t0, 9
	lw	t1, 24(t1)
.main.bb266:
	li	a2, 0
	subw	t1, t1, a2
	sltiu	t1, t1, 1
.main.bb104:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
	beqz	t1, .main.bb103
.main.bb106:
	la	t1, array
	addiw	t0, t0, 3
	lw	t1, 28(t1)
.main.bb267:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
.main.bb103:
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
	j	.main.bb101
.main.bb328:
	j	.main.bb103
.main.bb325:
	mv	t0, t1	# phi-copy
	mv	t1, t3	# phi-copy
	j	.main.bb94
.main.bb329:
	mv	t0, t1	# phi-copy
	mv	t1, a2	# phi-copy
	j	.main.bb104
.main.bb334:
	mv	t0, t1	# phi-copy
	mv	t1, a2	# phi-copy
	j	.main.bb114
.main.bb335:
	mv	t0, t1	# phi-copy
	mv	t1, a2	# phi-copy
	j	.main.bb120
.main.bb341:
	mv	t0, t1	# phi-copy
	mv	t1, t2	# phi-copy
	j	.main.bb129
.main.bb342:
	mv	t0, t1	# phi-copy
	mv	t1, t2	# phi-copy
	j	.main.bb135
	.size main, .-main
