
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
	addi	sp, sp, -208
	sd	s0, 112(sp)
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
	addi	s0, sp, 208
	mv	t0, a0
	mv	t1, a1
	mv	t2, a2
	mv	t3, a3
	mv	a2, a4
	mv	a3, a5
	mv	a4, a6
	mv	a5, a7
	lw	a6, 0(s0)
	lw	a7, 8(s0)
	mulw	t4, t0, t0
	mulw	a0, t4, t0
	mulw	a0, a0, a7
	mulw	a0, a0, a5
	addw	a0, a0, a3
	mulw	t5, a3, a4
	sd	t5, 72(sp)
	mulw	a1, t5, a5
	mulw	a1, a1, t2
	mulw	a1, a1, a4
	mulw	a1, a1, t1
	mulw	a1, a1, t0
	addw	a0, a0, a1
	mulw	s2, a6, t3
	addw	a0, a0, s2
	addw	a0, a0, a4
	mulw	t5, t2, a5
	sd	t5, 64(sp)
	mulw	a1, t5, a3
	addw	a0, a0, a1
	mulw	t5, a3, a7
	sd	t5, 0(sp)
	addw	a0, a0, t5
	mulw	t5, a4, a2
	sd	t5, 16(sp)
	addw	a0, a0, t5
	addw	a0, a0, a3
	mulw	s6, t1, t0
	mulw	a1, s6, a5
	mulw	a1, a1, a7
	addw	a0, a0, a1
	addw	a0, a0, a3
	addw	a0, a0, a6
	addw	a0, a0, a5
	mulw	t4, t4, a4
	addw	t4, a0, t4
	mulw	t5, t0, t3
	sd	t5, 88(sp)
	addw	t4, t4, t5
	mulw	t5, t0, t2
	sd	t5, 48(sp)
	mulw	s9, t5, a4
	mulw	a0, s9, t2
	mulw	a0, a0, a7
	addw	t4, t4, a0
	addw	t4, t4, a3
	addw	t4, t4, t3
	addw	t4, t4, a5
	mulw	t5, a4, t3
	sd	t5, 40(sp)
	mulw	a0, t5, t3
	addw	t4, t4, a0
	mulw	t5, a5, t3
	sd	t5, 8(sp)
	addw	t4, t4, t5
	mulw	a0, t1, a3
	mulw	a0, a0, a4
	addw	t4, t4, a0
	ld	t5, 88(sp)
	addw	t4, t4, t5
	mulw	t5, a7, t3
	sd	t5, 24(sp)
	addw	t4, t4, t5
	addw	t4, t4, a2
	addw	t4, t4, t0
	addw	t4, t4, a6
	mulw	a1, a2, a5
	mulw	s4, a1, t1
	addw	t4, t4, s4
	mulw	s4, t3, a2
	addw	t4, t4, s4
	addw	t4, t4, s2
	mulw	s2, a6, a2
	addw	t4, t4, s2
	addw	t4, t4, t3
	addw	t4, t4, t3
	addw	t4, t4, a6
	mulw	t5, a4, a5
	sd	t5, 96(sp)
	addw	t4, t4, t5
	addw	t4, t4, t2
	mulw	t5, t2, a7
	sd	t5, 80(sp)
	mulw	a0, t5, t3
	addw	t4, t4, a0
	addw	t4, t4, t1
	mulw	a0, s9, a4
	mulw	a0, a0, a2
	mulw	a0, a0, a3
	mulw	a0, a0, t2
	addw	t4, t4, a0
	addw	t4, t4, a4
	mulw	a0, a4, a6
	addw	t4, t4, a0
	addw	t4, t4, t0
	addw	t4, t4, t3
	addw	t4, t4, t2
	addw	t4, t4, t1
	mulw	a0, t2, t1
	addw	t4, t4, a0
	addw	t4, t4, a6
	mulw	t5, a2, a3
	sd	t5, 32(sp)
	addw	t4, t4, t5
	mulw	a0, s4, t2
	addw	t4, t4, a0
	addw	t4, t4, a4
	addw	t4, t4, t2
	ld	t5, 88(sp)
	addw	t4, t4, t5
	addw	t4, t4, t0
	mulw	a0, a7, t0
	addw	t4, t4, a0
	mulw	a0, t0, a3
	mulw	a0, a0, a6
	addw	t4, t4, a0
	addw	t4, t4, a4
	mulw	a0, a1, a3
	addw	t4, t4, a0
	addw	t4, t4, t0
	addw	t4, t4, a2
	addw	t4, t4, a4
	addw	t4, t4, a2
	addw	t4, t4, a4
	mulw	a0, t2, a6
	addw	t4, t4, a0
	ld	t5, 32(sp)
	mulw	a0, t5, t1
	mulw	a0, a0, a6
	mulw	a0, a0, a6
	mulw	a0, a0, a5
	addw	t4, t4, a0
	addw	t4, t4, a3
	ld	t5, 64(sp)
	mulw	a0, t5, t1
	mulw	a0, a0, a6
	mulw	a0, a0, a4
	addw	t4, t4, a0
	mulw	t5, a7, a5
	sd	t5, 104(sp)
	mulw	s9, t5, a7
	mulw	s10, s9, a2
	addw	t4, t4, s10
	addw	t4, t4, a2
	mulw	t5, t1, a6
	sd	t5, 56(sp)
	mulw	s8, t5, a2
	mulw	s9, s8, a2
	addw	t4, t4, s9
	addw	t4, t4, a6
	ld	t5, 64(sp)
	mulw	s3, t5, t3
	mulw	s1, s3, a3
	mulw	s5, s1, a7
	mulw	s7, s5, a2
	mulw	s11, s7, a6
	addw	t4, t4, s11
	addw	t4, t4, a6
	addw	t4, t4, a4
	mulw	a0, t1, t3
	addw	t4, t4, a0
	mulw	a0, s6, a6
	mulw	a0, a0, t3
	mulw	a0, a0, a4
	mulw	a0, a0, a7
	mulw	a0, a0, a2
	mulw	a0, a0, a6
	mulw	a0, a0, t2
	mulw	a0, a0, a6
	mulw	a0, a0, a6
	addw	t4, t4, a0
	mulw	s1, a3, t2
	addw	t4, t4, s1
	addw	t4, t4, t3
	mulw	a0, s2, t1
	addw	t4, t4, a0
	addw	t4, t4, a2
	addw	t4, t4, t1
	addw	t4, t4, t2
	mulw	s3, a6, a3
	addw	t4, t4, s3
	addw	t4, t4, t3
	addw	t4, t4, a2
	addw	t4, t4, t2
	addw	t4, t4, a5
	addw	t4, t4, a2
	addw	t4, t4, a4
	addw	t4, t4, t3
	addw	t4, t4, t0
	addw	t4, t4, t0
	mulw	s5, t2, a4
	mulw	s7, s5, t0
	mulw	a0, s7, a7
	mulw	a0, a0, a6
	addw	t4, t4, a0
	mulw	a0, t3, t2
	mulw	a0, a0, a3
	mulw	a0, a0, a5
	mulw	a0, a0, a3
	addw	t4, t4, a0
	mulw	s8, a6, t0
	addw	t4, t4, s8
	addw	t4, t4, t2
	addw	t4, t4, s3
	mulw	a0, a4, t1
	addw	t4, t4, a0
	addw	t4, t4, a5
	addw	t4, t4, a2
	addw	t4, t4, a3
	addw	t4, t4, t3
	mulw	a0, a1, a7
	addw	t4, t4, a0
	addw	t4, t4, s5
	addw	t4, t4, t3
	mulw	a0, s4, a7
	addw	t4, t4, a0
	addw	t4, t4, a3
	addw	t4, t4, t0
	mulw	a0, a2, t1
	mulw	a0, a0, t3
	mulw	a0, a0, a2
	addw	t4, t4, a0
	mulw	a0, s6, t3
	mulw	a0, a0, t3
	mulw	a0, a0, a3
	mulw	a0, a0, a4
	addw	t4, t4, a0
	addw	t4, t4, t3
	addw	t4, t4, a4
	ld	t5, 96(sp)
	addw	t4, t4, t5
	addw	t4, t4, a2
	mulw	a0, a5, t0
	addw	t4, t4, a0
	ld	t5, 80(sp)
	mulw	a0, t5, t0
	addw	t4, t4, a0
	ld	t5, 72(sp)
	mulw	a0, t5, a3
	addw	t4, t4, a0
	mulw	s3, a6, a6
	addw	t4, t4, s3
	addw	t4, t4, s5
	addw	t4, t4, t0
	ld	t5, 104(sp)
	addw	t4, t4, t5
	addw	t4, t4, t2
	addw	t4, t4, t2
	addw	t4, t4, t1
	addw	t4, t4, a7
	addw	t4, t4, t3
	addw	t4, t4, a5
	ld	t5, 48(sp)
	addw	t4, t4, t5
	addw	t4, t4, a5
	addw	t4, t4, a4
	mulw	a0, t2, t2
	mulw	a0, a0, a5
	addw	t4, t4, a0
	addw	t4, t4, a5
	addw	t4, t4, a3
	addw	t4, t4, s3
	mulw	s4, a3, t3
	mulw	a0, s4, t3
	mulw	a0, a0, t2
	addw	t4, t4, a0
	ld	t5, 56(sp)
	addw	t4, t4, t5
	addw	t4, t4, a5
	ld	t5, 88(sp)
	mulw	a0, t5, a6
	mulw	a0, a0, t2
	addw	t4, t4, a0
	addw	t4, t4, s1
	addw	t4, t4, a2
	addw	t4, t4, t3
	addw	t4, t4, a4
	addw	t4, t4, t3
	addw	t4, t4, t1
	mulw	s1, a3, a5
	addw	t4, t4, s1
	addw	t4, t4, t3
	mulw	a0, a7, a4
	addw	t4, t4, a0
	addw	t4, t4, a6
	addw	t4, t4, a4
	addw	t4, t4, a7
	ld	t5, 24(sp)
	mulw	a0, t5, a5
	mulw	a0, a0, a7
	mulw	a0, a0, t3
	addw	t4, t4, a0
	ld	t5, 40(sp)
	addw	t4, t4, t5
	ld	t5, 40(sp)
	addw	t4, t4, t5
	addw	t4, t4, t1
	mulw	a0, t3, t3
	mulw	a0, a0, a2
	addw	t4, t4, a0
	mulw	s5, t0, a4
	addw	t4, t4, s5
	mulw	a0, s7, a2
	addw	t4, t4, a0
	addw	t4, t4, a1
	mulw	a0, s8, t3
	addw	t4, t4, a0
	addw	t4, t4, a2
	addw	t4, t4, a3
	addw	t4, t4, a4
	ld	t5, 88(sp)
	addw	t4, t4, t5
	ld	t5, 16(sp)
	mulw	a0, t5, a4
	mulw	a0, a0, t3
	addw	t4, t4, a0
	addw	t4, t4, t3
	ld	t5, 80(sp)
	mulw	a0, t5, a4
	mulw	a0, a0, t1
	addw	t4, t4, a0
	mulw	a0, s6, a4
	addw	t4, t4, a0
	ld	t5, 88(sp)
	addw	t4, t4, t5
	addw	t4, t4, a5
	addw	t4, t4, t2
	addw	t4, t4, a4
	mulw	a0, s3, a7
	addw	t4, t4, a0
	addw	t4, t4, a7
	addw	t4, t4, a6
	addw	t4, t4, a2
	addw	t4, t4, t0
	addw	t4, t4, a3
	ld	t5, 64(sp)
	addw	t4, t4, t5
	mulw	a6, s4, a6
	addw	t4, t4, a6
	ld	t5, 24(sp)
	addw	t4, t4, t5
	ld	t5, 72(sp)
	addw	t4, t4, t5
	ld	t5, 8(sp)
	addw	t4, t4, t5
	addw	t4, t4, a3
	ld	t5, 0(sp)
	mulw	a0, t5, a4
	addw	t4, t4, a0
	addw	t4, t4, s1
	addw	t4, t4, t2
	ld	t5, 72(sp)
	addw	t4, t4, t5
	mulw	a0, s2, a5
	addw	t4, t4, a0
	addw	t4, t4, a7
	mulw	a5, s5, a5
	addw	a5, t4, a5
	ld	t5, 40(sp)
	addw	a5, a5, t5
	ld	t5, 32(sp)
	addw	a5, a5, t5
	mulw	t4, a2, a2
	mulw	t4, t4, a7
	addw	a5, a5, t4
	mulw	t0, t0, a2
	mulw	t0, t0, a7
	mulw	t0, t0, a7
	mulw	t0, t0, t1
	addw	t0, a5, t0
	addw	t0, t0, t1
	addw	t0, t0, a7
	ld	t5, 88(sp)
	mulw	t1, t5, a4
	mulw	t1, t1, t2
	addw	t0, t0, t1
	addw	t0, t0, a6
	addw	t0, t0, a4
	addw	t0, t0, t3
	addw	t0, t0, s4
	addw	t0, t0, a3
	addw	t0, t0, t2
	addw	t0, t0, a2
	addw	a0, t0, t3
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
	ld	s0, 112(sp)
	addi	sp, sp, 208
	ret
	.size func.eqargs.676to10.h14669754348576032141, .-func.eqargs.676to10.h14669754348576032141
