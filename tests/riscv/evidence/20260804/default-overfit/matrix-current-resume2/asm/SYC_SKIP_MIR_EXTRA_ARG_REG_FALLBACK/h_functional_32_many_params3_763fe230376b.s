
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -96
	sd	ra, 80(sp)
	li	t5, 0
	sd	t5, 0(sp)
	li	t5, 1
	sd	t5, 8(sp)
	li	t5, 8
	sd	t5, 16(sp)
	li	t5, 9
	sd	t5, 24(sp)
	li	t5, 5
	sd	t5, 32(sp)
	li	t5, 2
	sd	t5, 40(sp)
	li	t5, 6
	sd	t5, 48(sp)
	li	t5, 4
	sd	t5, 56(sp)
	li	t5, 7
	sd	t5, 64(sp)
	li	t5, 3
	sd	t5, 72(sp)
	ld	a0, 0(sp)
	ld	a1, 8(sp)
	ld	a2, 16(sp)
	ld	a3, 24(sp)
	ld	a4, 32(sp)
	ld	a5, 40(sp)
	ld	a6, 48(sp)
	ld	a7, 56(sp)
	ld	t5, 64(sp)
	sd	t5, 0(sp)
	ld	t5, 72(sp)
	sd	t5, 8(sp)
	call	func.eqargs.676to10.h14669754348576032141
	mv	t0, a0
	mv	a0, t0
	ld	ra, 80(sp)
	addi	sp, sp, 96
	ret
	.size main, .-main

  .globl func.eqargs.676to10.h14669754348576032141
  .type  func.eqargs.676to10.h14669754348576032141, @function
func.eqargs.676to10.h14669754348576032141:
.func.eqargs.676to10.h14669754348576032141.bb0:
	addi	sp, sp, -272
	sd	s0, 224(sp)
	sd	s1, 232(sp)
	sd	s2, 240(sp)
	sd	s3, 248(sp)
	sd	s4, 256(sp)
	addi	s0, sp, 272
	mv	t0, a0
	mv	t1, a1
	mv	t2, a2
	mv	t3, a3
	mv	t5, a4
	sd	t5, 16(sp)
	mv	t5, a5
	sd	t5, 88(sp)
	mv	a4, a6
	mv	a5, a7
	lw	a6, 0(s0)
	lw	a7, 8(s0)
	mulw	t5, t0, t0
	sd	t5, 0(sp)
	mulw	t4, t5, t0
	mulw	t4, t4, a7
	mulw	t4, t4, a5
	ld	t5, 88(sp)
	addw	t4, t4, t5
	ld	t5, 88(sp)
	mulw	t5, t5, a4
	sd	t5, 8(sp)
	mulw	s1, t5, a5
	mulw	a2, s1, t2
	mulw	a2, a2, a4
	mulw	a2, a2, t1
	mulw	a2, a2, t0
	addw	a2, t4, a2
	mulw	t4, a6, t3
	addw	a2, a2, t4
	addw	a2, a2, a4
	mulw	t5, t2, a5
	sd	t5, 24(sp)
	ld	t6, 88(sp)
	mulw	s1, t5, t6
	addw	a2, a2, s1
	ld	t5, 88(sp)
	mulw	t5, t5, a7
	sd	t5, 32(sp)
	addw	a2, a2, t5
	ld	t5, 16(sp)
	mulw	t5, a4, t5
	sd	t5, 40(sp)
	addw	a2, a2, t5
	ld	t5, 88(sp)
	addw	a2, a2, t5
	mulw	t5, t1, t0
	sd	t5, 48(sp)
	mulw	s1, t5, a5
	mulw	s2, s1, a7
	addw	a2, a2, s2
	ld	t5, 88(sp)
	addw	a2, a2, t5
	addw	a2, a2, a6
	addw	a2, a2, a5
	ld	t5, 0(sp)
	mulw	s3, t5, a4
	addw	a2, a2, s3
	mulw	t5, t0, t3
	sd	t5, 168(sp)
	addw	a2, a2, t5
	mulw	t5, t0, t2
	sd	t5, 56(sp)
	mulw	t5, t5, a4
	sd	t5, 64(sp)
	mulw	s2, t5, t2
	mulw	s3, s2, a7
	addw	a2, a2, s3
	ld	t5, 88(sp)
	addw	a2, a2, t5
	addw	a2, a2, t3
	addw	a2, a2, a5
	mulw	t5, a4, t3
	sd	t5, 72(sp)
	mulw	s2, t5, t3
	addw	a2, a2, s2
	mulw	t5, a5, t3
	sd	t5, 80(sp)
	addw	a2, a2, t5
	ld	t5, 88(sp)
	mulw	s2, t1, t5
	mulw	a3, s2, a4
	addw	a2, a2, a3
	ld	t5, 168(sp)
	addw	a2, a2, t5
	mulw	t5, a7, t3
	sd	t5, 96(sp)
	addw	a2, a2, t5
	ld	t5, 16(sp)
	addw	a2, a2, t5
	addw	a2, a2, t0
	addw	a2, a2, a6
	ld	t5, 16(sp)
	mulw	t5, t5, a5
	sd	t5, 184(sp)
	mulw	a3, t5, t1
	addw	a2, a2, a3
	ld	t5, 16(sp)
	mulw	t5, t3, t5
	sd	t5, 128(sp)
	addw	a2, a2, t5
	addw	a2, a2, t4
	ld	t5, 16(sp)
	mulw	t5, a6, t5
	sd	t5, 120(sp)
	addw	a2, a2, t5
	addw	a2, a2, t3
	addw	a2, a2, t3
	addw	a2, a2, a6
	mulw	t5, a4, a5
	sd	t5, 112(sp)
	addw	a2, a2, t5
	addw	a2, a2, t2
	mulw	t5, t2, a7
	sd	t5, 104(sp)
	mulw	s4, t5, t3
	addw	a2, a2, s4
	addw	a2, a2, t1
	ld	t5, 64(sp)
	mulw	s3, t5, a4
	ld	t5, 16(sp)
	mulw	t4, s3, t5
	ld	t5, 88(sp)
	mulw	t4, t4, t5
	mulw	t4, t4, t2
	addw	a2, a2, t4
	addw	a2, a2, a4
	mulw	t4, a4, a6
	addw	a2, a2, t4
	addw	a2, a2, t0
	addw	a2, a2, t3
	addw	a2, a2, t2
	addw	a2, a2, t1
	mulw	t4, t2, t1
	addw	a2, a2, t4
	addw	a2, a2, a6
	ld	t5, 16(sp)
	ld	t6, 88(sp)
	mulw	t5, t5, t6
	sd	t5, 136(sp)
	addw	a2, a2, t5
	ld	t5, 128(sp)
	mulw	a3, t5, t2
	addw	a2, a2, a3
	addw	a2, a2, a4
	addw	a2, a2, t2
	ld	t5, 168(sp)
	addw	a2, a2, t5
	addw	a2, a2, t0
	mulw	a3, a7, t0
	addw	a2, a2, a3
	ld	t5, 88(sp)
	mulw	a3, t0, t5
	mulw	a3, a3, a6
	addw	a2, a2, a3
	addw	a2, a2, a4
	ld	t5, 184(sp)
	ld	t6, 88(sp)
	mulw	a3, t5, t6
	addw	a2, a2, a3
	addw	a2, a2, t0
	ld	t5, 16(sp)
	addw	a2, a2, t5
	addw	a2, a2, a4
	ld	t5, 16(sp)
	addw	a2, a2, t5
	addw	a2, a2, a4
	mulw	a3, t2, a6
	addw	a2, a2, a3
	ld	t5, 136(sp)
	mulw	a3, t5, t1
	mulw	a3, a3, a6
	mulw	a3, a3, a6
	mulw	a3, a3, a5
	addw	a2, a2, a3
	ld	t5, 88(sp)
	addw	a2, a2, t5
	ld	t5, 24(sp)
	mulw	a3, t5, t1
	mulw	a3, a3, a6
	mulw	a3, a3, a4
	addw	a2, a2, a3
	mulw	t5, a7, a5
	sd	t5, 176(sp)
	mulw	t4, t5, a7
	ld	t5, 16(sp)
	mulw	t4, t4, t5
	addw	a2, a2, t4
	ld	t5, 16(sp)
	addw	a2, a2, t5
	mulw	t5, t1, a6
	sd	t5, 144(sp)
	ld	t6, 16(sp)
	mulw	t4, t5, t6
	ld	t5, 16(sp)
	mulw	t4, t4, t5
	addw	a2, a2, t4
	addw	a2, a2, a6
	ld	t5, 24(sp)
	mulw	t4, t5, t3
	ld	t5, 88(sp)
	mulw	t4, t4, t5
	mulw	t4, t4, a7
	ld	t5, 16(sp)
	mulw	t4, t4, t5
	mulw	t4, t4, a6
	addw	a2, a2, t4
	addw	a2, a2, a6
	addw	a2, a2, a4
	mulw	t4, t1, t3
	addw	a2, a2, t4
	ld	t5, 48(sp)
	mulw	t4, t5, a6
	mulw	t4, t4, t3
	mulw	t4, t4, a4
	mulw	t4, t4, a7
	ld	t5, 16(sp)
	mulw	t4, t4, t5
	mulw	t4, t4, a6
	mulw	t4, t4, t2
	mulw	t4, t4, a6
	mulw	t4, t4, a6
	addw	a2, a2, t4
	ld	t5, 88(sp)
	mulw	t5, t5, t2
	sd	t5, 152(sp)
	addw	a2, a2, t5
	addw	a2, a2, t3
	ld	t5, 120(sp)
	mulw	t4, t5, t1
	addw	a2, a2, t4
	ld	t5, 16(sp)
	addw	a2, a2, t5
	addw	a2, a2, t1
	addw	a2, a2, t2
	ld	t5, 88(sp)
	mulw	t4, a6, t5
	addw	a2, a2, t4
	addw	a2, a2, t3
	ld	t5, 16(sp)
	addw	a2, a2, t5
	addw	a2, a2, t2
	addw	a2, a2, a5
	ld	t5, 16(sp)
	addw	a2, a2, t5
	addw	a2, a2, a4
	addw	a2, a2, t3
	addw	a2, a2, t0
	addw	a2, a2, t0
	mulw	s3, t2, a4
	mulw	t5, s3, t0
	sd	t5, 160(sp)
	mulw	s4, t5, a7
	mulw	s1, s4, a6
	addw	a2, a2, s1
	mulw	a3, t3, t2
	ld	t5, 88(sp)
	mulw	a3, a3, t5
	mulw	a3, a3, a5
	ld	t5, 88(sp)
	mulw	a3, a3, t5
	addw	a2, a2, a3
	mulw	t5, a6, t0
	sd	t5, 192(sp)
	addw	a2, a2, t5
	addw	a2, a2, t2
	addw	a2, a2, t4
	mulw	t4, a4, t1
	addw	a2, a2, t4
	addw	a2, a2, a5
	ld	t5, 16(sp)
	addw	a2, a2, t5
	ld	t5, 88(sp)
	addw	a2, a2, t5
	addw	a2, a2, t3
	ld	t5, 184(sp)
	mulw	t4, t5, a7
	addw	a2, a2, t4
	addw	a2, a2, s3
	addw	a2, a2, t3
	ld	t5, 128(sp)
	mulw	t4, t5, a7
	addw	a2, a2, t4
	ld	t5, 88(sp)
	addw	a2, a2, t5
	addw	a2, a2, t0
	ld	t5, 16(sp)
	mulw	t4, t5, t1
	mulw	t4, t4, t3
	ld	t5, 16(sp)
	mulw	t4, t4, t5
	addw	a2, a2, t4
	ld	t5, 48(sp)
	mulw	t4, t5, t3
	mulw	t4, t4, t3
	ld	t5, 88(sp)
	mulw	t4, t4, t5
	mulw	t4, t4, a4
	addw	a2, a2, t4
	addw	a2, a2, t3
	addw	a2, a2, a4
	ld	t5, 112(sp)
	addw	a2, a2, t5
	ld	t5, 16(sp)
	addw	a2, a2, t5
	mulw	t4, a5, t0
	addw	a2, a2, t4
	ld	t5, 104(sp)
	mulw	t4, t5, t0
	addw	a2, a2, t4
	ld	t5, 8(sp)
	ld	t6, 88(sp)
	mulw	t4, t5, t6
	addw	a2, a2, t4
	mulw	t4, a6, a6
	addw	a2, a2, t4
	addw	a2, a2, s3
	addw	a2, a2, t0
	ld	t5, 176(sp)
	addw	a2, a2, t5
	addw	a2, a2, t2
	addw	a2, a2, t2
	addw	a2, a2, t1
	addw	a2, a2, a7
	addw	a2, a2, t3
	addw	a2, a2, a5
	ld	t5, 56(sp)
	addw	a2, a2, t5
	addw	a2, a2, a5
	addw	a2, a2, a4
	mulw	s2, t2, t2
	mulw	a3, s2, a5
	addw	a2, a2, a3
	addw	a2, a2, a5
	ld	t5, 88(sp)
	addw	a2, a2, t5
	addw	a2, a2, t4
	ld	t5, 88(sp)
	mulw	t5, t5, t3
	sd	t5, 200(sp)
	mulw	a3, t5, t3
	mulw	a3, a3, t2
	addw	a2, a2, a3
	ld	t5, 144(sp)
	addw	a2, a2, t5
	addw	a2, a2, a5
	ld	t5, 168(sp)
	mulw	a3, t5, a6
	mulw	a3, a3, t2
	addw	a2, a2, a3
	ld	t5, 152(sp)
	addw	a2, a2, t5
	ld	t5, 16(sp)
	addw	a2, a2, t5
	addw	a2, a2, t3
	addw	a2, a2, a4
	addw	a2, a2, t3
	addw	a2, a2, t1
	ld	t5, 88(sp)
	mulw	t5, t5, a5
	sd	t5, 208(sp)
	addw	a2, a2, t5
	addw	a2, a2, t3
	mulw	a3, a7, a4
	addw	a2, a2, a3
	addw	a2, a2, a6
	addw	a2, a2, a4
	addw	a2, a2, a7
	ld	t5, 96(sp)
	mulw	a3, t5, a5
	mulw	a3, a3, a7
	mulw	a3, a3, t3
	addw	a2, a2, a3
	ld	t5, 72(sp)
	addw	a2, a2, t5
	ld	t5, 72(sp)
	addw	a2, a2, t5
	addw	a2, a2, t1
	mulw	a3, t3, t3
	ld	t5, 16(sp)
	mulw	a3, a3, t5
	addw	a2, a2, a3
	mulw	t5, t0, a4
	sd	t5, 216(sp)
	addw	a2, a2, t5
	ld	t5, 160(sp)
	ld	t6, 16(sp)
	mulw	a3, t5, t6
	addw	a2, a2, a3
	ld	t5, 184(sp)
	addw	a2, a2, t5
	ld	t5, 192(sp)
	mulw	a3, t5, t3
	addw	a2, a2, a3
	ld	t5, 16(sp)
	addw	a2, a2, t5
	ld	t5, 88(sp)
	addw	a2, a2, t5
	addw	a2, a2, a4
	ld	t5, 168(sp)
	addw	a2, a2, t5
	ld	t5, 40(sp)
	mulw	a3, t5, a4
	mulw	a3, a3, t3
	addw	a2, a2, a3
	addw	a2, a2, t3
	ld	t5, 104(sp)
	mulw	a3, t5, a4
	mulw	a3, a3, t1
	addw	a2, a2, a3
	ld	t5, 48(sp)
	mulw	a3, t5, a4
	addw	a2, a2, a3
	ld	t5, 168(sp)
	addw	a2, a2, t5
	addw	a2, a2, a5
	addw	a2, a2, t2
	addw	a2, a2, a4
	mulw	a3, t4, a7
	addw	a2, a2, a3
	addw	a2, a2, a7
	addw	a2, a2, a6
	ld	t5, 16(sp)
	addw	a2, a2, t5
	addw	a2, a2, t0
	ld	t5, 88(sp)
	addw	a2, a2, t5
	ld	t5, 24(sp)
	addw	a2, a2, t5
	ld	t5, 200(sp)
	mulw	a3, t5, a6
	addw	a2, a2, a3
	ld	t5, 96(sp)
	addw	a2, a2, t5
	ld	t5, 8(sp)
	addw	a2, a2, t5
	ld	t5, 80(sp)
	addw	a2, a2, t5
	ld	t5, 88(sp)
	addw	a2, a2, t5
	ld	t5, 32(sp)
	mulw	a6, t5, a4
	addw	a2, a2, a6
	ld	t5, 208(sp)
	addw	a2, a2, t5
	addw	a2, a2, t2
	ld	t5, 8(sp)
	addw	a2, a2, t5
	ld	t5, 120(sp)
	mulw	a6, t5, a5
	addw	a2, a2, a6
	addw	a2, a2, a7
	ld	t5, 216(sp)
	mulw	a5, t5, a5
	addw	a2, a2, a5
	ld	t5, 72(sp)
	addw	a2, a2, t5
	ld	t5, 136(sp)
	addw	a2, a2, t5
	ld	t5, 16(sp)
	mulw	a5, t5, t5
	mulw	a5, a5, a7
	addw	a2, a2, a5
	ld	t5, 16(sp)
	mulw	t0, t0, t5
	mulw	t0, t0, a7
	mulw	t0, t0, a7
	mulw	t0, t0, t1
	addw	t0, a2, t0
	addw	t0, t0, t1
	addw	t0, t0, a7
	ld	t5, 168(sp)
	mulw	t1, t5, a4
	mulw	t1, t1, t2
	addw	t0, t0, t1
	addw	t0, t0, a3
	addw	t0, t0, a4
	addw	t0, t0, t3
	ld	t5, 200(sp)
	addw	t0, t0, t5
	ld	t5, 88(sp)
	addw	t0, t0, t5
	addw	t0, t0, t2
	ld	t5, 16(sp)
	addw	t0, t0, t5
	addw	a0, t0, t3
	ld	s4, 256(sp)
	ld	s3, 248(sp)
	ld	s2, 240(sp)
	ld	s1, 232(sp)
	ld	s0, 224(sp)
	addi	sp, sp, 272
	ret
	.size func.eqargs.676to10.h14669754348576032141, .-func.eqargs.676to10.h14669754348576032141
