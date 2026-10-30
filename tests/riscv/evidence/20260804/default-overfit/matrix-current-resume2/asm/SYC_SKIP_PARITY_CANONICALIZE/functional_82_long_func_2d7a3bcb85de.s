
  .section .rodata
  .globl SHIFT_TABLE
  .align 2
  .type  SHIFT_TABLE, @object
  .size  SHIFT_TABLE, 64
SHIFT_TABLE:
  .word 1
  .word 2
  .word 4
  .word 8
  .word 16
  .word 32
  .word 64
  .word 128
  .word 256
  .word 512
  .word 1024
  .word 2048
  .word 4096
  .word 8192
  .word 16384
  .word 32768

  .text

  .globl long_func
  .type  long_func, @function
long_func:
.long_func.bb0:
	addi	sp, sp, -96
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	sd	s4, 40(sp)
	sd	s5, 48(sp)
	sd	s6, 56(sp)
	sd	s7, 64(sp)
	sd	s8, 72(sp)
	sd	s9, 80(sp)
	la	s1, SHIFT_TABLE
	li	t0, 1
	li	t1, 0
	li	t2, 2
.long_func.bb2:
	bge	zero, t1, .long_func.bb4
.long_func.bb3:
	li	t3, 0
	li	a2, 1
	li	a3, 0
	li	a4, 16
	mv	s2, t1	# phi-copy
	mv	s3, s1	# phi-copy
.long_func.bb5:
	bge	t3, a4, .long_func.bb7
.long_func.bb6:
	li	a6, 0
	srliw	s4, s2, 31	# const-div.pow2.bias-fold
	addw	s2, s2, s4
	andi	a5, s2, 1
	subw	a5, a5, s4
	subw	a5, a5, a6
	sltu	a5, zero, a5
	srliw	s4, a2, 31	# const-div.pow2.bias-fold
	addw	s5, a2, s4
	andi	a2, s5, 1
	subw	a2, a2, s4
	subw	a2, a2, a6
	sltu	a2, zero, a2
	and	a2, a5, a2
	beq	a2, zero, .long_func.bb546
.long_func.bb8:
	lw	a2, 0(s3)
	addw	a2, a3, a2
.long_func.bb9:
	sraiw	s2, s2, 1
	sraiw	a5, s5, 1
	addiw	t3, t3, 1
	addi	s3, s3, 4
	mv	a3, a2	# phi-copy
	mv	a2, a5	# phi-copy
	j	.long_func.bb5
.long_func.bb296:
	bge	t3, a3, .long_func.bb298
.long_func.bb297:
	srliw	s9, s8, 31	# const-div.pow2.bias-fold
	addw	s8, s8, s9
	andi	a4, s8, 1
	subw	a4, a4, s9
	beq	a4, zero, .long_func.bb300
.long_func.bb299:
	srliw	s9, s7, 31	# const-div.pow2.bias-fold
	addw	a4, s7, s9
	andi	a4, a4, 1
	subw	a4, a4, s9
	bne	a4, zero, .long_func.bb301
.long_func.bb302:
	lw	a4, 0(s6)
	addw	a2, a2, a4
	j	.long_func.bb301
.long_func.bb303:
.long_func.bb301:
	sraiw	s8, s8, 1
	srliw	a5, s7, 31	# const-div.pow2.bias-fold
	addiw	t3, t3, 1
	addw	a5, s7, a5
	addi	s6, s6, 4
	sraiw	s7, a5, 1
	j	.long_func.bb296
.long_func.bb300:
	srliw	s9, s7, 31	# const-div.pow2.bias-fold
	addw	a4, s7, s9
	andi	a4, a4, 1
	subw	a4, a4, s9
	beq	a4, zero, .long_func.bb301
.long_func.bb304:
	lw	a4, 0(s6)
	addw	a2, a2, a4
	j	.long_func.bb301
.long_func.bb305:
	j	.long_func.bb301
.long_func.bb594:
	j	.long_func.bb301
.long_func.bb595:
	j	.long_func.bb301
.long_func.bb298:
	li	t3, 0
	li	a3, 0
	li	a4, 16
	mv	s6, s1	# phi-copy
.long_func.bb306:
	bge	t3, a4, .long_func.bb308
.long_func.bb307:
	li	a6, 0
	srliw	s7, t2, 31	# const-div.pow2.bias-fold
	addw	s8, t2, s7
	li	a5, 0
	andi	t2, s8, 1
	subw	t2, t2, s7
	subw	t2, t2, a5
	sltu	t2, zero, t2
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	s5, s5, s7
	andi	a5, s5, 1
	subw	a5, a5, s7
	subw	a5, a5, a6
	sltu	a5, zero, a5
	and	t2, t2, a5
	beq	t2, zero, .long_func.bb596
.long_func.bb309:
	lw	t2, 0(s6)
	addw	t2, a3, t2
.long_func.bb310:
	sraiw	a3, s8, 1
	sraiw	s5, s5, 1
	addiw	t3, t3, 1
	addi	s6, s6, 4
	mv	t5, t2	# phi-cycle-break
	mv	t2, a3	# phi-copy
	mv	a3, t5	# phi-copy
	j	.long_func.bb306
.long_func.bb596:
	mv	t2, a3	# phi-copy
	j	.long_func.bb310
.long_func.bb308:
	slliw	t2, a3, 1
	li	t3, 0
	li	a3, 65535
	li	a4, 0
	li	a5, 16
	mv	s5, s1	# phi-copy
.long_func.bb311:
	bge	t3, a5, .long_func.bb313
.long_func.bb312:
	srliw	s6, t2, 31	# const-div.pow2.bias-fold
	addw	s7, t2, s6
	li	a6, 0
	andi	t2, s7, 1
	subw	t2, t2, s6
	subw	t2, t2, a6
	sltu	t2, zero, t2
	srliw	s6, a3, 31	# const-div.pow2.bias-fold
	addw	s8, a3, s6
	andi	a3, s8, 1
	subw	a3, a3, s6
	subw	a3, a3, a6
	sltu	a3, zero, a3
	and	t2, t2, a3
	beq	t2, zero, .long_func.bb597
.long_func.bb314:
	lw	t2, 0(s5)
	addw	t2, a4, t2
.long_func.bb315:
	sraiw	a3, s7, 1
	sraiw	a4, s8, 1
	addiw	t3, t3, 1
	addi	s5, s5, 4
	mv	t5, t2	# phi-cycle-break
	mv	t2, a3	# phi-copy
	mv	a3, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb311
.long_func.bb597:
	mv	t2, a4	# phi-copy
	j	.long_func.bb315
.long_func.bb313:
	mv	t2, a2	# phi-copy
	mv	s5, a4	# phi-copy
.long_func.bb293:
	beq	s5, zero, .long_func.bb292
.long_func.bb294:
	li	t3, 0
	li	a2, 0
	li	a3, 16
	mv	s6, s1	# phi-copy
	mv	s7, s5	# phi-copy
	mv	s8, t2	# phi-copy
	j	.long_func.bb296
.long_func.bb295:
.long_func.bb292:
	mv	s5, t0	# phi-copy
.long_func.bb316:
	beq	s5, zero, .long_func.bb318
.long_func.bb317:
	li	t3, 0
	li	a2, 0
	li	a3, 16
	mv	s6, s1	# phi-copy
	mv	s7, s5	# phi-copy
	mv	s8, t0	# phi-copy
.long_func.bb319:
	bge	t3, a3, .long_func.bb321
.long_func.bb320:
	srliw	s9, s8, 31	# const-div.pow2.bias-fold
	addw	s8, s8, s9
	andi	a4, s8, 1
	subw	a4, a4, s9
	beq	a4, zero, .long_func.bb323
.long_func.bb322:
	srliw	s9, s7, 31	# const-div.pow2.bias-fold
	addw	a4, s7, s9
	andi	a4, a4, 1
	subw	a4, a4, s9
	bne	a4, zero, .long_func.bb324
.long_func.bb325:
	lw	a4, 0(s6)
	addw	a2, a2, a4
	j	.long_func.bb324
.long_func.bb326:
.long_func.bb324:
	sraiw	s8, s8, 1
	srliw	a5, s7, 31	# const-div.pow2.bias-fold
	addiw	t3, t3, 1
	addw	a5, s7, a5
	addi	s6, s6, 4
	sraiw	s7, a5, 1
	j	.long_func.bb319
.long_func.bb323:
	srliw	s9, s7, 31	# const-div.pow2.bias-fold
	addw	a4, s7, s9
	andi	a4, a4, 1
	subw	a4, a4, s9
	beq	a4, zero, .long_func.bb324
.long_func.bb327:
	lw	a4, 0(s6)
	addw	a2, a2, a4
	j	.long_func.bb324
.long_func.bb328:
	j	.long_func.bb324
.long_func.bb598:
	j	.long_func.bb324
.long_func.bb599:
	j	.long_func.bb324
.long_func.bb321:
	li	t3, 0
	li	a3, 0
	li	a4, 16
	mv	s6, s1	# phi-copy
.long_func.bb329:
	bge	t3, a4, .long_func.bb331
.long_func.bb330:
	li	a6, 0
	srliw	s7, t0, 31	# const-div.pow2.bias-fold
	addw	s8, t0, s7
	li	a5, 0
	andi	t0, s8, 1
	subw	t0, t0, s7
	subw	t0, t0, a5
	sltu	t0, zero, t0
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	s5, s5, s7
	andi	a5, s5, 1
	subw	a5, a5, s7
	subw	a5, a5, a6
	sltu	a5, zero, a5
	and	t0, t0, a5
	beq	t0, zero, .long_func.bb600
.long_func.bb332:
	lw	t0, 0(s6)
	addw	t0, a3, t0
.long_func.bb333:
	sraiw	a3, s8, 1
	sraiw	s5, s5, 1
	addiw	t3, t3, 1
	addi	s6, s6, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a3	# phi-copy
	mv	a3, t5	# phi-copy
	j	.long_func.bb329
.long_func.bb600:
	mv	t0, a3	# phi-copy
	j	.long_func.bb333
.long_func.bb331:
	slliw	t0, a3, 1
	li	t3, 0
	li	a3, 65535
	li	a4, 0
	li	a5, 16
	mv	s5, s1	# phi-copy
.long_func.bb334:
	bge	t3, a5, .long_func.bb336
.long_func.bb335:
	srliw	s6, t0, 31	# const-div.pow2.bias-fold
	addw	s7, t0, s6
	li	a6, 0
	andi	t0, s7, 1
	subw	t0, t0, s6
	subw	t0, t0, a6
	sltu	t0, zero, t0
	srliw	s6, a3, 31	# const-div.pow2.bias-fold
	addw	s8, a3, s6
	andi	a3, s8, 1
	subw	a3, a3, s6
	subw	a3, a3, a6
	sltu	a3, zero, a3
	and	t0, t0, a3
	beq	t0, zero, .long_func.bb601
.long_func.bb337:
	lw	t0, 0(s5)
	addw	t0, a4, t0
.long_func.bb338:
	sraiw	a3, s7, 1
	sraiw	a4, s8, 1
	addiw	t3, t3, 1
	addi	s5, s5, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a3	# phi-copy
	mv	a3, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb334
.long_func.bb601:
	mv	t0, a4	# phi-copy
	j	.long_func.bb338
.long_func.bb336:
	mv	t0, a2	# phi-copy
	mv	s5, a4	# phi-copy
	j	.long_func.bb316
.long_func.bb318:
	li	t3, 32767
	bge	t3, s4, .long_func.bb341
.long_func.bb340:
	li	a2, 65536
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	t3, t3, 1
	addw	t3, t3, a2
	li	a2, 32768
	subw	t3, t3, a2
	j	.long_func.bb339
.long_func.bb342:
.long_func.bb339:
	mv	s4, t3	# phi-copy
.long_func.bb283:
	beq	s4, zero, .long_func.bb282
.long_func.bb284:
	li	t3, 0
	li	a2, 1
	li	a3, 0
	li	a4, 16
	mv	s5, s4	# phi-copy
	mv	s6, s1	# phi-copy
.long_func.bb286:
	bge	t3, a4, .long_func.bb288
.long_func.bb287:
	li	a6, 0
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	s5, s5, s7
	andi	a5, s5, 1
	subw	a5, a5, s7
	subw	a5, a5, a6
	sltu	a5, zero, a5
	srliw	s7, a2, 31	# const-div.pow2.bias-fold
	addw	s8, a2, s7
	andi	a2, s8, 1
	subw	a2, a2, s7
	subw	a2, a2, a6
	sltu	a2, zero, a2
	and	a2, a5, a2
	beq	a2, zero, .long_func.bb592
.long_func.bb289:
	lw	a2, 0(s6)
	addw	a2, a3, a2
.long_func.bb290:
	sraiw	s5, s5, 1
	sraiw	a5, s8, 1
	addiw	t3, t3, 1
	addi	s6, s6, 4
	mv	a3, a2	# phi-copy
	mv	a2, a5	# phi-copy
	j	.long_func.bb286
.long_func.bb592:
	mv	a2, a3	# phi-copy
	j	.long_func.bb290
.long_func.bb288:
	beq	a3, zero, .long_func.bb292
.long_func.bb291:
	mv	s5, t0	# phi-copy
	j	.long_func.bb293
.long_func.bb341:
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	t3, t3, 1
	j	.long_func.bb339
.long_func.bb593:
	j	.long_func.bb292
.long_func.bb356:
	bge	t3, a3, .long_func.bb358
.long_func.bb357:
	srliw	s9, s8, 31	# const-div.pow2.bias-fold
	addw	s8, s8, s9
	andi	a4, s8, 1
	subw	a4, a4, s9
	beq	a4, zero, .long_func.bb360
.long_func.bb359:
	srliw	s9, s7, 31	# const-div.pow2.bias-fold
	addw	a4, s7, s9
	andi	a4, a4, 1
	subw	a4, a4, s9
	bne	a4, zero, .long_func.bb361
.long_func.bb362:
	lw	a4, 0(s6)
	addw	a2, a2, a4
	j	.long_func.bb361
.long_func.bb363:
.long_func.bb361:
	sraiw	s8, s8, 1
	srliw	a5, s7, 31	# const-div.pow2.bias-fold
	addiw	t3, t3, 1
	addw	a5, s7, a5
	addi	s6, s6, 4
	sraiw	s7, a5, 1
	j	.long_func.bb356
.long_func.bb360:
	srliw	s9, s7, 31	# const-div.pow2.bias-fold
	addw	a4, s7, s9
	andi	a4, a4, 1
	subw	a4, a4, s9
	beq	a4, zero, .long_func.bb361
.long_func.bb364:
	lw	a4, 0(s6)
	addw	a2, a2, a4
	j	.long_func.bb361
.long_func.bb365:
	j	.long_func.bb361
.long_func.bb604:
	j	.long_func.bb361
.long_func.bb605:
	j	.long_func.bb361
.long_func.bb358:
	li	t3, 0
	li	a3, 0
	li	a4, 16
	mv	s6, s1	# phi-copy
.long_func.bb366:
	bge	t3, a4, .long_func.bb368
.long_func.bb367:
	li	a6, 0
	srliw	s7, t0, 31	# const-div.pow2.bias-fold
	addw	s8, t0, s7
	li	a5, 0
	andi	t0, s8, 1
	subw	t0, t0, s7
	subw	t0, t0, a5
	sltu	t0, zero, t0
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	s5, s5, s7
	andi	a5, s5, 1
	subw	a5, a5, s7
	subw	a5, a5, a6
	sltu	a5, zero, a5
	and	t0, t0, a5
	beq	t0, zero, .long_func.bb606
.long_func.bb369:
	lw	t0, 0(s6)
	addw	t0, a3, t0
.long_func.bb370:
	sraiw	a3, s8, 1
	sraiw	s5, s5, 1
	addiw	t3, t3, 1
	addi	s6, s6, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a3	# phi-copy
	mv	a3, t5	# phi-copy
	j	.long_func.bb366
.long_func.bb606:
	mv	t0, a3	# phi-copy
	j	.long_func.bb370
.long_func.bb368:
	slliw	t0, a3, 1
	li	t3, 0
	li	a3, 65535
	li	a4, 0
	li	a5, 16
	mv	s5, s1	# phi-copy
.long_func.bb371:
	bge	t3, a5, .long_func.bb373
.long_func.bb372:
	srliw	s6, t0, 31	# const-div.pow2.bias-fold
	addw	s7, t0, s6
	li	a6, 0
	andi	t0, s7, 1
	subw	t0, t0, s6
	subw	t0, t0, a6
	sltu	t0, zero, t0
	srliw	s6, a3, 31	# const-div.pow2.bias-fold
	addw	s8, a3, s6
	andi	a3, s8, 1
	subw	a3, a3, s6
	subw	a3, a3, a6
	sltu	a3, zero, a3
	and	t0, t0, a3
	beq	t0, zero, .long_func.bb607
.long_func.bb374:
	lw	t0, 0(s5)
	addw	t0, a4, t0
.long_func.bb375:
	sraiw	a3, s7, 1
	sraiw	a4, s8, 1
	addiw	t3, t3, 1
	addi	s5, s5, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a3	# phi-copy
	mv	a3, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb371
.long_func.bb607:
	mv	t0, a4	# phi-copy
	j	.long_func.bb375
.long_func.bb373:
	mv	t0, a2	# phi-copy
	mv	s5, a4	# phi-copy
.long_func.bb353:
	beq	s5, zero, .long_func.bb352
.long_func.bb354:
	li	t3, 0
	li	a2, 0
	li	a3, 16
	mv	s6, s1	# phi-copy
	mv	s7, s5	# phi-copy
	mv	s8, t0	# phi-copy
	j	.long_func.bb356
.long_func.bb355:
.long_func.bb352:
	mv	s5, t1	# phi-copy
.long_func.bb376:
	beq	s5, zero, .long_func.bb378
.long_func.bb377:
	li	t3, 0
	li	a2, 0
	li	a3, 16
	mv	s6, s1	# phi-copy
	mv	s7, s5	# phi-copy
	mv	s8, t1	# phi-copy
.long_func.bb379:
	bge	t3, a3, .long_func.bb381
.long_func.bb380:
	srliw	s9, s8, 31	# const-div.pow2.bias-fold
	addw	s8, s8, s9
	andi	a4, s8, 1
	subw	a4, a4, s9
	beq	a4, zero, .long_func.bb383
.long_func.bb382:
	srliw	s9, s7, 31	# const-div.pow2.bias-fold
	addw	a4, s7, s9
	andi	a4, a4, 1
	subw	a4, a4, s9
	bne	a4, zero, .long_func.bb384
.long_func.bb385:
	lw	a4, 0(s6)
	addw	a2, a2, a4
	j	.long_func.bb384
.long_func.bb386:
.long_func.bb384:
	sraiw	s8, s8, 1
	srliw	a5, s7, 31	# const-div.pow2.bias-fold
	addiw	t3, t3, 1
	addw	a5, s7, a5
	addi	s6, s6, 4
	sraiw	s7, a5, 1
	j	.long_func.bb379
.long_func.bb383:
	srliw	s9, s7, 31	# const-div.pow2.bias-fold
	addw	a4, s7, s9
	andi	a4, a4, 1
	subw	a4, a4, s9
	beq	a4, zero, .long_func.bb384
.long_func.bb387:
	lw	a4, 0(s6)
	addw	a2, a2, a4
	j	.long_func.bb384
.long_func.bb388:
	j	.long_func.bb384
.long_func.bb608:
	j	.long_func.bb384
.long_func.bb609:
	j	.long_func.bb384
.long_func.bb381:
	li	t3, 0
	li	a3, 0
	li	a4, 16
	mv	s6, s1	# phi-copy
.long_func.bb389:
	bge	t3, a4, .long_func.bb391
.long_func.bb390:
	li	a6, 0
	srliw	s7, t1, 31	# const-div.pow2.bias-fold
	addw	s8, t1, s7
	li	a5, 0
	andi	t1, s8, 1
	subw	t1, t1, s7
	subw	t1, t1, a5
	sltu	t1, zero, t1
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	s5, s5, s7
	andi	a5, s5, 1
	subw	a5, a5, s7
	subw	a5, a5, a6
	sltu	a5, zero, a5
	and	t1, t1, a5
	beq	t1, zero, .long_func.bb610
.long_func.bb392:
	lw	t1, 0(s6)
	addw	t1, a3, t1
.long_func.bb393:
	sraiw	a3, s8, 1
	sraiw	s5, s5, 1
	addiw	t3, t3, 1
	addi	s6, s6, 4
	mv	t5, t1	# phi-cycle-break
	mv	t1, a3	# phi-copy
	mv	a3, t5	# phi-copy
	j	.long_func.bb389
.long_func.bb610:
	mv	t1, a3	# phi-copy
	j	.long_func.bb393
.long_func.bb391:
	slliw	t1, a3, 1
	li	t3, 0
	li	a3, 65535
	li	a4, 0
	li	a5, 16
	mv	s5, s1	# phi-copy
.long_func.bb394:
	bge	t3, a5, .long_func.bb396
.long_func.bb395:
	srliw	s6, t1, 31	# const-div.pow2.bias-fold
	addw	s7, t1, s6
	li	a6, 0
	andi	t1, s7, 1
	subw	t1, t1, s6
	subw	t1, t1, a6
	sltu	t1, zero, t1
	srliw	s6, a3, 31	# const-div.pow2.bias-fold
	addw	s8, a3, s6
	andi	a3, s8, 1
	subw	a3, a3, s6
	subw	a3, a3, a6
	sltu	a3, zero, a3
	and	t1, t1, a3
	beq	t1, zero, .long_func.bb611
.long_func.bb397:
	lw	t1, 0(s5)
	addw	t1, a4, t1
.long_func.bb398:
	sraiw	a3, s7, 1
	sraiw	a4, s8, 1
	addiw	t3, t3, 1
	addi	s5, s5, 4
	mv	t5, t1	# phi-cycle-break
	mv	t1, a3	# phi-copy
	mv	a3, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb394
.long_func.bb611:
	mv	t1, a4	# phi-copy
	j	.long_func.bb398
.long_func.bb396:
	mv	t1, a2	# phi-copy
	mv	s5, a4	# phi-copy
	j	.long_func.bb376
.long_func.bb378:
	li	t3, 32767
	bge	t3, s4, .long_func.bb401
.long_func.bb400:
	li	a2, 65536
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	t3, t3, 1
	addw	t3, t3, a2
	li	a2, 32768
	subw	t3, t3, a2
	j	.long_func.bb399
.long_func.bb402:
.long_func.bb399:
	mv	s4, t3	# phi-copy
.long_func.bb343:
	beq	s4, zero, .long_func.bb345
.long_func.bb344:
	li	t3, 0
	li	a2, 1
	li	a3, 0
	li	a4, 16
	mv	s5, s4	# phi-copy
	mv	s6, s1	# phi-copy
.long_func.bb346:
	bge	t3, a4, .long_func.bb348
.long_func.bb347:
	li	a6, 0
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	s5, s5, s7
	andi	a5, s5, 1
	subw	a5, a5, s7
	subw	a5, a5, a6
	sltu	a5, zero, a5
	srliw	s7, a2, 31	# const-div.pow2.bias-fold
	addw	s8, a2, s7
	andi	a2, s8, 1
	subw	a2, a2, s7
	subw	a2, a2, a6
	sltu	a2, zero, a2
	and	a2, a5, a2
	beq	a2, zero, .long_func.bb602
.long_func.bb349:
	lw	a2, 0(s6)
	addw	a2, a3, a2
.long_func.bb350:
	sraiw	s5, s5, 1
	sraiw	a5, s8, 1
	addiw	t3, t3, 1
	addi	s6, s6, 4
	mv	a3, a2	# phi-copy
	mv	a2, a5	# phi-copy
	j	.long_func.bb346
.long_func.bb602:
	mv	a2, a3	# phi-copy
	j	.long_func.bb350
.long_func.bb348:
	beq	a3, zero, .long_func.bb352
.long_func.bb351:
	mv	s5, t1	# phi-copy
	j	.long_func.bb353
.long_func.bb401:
	srliw	t3, s4, 31	# const-div.pow2.bias-fold
	addw	t3, s4, t3
	sraiw	t3, t3, 1
	j	.long_func.bb399
.long_func.bb603:
	j	.long_func.bb352
.long_func.bb433:
	bge	a2, a4, .long_func.bb435
.long_func.bb434:
	srliw	s8, s7, 31	# const-div.pow2.bias-fold
	addw	s7, s7, s8
	andi	a5, s7, 1
	subw	a5, a5, s8
	beq	a5, zero, .long_func.bb437
.long_func.bb436:
	srliw	s8, s6, 31	# const-div.pow2.bias-fold
	addw	a5, s6, s8
	andi	a5, a5, 1
	subw	a5, a5, s8
	bne	a5, zero, .long_func.bb438
.long_func.bb439:
	lw	a5, 0(s5)
	addw	a3, a3, a5
	j	.long_func.bb438
.long_func.bb440:
.long_func.bb438:
	sraiw	s7, s7, 1
	srliw	a6, s6, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s6, a6
	addi	s5, s5, 4
	sraiw	s6, a6, 1
	j	.long_func.bb433
.long_func.bb437:
	srliw	s8, s6, 31	# const-div.pow2.bias-fold
	addw	a5, s6, s8
	andi	a5, a5, 1
	subw	a5, a5, s8
	beq	a5, zero, .long_func.bb438
.long_func.bb441:
	lw	a5, 0(s5)
	addw	a3, a3, a5
	j	.long_func.bb438
.long_func.bb442:
	j	.long_func.bb438
.long_func.bb616:
	j	.long_func.bb438
.long_func.bb617:
	j	.long_func.bb438
.long_func.bb435:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s5, s1	# phi-copy
.long_func.bb443:
	bge	a2, a5, .long_func.bb445
.long_func.bb444:
	li	a7, 0
	srliw	s6, t3, 31	# const-div.pow2.bias-fold
	addw	s7, t3, s6
	li	a6, 0
	andi	t3, s7, 1
	subw	t3, t3, s6
	subw	t3, t3, a6
	sltu	t3, zero, t3
	srliw	s6, s4, 31	# const-div.pow2.bias-fold
	addw	s4, s4, s6
	andi	a6, s4, 1
	subw	a6, a6, s6
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t3, t3, a6
	beq	t3, zero, .long_func.bb618
.long_func.bb446:
	lw	t3, 0(s5)
	addw	t3, a4, t3
.long_func.bb447:
	sraiw	a4, s7, 1
	sraiw	s4, s4, 1
	addiw	a2, a2, 1
	addi	s5, s5, 4
	mv	t5, t3	# phi-cycle-break
	mv	t3, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb443
.long_func.bb618:
	mv	t3, a4	# phi-copy
	j	.long_func.bb447
.long_func.bb445:
	slliw	t3, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s4, s1	# phi-copy
.long_func.bb448:
	bge	a2, a6, .long_func.bb450
.long_func.bb449:
	srliw	s5, t3, 31	# const-div.pow2.bias-fold
	addw	s6, t3, s5
	li	a7, 0
	andi	t3, s6, 1
	subw	t3, t3, s5
	subw	t3, t3, a7
	sltu	t3, zero, t3
	srliw	s5, a4, 31	# const-div.pow2.bias-fold
	addw	s7, a4, s5
	andi	a4, s7, 1
	subw	a4, a4, s5
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t3, t3, a4
	beq	t3, zero, .long_func.bb619
.long_func.bb451:
	lw	t3, 0(s4)
	addw	t3, a5, t3
.long_func.bb452:
	sraiw	a4, s6, 1
	sraiw	a5, s7, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t3	# phi-cycle-break
	mv	t3, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb448
.long_func.bb619:
	mv	t3, a5	# phi-copy
	j	.long_func.bb452
.long_func.bb450:
	mv	t3, a3	# phi-copy
	mv	s4, a5	# phi-copy
.long_func.bb430:
	beq	s4, zero, .long_func.bb429
.long_func.bb431:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s5, s1	# phi-copy
	mv	s6, s4	# phi-copy
	mv	s7, t3	# phi-copy
	j	.long_func.bb433
.long_func.bb432:
.long_func.bb429:
	mv	s4, t1	# phi-copy
.long_func.bb453:
	beq	s4, zero, .long_func.bb455
.long_func.bb454:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s5, s1	# phi-copy
	mv	s6, s4	# phi-copy
	mv	s7, t1	# phi-copy
.long_func.bb456:
	bge	a2, a4, .long_func.bb458
.long_func.bb457:
	srliw	s8, s7, 31	# const-div.pow2.bias-fold
	addw	s7, s7, s8
	andi	a5, s7, 1
	subw	a5, a5, s8
	beq	a5, zero, .long_func.bb460
.long_func.bb459:
	srliw	s8, s6, 31	# const-div.pow2.bias-fold
	addw	a5, s6, s8
	andi	a5, a5, 1
	subw	a5, a5, s8
	bne	a5, zero, .long_func.bb461
.long_func.bb462:
	lw	a5, 0(s5)
	addw	a3, a3, a5
	j	.long_func.bb461
.long_func.bb463:
.long_func.bb461:
	sraiw	s7, s7, 1
	srliw	a6, s6, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s6, a6
	addi	s5, s5, 4
	sraiw	s6, a6, 1
	j	.long_func.bb456
.long_func.bb460:
	srliw	s8, s6, 31	# const-div.pow2.bias-fold
	addw	a5, s6, s8
	andi	a5, a5, 1
	subw	a5, a5, s8
	beq	a5, zero, .long_func.bb461
.long_func.bb464:
	lw	a5, 0(s5)
	addw	a3, a3, a5
	j	.long_func.bb461
.long_func.bb465:
	j	.long_func.bb461
.long_func.bb620:
	j	.long_func.bb461
.long_func.bb621:
	j	.long_func.bb461
.long_func.bb458:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s5, s1	# phi-copy
.long_func.bb466:
	bge	a2, a5, .long_func.bb468
.long_func.bb467:
	li	a7, 0
	srliw	s6, t1, 31	# const-div.pow2.bias-fold
	addw	s7, t1, s6
	li	a6, 0
	andi	t1, s7, 1
	subw	t1, t1, s6
	subw	t1, t1, a6
	sltu	t1, zero, t1
	srliw	s6, s4, 31	# const-div.pow2.bias-fold
	addw	s4, s4, s6
	andi	a6, s4, 1
	subw	a6, a6, s6
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t1, t1, a6
	beq	t1, zero, .long_func.bb622
.long_func.bb469:
	lw	t1, 0(s5)
	addw	t1, a4, t1
.long_func.bb470:
	sraiw	a4, s7, 1
	sraiw	s4, s4, 1
	addiw	a2, a2, 1
	addi	s5, s5, 4
	mv	t5, t1	# phi-cycle-break
	mv	t1, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb466
.long_func.bb622:
	mv	t1, a4	# phi-copy
	j	.long_func.bb470
.long_func.bb468:
	slliw	t1, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s4, s1	# phi-copy
.long_func.bb471:
	bge	a2, a6, .long_func.bb473
.long_func.bb472:
	srliw	s5, t1, 31	# const-div.pow2.bias-fold
	addw	s6, t1, s5
	li	a7, 0
	andi	t1, s6, 1
	subw	t1, t1, s5
	subw	t1, t1, a7
	sltu	t1, zero, t1
	srliw	s5, a4, 31	# const-div.pow2.bias-fold
	addw	s7, a4, s5
	andi	a4, s7, 1
	subw	a4, a4, s5
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t1, t1, a4
	beq	t1, zero, .long_func.bb623
.long_func.bb474:
	lw	t1, 0(s4)
	addw	t1, a5, t1
.long_func.bb475:
	sraiw	a4, s6, 1
	sraiw	a5, s7, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t1	# phi-cycle-break
	mv	t1, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb471
.long_func.bb623:
	mv	t1, a5	# phi-copy
	j	.long_func.bb475
.long_func.bb473:
	mv	t1, a3	# phi-copy
	mv	s4, a5	# phi-copy
	j	.long_func.bb453
.long_func.bb455:
	li	a2, 32767
	bge	a2, s3, .long_func.bb478
.long_func.bb477:
	li	a3, 65536
	srliw	a2, s3, 31	# const-div.pow2.bias-fold
	addw	a2, s3, a2
	sraiw	a2, a2, 1
	addw	a2, a2, a3
	li	a3, 32768
	subw	a2, a2, a3
	j	.long_func.bb476
.long_func.bb479:
.long_func.bb476:
	mv	s3, a2	# phi-copy
.long_func.bb420:
	beq	s3, zero, .long_func.bb419
.long_func.bb421:
	li	a2, 0
	li	a3, 1
	li	a4, 0
	li	a5, 16
	mv	s4, s3	# phi-copy
	mv	s5, s1	# phi-copy
.long_func.bb423:
	bge	a2, a5, .long_func.bb425
.long_func.bb424:
	li	a7, 0
	srliw	s6, s4, 31	# const-div.pow2.bias-fold
	addw	s4, s4, s6
	andi	a6, s4, 1
	subw	a6, a6, s6
	subw	a6, a6, a7
	sltu	a6, zero, a6
	srliw	s6, a3, 31	# const-div.pow2.bias-fold
	addw	s7, a3, s6
	andi	a3, s7, 1
	subw	a3, a3, s6
	subw	a3, a3, a7
	sltu	a3, zero, a3
	and	a3, a6, a3
	beq	a3, zero, .long_func.bb614
.long_func.bb426:
	lw	a3, 0(s5)
	addw	a3, a4, a3
.long_func.bb427:
	sraiw	s4, s4, 1
	sraiw	a6, s7, 1
	addiw	a2, a2, 1
	addi	s5, s5, 4
	mv	a4, a3	# phi-copy
	mv	a3, a6	# phi-copy
	j	.long_func.bb423
.long_func.bb614:
	mv	a3, a4	# phi-copy
	j	.long_func.bb427
.long_func.bb425:
	beq	a4, zero, .long_func.bb429
.long_func.bb428:
	mv	s4, t1	# phi-copy
	j	.long_func.bb430
.long_func.bb478:
	srliw	a2, s3, 31	# const-div.pow2.bias-fold
	addw	a2, s3, a2
	sraiw	a2, a2, 1
	j	.long_func.bb476
.long_func.bb615:
	j	.long_func.bb429
.long_func.bb493:
	bge	a2, a4, .long_func.bb495
.long_func.bb494:
	srliw	s8, s7, 31	# const-div.pow2.bias-fold
	addw	s7, s7, s8
	andi	a5, s7, 1
	subw	a5, a5, s8
	beq	a5, zero, .long_func.bb497
.long_func.bb496:
	srliw	s8, s6, 31	# const-div.pow2.bias-fold
	addw	a5, s6, s8
	andi	a5, a5, 1
	subw	a5, a5, s8
	bne	a5, zero, .long_func.bb498
.long_func.bb499:
	lw	a5, 0(s5)
	addw	a3, a3, a5
	j	.long_func.bb498
.long_func.bb500:
.long_func.bb498:
	sraiw	s7, s7, 1
	srliw	a6, s6, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s6, a6
	addi	s5, s5, 4
	sraiw	s6, a6, 1
	j	.long_func.bb493
.long_func.bb497:
	srliw	s8, s6, 31	# const-div.pow2.bias-fold
	addw	a5, s6, s8
	andi	a5, a5, 1
	subw	a5, a5, s8
	beq	a5, zero, .long_func.bb498
.long_func.bb501:
	lw	a5, 0(s5)
	addw	a3, a3, a5
	j	.long_func.bb498
.long_func.bb502:
	j	.long_func.bb498
.long_func.bb626:
	j	.long_func.bb498
.long_func.bb627:
	j	.long_func.bb498
.long_func.bb495:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s5, s1	# phi-copy
.long_func.bb503:
	bge	a2, a5, .long_func.bb505
.long_func.bb504:
	li	a7, 0
	srliw	s6, t1, 31	# const-div.pow2.bias-fold
	addw	s7, t1, s6
	li	a6, 0
	andi	t1, s7, 1
	subw	t1, t1, s6
	subw	t1, t1, a6
	sltu	t1, zero, t1
	srliw	s6, s4, 31	# const-div.pow2.bias-fold
	addw	s4, s4, s6
	andi	a6, s4, 1
	subw	a6, a6, s6
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t1, t1, a6
	beq	t1, zero, .long_func.bb628
.long_func.bb506:
	lw	t1, 0(s5)
	addw	t1, a4, t1
.long_func.bb507:
	sraiw	a4, s7, 1
	sraiw	s4, s4, 1
	addiw	a2, a2, 1
	addi	s5, s5, 4
	mv	t5, t1	# phi-cycle-break
	mv	t1, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb503
.long_func.bb628:
	mv	t1, a4	# phi-copy
	j	.long_func.bb507
.long_func.bb505:
	slliw	t1, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s4, s1	# phi-copy
.long_func.bb508:
	bge	a2, a6, .long_func.bb510
.long_func.bb509:
	srliw	s5, t1, 31	# const-div.pow2.bias-fold
	addw	s6, t1, s5
	li	a7, 0
	andi	t1, s6, 1
	subw	t1, t1, s5
	subw	t1, t1, a7
	sltu	t1, zero, t1
	srliw	s5, a4, 31	# const-div.pow2.bias-fold
	addw	s7, a4, s5
	andi	a4, s7, 1
	subw	a4, a4, s5
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t1, t1, a4
	beq	t1, zero, .long_func.bb629
.long_func.bb511:
	lw	t1, 0(s4)
	addw	t1, a5, t1
.long_func.bb512:
	sraiw	a4, s6, 1
	sraiw	a5, s7, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t1	# phi-cycle-break
	mv	t1, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb508
.long_func.bb629:
	mv	t1, a5	# phi-copy
	j	.long_func.bb512
.long_func.bb510:
	mv	t1, a3	# phi-copy
	mv	s4, a5	# phi-copy
.long_func.bb490:
	beq	s4, zero, .long_func.bb489
.long_func.bb491:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s5, s1	# phi-copy
	mv	s6, s4	# phi-copy
	mv	s7, t1	# phi-copy
	j	.long_func.bb493
.long_func.bb492:
.long_func.bb489:
	mv	s4, t2	# phi-copy
.long_func.bb513:
	beq	s4, zero, .long_func.bb515
.long_func.bb514:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s5, s1	# phi-copy
	mv	s6, s4	# phi-copy
	mv	s7, t2	# phi-copy
.long_func.bb516:
	bge	a2, a4, .long_func.bb518
.long_func.bb517:
	srliw	s8, s7, 31	# const-div.pow2.bias-fold
	addw	s7, s7, s8
	andi	a5, s7, 1
	subw	a5, a5, s8
	beq	a5, zero, .long_func.bb520
.long_func.bb519:
	srliw	s8, s6, 31	# const-div.pow2.bias-fold
	addw	a5, s6, s8
	andi	a5, a5, 1
	subw	a5, a5, s8
	bne	a5, zero, .long_func.bb521
.long_func.bb522:
	lw	a5, 0(s5)
	addw	a3, a3, a5
	j	.long_func.bb521
.long_func.bb523:
.long_func.bb521:
	sraiw	s7, s7, 1
	srliw	a6, s6, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s6, a6
	addi	s5, s5, 4
	sraiw	s6, a6, 1
	j	.long_func.bb516
.long_func.bb520:
	srliw	s8, s6, 31	# const-div.pow2.bias-fold
	addw	a5, s6, s8
	andi	a5, a5, 1
	subw	a5, a5, s8
	beq	a5, zero, .long_func.bb521
.long_func.bb524:
	lw	a5, 0(s5)
	addw	a3, a3, a5
	j	.long_func.bb521
.long_func.bb525:
	j	.long_func.bb521
.long_func.bb630:
	j	.long_func.bb521
.long_func.bb631:
	j	.long_func.bb521
.long_func.bb518:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s5, s1	# phi-copy
.long_func.bb526:
	bge	a2, a5, .long_func.bb528
.long_func.bb527:
	li	a7, 0
	srliw	s6, t2, 31	# const-div.pow2.bias-fold
	addw	s7, t2, s6
	li	a6, 0
	andi	t2, s7, 1
	subw	t2, t2, s6
	subw	t2, t2, a6
	sltu	t2, zero, t2
	srliw	s6, s4, 31	# const-div.pow2.bias-fold
	addw	s4, s4, s6
	andi	a6, s4, 1
	subw	a6, a6, s6
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t2, t2, a6
	beq	t2, zero, .long_func.bb632
.long_func.bb529:
	lw	t2, 0(s5)
	addw	t2, a4, t2
.long_func.bb530:
	sraiw	a4, s7, 1
	sraiw	s4, s4, 1
	addiw	a2, a2, 1
	addi	s5, s5, 4
	mv	t5, t2	# phi-cycle-break
	mv	t2, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb526
.long_func.bb632:
	mv	t2, a4	# phi-copy
	j	.long_func.bb530
.long_func.bb528:
	slliw	t2, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s4, s1	# phi-copy
.long_func.bb531:
	bge	a2, a6, .long_func.bb533
.long_func.bb532:
	srliw	s5, t2, 31	# const-div.pow2.bias-fold
	addw	s6, t2, s5
	li	a7, 0
	andi	t2, s6, 1
	subw	t2, t2, s5
	subw	t2, t2, a7
	sltu	t2, zero, t2
	srliw	s5, a4, 31	# const-div.pow2.bias-fold
	addw	s7, a4, s5
	andi	a4, s7, 1
	subw	a4, a4, s5
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t2, t2, a4
	beq	t2, zero, .long_func.bb633
.long_func.bb534:
	lw	t2, 0(s4)
	addw	t2, a5, t2
.long_func.bb535:
	sraiw	a4, s6, 1
	sraiw	a5, s7, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t2	# phi-cycle-break
	mv	t2, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb531
.long_func.bb633:
	mv	t2, a5	# phi-copy
	j	.long_func.bb535
.long_func.bb533:
	mv	t2, a3	# phi-copy
	mv	s4, a5	# phi-copy
	j	.long_func.bb513
.long_func.bb515:
	li	a2, 32767
	bge	a2, s3, .long_func.bb538
.long_func.bb537:
	li	a3, 65536
	srliw	a2, s3, 31	# const-div.pow2.bias-fold
	addw	a2, s3, a2
	sraiw	a2, a2, 1
	addw	a2, a2, a3
	li	a3, 32768
	subw	a2, a2, a3
	j	.long_func.bb536
.long_func.bb539:
.long_func.bb536:
	mv	s3, a2	# phi-copy
.long_func.bb480:
	beq	s3, zero, .long_func.bb482
.long_func.bb481:
	li	a2, 0
	li	a3, 1
	li	a4, 0
	li	a5, 16
	mv	s4, s3	# phi-copy
	mv	s5, s1	# phi-copy
.long_func.bb483:
	bge	a2, a5, .long_func.bb485
.long_func.bb484:
	li	a7, 0
	srliw	s6, s4, 31	# const-div.pow2.bias-fold
	addw	s4, s4, s6
	andi	a6, s4, 1
	subw	a6, a6, s6
	subw	a6, a6, a7
	sltu	a6, zero, a6
	srliw	s6, a3, 31	# const-div.pow2.bias-fold
	addw	s7, a3, s6
	andi	a3, s7, 1
	subw	a3, a3, s6
	subw	a3, a3, a7
	sltu	a3, zero, a3
	and	a3, a6, a3
	beq	a3, zero, .long_func.bb624
.long_func.bb486:
	lw	a3, 0(s5)
	addw	a3, a4, a3
.long_func.bb487:
	sraiw	s4, s4, 1
	sraiw	a6, s7, 1
	addiw	a2, a2, 1
	addi	s5, s5, 4
	mv	a4, a3	# phi-copy
	mv	a3, a6	# phi-copy
	j	.long_func.bb483
.long_func.bb624:
	mv	a3, a4	# phi-copy
	j	.long_func.bb487
.long_func.bb485:
	beq	a4, zero, .long_func.bb489
.long_func.bb488:
	mv	s4, t2	# phi-copy
	j	.long_func.bb490
.long_func.bb538:
	srliw	a2, s3, 31	# const-div.pow2.bias-fold
	addw	a2, s3, a2
	sraiw	a2, a2, 1
	j	.long_func.bb536
.long_func.bb625:
	j	.long_func.bb489
.long_func.bb345:
	li	t1, 32767
	bge	t1, s3, .long_func.bb405
.long_func.bb404:
	li	t3, 65536
	srliw	t1, s3, 31	# const-div.pow2.bias-fold
	addw	t1, s3, t1
	sraiw	t1, t1, 1
	addw	t1, t1, t3
	li	t3, 32768
	subw	t1, t1, t3
	j	.long_func.bb403
.long_func.bb406:
.long_func.bb403:
	mv	s3, t1	# phi-copy
	mv	t1, t0	# phi-copy
	mv	t0, t2	# phi-copy
.long_func.bb273:
	bge	zero, s3, .long_func.bb275
.long_func.bb274:
	li	t2, 0
	li	t3, 1
	li	a2, 0
	li	a3, 16
	mv	s4, s3	# phi-copy
	mv	s5, s1	# phi-copy
.long_func.bb276:
	bge	t2, a3, .long_func.bb278
.long_func.bb277:
	li	a5, 0
	srliw	s6, s4, 31	# const-div.pow2.bias-fold
	addw	s4, s4, s6
	andi	a4, s4, 1
	subw	a4, a4, s6
	subw	a4, a4, a5
	sltu	a4, zero, a4
	srliw	s6, t3, 31	# const-div.pow2.bias-fold
	addw	s7, t3, s6
	andi	t3, s7, 1
	subw	t3, t3, s6
	subw	t3, t3, a5
	sltu	t3, zero, t3
	and	t3, a4, t3
	beq	t3, zero, .long_func.bb590
.long_func.bb279:
	lw	t3, 0(s5)
	addw	t3, a2, t3
.long_func.bb280:
	sraiw	s4, s4, 1
	sraiw	a4, s7, 1
	addiw	t2, t2, 1
	addi	s5, s5, 4
	mv	a2, t3	# phi-copy
	mv	t3, a4	# phi-copy
	j	.long_func.bb276
.long_func.bb590:
	mv	t3, a2	# phi-copy
	j	.long_func.bb280
.long_func.bb278:
	beq	a2, zero, .long_func.bb591
.long_func.bb281:
	li	t2, 0
	mv	s4, t1	# phi-copy
	j	.long_func.bb283
.long_func.bb482:
	li	t2, 32767
	bge	t2, s2, .long_func.bb542
.long_func.bb541:
	li	a2, 65536
	srliw	t2, s2, 31	# const-div.pow2.bias-fold
	addw	t2, s2, t2
	sraiw	t2, t2, 1
	addw	t2, t2, a2
	li	a2, 32768
	subw	t2, t2, a2
	j	.long_func.bb540
.long_func.bb543:
.long_func.bb540:
	mv	s2, t2	# phi-copy
	mv	t2, t1	# phi-copy
	mv	t1, t3	# phi-copy
.long_func.bb410:
	bge	zero, s2, .long_func.bb412
.long_func.bb411:
	li	t3, 0
	li	a2, 1
	li	a3, 0
	li	a4, 16
	mv	s3, s2	# phi-copy
	mv	s4, s1	# phi-copy
.long_func.bb413:
	bge	t3, a4, .long_func.bb415
.long_func.bb414:
	li	a6, 0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a5, s3, 1
	subw	a5, a5, s5
	subw	a5, a5, a6
	sltu	a5, zero, a5
	srliw	s5, a2, 31	# const-div.pow2.bias-fold
	addw	s6, a2, s5
	andi	a2, s6, 1
	subw	a2, a2, s5
	subw	a2, a2, a6
	sltu	a2, zero, a2
	and	a2, a5, a2
	beq	a2, zero, .long_func.bb612
.long_func.bb416:
	lw	a2, 0(s4)
	addw	a2, a3, a2
.long_func.bb417:
	sraiw	s3, s3, 1
	sraiw	a5, s6, 1
	addiw	t3, t3, 1
	addi	s4, s4, 4
	mv	a3, a2	# phi-copy
	mv	a2, a5	# phi-copy
	j	.long_func.bb413
.long_func.bb612:
	mv	a2, a3	# phi-copy
	j	.long_func.bb417
.long_func.bb415:
	beq	a3, zero, .long_func.bb613
.long_func.bb418:
	li	t3, 0
	mv	s3, t2	# phi-copy
	j	.long_func.bb420
.long_func.bb285:
.long_func.bb282:
	li	t0, 0
	mv	s4, t1	# phi-copy
	j	.long_func.bb343
.long_func.bb422:
.long_func.bb419:
	li	t1, 0
	mv	s3, t2	# phi-copy
	j	.long_func.bb480
.long_func.bb25:
	bge	a2, a4, .long_func.bb27
.long_func.bb26:
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	s6, s6, s7
	andi	a5, s6, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb29
.long_func.bb28:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	bne	a5, zero, .long_func.bb30
.long_func.bb31:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb30
.long_func.bb32:
.long_func.bb30:
	sraiw	s6, s6, 1
	srliw	a6, s5, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s5, a6
	addi	s4, s4, 4
	sraiw	s5, a6, 1
	j	.long_func.bb25
.long_func.bb29:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb30
.long_func.bb33:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb30
.long_func.bb34:
	j	.long_func.bb30
.long_func.bb550:
	j	.long_func.bb30
.long_func.bb551:
	j	.long_func.bb30
.long_func.bb27:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s4, s1	# phi-copy
.long_func.bb35:
	bge	a2, a5, .long_func.bb37
.long_func.bb36:
	li	a7, 0
	srliw	s5, t3, 31	# const-div.pow2.bias-fold
	addw	s6, t3, s5
	li	a6, 0
	andi	t3, s6, 1
	subw	t3, t3, s5
	subw	t3, t3, a6
	sltu	t3, zero, t3
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t3, t3, a6
	beq	t3, zero, .long_func.bb552
.long_func.bb38:
	lw	t3, 0(s4)
	addw	t3, a4, t3
.long_func.bb39:
	sraiw	a4, s6, 1
	sraiw	s3, s3, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t3	# phi-cycle-break
	mv	t3, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb35
.long_func.bb552:
	mv	t3, a4	# phi-copy
	j	.long_func.bb39
.long_func.bb37:
	slliw	t3, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s3, s1	# phi-copy
.long_func.bb40:
	bge	a2, a6, .long_func.bb42
.long_func.bb41:
	srliw	s4, t3, 31	# const-div.pow2.bias-fold
	addw	s5, t3, s4
	li	a7, 0
	andi	t3, s5, 1
	subw	t3, t3, s4
	subw	t3, t3, a7
	sltu	t3, zero, t3
	srliw	s4, a4, 31	# const-div.pow2.bias-fold
	addw	s6, a4, s4
	andi	a4, s6, 1
	subw	a4, a4, s4
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t3, t3, a4
	beq	t3, zero, .long_func.bb553
.long_func.bb43:
	lw	t3, 0(s3)
	addw	t3, a5, t3
.long_func.bb44:
	sraiw	a4, s5, 1
	sraiw	a5, s6, 1
	addiw	a2, a2, 1
	addi	s3, s3, 4
	mv	t5, t3	# phi-cycle-break
	mv	t3, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb40
.long_func.bb553:
	mv	t3, a5	# phi-copy
	j	.long_func.bb44
.long_func.bb42:
	mv	t3, a3	# phi-copy
	mv	s3, a5	# phi-copy
.long_func.bb22:
	beq	s3, zero, .long_func.bb21
.long_func.bb23:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s4, s1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t3	# phi-copy
	j	.long_func.bb25
.long_func.bb24:
.long_func.bb21:
	mv	s3, t0	# phi-copy
.long_func.bb45:
	beq	s3, zero, .long_func.bb47
.long_func.bb46:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s4, s1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t0	# phi-copy
.long_func.bb48:
	bge	a2, a4, .long_func.bb50
.long_func.bb49:
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	s6, s6, s7
	andi	a5, s6, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb52
.long_func.bb51:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	bne	a5, zero, .long_func.bb53
.long_func.bb54:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb53
.long_func.bb55:
.long_func.bb53:
	sraiw	s6, s6, 1
	srliw	a6, s5, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s5, a6
	addi	s4, s4, 4
	sraiw	s5, a6, 1
	j	.long_func.bb48
.long_func.bb52:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb53
.long_func.bb56:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb53
.long_func.bb57:
	j	.long_func.bb53
.long_func.bb554:
	j	.long_func.bb53
.long_func.bb555:
	j	.long_func.bb53
.long_func.bb50:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s4, s1	# phi-copy
.long_func.bb58:
	bge	a2, a5, .long_func.bb60
.long_func.bb59:
	li	a7, 0
	srliw	s5, t0, 31	# const-div.pow2.bias-fold
	addw	s6, t0, s5
	li	a6, 0
	andi	t0, s6, 1
	subw	t0, t0, s5
	subw	t0, t0, a6
	sltu	t0, zero, t0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t0, t0, a6
	beq	t0, zero, .long_func.bb556
.long_func.bb61:
	lw	t0, 0(s4)
	addw	t0, a4, t0
.long_func.bb62:
	sraiw	a4, s6, 1
	sraiw	s3, s3, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb58
.long_func.bb556:
	mv	t0, a4	# phi-copy
	j	.long_func.bb62
.long_func.bb60:
	slliw	t0, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s3, s1	# phi-copy
.long_func.bb63:
	bge	a2, a6, .long_func.bb65
.long_func.bb64:
	srliw	s4, t0, 31	# const-div.pow2.bias-fold
	addw	s5, t0, s4
	li	a7, 0
	andi	t0, s5, 1
	subw	t0, t0, s4
	subw	t0, t0, a7
	sltu	t0, zero, t0
	srliw	s4, a4, 31	# const-div.pow2.bias-fold
	addw	s6, a4, s4
	andi	a4, s6, 1
	subw	a4, a4, s4
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t0, t0, a4
	beq	t0, zero, .long_func.bb557
.long_func.bb66:
	lw	t0, 0(s3)
	addw	t0, a5, t0
.long_func.bb67:
	sraiw	a4, s5, 1
	sraiw	a5, s6, 1
	addiw	a2, a2, 1
	addi	s3, s3, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb63
.long_func.bb557:
	mv	t0, a5	# phi-copy
	j	.long_func.bb67
.long_func.bb65:
	mv	t0, a3	# phi-copy
	mv	s3, a5	# phi-copy
	j	.long_func.bb45
.long_func.bb47:
	li	a2, 32767
	bge	a2, s2, .long_func.bb70
.long_func.bb69:
	li	a3, 65536
	srliw	a2, s2, 31	# const-div.pow2.bias-fold
	addw	a2, s2, a2
	sraiw	a2, a2, 1
	addw	a2, a2, a3
	li	a3, 32768
	subw	a2, a2, a3
	j	.long_func.bb68
.long_func.bb71:
.long_func.bb68:
	mv	s2, a2	# phi-copy
.long_func.bb12:
	beq	s2, zero, .long_func.bb11
.long_func.bb13:
	li	a2, 0
	li	a3, 1
	li	a4, 0
	li	a5, 16
	mv	s3, s2	# phi-copy
	mv	s4, s1	# phi-copy
.long_func.bb15:
	bge	a2, a5, .long_func.bb17
.long_func.bb16:
	li	a7, 0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	srliw	s5, a3, 31	# const-div.pow2.bias-fold
	addw	s6, a3, s5
	andi	a3, s6, 1
	subw	a3, a3, s5
	subw	a3, a3, a7
	sltu	a3, zero, a3
	and	a3, a6, a3
	beq	a3, zero, .long_func.bb548
.long_func.bb18:
	lw	a3, 0(s4)
	addw	a3, a4, a3
.long_func.bb19:
	sraiw	s3, s3, 1
	sraiw	a6, s6, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	a4, a3	# phi-copy
	mv	a3, a6	# phi-copy
	j	.long_func.bb15
.long_func.bb548:
	mv	a3, a4	# phi-copy
	j	.long_func.bb19
.long_func.bb17:
	beq	a4, zero, .long_func.bb21
.long_func.bb20:
	mv	s3, t0	# phi-copy
	j	.long_func.bb22
.long_func.bb85:
	bge	a2, a4, .long_func.bb87
.long_func.bb86:
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	s6, s6, s7
	andi	a5, s6, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb89
.long_func.bb88:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	bne	a5, zero, .long_func.bb90
.long_func.bb91:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb90
.long_func.bb92:
.long_func.bb90:
	sraiw	s6, s6, 1
	srliw	a6, s5, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s5, a6
	addi	s4, s4, 4
	sraiw	s5, a6, 1
	j	.long_func.bb85
.long_func.bb89:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb90
.long_func.bb93:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb90
.long_func.bb94:
	j	.long_func.bb90
.long_func.bb560:
	j	.long_func.bb90
.long_func.bb561:
	j	.long_func.bb90
.long_func.bb87:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s4, s1	# phi-copy
.long_func.bb95:
	bge	a2, a5, .long_func.bb97
.long_func.bb96:
	li	a7, 0
	srliw	s5, t0, 31	# const-div.pow2.bias-fold
	addw	s6, t0, s5
	li	a6, 0
	andi	t0, s6, 1
	subw	t0, t0, s5
	subw	t0, t0, a6
	sltu	t0, zero, t0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t0, t0, a6
	beq	t0, zero, .long_func.bb562
.long_func.bb98:
	lw	t0, 0(s4)
	addw	t0, a4, t0
.long_func.bb99:
	sraiw	a4, s6, 1
	sraiw	s3, s3, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb95
.long_func.bb562:
	mv	t0, a4	# phi-copy
	j	.long_func.bb99
.long_func.bb97:
	slliw	t0, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s3, s1	# phi-copy
.long_func.bb100:
	bge	a2, a6, .long_func.bb102
.long_func.bb101:
	srliw	s4, t0, 31	# const-div.pow2.bias-fold
	addw	s5, t0, s4
	li	a7, 0
	andi	t0, s5, 1
	subw	t0, t0, s4
	subw	t0, t0, a7
	sltu	t0, zero, t0
	srliw	s4, a4, 31	# const-div.pow2.bias-fold
	addw	s6, a4, s4
	andi	a4, s6, 1
	subw	a4, a4, s4
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t0, t0, a4
	beq	t0, zero, .long_func.bb563
.long_func.bb103:
	lw	t0, 0(s3)
	addw	t0, a5, t0
.long_func.bb104:
	sraiw	a4, s5, 1
	sraiw	a5, s6, 1
	addiw	a2, a2, 1
	addi	s3, s3, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb100
.long_func.bb563:
	mv	t0, a5	# phi-copy
	j	.long_func.bb104
.long_func.bb102:
	mv	t0, a3	# phi-copy
	mv	s3, a5	# phi-copy
.long_func.bb82:
	beq	s3, zero, .long_func.bb81
.long_func.bb83:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s4, s1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t0	# phi-copy
	j	.long_func.bb85
.long_func.bb84:
.long_func.bb81:
	mv	s3, t2	# phi-copy
.long_func.bb105:
	beq	s3, zero, .long_func.bb107
.long_func.bb106:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s4, s1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t2	# phi-copy
.long_func.bb108:
	bge	a2, a4, .long_func.bb110
.long_func.bb109:
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	s6, s6, s7
	andi	a5, s6, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb112
.long_func.bb111:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	bne	a5, zero, .long_func.bb113
.long_func.bb114:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb113
.long_func.bb115:
.long_func.bb113:
	sraiw	s6, s6, 1
	srliw	a6, s5, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s5, a6
	addi	s4, s4, 4
	sraiw	s5, a6, 1
	j	.long_func.bb108
.long_func.bb112:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb113
.long_func.bb116:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb113
.long_func.bb117:
	j	.long_func.bb113
.long_func.bb564:
	j	.long_func.bb113
.long_func.bb565:
	j	.long_func.bb113
.long_func.bb110:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s4, s1	# phi-copy
.long_func.bb118:
	bge	a2, a5, .long_func.bb120
.long_func.bb119:
	li	a7, 0
	srliw	s5, t2, 31	# const-div.pow2.bias-fold
	addw	s6, t2, s5
	li	a6, 0
	andi	t2, s6, 1
	subw	t2, t2, s5
	subw	t2, t2, a6
	sltu	t2, zero, t2
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t2, t2, a6
	beq	t2, zero, .long_func.bb566
.long_func.bb121:
	lw	t2, 0(s4)
	addw	t2, a4, t2
.long_func.bb122:
	sraiw	a4, s6, 1
	sraiw	s3, s3, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t2	# phi-cycle-break
	mv	t2, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb118
.long_func.bb566:
	mv	t2, a4	# phi-copy
	j	.long_func.bb122
.long_func.bb120:
	slliw	t2, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s3, s1	# phi-copy
.long_func.bb123:
	bge	a2, a6, .long_func.bb125
.long_func.bb124:
	srliw	s4, t2, 31	# const-div.pow2.bias-fold
	addw	s5, t2, s4
	li	a7, 0
	andi	t2, s5, 1
	subw	t2, t2, s4
	subw	t2, t2, a7
	sltu	t2, zero, t2
	srliw	s4, a4, 31	# const-div.pow2.bias-fold
	addw	s6, a4, s4
	andi	a4, s6, 1
	subw	a4, a4, s4
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t2, t2, a4
	beq	t2, zero, .long_func.bb567
.long_func.bb126:
	lw	t2, 0(s3)
	addw	t2, a5, t2
.long_func.bb127:
	sraiw	a4, s5, 1
	sraiw	a5, s6, 1
	addiw	a2, a2, 1
	addi	s3, s3, 4
	mv	t5, t2	# phi-cycle-break
	mv	t2, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb123
.long_func.bb567:
	mv	t2, a5	# phi-copy
	j	.long_func.bb127
.long_func.bb125:
	mv	t2, a3	# phi-copy
	mv	s3, a5	# phi-copy
	j	.long_func.bb105
.long_func.bb107:
	li	a2, 32767
	bge	a2, s2, .long_func.bb130
.long_func.bb129:
	li	a3, 65536
	srliw	a2, s2, 31	# const-div.pow2.bias-fold
	addw	a2, s2, a2
	sraiw	a2, a2, 1
	addw	a2, a2, a3
	li	a3, 32768
	subw	a2, a2, a3
	j	.long_func.bb128
.long_func.bb131:
.long_func.bb128:
	mv	s2, a2	# phi-copy
.long_func.bb72:
	beq	s2, zero, .long_func.bb74
.long_func.bb73:
	li	a2, 0
	li	a3, 1
	li	a4, 0
	li	a5, 16
	mv	s3, s2	# phi-copy
	mv	s4, s1	# phi-copy
.long_func.bb75:
	bge	a2, a5, .long_func.bb77
.long_func.bb76:
	li	a7, 0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	srliw	s5, a3, 31	# const-div.pow2.bias-fold
	addw	s6, a3, s5
	andi	a3, s6, 1
	subw	a3, a3, s5
	subw	a3, a3, a7
	sltu	a3, zero, a3
	and	a3, a6, a3
	beq	a3, zero, .long_func.bb558
.long_func.bb78:
	lw	a3, 0(s4)
	addw	a3, a4, a3
.long_func.bb79:
	sraiw	s3, s3, 1
	sraiw	a6, s6, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	a4, a3	# phi-copy
	mv	a3, a6	# phi-copy
	j	.long_func.bb75
.long_func.bb558:
	mv	a3, a4	# phi-copy
	j	.long_func.bb79
.long_func.bb77:
	beq	a4, zero, .long_func.bb81
.long_func.bb80:
	mv	s3, t2	# phi-copy
	j	.long_func.bb82
.long_func.bb159:
	bge	a2, a4, .long_func.bb161
.long_func.bb160:
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	s6, s6, s7
	andi	a5, s6, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb163
.long_func.bb162:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	bne	a5, zero, .long_func.bb164
.long_func.bb165:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb164
.long_func.bb166:
.long_func.bb164:
	sraiw	s6, s6, 1
	srliw	a6, s5, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s5, a6
	addi	s4, s4, 4
	sraiw	s5, a6, 1
	j	.long_func.bb159
.long_func.bb163:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb164
.long_func.bb167:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb164
.long_func.bb168:
	j	.long_func.bb164
.long_func.bb572:
	j	.long_func.bb164
.long_func.bb573:
	j	.long_func.bb164
.long_func.bb161:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s4, s1	# phi-copy
.long_func.bb169:
	bge	a2, a5, .long_func.bb171
.long_func.bb170:
	li	a7, 0
	srliw	s5, t3, 31	# const-div.pow2.bias-fold
	addw	s6, t3, s5
	li	a6, 0
	andi	t3, s6, 1
	subw	t3, t3, s5
	subw	t3, t3, a6
	sltu	t3, zero, t3
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t3, t3, a6
	beq	t3, zero, .long_func.bb574
.long_func.bb172:
	lw	t3, 0(s4)
	addw	t3, a4, t3
.long_func.bb173:
	sraiw	a4, s6, 1
	sraiw	s3, s3, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t3	# phi-cycle-break
	mv	t3, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb169
.long_func.bb574:
	mv	t3, a4	# phi-copy
	j	.long_func.bb173
.long_func.bb171:
	slliw	t3, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s3, s1	# phi-copy
.long_func.bb174:
	bge	a2, a6, .long_func.bb176
.long_func.bb175:
	srliw	s4, t3, 31	# const-div.pow2.bias-fold
	addw	s5, t3, s4
	li	a7, 0
	andi	t3, s5, 1
	subw	t3, t3, s4
	subw	t3, t3, a7
	sltu	t3, zero, t3
	srliw	s4, a4, 31	# const-div.pow2.bias-fold
	addw	s6, a4, s4
	andi	a4, s6, 1
	subw	a4, a4, s4
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t3, t3, a4
	beq	t3, zero, .long_func.bb575
.long_func.bb177:
	lw	t3, 0(s3)
	addw	t3, a5, t3
.long_func.bb178:
	sraiw	a4, s5, 1
	sraiw	a5, s6, 1
	addiw	a2, a2, 1
	addi	s3, s3, 4
	mv	t5, t3	# phi-cycle-break
	mv	t3, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb174
.long_func.bb575:
	mv	t3, a5	# phi-copy
	j	.long_func.bb178
.long_func.bb176:
	mv	t3, a3	# phi-copy
	mv	s3, a5	# phi-copy
.long_func.bb156:
	beq	s3, zero, .long_func.bb155
.long_func.bb157:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s4, s1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t3	# phi-copy
	j	.long_func.bb159
.long_func.bb158:
.long_func.bb155:
	mv	s3, t0	# phi-copy
.long_func.bb179:
	beq	s3, zero, .long_func.bb181
.long_func.bb180:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s4, s1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t0	# phi-copy
.long_func.bb182:
	bge	a2, a4, .long_func.bb184
.long_func.bb183:
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	s6, s6, s7
	andi	a5, s6, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb186
.long_func.bb185:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	bne	a5, zero, .long_func.bb187
.long_func.bb188:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb187
.long_func.bb189:
.long_func.bb187:
	sraiw	s6, s6, 1
	srliw	a6, s5, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s5, a6
	addi	s4, s4, 4
	sraiw	s5, a6, 1
	j	.long_func.bb182
.long_func.bb186:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb187
.long_func.bb190:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb187
.long_func.bb191:
	j	.long_func.bb187
.long_func.bb576:
	j	.long_func.bb187
.long_func.bb577:
	j	.long_func.bb187
.long_func.bb184:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s4, s1	# phi-copy
.long_func.bb192:
	bge	a2, a5, .long_func.bb194
.long_func.bb193:
	li	a7, 0
	srliw	s5, t0, 31	# const-div.pow2.bias-fold
	addw	s6, t0, s5
	li	a6, 0
	andi	t0, s6, 1
	subw	t0, t0, s5
	subw	t0, t0, a6
	sltu	t0, zero, t0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t0, t0, a6
	beq	t0, zero, .long_func.bb578
.long_func.bb195:
	lw	t0, 0(s4)
	addw	t0, a4, t0
.long_func.bb196:
	sraiw	a4, s6, 1
	sraiw	s3, s3, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb192
.long_func.bb578:
	mv	t0, a4	# phi-copy
	j	.long_func.bb196
.long_func.bb194:
	slliw	t0, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s3, s1	# phi-copy
.long_func.bb197:
	bge	a2, a6, .long_func.bb199
.long_func.bb198:
	srliw	s4, t0, 31	# const-div.pow2.bias-fold
	addw	s5, t0, s4
	li	a7, 0
	andi	t0, s5, 1
	subw	t0, t0, s4
	subw	t0, t0, a7
	sltu	t0, zero, t0
	srliw	s4, a4, 31	# const-div.pow2.bias-fold
	addw	s6, a4, s4
	andi	a4, s6, 1
	subw	a4, a4, s4
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t0, t0, a4
	beq	t0, zero, .long_func.bb579
.long_func.bb200:
	lw	t0, 0(s3)
	addw	t0, a5, t0
.long_func.bb201:
	sraiw	a4, s5, 1
	sraiw	a5, s6, 1
	addiw	a2, a2, 1
	addi	s3, s3, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb197
.long_func.bb579:
	mv	t0, a5	# phi-copy
	j	.long_func.bb201
.long_func.bb199:
	mv	t0, a3	# phi-copy
	mv	s3, a5	# phi-copy
	j	.long_func.bb179
.long_func.bb181:
	li	a2, 32767
	bge	a2, s2, .long_func.bb204
.long_func.bb203:
	li	a3, 65536
	srliw	a2, s2, 31	# const-div.pow2.bias-fold
	addw	a2, s2, a2
	sraiw	a2, a2, 1
	addw	a2, a2, a3
	li	a3, 32768
	subw	a2, a2, a3
	j	.long_func.bb202
.long_func.bb205:
.long_func.bb202:
	mv	s2, a2	# phi-copy
.long_func.bb146:
	beq	s2, zero, .long_func.bb145
.long_func.bb147:
	li	a2, 0
	li	a3, 1
	li	a4, 0
	li	a5, 16
	mv	s3, s2	# phi-copy
	mv	s4, s1	# phi-copy
.long_func.bb149:
	bge	a2, a5, .long_func.bb151
.long_func.bb150:
	li	a7, 0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	srliw	s5, a3, 31	# const-div.pow2.bias-fold
	addw	s6, a3, s5
	andi	a3, s6, 1
	subw	a3, a3, s5
	subw	a3, a3, a7
	sltu	a3, zero, a3
	and	a3, a6, a3
	beq	a3, zero, .long_func.bb570
.long_func.bb152:
	lw	a3, 0(s4)
	addw	a3, a4, a3
.long_func.bb153:
	sraiw	s3, s3, 1
	sraiw	a6, s6, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	a4, a3	# phi-copy
	mv	a3, a6	# phi-copy
	j	.long_func.bb149
.long_func.bb570:
	mv	a3, a4	# phi-copy
	j	.long_func.bb153
.long_func.bb151:
	beq	a4, zero, .long_func.bb155
.long_func.bb154:
	mv	s3, t0	# phi-copy
	j	.long_func.bb156
.long_func.bb219:
	bge	a2, a4, .long_func.bb221
.long_func.bb220:
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	s6, s6, s7
	andi	a5, s6, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb223
.long_func.bb222:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	bne	a5, zero, .long_func.bb224
.long_func.bb225:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb224
.long_func.bb226:
.long_func.bb224:
	sraiw	s6, s6, 1
	srliw	a6, s5, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s5, a6
	addi	s4, s4, 4
	sraiw	s5, a6, 1
	j	.long_func.bb219
.long_func.bb223:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb224
.long_func.bb227:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb224
.long_func.bb228:
	j	.long_func.bb224
.long_func.bb582:
	j	.long_func.bb224
.long_func.bb583:
	j	.long_func.bb224
.long_func.bb221:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s4, s1	# phi-copy
.long_func.bb229:
	bge	a2, a5, .long_func.bb231
.long_func.bb230:
	li	a7, 0
	srliw	s5, t0, 31	# const-div.pow2.bias-fold
	addw	s6, t0, s5
	li	a6, 0
	andi	t0, s6, 1
	subw	t0, t0, s5
	subw	t0, t0, a6
	sltu	t0, zero, t0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t0, t0, a6
	beq	t0, zero, .long_func.bb584
.long_func.bb232:
	lw	t0, 0(s4)
	addw	t0, a4, t0
.long_func.bb233:
	sraiw	a4, s6, 1
	sraiw	s3, s3, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb229
.long_func.bb584:
	mv	t0, a4	# phi-copy
	j	.long_func.bb233
.long_func.bb231:
	slliw	t0, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s3, s1	# phi-copy
.long_func.bb234:
	bge	a2, a6, .long_func.bb236
.long_func.bb235:
	srliw	s4, t0, 31	# const-div.pow2.bias-fold
	addw	s5, t0, s4
	li	a7, 0
	andi	t0, s5, 1
	subw	t0, t0, s4
	subw	t0, t0, a7
	sltu	t0, zero, t0
	srliw	s4, a4, 31	# const-div.pow2.bias-fold
	addw	s6, a4, s4
	andi	a4, s6, 1
	subw	a4, a4, s4
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t0, t0, a4
	beq	t0, zero, .long_func.bb585
.long_func.bb237:
	lw	t0, 0(s3)
	addw	t0, a5, t0
.long_func.bb238:
	sraiw	a4, s5, 1
	sraiw	a5, s6, 1
	addiw	a2, a2, 1
	addi	s3, s3, 4
	mv	t5, t0	# phi-cycle-break
	mv	t0, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb234
.long_func.bb585:
	mv	t0, a5	# phi-copy
	j	.long_func.bb238
.long_func.bb236:
	mv	t0, a3	# phi-copy
	mv	s3, a5	# phi-copy
.long_func.bb216:
	beq	s3, zero, .long_func.bb215
.long_func.bb217:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s4, s1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t0	# phi-copy
	j	.long_func.bb219
.long_func.bb218:
.long_func.bb215:
	mv	s3, t2	# phi-copy
.long_func.bb239:
	beq	s3, zero, .long_func.bb241
.long_func.bb240:
	li	a2, 0
	li	a3, 0
	li	a4, 16
	mv	s4, s1	# phi-copy
	mv	s5, s3	# phi-copy
	mv	s6, t2	# phi-copy
.long_func.bb242:
	bge	a2, a4, .long_func.bb244
.long_func.bb243:
	srliw	s7, s6, 31	# const-div.pow2.bias-fold
	addw	s6, s6, s7
	andi	a5, s6, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb246
.long_func.bb245:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	bne	a5, zero, .long_func.bb247
.long_func.bb248:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb247
.long_func.bb249:
.long_func.bb247:
	sraiw	s6, s6, 1
	srliw	a6, s5, 31	# const-div.pow2.bias-fold
	addiw	a2, a2, 1
	addw	a6, s5, a6
	addi	s4, s4, 4
	sraiw	s5, a6, 1
	j	.long_func.bb242
.long_func.bb246:
	srliw	s7, s5, 31	# const-div.pow2.bias-fold
	addw	a5, s5, s7
	andi	a5, a5, 1
	subw	a5, a5, s7
	beq	a5, zero, .long_func.bb247
.long_func.bb250:
	lw	a5, 0(s4)
	addw	a3, a3, a5
	j	.long_func.bb247
.long_func.bb251:
	j	.long_func.bb247
.long_func.bb586:
	j	.long_func.bb247
.long_func.bb587:
	j	.long_func.bb247
.long_func.bb244:
	li	a2, 0
	li	a4, 0
	li	a5, 16
	mv	s4, s1	# phi-copy
.long_func.bb252:
	bge	a2, a5, .long_func.bb254
.long_func.bb253:
	li	a7, 0
	srliw	s5, t2, 31	# const-div.pow2.bias-fold
	addw	s6, t2, s5
	li	a6, 0
	andi	t2, s6, 1
	subw	t2, t2, s5
	subw	t2, t2, a6
	sltu	t2, zero, t2
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	and	t2, t2, a6
	beq	t2, zero, .long_func.bb588
.long_func.bb255:
	lw	t2, 0(s4)
	addw	t2, a4, t2
.long_func.bb256:
	sraiw	a4, s6, 1
	sraiw	s3, s3, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	t5, t2	# phi-cycle-break
	mv	t2, a4	# phi-copy
	mv	a4, t5	# phi-copy
	j	.long_func.bb252
.long_func.bb588:
	mv	t2, a4	# phi-copy
	j	.long_func.bb256
.long_func.bb254:
	slliw	t2, a4, 1
	li	a2, 0
	li	a4, 65535
	li	a5, 0
	li	a6, 16
	mv	s3, s1	# phi-copy
.long_func.bb257:
	bge	a2, a6, .long_func.bb259
.long_func.bb258:
	srliw	s4, t2, 31	# const-div.pow2.bias-fold
	addw	s5, t2, s4
	li	a7, 0
	andi	t2, s5, 1
	subw	t2, t2, s4
	subw	t2, t2, a7
	sltu	t2, zero, t2
	srliw	s4, a4, 31	# const-div.pow2.bias-fold
	addw	s6, a4, s4
	andi	a4, s6, 1
	subw	a4, a4, s4
	subw	a4, a4, a7
	sltu	a4, zero, a4
	and	t2, t2, a4
	beq	t2, zero, .long_func.bb589
.long_func.bb260:
	lw	t2, 0(s3)
	addw	t2, a5, t2
.long_func.bb261:
	sraiw	a4, s5, 1
	sraiw	a5, s6, 1
	addiw	a2, a2, 1
	addi	s3, s3, 4
	mv	t5, t2	# phi-cycle-break
	mv	t2, a4	# phi-copy
	mv	a4, a5	# phi-copy
	mv	a5, t5	# phi-copy
	j	.long_func.bb257
.long_func.bb589:
	mv	t2, a5	# phi-copy
	j	.long_func.bb261
.long_func.bb259:
	mv	t2, a3	# phi-copy
	mv	s3, a5	# phi-copy
	j	.long_func.bb239
.long_func.bb241:
	li	a2, 32767
	bge	a2, s2, .long_func.bb264
.long_func.bb263:
	li	a3, 65536
	srliw	a2, s2, 31	# const-div.pow2.bias-fold
	addw	a2, s2, a2
	sraiw	a2, a2, 1
	addw	a2, a2, a3
	li	a3, 32768
	subw	a2, a2, a3
	j	.long_func.bb262
.long_func.bb265:
.long_func.bb262:
	mv	s2, a2	# phi-copy
.long_func.bb206:
	beq	s2, zero, .long_func.bb208
.long_func.bb207:
	li	a2, 0
	li	a3, 1
	li	a4, 0
	li	a5, 16
	mv	s3, s2	# phi-copy
	mv	s4, s1	# phi-copy
.long_func.bb209:
	bge	a2, a5, .long_func.bb211
.long_func.bb210:
	li	a7, 0
	srliw	s5, s3, 31	# const-div.pow2.bias-fold
	addw	s3, s3, s5
	andi	a6, s3, 1
	subw	a6, a6, s5
	subw	a6, a6, a7
	sltu	a6, zero, a6
	srliw	s5, a3, 31	# const-div.pow2.bias-fold
	addw	s6, a3, s5
	andi	a3, s6, 1
	subw	a3, a3, s5
	subw	a3, a3, a7
	sltu	a3, zero, a3
	and	a3, a6, a3
	beq	a3, zero, .long_func.bb580
.long_func.bb212:
	lw	a3, 0(s4)
	addw	a3, a4, a3
.long_func.bb213:
	sraiw	s3, s3, 1
	sraiw	a6, s6, 1
	addiw	a2, a2, 1
	addi	s4, s4, 4
	mv	a4, a3	# phi-copy
	mv	a3, a6	# phi-copy
	j	.long_func.bb209
.long_func.bb580:
	mv	a3, a4	# phi-copy
	j	.long_func.bb213
.long_func.bb211:
	beq	a4, zero, .long_func.bb215
.long_func.bb214:
	mv	s3, t2	# phi-copy
	j	.long_func.bb216
.long_func.bb70:
	srliw	a2, s2, 31	# const-div.pow2.bias-fold
	addw	a2, s2, a2
	sraiw	a2, a2, 1
	j	.long_func.bb68
.long_func.bb130:
	srliw	a2, s2, 31	# const-div.pow2.bias-fold
	addw	a2, s2, a2
	sraiw	a2, a2, 1
	j	.long_func.bb128
.long_func.bb204:
	srliw	a2, s2, 31	# const-div.pow2.bias-fold
	addw	a2, s2, a2
	sraiw	a2, a2, 1
	j	.long_func.bb202
.long_func.bb264:
	srliw	a2, s2, 31	# const-div.pow2.bias-fold
	addw	a2, s2, a2
	sraiw	a2, a2, 1
	j	.long_func.bb262
.long_func.bb405:
	srliw	t1, s3, 31	# const-div.pow2.bias-fold
	addw	t1, s3, t1
	sraiw	t1, t1, 1
	j	.long_func.bb403
.long_func.bb542:
	srliw	t2, s2, 31	# const-div.pow2.bias-fold
	addw	t2, s2, t2
	sraiw	t2, t2, 1
	j	.long_func.bb540
.long_func.bb546:
	mv	a2, a3	# phi-copy
	j	.long_func.bb9
.long_func.bb549:
	j	.long_func.bb21
.long_func.bb559:
	j	.long_func.bb81
.long_func.bb571:
	j	.long_func.bb155
.long_func.bb581:
	j	.long_func.bb215
.long_func.bb591:
	mv	t2, t0	# phi-copy
	j	.long_func.bb282
.long_func.bb613:
	mv	t3, t1	# phi-copy
	j	.long_func.bb419
.long_func.bb7:
	beq	a3, zero, .long_func.bb547
.long_func.bb10:
	li	t3, 0
	mv	s2, t2	# phi-copy
	j	.long_func.bb12
.long_func.bb74:
	li	t2, 32767
	bge	t2, t1, .long_func.bb134
.long_func.bb133:
	srliw	t2, t1, 31	# const-div.pow2.bias-fold
	addw	t1, t1, t2
	sraiw	t1, t1, 1
	li	t2, 65536
	addw	t1, t1, t2
	li	t2, 32768
	subw	t1, t1, t2
	j	.long_func.bb132
.long_func.bb135:
.long_func.bb132:
	mv	t2, t0	# phi-copy
	mv	t0, t3	# phi-copy
	blt	zero, t1, .long_func.bb3	# rotated-loop-branch
.long_func.bb4:
	mv	a0, t0
	call	putint
	li	a0, 10
	call	putch
	li	t0, 1
	li	t1, 1
	li	t2, 2
.long_func.bb136:
	bge	zero, t1, .long_func.bb138
.long_func.bb137:
	li	t3, 0
	li	a2, 1
	li	a3, 0
	li	a4, 16
	mv	s2, t1	# phi-copy
	mv	s3, s1	# phi-copy
.long_func.bb139:
	bge	t3, a4, .long_func.bb141
.long_func.bb140:
	li	a6, 0
	srliw	s4, s2, 31	# const-div.pow2.bias-fold
	addw	s2, s2, s4
	andi	a5, s2, 1
	subw	a5, a5, s4
	subw	a5, a5, a6
	sltu	a5, zero, a5
	srliw	s4, a2, 31	# const-div.pow2.bias-fold
	addw	s5, a2, s4
	andi	a2, s5, 1
	subw	a2, a2, s4
	subw	a2, a2, a6
	sltu	a2, zero, a2
	and	a2, a5, a2
	beq	a2, zero, .long_func.bb568
.long_func.bb142:
	lw	a2, 0(s3)
	addw	a2, a3, a2
.long_func.bb143:
	sraiw	s2, s2, 1
	sraiw	a5, s5, 1
	addiw	t3, t3, 1
	addi	s3, s3, 4
	mv	a3, a2	# phi-copy
	mv	a2, a5	# phi-copy
	j	.long_func.bb139
.long_func.bb568:
	mv	a2, a3	# phi-copy
	j	.long_func.bb143
.long_func.bb141:
	beq	a3, zero, .long_func.bb569
.long_func.bb144:
	li	t3, 0
	mv	s2, t2	# phi-copy
	j	.long_func.bb146
.long_func.bb208:
	li	t2, 32767
	bge	t2, t1, .long_func.bb268
.long_func.bb267:
	srliw	t2, t1, 31	# const-div.pow2.bias-fold
	addw	t1, t1, t2
	sraiw	t1, t1, 1
	li	t2, 65536
	addw	t1, t1, t2
	li	t2, 32768
	subw	t1, t1, t2
	j	.long_func.bb266
.long_func.bb269:
.long_func.bb266:
	mv	t2, t0	# phi-copy
	mv	t0, t3	# phi-copy
	blt	zero, t1, .long_func.bb137	# rotated-loop-branch
.long_func.bb138:
	mv	a0, t0
	call	putint
	li	a0, 10
	call	putch
	li	s2, 2
.long_func.bb270:
	li	t0, 16
	bge	s2, t0, .long_func.bb272
.long_func.bb271:
	li	t0, 1
	li	t1, 2
	mv	s3, s2	# phi-copy
	j	.long_func.bb273
.long_func.bb14:
.long_func.bb11:
	li	t0, 0
	mv	s2, t2	# phi-copy
	j	.long_func.bb72
.long_func.bb148:
.long_func.bb145:
	li	t0, 0
	mv	s2, t2	# phi-copy
	j	.long_func.bb206
.long_func.bb275:
	mv	a0, t0
	call	putint
	li	a0, 10
	call	putch
	addiw	s2, s2, 1
	j	.long_func.bb270
.long_func.bb412:
	slliw	t2, t0, 2
	la	t3, SHIFT_TABLE
	add	t2, t3, t2
	lw	t2, 0(t2)
	bne	t2, t1, .long_func.bb544
.long_func.bb545:
	addiw	t0, t0, 1
.long_func.bb407:
	li	t1, 16
	bge	t0, t1, .long_func.bb409
.long_func.bb408:
	li	t1, 1
	li	t2, 2
	mv	s2, t0	# phi-copy
	j	.long_func.bb410
.long_func.bb134:
	srliw	t2, t1, 31	# const-div.pow2.bias-fold
	addw	t1, t1, t2
	sraiw	t1, t1, 1
	j	.long_func.bb132
.long_func.bb268:
	srliw	t2, t1, 31	# const-div.pow2.bias-fold
	addw	t1, t1, t2
	sraiw	t1, t1, 1
	j	.long_func.bb266
.long_func.bb547:
	mv	t3, t0	# phi-copy
	j	.long_func.bb11
.long_func.bb569:
	mv	t3, t0	# phi-copy
	j	.long_func.bb145
.long_func.bb272:
	li	t0, 0
	j	.long_func.bb407
.long_func.bb409:
	li	t0, 0
.long_func.bb1:
	mv	a0, t0
	ld	s9, 80(sp)
	ld	s8, 72(sp)
	ld	s7, 64(sp)
	ld	s6, 56(sp)
	ld	s5, 48(sp)
	ld	s4, 40(sp)
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 96
	ret
.long_func.bb544:
	li	t0, 1
	j	.long_func.bb1
	.size long_func, .-long_func

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -16
	sd	ra, 0(sp)
	call	long_func
	mv	t0, a0
	mv	a0, t0
	ld	ra, 0(sp)
	addi	sp, sp, 16
	ret
	.size main, .-main
