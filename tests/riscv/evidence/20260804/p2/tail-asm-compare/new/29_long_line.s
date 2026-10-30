
  .text

  .globl fib
  .type  fib, @function
fib:
.fib.bb0:
	li	t6, -2144
	add	sp, sp, t6
	li	t6, 2048
	add	t6, sp, t6
	sd	ra, 0(t6)
	li	t6, 2056
	add	t6, sp, t6
	sd	s1, 0(t6)
	li	t6, 2064
	add	t6, sp, t6
	sd	s2, 0(t6)
	li	t6, 2072
	add	t6, sp, t6
	sd	s3, 0(t6)
	li	t6, 2080
	add	t6, sp, t6
	sd	s4, 0(t6)
	li	t6, 2088
	add	t6, sp, t6
	sd	s5, 0(t6)
	li	t6, 2096
	add	t6, sp, t6
	sd	s6, 0(t6)
	li	t6, 2104
	add	t6, sp, t6
	sd	s7, 0(t6)
	li	t6, 2112
	add	t6, sp, t6
	sd	s8, 0(t6)
	li	t6, 2120
	add	t6, sp, t6
	sd	s9, 0(t6)
	li	t6, 2128
	add	t6, sp, t6
	sd	s10, 0(t6)
	li	t6, 2136
	add	t6, sp, t6
	sd	s11, 0(t6)
	mv	t0, a0
	li	t1, 2
	blt	t1, t0, .fib.bb34
.fib.bb2:
	li	t0, 1
.fib.bb1:
	mv	a0, t0
	li	t6, 2136
	add	t6, sp, t6
	ld	s11, 0(t6)
	li	t6, 2128
	add	t6, sp, t6
	ld	s10, 0(t6)
	li	t6, 2120
	add	t6, sp, t6
	ld	s9, 0(t6)
	li	t6, 2112
	add	t6, sp, t6
	ld	s8, 0(t6)
	li	t6, 2104
	add	t6, sp, t6
	ld	s7, 0(t6)
	li	t6, 2096
	add	t6, sp, t6
	ld	s6, 0(t6)
	li	t6, 2088
	add	t6, sp, t6
	ld	s5, 0(t6)
	li	t6, 2080
	add	t6, sp, t6
	ld	s4, 0(t6)
	li	t6, 2072
	add	t6, sp, t6
	ld	s3, 0(t6)
	li	t6, 2064
	add	t6, sp, t6
	ld	s2, 0(t6)
	li	t6, 2056
	add	t6, sp, t6
	ld	s1, 0(t6)
	li	t6, 2048
	add	t6, sp, t6
	ld	ra, 0(t6)
	li	t6, 2144
	add	sp, sp, t6
	ret
.fib.bb3:
	j	.fib.bb34
.fib.bb4:
	j	.fib.bb34
.fib.bb5:
	j	.fib.bb34
.fib.bb6:
	j	.fib.bb34
.fib.bb7:
	j	.fib.bb34
.fib.bb8:
	j	.fib.bb34
.fib.bb9:
	j	.fib.bb34
.fib.bb10:
	j	.fib.bb34
.fib.bb11:
	j	.fib.bb34
.fib.bb12:
	j	.fib.bb34
.fib.bb13:
	j	.fib.bb34
.fib.bb14:
	j	.fib.bb34
.fib.bb15:
	j	.fib.bb34
.fib.bb16:
	j	.fib.bb34
.fib.bb17:
	j	.fib.bb34
.fib.bb18:
	j	.fib.bb34
.fib.bb19:
	j	.fib.bb34
.fib.bb20:
	j	.fib.bb34
.fib.bb21:
	j	.fib.bb34
.fib.bb22:
	j	.fib.bb34
.fib.bb23:
	j	.fib.bb34
.fib.bb24:
	j	.fib.bb34
.fib.bb25:
	j	.fib.bb34
.fib.bb26:
	j	.fib.bb34
.fib.bb27:
	j	.fib.bb34
.fib.bb28:
	j	.fib.bb34
.fib.bb29:
	j	.fib.bb34
.fib.bb30:
	j	.fib.bb34
.fib.bb31:
	j	.fib.bb34
.fib.bb32:
	j	.fib.bb34
.fib.bb33:
.fib.bb34:
	li	t1, 0
	li	t3, 0
	ori	t2, t1, 1
	subw	t2, t2, t3
	sltu	t2, zero, t2
.fib.bb35:
	andi	t1, t1, 1
.fib.bb36:
	li	t3, 0
	subw	t3, t1, t3
	sltiu	t3, t3, 1
.fib.bb37:
	and	t2, t2, t3
	li	t3, 0
	subw	t2, t2, t3
	sltu	t2, zero, t2
	j	.fib.bb39
.fib.bb38:
.fib.bb39:
	andi	t5, t2, 1
	sd	t5, 8(sp)
	j	.fib.bb42
.fib.bb40:
	j	.fib.bb42
.fib.bb41:
.fib.bb42:
	li	t3, 1
	li	a2, 0
	or	a3, t3, a2
.fib.bb43:
	and	t3, t3, a2
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb44:
	li	a2, 0
	subw	a2, t3, a2
	sltiu	a2, a2, 1
.fib.bb45:
	and	a2, a3, a2
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.fib.bb46:
	or	a3, a2, t1
	li	a4, 0
	subw	a3, a3, a4
	sltu	a3, zero, a3
.fib.bb47:
	and	t1, a2, t1
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
.fib.bb48:
	li	a2, 0
	subw	a2, t1, a2
	sltiu	a2, a2, 1
.fib.bb49:
	and	a2, a3, a2
	li	a3, 0
	subw	a2, a2, a3
	sltu	t5, zero, a2
	sd	t5, 16(sp)
	j	.fib.bb52
.fib.bb50:
	j	.fib.bb52
.fib.bb51:
.fib.bb52:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb53:
	li	t3, 1
	li	a3, 0
	or	a4, t3, a3
.fib.bb54:
	and	t3, t3, a3
	li	a3, 0
	subw	t3, t3, a3
	sltu	t3, zero, t3
.fib.bb55:
	li	a3, 0
	subw	a3, t3, a3
	sltiu	a3, a3, 1
.fib.bb56:
	and	a3, a4, a3
	li	a4, 0
	subw	a3, a3, a4
	sltu	a3, zero, a3
.fib.bb57:
	or	a4, a3, t1
	li	a5, 0
	subw	a4, a4, a5
	sltu	a4, zero, a4
.fib.bb58:
	and	t1, a3, t1
	li	a3, 0
	subw	t1, t1, a3
	sltu	t1, zero, t1
.fib.bb59:
	li	a3, 0
	subw	a3, t1, a3
	sltiu	a3, a3, 1
.fib.bb60:
	and	a3, a4, a3
	li	a4, 0
	subw	a3, a3, a4
	sltu	t5, zero, a3
	sd	t5, 24(sp)
	j	.fib.bb63
.fib.bb61:
	j	.fib.bb63
.fib.bb62:
.fib.bb63:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb64:
	li	t3, 1
	li	a4, 0
	or	a5, t3, a4
.fib.bb65:
	and	t3, t3, a4
	li	a4, 0
	subw	t3, t3, a4
	sltu	t3, zero, t3
.fib.bb66:
	li	a4, 0
	subw	a4, t3, a4
	sltiu	a4, a4, 1
.fib.bb67:
	and	a4, a5, a4
	li	a5, 0
	subw	a4, a4, a5
	sltu	a4, zero, a4
.fib.bb68:
	or	a5, a4, t1
	li	a6, 0
	subw	a5, a5, a6
	sltu	a5, zero, a5
.fib.bb69:
	and	t1, a4, t1
	li	a4, 0
	subw	t1, t1, a4
	sltu	t1, zero, t1
.fib.bb70:
	li	a4, 0
	subw	a4, t1, a4
	sltiu	a4, a4, 1
.fib.bb71:
	and	a4, a5, a4
	li	a5, 0
	subw	a4, a4, a5
	sltu	t5, zero, a4
	sd	t5, 32(sp)
	j	.fib.bb74
.fib.bb72:
	j	.fib.bb74
.fib.bb73:
.fib.bb74:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb75:
	li	t3, 1
	li	a5, 0
	or	a6, t3, a5
.fib.bb76:
	and	t3, t3, a5
	li	a5, 0
	subw	t3, t3, a5
	sltu	t3, zero, t3
.fib.bb77:
	li	a5, 0
	subw	a5, t3, a5
	sltiu	a5, a5, 1
.fib.bb78:
	and	a5, a6, a5
	li	a6, 0
	subw	a5, a5, a6
	sltu	a5, zero, a5
.fib.bb79:
	or	a6, a5, t1
	li	a7, 0
	subw	a6, a6, a7
	sltu	a6, zero, a6
.fib.bb80:
	and	t1, a5, t1
	li	a5, 0
	subw	t1, t1, a5
	sltu	t1, zero, t1
.fib.bb81:
	li	a5, 0
	subw	a5, t1, a5
	sltiu	a5, a5, 1
.fib.bb82:
	and	a5, a6, a5
	li	a6, 0
	subw	a5, a5, a6
	sltu	t5, zero, a5
	sd	t5, 40(sp)
	j	.fib.bb85
.fib.bb83:
	j	.fib.bb85
.fib.bb84:
.fib.bb85:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb86:
	li	t3, 1
	li	a6, 0
	or	a7, t3, a6
.fib.bb87:
	and	t3, t3, a6
	li	a6, 0
	subw	t3, t3, a6
	sltu	t3, zero, t3
.fib.bb88:
	li	a6, 0
	subw	a6, t3, a6
	sltiu	a6, a6, 1
.fib.bb89:
	and	a6, a7, a6
	li	a7, 0
	subw	a6, a6, a7
	sltu	a6, zero, a6
.fib.bb90:
	or	a7, a6, t1
	li	t4, 0
	subw	a7, a7, t4
	sltu	a7, zero, a7
.fib.bb91:
	and	t1, a6, t1
	li	a6, 0
	subw	t1, t1, a6
	sltu	t1, zero, t1
.fib.bb92:
	li	a6, 0
	subw	a6, t1, a6
	sltiu	a6, a6, 1
.fib.bb93:
	and	a6, a7, a6
	li	a7, 0
	subw	a6, a6, a7
	sltu	t5, zero, a6
	sd	t5, 48(sp)
	j	.fib.bb96
.fib.bb94:
	j	.fib.bb96
.fib.bb95:
.fib.bb96:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb97:
	li	t3, 1
	li	a7, 0
	or	t4, t3, a7
.fib.bb98:
	and	t3, t3, a7
	li	a7, 0
	subw	t3, t3, a7
	sltu	t3, zero, t3
.fib.bb99:
	li	a7, 0
	subw	a7, t3, a7
	sltiu	a7, a7, 1
.fib.bb100:
	and	a7, t4, a7
	li	t4, 0
	subw	a7, a7, t4
	sltu	a7, zero, a7
.fib.bb101:
	or	t4, a7, t1
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
.fib.bb102:
	and	t1, a7, t1
	li	a7, 0
	subw	t1, t1, a7
	sltu	t1, zero, t1
.fib.bb103:
	li	a7, 0
	subw	a7, t1, a7
	sltiu	a7, a7, 1
.fib.bb104:
	and	a7, t4, a7
	li	t4, 0
	subw	a7, a7, t4
	sltu	t5, zero, a7
	sd	t5, 56(sp)
	j	.fib.bb107
.fib.bb105:
	j	.fib.bb107
.fib.bb106:
.fib.bb107:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb108:
	li	t3, 1
	li	t4, 0
	or	a0, t3, t4
.fib.bb109:
	and	t3, t3, t4
	li	t4, 0
	subw	t3, t3, t4
	sltu	t3, zero, t3
.fib.bb110:
	li	t4, 0
	subw	t4, t3, t4
	sltiu	t4, t4, 1
.fib.bb111:
	and	t4, a0, t4
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
.fib.bb112:
	or	a0, t4, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb113:
	and	t1, t4, t1
	li	t4, 0
	subw	t1, t1, t4
	sltu	t1, zero, t1
.fib.bb114:
	li	t4, 0
	subw	t4, t1, t4
	sltiu	t4, t4, 1
.fib.bb115:
	and	t4, a0, t4
	li	a0, 0
	subw	t4, t4, a0
	sltu	t5, zero, t4
	sd	t5, 64(sp)
	j	.fib.bb118
.fib.bb116:
	j	.fib.bb118
.fib.bb117:
.fib.bb118:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb119:
	li	t3, 1
	li	a0, 0
	or	t2, t3, a0
.fib.bb120:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb121:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb122:
	and	t2, t2, a0
	li	a0, 0
	subw	t2, t2, a0
	sltu	t2, zero, t2
.fib.bb123:
	or	a0, t2, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb124:
	and	t1, t2, t1
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
.fib.bb125:
	li	t2, 0
	subw	t2, t1, t2
	sltiu	t2, t2, 1
.fib.bb126:
	and	t2, a0, t2
	li	a0, 0
	subw	t2, t2, a0
	sltu	t2, zero, t2
	j	.fib.bb129
.fib.bb127:
	j	.fib.bb129
.fib.bb128:
.fib.bb129:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb130:
	li	t3, 1
	li	a0, 0
	or	a2, t3, a0
.fib.bb131:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb132:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb133:
	and	a2, a2, a0
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
.fib.bb134:
	or	a0, a2, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb135:
	and	t1, a2, t1
	li	a2, 0
	subw	t1, t1, a2
	sltu	t1, zero, t1
.fib.bb136:
	li	a2, 0
	subw	a2, t1, a2
	sltiu	a2, a2, 1
.fib.bb137:
	and	a2, a0, a2
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
	j	.fib.bb140
.fib.bb138:
	j	.fib.bb140
.fib.bb139:
.fib.bb140:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb141:
	li	t3, 1
	li	a0, 0
	or	a3, t3, a0
.fib.bb142:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb143:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb144:
	and	a3, a3, a0
	li	a0, 0
	subw	a3, a3, a0
	sltu	a3, zero, a3
.fib.bb145:
	or	a0, a3, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb146:
	and	t1, a3, t1
	li	a3, 0
	subw	t1, t1, a3
	sltu	t1, zero, t1
.fib.bb147:
	li	a3, 0
	subw	a3, t1, a3
	sltiu	a3, a3, 1
.fib.bb148:
	and	a3, a0, a3
	li	a0, 0
	subw	a3, a3, a0
	sltu	a3, zero, a3
	j	.fib.bb151
.fib.bb149:
	j	.fib.bb151
.fib.bb150:
.fib.bb151:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb152:
	li	t3, 1
	li	a0, 0
	or	a4, t3, a0
.fib.bb153:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb154:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb155:
	and	a4, a4, a0
	li	a0, 0
	subw	a4, a4, a0
	sltu	a4, zero, a4
.fib.bb156:
	or	a0, a4, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb157:
	and	t1, a4, t1
	li	a4, 0
	subw	t1, t1, a4
	sltu	t1, zero, t1
.fib.bb158:
	li	a4, 0
	subw	a4, t1, a4
	sltiu	a4, a4, 1
.fib.bb159:
	and	a4, a0, a4
	li	a0, 0
	subw	a4, a4, a0
	sltu	a4, zero, a4
	j	.fib.bb162
.fib.bb160:
	j	.fib.bb162
.fib.bb161:
.fib.bb162:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb163:
	li	t3, 1
	li	a0, 0
	or	a5, t3, a0
.fib.bb164:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb165:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb166:
	and	a5, a5, a0
	li	a0, 0
	subw	a5, a5, a0
	sltu	a5, zero, a5
.fib.bb167:
	or	a0, a5, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb168:
	and	t1, a5, t1
	li	a5, 0
	subw	t1, t1, a5
	sltu	t1, zero, t1
.fib.bb169:
	li	a5, 0
	subw	a5, t1, a5
	sltiu	a5, a5, 1
.fib.bb170:
	and	a5, a0, a5
	li	a0, 0
	subw	a5, a5, a0
	sltu	a5, zero, a5
	j	.fib.bb173
.fib.bb171:
	j	.fib.bb173
.fib.bb172:
.fib.bb173:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb174:
	li	t3, 1
	li	a0, 0
	or	a6, t3, a0
.fib.bb175:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb176:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb177:
	and	a6, a6, a0
	li	a0, 0
	subw	a6, a6, a0
	sltu	a6, zero, a6
.fib.bb178:
	or	a0, a6, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb179:
	and	t1, a6, t1
	li	a6, 0
	subw	t1, t1, a6
	sltu	t1, zero, t1
.fib.bb180:
	li	a6, 0
	subw	a6, t1, a6
	sltiu	a6, a6, 1
.fib.bb181:
	and	a6, a0, a6
	li	a0, 0
	subw	a6, a6, a0
	sltu	a6, zero, a6
	j	.fib.bb184
.fib.bb182:
	j	.fib.bb184
.fib.bb183:
.fib.bb184:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb185:
	li	t3, 1
	li	a0, 0
	or	a7, t3, a0
.fib.bb186:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb187:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb188:
	and	a7, a7, a0
	li	a0, 0
	subw	a7, a7, a0
	sltu	a7, zero, a7
.fib.bb189:
	or	a0, a7, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb190:
	and	t1, a7, t1
	li	a7, 0
	subw	t1, t1, a7
	sltu	t1, zero, t1
.fib.bb191:
	li	a7, 0
	subw	a7, t1, a7
	sltiu	a7, a7, 1
.fib.bb192:
	and	a7, a0, a7
	li	a0, 0
	subw	a7, a7, a0
	sltu	a7, zero, a7
	j	.fib.bb195
.fib.bb193:
	j	.fib.bb195
.fib.bb194:
.fib.bb195:
	or	t1, t3, t1
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
.fib.bb196:
	li	t3, 1
	li	a0, 0
	or	t4, t3, a0
.fib.bb197:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb198:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb199:
	and	t4, t4, a0
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
.fib.bb200:
	or	a0, t4, t1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb201:
	and	t1, t4, t1
	li	t4, 0
	subw	t1, t1, t4
	sltu	t1, zero, t1
.fib.bb202:
	li	t4, 0
	subw	t4, t1, t4
	sltiu	t4, t4, 1
.fib.bb203:
	and	t4, a0, t4
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
	j	.fib.bb206
.fib.bb204:
	j	.fib.bb206
.fib.bb205:
.fib.bb206:
	or	t1, t3, t1
	bne	t1, zero, .fib.bb209
.fib.bb208:
.fib.bb209:
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t1
	andi	t3, t0, 1
	subw	t1, t3, t1
	li	t3, 0
	slt	t3, t1, t3
	subw	a0, zero, t1
	li	a1, 0
	subw	t3, a1, t3
	xor	a0, t1, a0
	and	t3, a0, t3
	xor	t5, t1, t3
	sd	t5, 72(sp)
.fib.bb210:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 80(sp)
.fib.bb211:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 88(sp)
.fib.bb212:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 96(sp)
.fib.bb213:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 104(sp)
.fib.bb214:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 112(sp)
.fib.bb215:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 120(sp)
.fib.bb216:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 128(sp)
.fib.bb217:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 136(sp)
.fib.bb218:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 144(sp)
.fib.bb219:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 152(sp)
.fib.bb220:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 160(sp)
.fib.bb221:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 168(sp)
.fib.bb222:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 176(sp)
.fib.bb223:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a0, t0, 1
	subw	t3, a0, t3
	li	a0, 0
	slt	a0, t3, a0
	subw	a1, zero, t3
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t3, a1
	and	t1, a0, t1
	xor	t5, t3, t1
	sd	t5, 192(sp)
.fib.bb224:
	sraiw	t0, t0, 1
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	t0, t0, 1
	subw	t0, t0, t3
	li	t3, 0
	slt	t3, t0, t3
	subw	a0, zero, t0
	li	a1, 0
	subw	t3, a1, t3
	xor	a0, t0, a0
	and	t3, a0, t3
	xor	t5, t0, t3
	sd	t5, 184(sp)
.fib.bb225:
	slliw	t3, t4, 1
	addw	t3, t3, a7
	slliw	t3, t3, 1
	addw	t3, t3, a6
	slliw	t3, t3, 1
	addw	t3, t3, a5
	slliw	t3, t3, 1
	addw	t3, t3, a4
	slliw	t3, t3, 1
	addw	t3, t3, a3
	slliw	t3, t3, 1
	addw	t3, t3, a2
	slliw	t3, t3, 1
	addw	t2, t3, t2
	slliw	t2, t2, 1
	ld	t5, 64(sp)
	addw	t2, t2, t5
	slliw	t2, t2, 1
	ld	t5, 56(sp)
	addw	t2, t2, t5
	slliw	t2, t2, 1
	ld	t5, 48(sp)
	addw	t2, t2, t5
	slliw	t2, t2, 1
	ld	t5, 40(sp)
	addw	t2, t2, t5
	slliw	t2, t2, 1
	ld	t5, 32(sp)
	addw	t2, t2, t5
	slliw	t2, t2, 1
	ld	t5, 24(sp)
	addw	t2, t2, t5
	slliw	t2, t2, 1
	ld	t5, 16(sp)
	addw	t2, t2, t5
	slliw	t2, t2, 1
	ld	t5, 8(sp)
	addw	t2, t2, t5
	srliw	t3, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t3
	andi	a2, t2, 1
	subw	t3, a2, t3
	li	a2, 0
	slt	a2, t3, a2
	subw	a3, zero, t3
	li	a4, 0
	subw	a2, a4, a2
	xor	a3, t3, a3
	and	a2, a3, a2
	xor	t3, t3, a2
.fib.bb226:
	sraiw	t2, t2, 1
	li	a5, 0
	srliw	a2, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a2
	andi	a3, t2, 1
	subw	a2, a3, a2
	li	a3, 0
	subw	a4, zero, a2
	slt	a3, a2, a3
	xor	a4, a2, a4
	subw	a3, a5, a3
	and	a3, a4, a3
	xor	a2, a2, a3
.fib.bb227:
	sraiw	t2, t2, 1
	li	a6, 0
	srliw	a3, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a3
	andi	a4, t2, 1
	subw	a3, a4, a3
	li	a4, 0
	subw	a5, zero, a3
	slt	a4, a3, a4
	xor	a5, a3, a5
	subw	a4, a6, a4
	and	a4, a5, a4
	xor	a3, a3, a4
.fib.bb228:
	sraiw	t2, t2, 1
	li	a7, 0
	srliw	a4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a4
	andi	a5, t2, 1
	subw	a4, a5, a4
	li	a5, 0
	subw	a6, zero, a4
	slt	a5, a4, a5
	xor	a6, a4, a6
	subw	a5, a7, a5
	and	a5, a6, a5
	xor	a4, a4, a5
.fib.bb229:
	sraiw	t2, t2, 1
	li	t4, 0
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	li	a6, 0
	subw	a7, zero, a5
	slt	a6, a5, a6
	xor	a7, a5, a7
	subw	a6, t4, a6
	and	a6, a7, a6
	xor	a5, a5, a6
.fib.bb230:
	sraiw	t2, t2, 1
	li	a0, 0
	srliw	a6, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a6
	andi	a7, t2, 1
	subw	a6, a7, a6
	li	a7, 0
	subw	t4, zero, a6
	slt	a7, a6, a7
	xor	t4, a6, t4
	subw	a7, a0, a7
	and	a7, t4, a7
	xor	a6, a6, a7
.fib.bb231:
	sraiw	t2, t2, 1
	li	a1, 0
	srliw	a7, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a7
	andi	t4, t2, 1
	subw	a7, t4, a7
	li	t4, 0
	subw	a0, zero, a7
	slt	t4, a7, t4
	xor	a0, a7, a0
	subw	t4, a1, t4
	and	t4, a0, t4
	xor	a7, a7, t4
.fib.bb232:
	sraiw	t2, t2, 1
	li	t0, 0
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	a0, t2, 1
	subw	t4, a0, t4
	li	a0, 0
	subw	a1, zero, t4
	slt	a0, t4, a0
	subw	t0, t0, a0
	xor	a0, t4, a1
	and	t0, a0, t0
	xor	t0, t4, t0
.fib.bb233:
	sraiw	t2, t2, 1
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	a0, t2, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t4, a1
	and	t1, a0, t1
	xor	t5, t4, t1
	sd	t5, 200(sp)
.fib.bb234:
	sraiw	t2, t2, 1
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	a0, t2, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t4, a1
	and	t1, a0, t1
	xor	t5, t4, t1
	sd	t5, 208(sp)
.fib.bb235:
	sraiw	t2, t2, 1
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	a0, t2, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t4, a1
	and	t1, a0, t1
	xor	t5, t4, t1
	sd	t5, 216(sp)
.fib.bb236:
	sraiw	t2, t2, 1
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	a0, t2, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t4, a1
	and	t1, a0, t1
	xor	t5, t4, t1
	sd	t5, 224(sp)
.fib.bb237:
	sraiw	t2, t2, 1
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	a0, t2, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t4, a1
	and	t1, a0, t1
	xor	t5, t4, t1
	sd	t5, 232(sp)
.fib.bb238:
	sraiw	t2, t2, 1
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	a0, t2, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	t1, 0
	subw	t1, t1, a0
	xor	a0, t4, a1
	and	t1, a0, t1
	xor	t5, t4, t1
	sd	t5, 240(sp)
.fib.bb239:
	sraiw	t2, t2, 1
	li	t1, 0
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	a0, t2, 1
	subw	t4, a0, t4
	li	a0, 0
	subw	a1, zero, t4
	slt	a0, t4, a0
	subw	t1, t1, a0
	xor	a0, t4, a1
	and	t1, a0, t1
	xor	t1, t4, t1
.fib.bb240:
	sraiw	t2, t2, 1
	li	a1, 0
	srliw	t4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t4
	andi	t2, t2, 1
	subw	t2, t2, t4
	li	t4, 0
	subw	a0, zero, t2
	slt	t4, t2, t4
	xor	a0, t2, a0
	subw	t4, a1, t4
	and	t4, a0, t4
	xor	t2, t2, t4
.fib.bb241:
	li	t4, 0
	ld	t5, 72(sp)
	subw	t4, t5, t4
	sltu	s1, zero, t4
	li	t4, 0
	subw	t3, t3, t4
	sltu	t3, zero, t3
	or	t4, s1, t3
.fib.bb242:
	and	t3, s1, t3
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb243:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb244:
	and	t4, t4, a0
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
	j	.fib.bb246
.fib.bb245:
.fib.bb246:
	andi	t5, t4, 1
	sd	t5, 248(sp)
	j	.fib.bb249
.fib.bb247:
	j	.fib.bb249
.fib.bb248:
.fib.bb249:
	li	a0, 0
	ld	t5, 80(sp)
	subw	a0, t5, a0
	sltu	s2, zero, a0
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
	or	a0, s2, a2
.fib.bb250:
	and	a2, s2, a2
	li	a1, 0
	subw	a2, a2, a1
	sltu	a2, zero, a2
.fib.bb251:
	li	a1, 0
	subw	a1, a2, a1
	sltiu	a1, a1, 1
.fib.bb252:
	and	a0, a0, a1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb253:
	or	a1, a0, t3
	li	t4, 0
	subw	t4, a1, t4
	sltu	t4, zero, t4
.fib.bb254:
	and	t3, a0, t3
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb255:
	li	a0, 0
	subw	a0, t3, a0
	sltiu	a0, a0, 1
.fib.bb256:
	and	t4, t4, a0
	li	a0, 0
	subw	t4, t4, a0
	sltu	t5, zero, t4
	sd	t5, 256(sp)
	j	.fib.bb259
.fib.bb257:
	j	.fib.bb259
.fib.bb258:
.fib.bb259:
	or	t3, a2, t3
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb260:
	li	a2, 0
	ld	t5, 88(sp)
	subw	a2, t5, a2
	sltu	s3, zero, a2
	li	a2, 0
	subw	a2, a3, a2
	sltu	a2, zero, a2
	or	a3, s3, a2
.fib.bb261:
	and	a2, s3, a2
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
.fib.bb262:
	li	a0, 0
	subw	a0, a2, a0
	sltiu	a0, a0, 1
.fib.bb263:
	and	a3, a3, a0
	li	a0, 0
	subw	a3, a3, a0
	sltu	a3, zero, a3
.fib.bb264:
	or	a0, a3, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb265:
	and	t3, a3, t3
	li	a3, 0
	subw	t3, t3, a3
	sltu	t3, zero, t3
.fib.bb266:
	li	a3, 0
	subw	a3, t3, a3
	sltiu	a3, a3, 1
.fib.bb267:
	and	a3, a0, a3
	li	a0, 0
	subw	a3, a3, a0
	sltu	t5, zero, a3
	sd	t5, 264(sp)
	j	.fib.bb270
.fib.bb268:
	j	.fib.bb270
.fib.bb269:
.fib.bb270:
	or	t3, a2, t3
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb271:
	li	a2, 0
	ld	t5, 96(sp)
	subw	a2, t5, a2
	sltu	s4, zero, a2
	li	a2, 0
	subw	a2, a4, a2
	sltu	a2, zero, a2
	or	a4, s4, a2
.fib.bb272:
	and	a2, s4, a2
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
.fib.bb273:
	li	a0, 0
	subw	a0, a2, a0
	sltiu	a0, a0, 1
.fib.bb274:
	and	a4, a4, a0
	li	a0, 0
	subw	a4, a4, a0
	sltu	a4, zero, a4
.fib.bb275:
	or	a0, a4, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb276:
	and	t3, a4, t3
	li	a4, 0
	subw	t3, t3, a4
	sltu	t3, zero, t3
.fib.bb277:
	li	a4, 0
	subw	a4, t3, a4
	sltiu	a4, a4, 1
.fib.bb278:
	and	a4, a0, a4
	li	a0, 0
	subw	a4, a4, a0
	sltu	t5, zero, a4
	sd	t5, 272(sp)
	j	.fib.bb281
.fib.bb279:
	j	.fib.bb281
.fib.bb280:
.fib.bb281:
	or	t3, a2, t3
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb282:
	li	a2, 0
	ld	t5, 104(sp)
	subw	a2, t5, a2
	sltu	s5, zero, a2
	li	a2, 0
	subw	a2, a5, a2
	sltu	a2, zero, a2
	or	a5, s5, a2
.fib.bb283:
	and	a2, s5, a2
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
.fib.bb284:
	li	a0, 0
	subw	a0, a2, a0
	sltiu	a0, a0, 1
.fib.bb285:
	and	a5, a5, a0
	li	a0, 0
	subw	a5, a5, a0
	sltu	a5, zero, a5
.fib.bb286:
	or	a0, a5, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb287:
	and	t3, a5, t3
	li	a5, 0
	subw	t3, t3, a5
	sltu	t3, zero, t3
.fib.bb288:
	li	a5, 0
	subw	a5, t3, a5
	sltiu	a5, a5, 1
.fib.bb289:
	and	a5, a0, a5
	li	a0, 0
	subw	a5, a5, a0
	sltu	t5, zero, a5
	sd	t5, 288(sp)
	j	.fib.bb292
.fib.bb290:
	j	.fib.bb292
.fib.bb291:
.fib.bb292:
	or	t3, a2, t3
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb293:
	li	a2, 0
	ld	t5, 112(sp)
	subw	a2, t5, a2
	sltu	s6, zero, a2
	li	a2, 0
	subw	a2, a6, a2
	sltu	a2, zero, a2
	or	a6, s6, a2
.fib.bb294:
	and	a2, s6, a2
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
.fib.bb295:
	li	a0, 0
	subw	a0, a2, a0
	sltiu	a0, a0, 1
.fib.bb296:
	and	a6, a6, a0
	li	a0, 0
	subw	a6, a6, a0
	sltu	a6, zero, a6
.fib.bb297:
	or	a0, a6, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb298:
	and	t3, a6, t3
	li	a6, 0
	subw	t3, t3, a6
	sltu	t3, zero, t3
.fib.bb299:
	li	a6, 0
	subw	a6, t3, a6
	sltiu	a6, a6, 1
.fib.bb300:
	and	a6, a0, a6
	li	a0, 0
	subw	a6, a6, a0
	sltu	t5, zero, a6
	sd	t5, 304(sp)
	j	.fib.bb303
.fib.bb301:
	j	.fib.bb303
.fib.bb302:
.fib.bb303:
	or	t3, a2, t3
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb304:
	li	a2, 0
	ld	t5, 120(sp)
	subw	a2, t5, a2
	sltu	s7, zero, a2
	li	a2, 0
	subw	a2, a7, a2
	sltu	a2, zero, a2
	or	a7, s7, a2
.fib.bb305:
	and	a2, s7, a2
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
.fib.bb306:
	li	a0, 0
	subw	a0, a2, a0
	sltiu	a0, a0, 1
.fib.bb307:
	and	a7, a7, a0
	li	a0, 0
	subw	a7, a7, a0
	sltu	a7, zero, a7
.fib.bb308:
	or	a0, a7, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb309:
	and	t3, a7, t3
	li	a7, 0
	subw	t3, t3, a7
	sltu	t3, zero, t3
.fib.bb310:
	li	a7, 0
	subw	a7, t3, a7
	sltiu	a7, a7, 1
.fib.bb311:
	and	a7, a0, a7
	li	a0, 0
	subw	a7, a7, a0
	sltu	t5, zero, a7
	sd	t5, 320(sp)
	j	.fib.bb314
.fib.bb312:
	j	.fib.bb314
.fib.bb313:
.fib.bb314:
	or	t3, a2, t3
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb315:
	li	a2, 0
	ld	t5, 128(sp)
	subw	a2, t5, a2
	sltu	s8, zero, a2
	li	a2, 0
	subw	t0, t0, a2
	sltu	t0, zero, t0
	or	a2, s8, t0
.fib.bb316:
	and	t0, s8, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb317:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb318:
	and	a2, a2, a0
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
.fib.bb319:
	or	a0, a2, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb320:
	and	t3, a2, t3
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb321:
	li	a2, 0
	subw	a2, t3, a2
	sltiu	a2, a2, 1
.fib.bb322:
	and	a2, a0, a2
	li	a0, 0
	subw	a2, a2, a0
	sltu	a2, zero, a2
	j	.fib.bb325
.fib.bb323:
	j	.fib.bb325
.fib.bb324:
.fib.bb325:
	or	t0, t0, t3
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb326:
	li	t3, 0
	ld	t5, 136(sp)
	subw	t3, t5, t3
	sltu	t5, zero, t3
	sd	t5, 408(sp)
	li	t3, 0
	ld	t5, 200(sp)
	subw	t3, t5, t3
	sltu	t3, zero, t3
	ld	t5, 408(sp)
	or	a0, t5, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb327:
	ld	t5, 408(sp)
	and	t3, t5, t3
	li	a1, 0
	subw	t3, t3, a1
	sltu	t3, zero, t3
.fib.bb328:
	li	a1, 0
	subw	a1, t3, a1
	sltiu	a1, a1, 1
.fib.bb329:
	and	a0, a0, a1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb330:
	or	a1, a0, t0
	li	t4, 0
	subw	t4, a1, t4
	sltu	t4, zero, t4
.fib.bb331:
	and	t0, a0, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb332:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb333:
	and	t4, t4, a0
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
	j	.fib.bb336
.fib.bb334:
	j	.fib.bb336
.fib.bb335:
.fib.bb336:
	or	t0, t3, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb337:
	li	t3, 0
	ld	t5, 144(sp)
	subw	t3, t5, t3
	sltu	t5, zero, t3
	sd	t5, 400(sp)
	li	t3, 0
	ld	t5, 208(sp)
	subw	t3, t5, t3
	sltu	t3, zero, t3
	ld	t5, 400(sp)
	or	a0, t5, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb338:
	ld	t5, 400(sp)
	and	t3, t5, t3
	li	a1, 0
	subw	t3, t3, a1
	sltu	t3, zero, t3
.fib.bb339:
	li	a1, 0
	subw	a1, t3, a1
	sltiu	a1, a1, 1
.fib.bb340:
	and	a0, a0, a1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb341:
	or	a1, a0, t0
	li	a3, 0
	subw	a3, a1, a3
	sltu	a3, zero, a3
.fib.bb342:
	and	t0, a0, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb343:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb344:
	and	a3, a3, a0
	li	a0, 0
	subw	a3, a3, a0
	sltu	a3, zero, a3
	j	.fib.bb347
.fib.bb345:
	j	.fib.bb347
.fib.bb346:
.fib.bb347:
	or	t0, t3, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb348:
	li	t3, 0
	ld	t5, 152(sp)
	subw	t3, t5, t3
	sltu	t5, zero, t3
	sd	t5, 280(sp)
	li	t3, 0
	ld	t5, 216(sp)
	subw	t3, t5, t3
	sltu	t3, zero, t3
	ld	t5, 280(sp)
	or	a0, t5, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb349:
	ld	t5, 280(sp)
	and	t3, t5, t3
	li	a1, 0
	subw	t3, t3, a1
	sltu	t3, zero, t3
.fib.bb350:
	li	a1, 0
	subw	a1, t3, a1
	sltiu	a1, a1, 1
.fib.bb351:
	and	a0, a0, a1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb352:
	or	a1, a0, t0
	li	a4, 0
	subw	a4, a1, a4
	sltu	a4, zero, a4
.fib.bb353:
	and	t0, a0, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb354:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb355:
	and	a4, a4, a0
	li	a0, 0
	subw	a4, a4, a0
	sltu	a4, zero, a4
	j	.fib.bb358
.fib.bb356:
	j	.fib.bb358
.fib.bb357:
.fib.bb358:
	or	t0, t3, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb359:
	li	t3, 0
	ld	t5, 160(sp)
	subw	t3, t5, t3
	sltu	t5, zero, t3
	sd	t5, 296(sp)
	li	t3, 0
	ld	t5, 224(sp)
	subw	t3, t5, t3
	sltu	t3, zero, t3
	ld	t5, 296(sp)
	or	a0, t5, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb360:
	ld	t5, 296(sp)
	and	t3, t5, t3
	li	a1, 0
	subw	t3, t3, a1
	sltu	t3, zero, t3
.fib.bb361:
	li	a1, 0
	subw	a1, t3, a1
	sltiu	a1, a1, 1
.fib.bb362:
	and	a0, a0, a1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb363:
	or	a1, a0, t0
	li	a5, 0
	subw	a5, a1, a5
	sltu	a5, zero, a5
.fib.bb364:
	and	t0, a0, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb365:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb366:
	and	a5, a5, a0
	li	a0, 0
	subw	a5, a5, a0
	sltu	a5, zero, a5
	j	.fib.bb369
.fib.bb367:
	j	.fib.bb369
.fib.bb368:
.fib.bb369:
	or	t0, t3, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb370:
	li	t3, 0
	ld	t5, 168(sp)
	subw	t3, t5, t3
	sltu	t5, zero, t3
	sd	t5, 312(sp)
	li	t3, 0
	ld	t5, 232(sp)
	subw	t3, t5, t3
	sltu	t3, zero, t3
	ld	t5, 312(sp)
	or	a0, t5, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb371:
	ld	t5, 312(sp)
	and	t3, t5, t3
	li	a1, 0
	subw	t3, t3, a1
	sltu	t3, zero, t3
.fib.bb372:
	li	a1, 0
	subw	a1, t3, a1
	sltiu	a1, a1, 1
.fib.bb373:
	and	a0, a0, a1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb374:
	or	a1, a0, t0
	li	a6, 0
	subw	a6, a1, a6
	sltu	a6, zero, a6
.fib.bb375:
	and	t0, a0, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb376:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb377:
	and	a6, a6, a0
	li	a0, 0
	subw	a6, a6, a0
	sltu	a6, zero, a6
	j	.fib.bb380
.fib.bb378:
	j	.fib.bb380
.fib.bb379:
.fib.bb380:
	or	t0, t3, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb381:
	li	t3, 0
	ld	t5, 176(sp)
	subw	t3, t5, t3
	sltu	t5, zero, t3
	sd	t5, 328(sp)
	li	t3, 0
	ld	t5, 240(sp)
	subw	t3, t5, t3
	sltu	t3, zero, t3
	ld	t5, 328(sp)
	or	a0, t5, t3
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb382:
	ld	t5, 328(sp)
	and	t3, t5, t3
	li	a1, 0
	subw	t3, t3, a1
	sltu	t3, zero, t3
.fib.bb383:
	li	a1, 0
	subw	a1, t3, a1
	sltiu	a1, a1, 1
.fib.bb384:
	and	a0, a0, a1
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb385:
	or	a1, a0, t0
	li	a7, 0
	subw	a7, a1, a7
	sltu	a7, zero, a7
.fib.bb386:
	and	t0, a0, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb387:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb388:
	and	a7, a7, a0
	li	a0, 0
	subw	a7, a7, a0
	sltu	a7, zero, a7
	j	.fib.bb391
.fib.bb389:
	j	.fib.bb391
.fib.bb390:
.fib.bb391:
	or	t0, t3, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb392:
	li	t3, 0
	ld	t5, 192(sp)
	subw	t3, t5, t3
	sltu	t5, zero, t3
	sd	t5, 336(sp)
	li	t3, 0
	subw	t1, t1, t3
	sltu	t1, zero, t1
	or	t3, t5, t1
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb393:
	ld	t5, 336(sp)
	and	t1, t5, t1
	li	a0, 0
	subw	t1, t1, a0
	sltu	t1, zero, t1
.fib.bb394:
	li	a0, 0
	subw	a0, t1, a0
	sltiu	a0, a0, 1
.fib.bb395:
	and	t3, t3, a0
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
.fib.bb396:
	or	a0, t3, t0
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb397:
	and	t0, t3, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb398:
	li	t3, 0
	subw	t3, t0, t3
	sltiu	t3, t3, 1
.fib.bb399:
	and	t3, a0, t3
	li	a0, 0
	subw	t3, t3, a0
	sltu	t3, zero, t3
	j	.fib.bb402
.fib.bb400:
	j	.fib.bb402
.fib.bb401:
.fib.bb402:
	or	t0, t1, t0
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
.fib.bb403:
	li	t1, 0
	ld	t5, 184(sp)
	subw	t1, t5, t1
	sltu	t5, zero, t1
	sd	t5, 392(sp)
	li	t1, 0
	subw	t1, t2, t1
	sltu	t1, zero, t1
	or	t2, t5, t1
	li	a0, 0
	subw	t2, t2, a0
	sltu	t2, zero, t2
.fib.bb404:
	ld	t5, 392(sp)
	and	t1, t5, t1
	li	a0, 0
	subw	t1, t1, a0
	sltu	t1, zero, t1
.fib.bb405:
	li	a0, 0
	subw	a0, t1, a0
	sltiu	a0, a0, 1
.fib.bb406:
	and	t2, t2, a0
	li	a0, 0
	subw	t2, t2, a0
	sltu	t2, zero, t2
.fib.bb407:
	or	a0, t2, t0
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb408:
	and	t0, t2, t0
	li	t2, 0
	subw	t0, t0, t2
	sltu	t0, zero, t0
.fib.bb409:
	li	t2, 0
	subw	t2, t0, t2
	sltiu	t2, t2, 1
.fib.bb410:
	and	t2, a0, t2
	li	a0, 0
	subw	t2, t2, a0
	sltu	t2, zero, t2
	j	.fib.bb413
.fib.bb411:
	j	.fib.bb413
.fib.bb412:
.fib.bb413:
	or	t0, t1, t0
	bne	t0, zero, .fib.bb416
.fib.bb415:
.fib.bb416:
	slliw	t0, t2, 1
	addw	t0, t0, t3
	slliw	t0, t0, 1
	addw	t0, t0, a7
	slliw	t0, t0, 1
	addw	t0, t0, a6
	slliw	t0, t0, 1
	addw	t0, t0, a5
	slliw	t0, t0, 1
	addw	t0, t0, a4
	slliw	t0, t0, 1
	addw	t0, t0, a3
	slliw	t0, t0, 1
	addw	t0, t0, t4
	slliw	t0, t0, 1
	addw	t0, t0, a2
	slliw	t0, t0, 1
	ld	t5, 320(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 304(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 288(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 272(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 264(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 256(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 248(sp)
	addw	a0, t0, t5
	call	fib
	mv	t5, a0
	sd	t5, 344(sp)
	j	.fib.bb432
.fib.bb417:
	j	.fib.bb432
.fib.bb418:
	j	.fib.bb432
.fib.bb419:
	j	.fib.bb432
.fib.bb420:
	j	.fib.bb432
.fib.bb421:
	j	.fib.bb432
.fib.bb422:
	j	.fib.bb432
.fib.bb423:
	j	.fib.bb432
.fib.bb424:
	j	.fib.bb432
.fib.bb425:
	j	.fib.bb432
.fib.bb426:
	j	.fib.bb432
.fib.bb427:
	j	.fib.bb432
.fib.bb428:
	j	.fib.bb432
.fib.bb429:
	j	.fib.bb432
.fib.bb430:
	j	.fib.bb432
.fib.bb431:
.fib.bb432:
	li	t0, 1
	li	t2, 0
	ori	t1, t0, 1
	subw	t1, t1, t2
	sltu	t1, zero, t1
.fib.bb433:
	andi	t0, t0, 1
.fib.bb434:
	li	t2, 0
	subw	t2, t0, t2
	sltiu	t2, t2, 1
.fib.bb435:
	and	t1, t1, t2
	li	t2, 0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	j	.fib.bb437
.fib.bb436:
.fib.bb437:
	andi	t5, t1, 1
	sd	t5, 352(sp)
	j	.fib.bb440
.fib.bb438:
	j	.fib.bb440
.fib.bb439:
.fib.bb440:
	li	t2, 0
.fib.bb441:
	andi	t2, t2, 1
.fib.bb442:
	or	t3, t2, t0
	li	a2, 0
	subw	t3, t3, a2
	sltu	t3, zero, t3
.fib.bb443:
	and	t0, t2, t0
	li	t2, 0
	subw	t0, t0, t2
	sltu	t0, zero, t0
.fib.bb444:
	li	t2, 0
	subw	t2, t0, t2
	sltiu	t2, t2, 1
.fib.bb445:
	and	t2, t3, t2
	li	t3, 0
	subw	t2, t2, t3
	sltu	t5, zero, t2
	sd	t5, 360(sp)
	j	.fib.bb448
.fib.bb446:
	j	.fib.bb448
.fib.bb447:
.fib.bb448:
	li	t3, 1
.fib.bb449:
	andi	t3, t3, 1
.fib.bb450:
	or	a2, t3, t0
	li	a3, 0
	subw	a2, a2, a3
	sltu	a2, zero, a2
.fib.bb451:
	and	t0, t3, t0
	li	t3, 0
	subw	t0, t0, t3
	sltu	t0, zero, t0
.fib.bb452:
	li	t3, 0
	subw	t3, t0, t3
	sltiu	t3, t3, 1
.fib.bb453:
	and	t3, a2, t3
	li	a2, 0
	subw	t3, t3, a2
	sltu	t5, zero, t3
	sd	t5, 368(sp)
	j	.fib.bb456
.fib.bb454:
	j	.fib.bb456
.fib.bb455:
.fib.bb456:
	li	a2, 1
.fib.bb457:
	andi	a2, a2, 1
.fib.bb458:
	or	a3, a2, t0
	li	a4, 0
	subw	a3, a3, a4
	sltu	a3, zero, a3
.fib.bb459:
	and	t0, a2, t0
	li	a2, 0
	subw	t0, t0, a2
	sltu	t0, zero, t0
.fib.bb460:
	li	a2, 0
	subw	a2, t0, a2
	sltiu	a2, a2, 1
.fib.bb461:
	and	a2, a3, a2
	li	a3, 0
	subw	a2, a2, a3
	sltu	t5, zero, a2
	sd	t5, 376(sp)
	j	.fib.bb464
.fib.bb462:
	j	.fib.bb464
.fib.bb463:
.fib.bb464:
	li	a3, 1
.fib.bb465:
	andi	a3, a3, 1
.fib.bb466:
	or	a4, a3, t0
	li	a5, 0
	subw	a4, a4, a5
	sltu	a4, zero, a4
.fib.bb467:
	and	t0, a3, t0
	li	a3, 0
	subw	t0, t0, a3
	sltu	t0, zero, t0
.fib.bb468:
	li	a3, 0
	subw	a3, t0, a3
	sltiu	a3, a3, 1
.fib.bb469:
	and	a3, a4, a3
	li	a4, 0
	subw	a3, a3, a4
	sltu	t5, zero, a3
	sd	t5, 384(sp)
	j	.fib.bb472
.fib.bb470:
	j	.fib.bb472
.fib.bb471:
.fib.bb472:
	li	a4, 1
.fib.bb473:
	andi	a4, a4, 1
.fib.bb474:
	or	a5, a4, t0
	li	a6, 0
	subw	a5, a5, a6
	sltu	a5, zero, a5
.fib.bb475:
	and	t0, a4, t0
	li	a4, 0
	subw	t0, t0, a4
	sltu	t0, zero, t0
.fib.bb476:
	li	a4, 0
	subw	a4, t0, a4
	sltiu	a4, a4, 1
.fib.bb477:
	and	a4, a5, a4
	li	a5, 0
	subw	a4, a4, a5
	sltu	a4, zero, a4
	j	.fib.bb480
.fib.bb478:
	j	.fib.bb480
.fib.bb479:
.fib.bb480:
	li	a5, 1
.fib.bb481:
	andi	a5, a5, 1
.fib.bb482:
	or	a6, a5, t0
	li	a7, 0
	subw	a6, a6, a7
	sltu	a6, zero, a6
.fib.bb483:
	and	t0, a5, t0
	li	a5, 0
	subw	t0, t0, a5
	sltu	t0, zero, t0
.fib.bb484:
	li	a5, 0
	subw	a5, t0, a5
	sltiu	a5, a5, 1
.fib.bb485:
	and	a5, a6, a5
	li	a6, 0
	subw	a5, a5, a6
	sltu	a5, zero, a5
	j	.fib.bb488
.fib.bb486:
	j	.fib.bb488
.fib.bb487:
.fib.bb488:
	li	a6, 1
.fib.bb489:
	andi	a6, a6, 1
.fib.bb490:
	or	a7, a6, t0
	li	t4, 0
	subw	a7, a7, t4
	sltu	a7, zero, a7
.fib.bb491:
	and	t0, a6, t0
	li	a6, 0
	subw	t0, t0, a6
	sltu	t0, zero, t0
.fib.bb492:
	li	a6, 0
	subw	a6, t0, a6
	sltiu	a6, a6, 1
.fib.bb493:
	and	a6, a7, a6
	li	a7, 0
	subw	a6, a6, a7
	sltu	a6, zero, a6
	j	.fib.bb496
.fib.bb494:
	j	.fib.bb496
.fib.bb495:
.fib.bb496:
	li	a7, 1
.fib.bb497:
	andi	a7, a7, 1
.fib.bb498:
	or	t4, a7, t0
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
.fib.bb499:
	and	t0, a7, t0
	li	a7, 0
	subw	t0, t0, a7
	sltu	t0, zero, t0
.fib.bb500:
	li	a7, 0
	subw	a7, t0, a7
	sltiu	a7, a7, 1
.fib.bb501:
	and	a7, t4, a7
	li	t4, 0
	subw	a7, a7, t4
	sltu	a7, zero, a7
	j	.fib.bb504
.fib.bb502:
	j	.fib.bb504
.fib.bb503:
.fib.bb504:
	li	t4, 1
.fib.bb505:
	andi	t4, t4, 1
.fib.bb506:
	or	a0, t4, t0
	li	a1, 0
	subw	a0, a0, a1
	sltu	a0, zero, a0
.fib.bb507:
	and	t0, t4, t0
	li	t4, 0
	subw	t0, t0, t4
	sltu	t0, zero, t0
.fib.bb508:
	li	t4, 0
	subw	t4, t0, t4
	sltiu	t4, t4, 1
.fib.bb509:
	and	t4, a0, t4
	li	a0, 0
	subw	t4, t4, a0
	sltu	t4, zero, t4
	j	.fib.bb512
.fib.bb510:
	j	.fib.bb512
.fib.bb511:
.fib.bb512:
	li	a0, 1
.fib.bb513:
	andi	a0, a0, 1
.fib.bb514:
	or	a1, a0, t0
	li	t1, 0
	subw	t1, a1, t1
	sltu	t1, zero, t1
.fib.bb515:
	and	t0, a0, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb516:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb517:
	and	t1, t1, a0
	li	a0, 0
	subw	t1, t1, a0
	sltu	t1, zero, t1
	j	.fib.bb520
.fib.bb518:
	j	.fib.bb520
.fib.bb519:
.fib.bb520:
	li	a0, 1
.fib.bb521:
	andi	a0, a0, 1
.fib.bb522:
	or	a1, a0, t0
	li	t2, 0
	subw	t2, a1, t2
	sltu	t2, zero, t2
.fib.bb523:
	and	t0, a0, t0
	li	a0, 0
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb524:
	li	a0, 0
	subw	a0, t0, a0
	sltiu	a0, a0, 1
.fib.bb525:
	and	t2, t2, a0
	li	a0, 0
	subw	t2, t2, a0
	sltu	t2, zero, t2
	j	.fib.bb528
.fib.bb526:
	j	.fib.bb528
.fib.bb527:
.fib.bb528:
	li	a0, 1
	bne	a0, zero, .fib.bb529
.fib.bb530:
	li	a0, 0
	addi	a1, sp, 468
	sw	a0, 0(a1)
.fib.bb531:
	addi	a0, sp, 468
	li	a1, 0
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	bne	a0, zero, .fib.bb532
.fib.bb533:
	li	a0, 0
	addi	a1, sp, 464
	sw	a0, 0(a1)
.fib.bb534:
	addi	a0, sp, 464
	li	a1, 0
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	or	a1, a0, t0
	bne	a1, zero, .fib.bb535
.fib.bb536:
	li	a1, 0
	addi	t3, sp, 472
	sw	a1, 0(t3)
.fib.bb537:
	and	t0, a0, t0
	bne	t0, zero, .fib.bb538
.fib.bb539:
	li	t3, 0
	addi	a0, sp, 476
	sw	t3, 0(a0)
.fib.bb540:
	addi	t3, sp, 476
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb541
.fib.bb542:
	li	t3, 0
	addi	a0, sp, 480
	sw	t3, 0(a0)
.fib.bb543:
	addi	t3, sp, 472
	li	a0, 0
	lw	t3, 0(t3)
	li	a1, 0
	subw	t3, t3, a0
	addi	a0, sp, 480
	sltu	t3, zero, t3
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	and	t3, t3, a0
.fib.bb544:
	bne	t0, zero, .fib.bb545
.fib.bb546:
	li	t0, 0
	addi	a0, sp, 484
	sw	t0, 0(a0)
.fib.bb547:
	addi	t0, sp, 484
	li	a0, 0
	lw	t0, 0(t0)
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb548:
	li	a0, 1
	bne	a0, zero, .fib.bb549
.fib.bb550:
	li	a0, 0
	addi	a1, sp, 492
	sw	a0, 0(a1)
.fib.bb551:
	addi	a0, sp, 492
	li	a1, 0
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	bne	a0, zero, .fib.bb552
.fib.bb553:
	li	a0, 0
	addi	a1, sp, 488
	sw	a0, 0(a1)
.fib.bb554:
	addi	a0, sp, 488
	li	a1, 0
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	or	a1, a0, t0
	bne	a1, zero, .fib.bb555
.fib.bb556:
	li	a1, 0
	addi	a2, sp, 496
	sw	a1, 0(a2)
.fib.bb557:
	and	t0, a0, t0
	bne	t0, zero, .fib.bb558
.fib.bb559:
	li	a2, 0
	addi	a0, sp, 500
	sw	a2, 0(a0)
.fib.bb560:
	addi	a2, sp, 500
	lw	a2, 0(a2)
	beq	a2, zero, .fib.bb561
.fib.bb562:
	li	a2, 0
	addi	a0, sp, 504
	sw	a2, 0(a0)
.fib.bb563:
	addi	a2, sp, 496
	li	a0, 0
	lw	a2, 0(a2)
	li	a1, 0
	subw	a2, a2, a0
	addi	a0, sp, 504
	sltu	a2, zero, a2
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	and	a2, a2, a0
.fib.bb564:
	bne	t0, zero, .fib.bb565
.fib.bb566:
	li	t0, 0
	addi	a0, sp, 508
	sw	t0, 0(a0)
.fib.bb567:
	addi	t0, sp, 508
	li	a0, 0
	lw	t0, 0(t0)
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb568:
	li	a0, 1
	bne	a0, zero, .fib.bb569
.fib.bb570:
	li	a0, 0
	addi	a1, sp, 516
	sw	a0, 0(a1)
.fib.bb571:
	addi	a0, sp, 516
	li	a1, 0
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	bne	a0, zero, .fib.bb572
.fib.bb573:
	li	a0, 0
	addi	a1, sp, 512
	sw	a0, 0(a1)
.fib.bb574:
	addi	a0, sp, 512
	li	a1, 0
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	or	a1, a0, t0
	bne	a1, zero, .fib.bb575
.fib.bb576:
	li	a1, 0
	addi	a3, sp, 520
	sw	a1, 0(a3)
.fib.bb577:
	and	t0, a0, t0
	bne	t0, zero, .fib.bb578
.fib.bb579:
	li	a3, 0
	addi	a0, sp, 524
	sw	a3, 0(a0)
.fib.bb580:
	addi	a3, sp, 524
	lw	a3, 0(a3)
	beq	a3, zero, .fib.bb581
.fib.bb582:
	li	a3, 0
	addi	a0, sp, 528
	sw	a3, 0(a0)
.fib.bb583:
	addi	a3, sp, 520
	li	a0, 0
	lw	a3, 0(a3)
	li	a1, 0
	subw	a3, a3, a0
	addi	a0, sp, 528
	sltu	a3, zero, a3
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	and	a3, a3, a0
.fib.bb584:
	bne	t0, zero, .fib.bb585
.fib.bb586:
	li	t0, 0
	addi	a0, sp, 532
	sw	t0, 0(a0)
.fib.bb587:
	addi	t0, sp, 532
	li	a0, 0
	lw	t0, 0(t0)
	subw	t0, t0, a0
	sltu	t0, zero, t0
.fib.bb588:
	li	a0, 1
	bne	a0, zero, .fib.bb589
.fib.bb590:
	li	a0, 0
	addi	a1, sp, 540
	sw	a0, 0(a1)
.fib.bb591:
	addi	a0, sp, 540
	li	a1, 0
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	bne	a0, zero, .fib.bb592
.fib.bb593:
	li	a0, 0
	addi	a1, sp, 536
	sw	a0, 0(a1)
.fib.bb594:
	addi	a0, sp, 536
	li	a1, 0
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	or	a1, a0, t0
	bne	a1, zero, .fib.bb595
.fib.bb596:
	li	a1, 0
	addi	s11, sp, 544
	sw	a1, 0(s11)
.fib.bb597:
	and	t0, a0, t0
	bne	t0, zero, .fib.bb598
.fib.bb599:
	li	a0, 0
	addi	a1, sp, 548
	sw	a0, 0(a1)
.fib.bb600:
	addi	a0, sp, 548
	lw	a0, 0(a0)
	beq	a0, zero, .fib.bb601
.fib.bb602:
	li	a0, 0
	addi	a1, sp, 552
	sw	a0, 0(a1)
.fib.bb603:
	addi	a0, sp, 544
	li	a1, 0
	lw	a0, 0(a0)
	li	s9, 0
	subw	a0, a0, a1
	addi	a1, sp, 552
	sltu	a0, zero, a0
	lw	a1, 0(a1)
	subw	a1, a1, s9
	sltu	a1, zero, a1
	and	s9, a0, a1
.fib.bb604:
	bne	t0, zero, .fib.bb605
.fib.bb606:
	li	t0, 0
	addi	a0, sp, 556
	sw	t0, 0(a0)
.fib.bb607:
	addi	t0, sp, 556
	lw	t0, 0(t0)
	bne	t0, zero, .fib.bb626
.fib.bb609:
	j	.fib.bb626
.fib.bb610:
	j	.fib.bb626
.fib.bb611:
	j	.fib.bb626
.fib.bb612:
	j	.fib.bb626
.fib.bb613:
	j	.fib.bb626
.fib.bb614:
	j	.fib.bb626
.fib.bb615:
	j	.fib.bb626
.fib.bb616:
	j	.fib.bb626
.fib.bb617:
	j	.fib.bb626
.fib.bb618:
	j	.fib.bb626
.fib.bb619:
	j	.fib.bb626
.fib.bb620:
	j	.fib.bb626
.fib.bb621:
	j	.fib.bb626
.fib.bb622:
	j	.fib.bb626
.fib.bb623:
	j	.fib.bb626
.fib.bb624:
	j	.fib.bb626
.fib.bb625:
.fib.bb626:
	slliw	t0, s9, 1
	addw	t0, t0, a3
	slliw	t0, t0, 1
	addw	t0, t0, a2
	slliw	t0, t0, 1
	addw	t0, t0, t3
	slliw	t0, t0, 1
	addw	t0, t0, t2
	slliw	t0, t0, 1
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addw	t0, t0, t4
	slliw	t0, t0, 1
	addw	t0, t0, a7
	slliw	t0, t0, 1
	addw	t0, t0, a6
	slliw	t0, t0, 1
	addw	t0, t0, a5
	slliw	t0, t0, 1
	addw	t0, t0, a4
	slliw	t0, t0, 1
	ld	t5, 384(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 376(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 368(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 360(sp)
	addw	t0, t0, t5
	slliw	t0, t0, 1
	ld	t5, 352(sp)
	addw	t0, t0, t5
	srliw	t1, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t1
	andi	t2, t0, 1
	subw	t1, t2, t1
	li	t2, 0
	slt	t2, t1, t2
	subw	t3, zero, t1
	li	a2, 0
	subw	t2, a2, t2
	xor	t3, t1, t3
	and	t2, t3, t2
	xor	t1, t1, t2
.fib.bb627:
	sraiw	t0, t0, 1
	li	a3, 0
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	t3, t0, 1
	subw	t2, t3, t2
	li	t3, 0
	subw	a2, zero, t2
	slt	t3, t2, t3
	xor	a2, t2, a2
	subw	t3, a3, t3
	and	t3, a2, t3
	xor	t2, t2, t3
.fib.bb628:
	sraiw	t0, t0, 1
	li	a4, 0
	srliw	t3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t3
	andi	a2, t0, 1
	subw	t3, a2, t3
	li	a2, 0
	subw	a3, zero, t3
	slt	a2, t3, a2
	xor	a3, t3, a3
	subw	a2, a4, a2
	and	a2, a3, a2
	xor	t3, t3, a2
.fib.bb629:
	sraiw	t0, t0, 1
	li	a5, 0
	srliw	a2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, a2
	andi	a3, t0, 1
	subw	a2, a3, a2
	li	a3, 0
	subw	a4, zero, a2
	slt	a3, a2, a3
	xor	a4, a2, a4
	subw	a3, a5, a3
	and	a3, a4, a3
	xor	a2, a2, a3
.fib.bb630:
	sraiw	t0, t0, 1
	li	a6, 0
	srliw	a3, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, a3
	andi	a4, t0, 1
	subw	a3, a4, a3
	li	a4, 0
	subw	a5, zero, a3
	slt	a4, a3, a4
	xor	a5, a3, a5
	subw	a4, a6, a4
	and	a4, a5, a4
	xor	a3, a3, a4
.fib.bb631:
	sraiw	t0, t0, 1
	li	a7, 0
	srliw	a4, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, a4
	andi	a5, t0, 1
	subw	a4, a5, a4
	li	a5, 0
	subw	a6, zero, a4
	slt	a5, a4, a5
	xor	a6, a4, a6
	subw	a5, a7, a5
	and	a5, a6, a5
	xor	a4, a4, a5
.fib.bb632:
	sraiw	t0, t0, 1
	li	t4, 0
	srliw	a5, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, a5
	andi	a6, t0, 1
	subw	a5, a6, a5
	li	a6, 0
	subw	a7, zero, a5
	slt	a6, a5, a6
	xor	a7, a5, a7
	subw	a6, t4, a6
	and	a6, a7, a6
	xor	a5, a5, a6
.fib.bb633:
	sraiw	t0, t0, 1
	li	a0, 0
	srliw	a6, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, a6
	andi	a7, t0, 1
	subw	a6, a7, a6
	li	a7, 0
	subw	t4, zero, a6
	slt	a7, a6, a7
	xor	t4, a6, t4
	subw	a7, a0, a7
	and	a7, t4, a7
	xor	a6, a6, a7
.fib.bb634:
	sraiw	t0, t0, 1
	srliw	a7, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, a7
	andi	t4, t0, 1
	subw	a7, t4, a7
	li	t4, 0
	slt	t4, a7, t4
	subw	a0, zero, a7
	li	a1, 0
	subw	t4, a1, t4
	xor	a0, a7, a0
	and	t4, a0, t4
	xor	t5, a7, t4
	sd	t5, 416(sp)
.fib.bb635:
	sraiw	t0, t0, 1
	srliw	t4, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t4
	andi	a0, t0, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	a7, 0
	subw	a7, a7, a0
	xor	a0, t4, a1
	and	a7, a0, a7
	xor	t5, t4, a7
	sd	t5, 424(sp)
.fib.bb636:
	sraiw	t0, t0, 1
	srliw	t4, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t4
	andi	a0, t0, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	a7, 0
	subw	a7, a7, a0
	xor	a0, t4, a1
	and	a7, a0, a7
	xor	t5, t4, a7
	sd	t5, 432(sp)
.fib.bb637:
	sraiw	t0, t0, 1
	srliw	t4, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t4
	andi	a0, t0, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	a7, 0
	subw	a7, a7, a0
	xor	a0, t4, a1
	and	a7, a0, a7
	xor	t5, t4, a7
	sd	t5, 440(sp)
.fib.bb638:
	sraiw	t0, t0, 1
	srliw	t4, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t4
	andi	a0, t0, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	a7, 0
	subw	a7, a7, a0
	xor	a0, t4, a1
	and	a7, a0, a7
	xor	t5, t4, a7
	sd	t5, 448(sp)
.fib.bb639:
	sraiw	t0, t0, 1
	srliw	t4, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t4
	andi	a0, t0, 1
	subw	t4, a0, t4
	li	a0, 0
	slt	a0, t4, a0
	subw	a1, zero, t4
	li	a7, 0
	subw	a7, a7, a0
	xor	a0, t4, a1
	and	a7, a0, a7
	xor	t5, t4, a7
	sd	t5, 456(sp)
.fib.bb640:
	sraiw	t0, t0, 1
	li	a7, 0
	srliw	t4, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t4
	andi	a0, t0, 1
	subw	t4, a0, t4
	li	a0, 0
	subw	a1, zero, t4
	slt	a0, t4, a0
	subw	a7, a7, a0
	xor	a0, t4, a1
	and	a7, a0, a7
	xor	a7, t4, a7
.fib.bb641:
	sraiw	t0, t0, 1
	li	a1, 0
	srliw	t4, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t4
	andi	t0, t0, 1
	subw	t0, t0, t4
	li	t4, 0
	subw	a0, zero, t0
	slt	t4, t0, t4
	xor	a0, t0, a0
	subw	t4, a1, t4
	and	t4, a0, t4
	xor	t0, t0, t4
.fib.bb642:
	li	t4, 0
	subw	t1, t1, t4
	sltu	t1, zero, t1
	or	t4, s1, t1
	bne	t4, zero, .fib.bb643
.fib.bb644:
	li	t4, 0
	addi	a0, sp, 688
	sw	t4, 0(a0)
.fib.bb645:
	and	t1, s1, t1
	bne	t1, zero, .fib.bb646
.fib.bb647:
	li	t4, 0
	addi	a0, sp, 692
	sw	t4, 0(a0)
.fib.bb648:
	addi	t4, sp, 692
	lw	t4, 0(t4)
	beq	t4, zero, .fib.bb649
.fib.bb650:
	li	t4, 0
	addi	a0, sp, 696
	sw	t4, 0(a0)
.fib.bb651:
	addi	t4, sp, 688
	li	a0, 0
	lw	t4, 0(t4)
	li	a1, 0
	subw	t4, t4, a0
	addi	a0, sp, 696
	sltu	t4, zero, t4
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	and	t4, t4, a0
	bne	t4, zero, .fib.bb652
.fib.bb653:
	li	t4, 0
	addi	a0, sp, 684
	sw	t4, 0(a0)
.fib.bb654:
	addi	t4, sp, 684
	lw	t4, 0(t4)
	bne	t4, zero, .fib.bb655
.fib.bb656:
	li	t4, 0
	addi	a0, sp, 700
	sw	t4, 0(a0)
.fib.bb657:
	addi	t4, sp, 700
	li	a0, 0
	lw	t4, 0(t4)
	subw	t4, t4, a0
	sltu	t4, zero, t4
	bne	t4, zero, .fib.bb658
.fib.bb659:
	li	t4, 0
	addi	a0, sp, 620
	sw	t4, 0(a0)
.fib.bb660:
	bne	t1, zero, .fib.bb661
.fib.bb662:
	li	t1, 0
	addi	t4, sp, 704
	sw	t1, 0(t4)
.fib.bb663:
	addi	t1, sp, 704
	lw	t1, 0(t1)
	bne	t1, zero, .fib.bb664
.fib.bb665:
	li	t1, 0
	addi	t4, sp, 560
	sw	t1, 0(t4)
.fib.bb666:
	li	t1, 0
	subw	t1, t2, t1
	sltu	t1, zero, t1
	or	t2, s2, t1
	bne	t2, zero, .fib.bb667
.fib.bb668:
	li	t2, 0
	addi	t4, sp, 712
	sw	t2, 0(t4)
.fib.bb669:
	and	t1, s2, t1
	bne	t1, zero, .fib.bb670
.fib.bb671:
	li	t2, 0
	addi	t4, sp, 716
	sw	t2, 0(t4)
.fib.bb672:
	addi	t2, sp, 716
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb673
.fib.bb674:
	li	t2, 0
	addi	t4, sp, 720
	sw	t2, 0(t4)
.fib.bb675:
	addi	t2, sp, 712
	li	t4, 0
	lw	t2, 0(t2)
	li	a0, 0
	subw	t2, t2, t4
	addi	t4, sp, 720
	sltu	t2, zero, t2
	lw	t4, 0(t4)
	subw	t4, t4, a0
	sltu	t4, zero, t4
	and	t2, t2, t4
	bne	t2, zero, .fib.bb676
.fib.bb677:
	li	t2, 0
	addi	t4, sp, 708
	sw	t2, 0(t4)
.fib.bb678:
	addi	t2, sp, 708
	li	t4, 0
	lw	t2, 0(t2)
	li	a0, 0
	subw	t2, t2, t4
	addi	t4, sp, 560
	sltu	t2, zero, t2
	lw	t4, 0(t4)
	subw	t4, t4, a0
	sltu	t4, zero, t4
	or	a0, t2, t4
	bne	a0, zero, .fib.bb679
.fib.bb680:
	li	a0, 0
	addi	a1, sp, 724
	sw	a0, 0(a1)
.fib.bb681:
	and	t2, t2, t4
	bne	t2, zero, .fib.bb682
.fib.bb683:
	li	t4, 0
	addi	a0, sp, 728
	sw	t4, 0(a0)
.fib.bb684:
	addi	t4, sp, 728
	lw	t4, 0(t4)
	beq	t4, zero, .fib.bb685
.fib.bb686:
	li	t4, 0
	addi	a0, sp, 732
	sw	t4, 0(a0)
.fib.bb687:
	addi	t4, sp, 724
	li	a0, 0
	lw	t4, 0(t4)
	li	a1, 0
	subw	t4, t4, a0
	addi	a0, sp, 732
	sltu	t4, zero, t4
	lw	a0, 0(a0)
	subw	a0, a0, a1
	sltu	a0, zero, a0
	and	t4, t4, a0
	bne	t4, zero, .fib.bb688
.fib.bb689:
	li	t4, 0
	addi	a0, sp, 624
	sw	t4, 0(a0)
.fib.bb690:
	bne	t1, zero, .fib.bb691
.fib.bb692:
	li	t1, 0
	addi	t4, sp, 736
	sw	t1, 0(t4)
.fib.bb693:
	bne	t2, zero, .fib.bb694
.fib.bb695:
	li	t1, 0
	addi	t2, sp, 740
	sw	t1, 0(t2)
.fib.bb696:
	addi	t1, sp, 736
	li	t2, 0
	lw	t1, 0(t1)
	li	t4, 0
	subw	t1, t1, t2
	addi	t2, sp, 740
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t4
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb697
.fib.bb698:
	li	t1, 0
	addi	t2, sp, 564
	sw	t1, 0(t2)
.fib.bb699:
	li	t1, 0
	subw	t1, t3, t1
	sltu	t1, zero, t1
	or	t2, s3, t1
	bne	t2, zero, .fib.bb700
.fib.bb701:
	li	t2, 0
	addi	t3, sp, 748
	sw	t2, 0(t3)
.fib.bb702:
	and	t1, s3, t1
	bne	t1, zero, .fib.bb703
.fib.bb704:
	li	t2, 0
	addi	t3, sp, 752
	sw	t2, 0(t3)
.fib.bb705:
	addi	t2, sp, 752
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb706
.fib.bb707:
	li	t2, 0
	addi	t3, sp, 756
	sw	t2, 0(t3)
.fib.bb708:
	addi	t2, sp, 748
	li	t3, 0
	lw	t2, 0(t2)
	li	t4, 0
	subw	t2, t2, t3
	addi	t3, sp, 756
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, t4
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb709
.fib.bb710:
	li	t2, 0
	addi	t3, sp, 744
	sw	t2, 0(t3)
.fib.bb711:
	addi	t2, sp, 744
	li	t3, 0
	lw	t2, 0(t2)
	li	t4, 0
	subw	t2, t2, t3
	addi	t3, sp, 564
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, t4
	sltu	t3, zero, t3
	or	t4, t2, t3
	bne	t4, zero, .fib.bb712
.fib.bb713:
	li	t4, 0
	addi	a0, sp, 760
	sw	t4, 0(a0)
.fib.bb714:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb715
.fib.bb716:
	li	t3, 0
	addi	t4, sp, 764
	sw	t3, 0(t4)
.fib.bb717:
	addi	t3, sp, 764
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb718
.fib.bb719:
	li	t3, 0
	addi	t4, sp, 768
	sw	t3, 0(t4)
.fib.bb720:
	addi	t3, sp, 760
	li	t4, 0
	lw	t3, 0(t3)
	li	a0, 0
	subw	t3, t3, t4
	addi	t4, sp, 768
	sltu	t3, zero, t3
	lw	t4, 0(t4)
	subw	t4, t4, a0
	sltu	t4, zero, t4
	and	t3, t3, t4
	bne	t3, zero, .fib.bb721
.fib.bb722:
	li	t3, 0
	addi	t4, sp, 628
	sw	t3, 0(t4)
.fib.bb723:
	bne	t1, zero, .fib.bb724
.fib.bb725:
	li	t1, 0
	addi	t3, sp, 772
	sw	t1, 0(t3)
.fib.bb726:
	bne	t2, zero, .fib.bb727
.fib.bb728:
	li	t1, 0
	addi	t2, sp, 776
	sw	t1, 0(t2)
.fib.bb729:
	addi	t1, sp, 772
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 776
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb730
.fib.bb731:
	li	t1, 0
	addi	t2, sp, 568
	sw	t1, 0(t2)
.fib.bb732:
	li	t1, 0
	subw	t1, a2, t1
	sltu	t1, zero, t1
	or	t2, s4, t1
	bne	t2, zero, .fib.bb733
.fib.bb734:
	li	t2, 0
	addi	t3, sp, 784
	sw	t2, 0(t3)
.fib.bb735:
	and	t1, s4, t1
	bne	t1, zero, .fib.bb736
.fib.bb737:
	li	t2, 0
	addi	t3, sp, 788
	sw	t2, 0(t3)
.fib.bb738:
	addi	t2, sp, 788
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb739
.fib.bb740:
	li	t2, 0
	addi	t3, sp, 792
	sw	t2, 0(t3)
.fib.bb741:
	addi	t2, sp, 784
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 792
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb742
.fib.bb743:
	li	t2, 0
	addi	t3, sp, 780
	sw	t2, 0(t3)
.fib.bb744:
	addi	t2, sp, 780
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 568
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb745
.fib.bb746:
	li	a2, 0
	addi	t4, sp, 796
	sw	a2, 0(t4)
.fib.bb747:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb748
.fib.bb749:
	li	t3, 0
	addi	a2, sp, 800
	sw	t3, 0(a2)
.fib.bb750:
	addi	t3, sp, 800
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb751
.fib.bb752:
	li	t3, 0
	addi	a2, sp, 804
	sw	t3, 0(a2)
.fib.bb753:
	addi	t3, sp, 796
	li	a2, 0
	lw	t3, 0(t3)
	li	t4, 0
	subw	t3, t3, a2
	addi	a2, sp, 804
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, t4
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb754
.fib.bb755:
	li	t3, 0
	addi	a2, sp, 632
	sw	t3, 0(a2)
.fib.bb756:
	bne	t1, zero, .fib.bb757
.fib.bb758:
	li	t1, 0
	addi	t3, sp, 808
	sw	t1, 0(t3)
.fib.bb759:
	bne	t2, zero, .fib.bb760
.fib.bb761:
	li	t1, 0
	addi	t2, sp, 812
	sw	t1, 0(t2)
.fib.bb762:
	addi	t1, sp, 808
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 812
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb763
.fib.bb764:
	li	t1, 0
	addi	t2, sp, 572
	sw	t1, 0(t2)
.fib.bb765:
	li	t1, 0
	subw	t1, a3, t1
	sltu	t1, zero, t1
	or	t2, s5, t1
	bne	t2, zero, .fib.bb766
.fib.bb767:
	li	t2, 0
	addi	t3, sp, 820
	sw	t2, 0(t3)
.fib.bb768:
	and	t1, s5, t1
	bne	t1, zero, .fib.bb769
.fib.bb770:
	li	t2, 0
	addi	t3, sp, 824
	sw	t2, 0(t3)
.fib.bb771:
	addi	t2, sp, 824
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb772
.fib.bb773:
	li	t2, 0
	addi	t3, sp, 828
	sw	t2, 0(t3)
.fib.bb774:
	addi	t2, sp, 820
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 828
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb775
.fib.bb776:
	li	t2, 0
	addi	t3, sp, 816
	sw	t2, 0(t3)
.fib.bb777:
	addi	t2, sp, 816
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 572
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb778
.fib.bb779:
	li	a2, 0
	addi	a3, sp, 832
	sw	a2, 0(a3)
.fib.bb780:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb781
.fib.bb782:
	li	t3, 0
	addi	a2, sp, 836
	sw	t3, 0(a2)
.fib.bb783:
	addi	t3, sp, 836
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb784
.fib.bb785:
	li	t3, 0
	addi	a2, sp, 840
	sw	t3, 0(a2)
.fib.bb786:
	addi	t3, sp, 832
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 840
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb787
.fib.bb788:
	li	t3, 0
	addi	a2, sp, 636
	sw	t3, 0(a2)
.fib.bb789:
	bne	t1, zero, .fib.bb790
.fib.bb791:
	li	t1, 0
	addi	t3, sp, 844
	sw	t1, 0(t3)
.fib.bb792:
	bne	t2, zero, .fib.bb793
.fib.bb794:
	li	t1, 0
	addi	t2, sp, 848
	sw	t1, 0(t2)
.fib.bb795:
	addi	t1, sp, 844
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 848
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb796
.fib.bb797:
	li	t1, 0
	addi	t2, sp, 576
	sw	t1, 0(t2)
.fib.bb798:
	li	t1, 0
	subw	t1, a4, t1
	sltu	t1, zero, t1
	or	t2, s6, t1
	bne	t2, zero, .fib.bb799
.fib.bb800:
	li	t2, 0
	addi	t3, sp, 856
	sw	t2, 0(t3)
.fib.bb801:
	and	t1, s6, t1
	bne	t1, zero, .fib.bb802
.fib.bb803:
	li	t2, 0
	addi	t3, sp, 860
	sw	t2, 0(t3)
.fib.bb804:
	addi	t2, sp, 860
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb805
.fib.bb806:
	li	t2, 0
	addi	t3, sp, 864
	sw	t2, 0(t3)
.fib.bb807:
	addi	t2, sp, 856
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 864
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb808
.fib.bb809:
	li	t2, 0
	addi	t3, sp, 852
	sw	t2, 0(t3)
.fib.bb810:
	addi	t2, sp, 852
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 576
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb811
.fib.bb812:
	li	a2, 0
	addi	a3, sp, 868
	sw	a2, 0(a3)
.fib.bb813:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb814
.fib.bb815:
	li	t3, 0
	addi	a2, sp, 872
	sw	t3, 0(a2)
.fib.bb816:
	addi	t3, sp, 872
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb817
.fib.bb818:
	li	t3, 0
	addi	a2, sp, 876
	sw	t3, 0(a2)
.fib.bb819:
	addi	t3, sp, 868
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 876
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb820
.fib.bb821:
	li	t3, 0
	addi	a2, sp, 640
	sw	t3, 0(a2)
.fib.bb822:
	bne	t1, zero, .fib.bb823
.fib.bb824:
	li	t1, 0
	addi	t3, sp, 880
	sw	t1, 0(t3)
.fib.bb825:
	bne	t2, zero, .fib.bb826
.fib.bb827:
	li	t1, 0
	addi	t2, sp, 884
	sw	t1, 0(t2)
.fib.bb828:
	addi	t1, sp, 880
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 884
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb829
.fib.bb830:
	li	t1, 0
	addi	t2, sp, 580
	sw	t1, 0(t2)
.fib.bb831:
	li	t1, 0
	subw	t1, a5, t1
	sltu	t1, zero, t1
	or	t2, s7, t1
	bne	t2, zero, .fib.bb832
.fib.bb833:
	li	t2, 0
	addi	t3, sp, 892
	sw	t2, 0(t3)
.fib.bb834:
	and	t1, s7, t1
	bne	t1, zero, .fib.bb835
.fib.bb836:
	li	t2, 0
	addi	t3, sp, 896
	sw	t2, 0(t3)
.fib.bb837:
	addi	t2, sp, 896
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb838
.fib.bb839:
	li	t2, 0
	addi	t3, sp, 900
	sw	t2, 0(t3)
.fib.bb840:
	addi	t2, sp, 892
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 900
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb841
.fib.bb842:
	li	t2, 0
	addi	t3, sp, 888
	sw	t2, 0(t3)
.fib.bb843:
	addi	t2, sp, 888
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 580
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb844
.fib.bb845:
	li	a2, 0
	addi	a3, sp, 904
	sw	a2, 0(a3)
.fib.bb846:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb847
.fib.bb848:
	li	t3, 0
	addi	a2, sp, 908
	sw	t3, 0(a2)
.fib.bb849:
	addi	t3, sp, 908
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb850
.fib.bb851:
	li	t3, 0
	addi	a2, sp, 912
	sw	t3, 0(a2)
.fib.bb852:
	addi	t3, sp, 904
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 912
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb853
.fib.bb854:
	li	t3, 0
	addi	a2, sp, 644
	sw	t3, 0(a2)
.fib.bb855:
	bne	t1, zero, .fib.bb856
.fib.bb857:
	li	t1, 0
	addi	t3, sp, 916
	sw	t1, 0(t3)
.fib.bb858:
	bne	t2, zero, .fib.bb859
.fib.bb860:
	li	t1, 0
	addi	t2, sp, 920
	sw	t1, 0(t2)
.fib.bb861:
	addi	t1, sp, 916
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 920
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb862
.fib.bb863:
	li	t1, 0
	addi	t2, sp, 584
	sw	t1, 0(t2)
.fib.bb864:
	li	t1, 0
	subw	t1, a6, t1
	sltu	t1, zero, t1
	or	t2, s8, t1
	bne	t2, zero, .fib.bb865
.fib.bb866:
	li	t2, 0
	addi	t3, sp, 928
	sw	t2, 0(t3)
.fib.bb867:
	and	t1, s8, t1
	bne	t1, zero, .fib.bb868
.fib.bb869:
	li	t2, 0
	addi	t3, sp, 932
	sw	t2, 0(t3)
.fib.bb870:
	addi	t2, sp, 932
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb871
.fib.bb872:
	li	t2, 0
	addi	t3, sp, 936
	sw	t2, 0(t3)
.fib.bb873:
	addi	t2, sp, 928
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 936
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb874
.fib.bb875:
	li	t2, 0
	addi	t3, sp, 924
	sw	t2, 0(t3)
.fib.bb876:
	addi	t2, sp, 924
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 584
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb877
.fib.bb878:
	li	a2, 0
	addi	a3, sp, 940
	sw	a2, 0(a3)
.fib.bb879:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb880
.fib.bb881:
	li	t3, 0
	addi	a2, sp, 944
	sw	t3, 0(a2)
.fib.bb882:
	addi	t3, sp, 944
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb883
.fib.bb884:
	li	t3, 0
	addi	a2, sp, 948
	sw	t3, 0(a2)
.fib.bb885:
	addi	t3, sp, 940
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 948
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb886
.fib.bb887:
	li	t3, 0
	addi	a2, sp, 648
	sw	t3, 0(a2)
.fib.bb888:
	bne	t1, zero, .fib.bb889
.fib.bb890:
	li	t1, 0
	addi	t3, sp, 952
	sw	t1, 0(t3)
.fib.bb891:
	bne	t2, zero, .fib.bb892
.fib.bb893:
	li	t1, 0
	addi	t2, sp, 956
	sw	t1, 0(t2)
.fib.bb894:
	addi	t1, sp, 952
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 956
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb895
.fib.bb896:
	li	t1, 0
	addi	t2, sp, 588
	sw	t1, 0(t2)
.fib.bb897:
	li	t1, 0
	ld	t5, 416(sp)
	subw	t1, t5, t1
	sltu	t1, zero, t1
	ld	t5, 408(sp)
	or	t2, t5, t1
	bne	t2, zero, .fib.bb898
.fib.bb899:
	li	t2, 0
	addi	t3, sp, 964
	sw	t2, 0(t3)
.fib.bb900:
	ld	t5, 408(sp)
	and	t1, t5, t1
	bne	t1, zero, .fib.bb901
.fib.bb902:
	li	t2, 0
	addi	t3, sp, 968
	sw	t2, 0(t3)
.fib.bb903:
	addi	t2, sp, 968
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb904
.fib.bb905:
	li	t2, 0
	addi	t3, sp, 972
	sw	t2, 0(t3)
.fib.bb906:
	addi	t2, sp, 964
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 972
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb907
.fib.bb908:
	li	t2, 0
	addi	t3, sp, 960
	sw	t2, 0(t3)
.fib.bb909:
	addi	t2, sp, 960
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 588
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb910
.fib.bb911:
	li	a2, 0
	addi	a3, sp, 976
	sw	a2, 0(a3)
.fib.bb912:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb913
.fib.bb914:
	li	t3, 0
	addi	a2, sp, 980
	sw	t3, 0(a2)
.fib.bb915:
	addi	t3, sp, 980
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb916
.fib.bb917:
	li	t3, 0
	addi	a2, sp, 984
	sw	t3, 0(a2)
.fib.bb918:
	addi	t3, sp, 976
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 984
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb919
.fib.bb920:
	li	t3, 0
	addi	a2, sp, 652
	sw	t3, 0(a2)
.fib.bb921:
	bne	t1, zero, .fib.bb922
.fib.bb923:
	li	t1, 0
	addi	t3, sp, 988
	sw	t1, 0(t3)
.fib.bb924:
	bne	t2, zero, .fib.bb925
.fib.bb926:
	li	t1, 0
	addi	t2, sp, 992
	sw	t1, 0(t2)
.fib.bb927:
	addi	t1, sp, 988
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 992
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb928
.fib.bb929:
	li	t1, 0
	addi	t2, sp, 592
	sw	t1, 0(t2)
.fib.bb930:
	li	t1, 0
	ld	t5, 424(sp)
	subw	t1, t5, t1
	sltu	t1, zero, t1
	ld	t5, 400(sp)
	or	t2, t5, t1
	bne	t2, zero, .fib.bb931
.fib.bb932:
	li	t2, 0
	addi	t3, sp, 1000
	sw	t2, 0(t3)
.fib.bb933:
	ld	t5, 400(sp)
	and	t1, t5, t1
	bne	t1, zero, .fib.bb934
.fib.bb935:
	li	t2, 0
	addi	t3, sp, 1004
	sw	t2, 0(t3)
.fib.bb936:
	addi	t2, sp, 1004
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb937
.fib.bb938:
	li	t2, 0
	addi	t3, sp, 1008
	sw	t2, 0(t3)
.fib.bb939:
	addi	t2, sp, 1000
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1008
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb940
.fib.bb941:
	li	t2, 0
	addi	t3, sp, 996
	sw	t2, 0(t3)
.fib.bb942:
	addi	t2, sp, 996
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 592
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb943
.fib.bb944:
	li	a2, 0
	addi	a3, sp, 1012
	sw	a2, 0(a3)
.fib.bb945:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb946
.fib.bb947:
	li	t3, 0
	addi	a2, sp, 1016
	sw	t3, 0(a2)
.fib.bb948:
	addi	t3, sp, 1016
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb949
.fib.bb950:
	li	t3, 0
	addi	a2, sp, 1020
	sw	t3, 0(a2)
.fib.bb951:
	addi	t3, sp, 1012
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 1020
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb952
.fib.bb953:
	li	t3, 0
	addi	a2, sp, 656
	sw	t3, 0(a2)
.fib.bb954:
	bne	t1, zero, .fib.bb955
.fib.bb956:
	li	t1, 0
	addi	t3, sp, 1024
	sw	t1, 0(t3)
.fib.bb957:
	bne	t2, zero, .fib.bb958
.fib.bb959:
	li	t1, 0
	addi	t2, sp, 1028
	sw	t1, 0(t2)
.fib.bb960:
	addi	t1, sp, 1024
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1028
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb961
.fib.bb962:
	li	t1, 0
	addi	t2, sp, 596
	sw	t1, 0(t2)
.fib.bb963:
	li	t1, 0
	ld	t5, 432(sp)
	subw	t1, t5, t1
	sltu	t1, zero, t1
	ld	t5, 280(sp)
	or	t2, t5, t1
	bne	t2, zero, .fib.bb964
.fib.bb965:
	li	t2, 0
	addi	t3, sp, 1036
	sw	t2, 0(t3)
.fib.bb966:
	ld	t5, 280(sp)
	and	t1, t5, t1
	bne	t1, zero, .fib.bb967
.fib.bb968:
	li	t2, 0
	addi	t3, sp, 1040
	sw	t2, 0(t3)
.fib.bb969:
	addi	t2, sp, 1040
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb970
.fib.bb971:
	li	t2, 0
	addi	t3, sp, 1044
	sw	t2, 0(t3)
.fib.bb972:
	addi	t2, sp, 1036
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1044
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb973
.fib.bb974:
	li	t2, 0
	addi	t3, sp, 1032
	sw	t2, 0(t3)
.fib.bb975:
	addi	t2, sp, 1032
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 596
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb976
.fib.bb977:
	li	a2, 0
	addi	a3, sp, 1048
	sw	a2, 0(a3)
.fib.bb978:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb979
.fib.bb980:
	li	t3, 0
	addi	a2, sp, 1052
	sw	t3, 0(a2)
.fib.bb981:
	addi	t3, sp, 1052
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb982
.fib.bb983:
	li	t3, 0
	addi	a2, sp, 1056
	sw	t3, 0(a2)
.fib.bb984:
	addi	t3, sp, 1048
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 1056
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb985
.fib.bb986:
	li	t3, 0
	addi	a2, sp, 660
	sw	t3, 0(a2)
.fib.bb987:
	bne	t1, zero, .fib.bb988
.fib.bb989:
	li	t1, 0
	addi	t3, sp, 1060
	sw	t1, 0(t3)
.fib.bb990:
	bne	t2, zero, .fib.bb991
.fib.bb992:
	li	t1, 0
	addi	t2, sp, 1064
	sw	t1, 0(t2)
.fib.bb993:
	addi	t1, sp, 1060
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1064
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb994
.fib.bb995:
	li	t1, 0
	addi	t2, sp, 600
	sw	t1, 0(t2)
.fib.bb996:
	li	t1, 0
	ld	t5, 440(sp)
	subw	t1, t5, t1
	sltu	t1, zero, t1
	ld	t5, 296(sp)
	or	t2, t5, t1
	bne	t2, zero, .fib.bb997
.fib.bb998:
	li	t2, 0
	addi	t3, sp, 1072
	sw	t2, 0(t3)
.fib.bb999:
	ld	t5, 296(sp)
	and	t1, t5, t1
	bne	t1, zero, .fib.bb1000
.fib.bb1001:
	li	t2, 0
	addi	t3, sp, 1076
	sw	t2, 0(t3)
.fib.bb1002:
	addi	t2, sp, 1076
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1003
.fib.bb1004:
	li	t2, 0
	addi	t3, sp, 1080
	sw	t2, 0(t3)
.fib.bb1005:
	addi	t2, sp, 1072
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1080
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1006
.fib.bb1007:
	li	t2, 0
	addi	t3, sp, 1068
	sw	t2, 0(t3)
.fib.bb1008:
	addi	t2, sp, 1068
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 600
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb1009
.fib.bb1010:
	li	a2, 0
	addi	a3, sp, 1084
	sw	a2, 0(a3)
.fib.bb1011:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1012
.fib.bb1013:
	li	t3, 0
	addi	a2, sp, 1088
	sw	t3, 0(a2)
.fib.bb1014:
	addi	t3, sp, 1088
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb1015
.fib.bb1016:
	li	t3, 0
	addi	a2, sp, 1092
	sw	t3, 0(a2)
.fib.bb1017:
	addi	t3, sp, 1084
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 1092
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb1018
.fib.bb1019:
	li	t3, 0
	addi	a2, sp, 664
	sw	t3, 0(a2)
.fib.bb1020:
	bne	t1, zero, .fib.bb1021
.fib.bb1022:
	li	t1, 0
	addi	t3, sp, 1096
	sw	t1, 0(t3)
.fib.bb1023:
	bne	t2, zero, .fib.bb1024
.fib.bb1025:
	li	t1, 0
	addi	t2, sp, 1100
	sw	t1, 0(t2)
.fib.bb1026:
	addi	t1, sp, 1096
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1100
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb1027
.fib.bb1028:
	li	t1, 0
	addi	t2, sp, 604
	sw	t1, 0(t2)
.fib.bb1029:
	li	t1, 0
	ld	t5, 448(sp)
	subw	t1, t5, t1
	sltu	t1, zero, t1
	ld	t5, 312(sp)
	or	t2, t5, t1
	bne	t2, zero, .fib.bb1030
.fib.bb1031:
	li	t2, 0
	addi	t3, sp, 1108
	sw	t2, 0(t3)
.fib.bb1032:
	ld	t5, 312(sp)
	and	t1, t5, t1
	bne	t1, zero, .fib.bb1033
.fib.bb1034:
	li	t2, 0
	addi	t3, sp, 1112
	sw	t2, 0(t3)
.fib.bb1035:
	addi	t2, sp, 1112
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1036
.fib.bb1037:
	li	t2, 0
	addi	t3, sp, 1116
	sw	t2, 0(t3)
.fib.bb1038:
	addi	t2, sp, 1108
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1116
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1039
.fib.bb1040:
	li	t2, 0
	addi	t3, sp, 1104
	sw	t2, 0(t3)
.fib.bb1041:
	addi	t2, sp, 1104
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 604
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb1042
.fib.bb1043:
	li	a2, 0
	addi	a3, sp, 1120
	sw	a2, 0(a3)
.fib.bb1044:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1045
.fib.bb1046:
	li	t3, 0
	addi	a2, sp, 1124
	sw	t3, 0(a2)
.fib.bb1047:
	addi	t3, sp, 1124
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb1048
.fib.bb1049:
	li	t3, 0
	addi	a2, sp, 1128
	sw	t3, 0(a2)
.fib.bb1050:
	addi	t3, sp, 1120
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 1128
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb1051
.fib.bb1052:
	li	t3, 0
	addi	a2, sp, 668
	sw	t3, 0(a2)
.fib.bb1053:
	bne	t1, zero, .fib.bb1054
.fib.bb1055:
	li	t1, 0
	addi	t3, sp, 1132
	sw	t1, 0(t3)
.fib.bb1056:
	bne	t2, zero, .fib.bb1057
.fib.bb1058:
	li	t1, 0
	addi	t2, sp, 1136
	sw	t1, 0(t2)
.fib.bb1059:
	addi	t1, sp, 1132
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1136
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb1060
.fib.bb1061:
	li	t1, 0
	addi	t2, sp, 608
	sw	t1, 0(t2)
.fib.bb1062:
	li	t1, 0
	ld	t5, 456(sp)
	subw	t1, t5, t1
	sltu	t1, zero, t1
	ld	t5, 328(sp)
	or	t2, t5, t1
	bne	t2, zero, .fib.bb1063
.fib.bb1064:
	li	t2, 0
	addi	t3, sp, 1144
	sw	t2, 0(t3)
.fib.bb1065:
	ld	t5, 328(sp)
	and	t1, t5, t1
	bne	t1, zero, .fib.bb1066
.fib.bb1067:
	li	t2, 0
	addi	t3, sp, 1148
	sw	t2, 0(t3)
.fib.bb1068:
	addi	t2, sp, 1148
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1069
.fib.bb1070:
	li	t2, 0
	addi	t3, sp, 1152
	sw	t2, 0(t3)
.fib.bb1071:
	addi	t2, sp, 1144
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1152
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1072
.fib.bb1073:
	li	t2, 0
	addi	t3, sp, 1140
	sw	t2, 0(t3)
.fib.bb1074:
	addi	t2, sp, 1140
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 608
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb1075
.fib.bb1076:
	li	a2, 0
	addi	a3, sp, 1156
	sw	a2, 0(a3)
.fib.bb1077:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1078
.fib.bb1079:
	li	t3, 0
	addi	a2, sp, 1160
	sw	t3, 0(a2)
.fib.bb1080:
	addi	t3, sp, 1160
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb1081
.fib.bb1082:
	li	t3, 0
	addi	a2, sp, 1164
	sw	t3, 0(a2)
.fib.bb1083:
	addi	t3, sp, 1156
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 1164
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb1084
.fib.bb1085:
	li	t3, 0
	addi	a2, sp, 672
	sw	t3, 0(a2)
.fib.bb1086:
	bne	t1, zero, .fib.bb1087
.fib.bb1088:
	li	t1, 0
	addi	t3, sp, 1168
	sw	t1, 0(t3)
.fib.bb1089:
	bne	t2, zero, .fib.bb1090
.fib.bb1091:
	li	t1, 0
	addi	t2, sp, 1172
	sw	t1, 0(t2)
.fib.bb1092:
	addi	t1, sp, 1168
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1172
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb1093
.fib.bb1094:
	li	t1, 0
	addi	t2, sp, 612
	sw	t1, 0(t2)
.fib.bb1095:
	li	t1, 0
	subw	t1, a7, t1
	sltu	t1, zero, t1
	ld	t5, 336(sp)
	or	t2, t5, t1
	bne	t2, zero, .fib.bb1096
.fib.bb1097:
	li	t2, 0
	addi	t3, sp, 1180
	sw	t2, 0(t3)
.fib.bb1098:
	ld	t5, 336(sp)
	and	t1, t5, t1
	bne	t1, zero, .fib.bb1099
.fib.bb1100:
	li	t2, 0
	addi	t3, sp, 1184
	sw	t2, 0(t3)
.fib.bb1101:
	addi	t2, sp, 1184
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1102
.fib.bb1103:
	li	t2, 0
	addi	t3, sp, 1188
	sw	t2, 0(t3)
.fib.bb1104:
	addi	t2, sp, 1180
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1188
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1105
.fib.bb1106:
	li	t2, 0
	addi	t3, sp, 1176
	sw	t2, 0(t3)
.fib.bb1107:
	addi	t2, sp, 1176
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 612
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	or	a2, t2, t3
	bne	a2, zero, .fib.bb1108
.fib.bb1109:
	li	a2, 0
	addi	a3, sp, 1192
	sw	a2, 0(a3)
.fib.bb1110:
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1111
.fib.bb1112:
	li	t3, 0
	addi	a2, sp, 1196
	sw	t3, 0(a2)
.fib.bb1113:
	addi	t3, sp, 1196
	lw	t3, 0(t3)
	beq	t3, zero, .fib.bb1114
.fib.bb1115:
	li	t3, 0
	addi	a2, sp, 1200
	sw	t3, 0(a2)
.fib.bb1116:
	addi	t3, sp, 1192
	li	a2, 0
	lw	t3, 0(t3)
	li	a3, 0
	subw	t3, t3, a2
	addi	a2, sp, 1200
	sltu	t3, zero, t3
	lw	a2, 0(a2)
	subw	a2, a2, a3
	sltu	a2, zero, a2
	and	t3, t3, a2
	bne	t3, zero, .fib.bb1117
.fib.bb1118:
	li	t3, 0
	addi	a2, sp, 676
	sw	t3, 0(a2)
.fib.bb1119:
	bne	t1, zero, .fib.bb1120
.fib.bb1121:
	li	t1, 0
	addi	t3, sp, 1204
	sw	t1, 0(t3)
.fib.bb1122:
	bne	t2, zero, .fib.bb1123
.fib.bb1124:
	li	t1, 0
	addi	t2, sp, 1208
	sw	t1, 0(t2)
.fib.bb1125:
	addi	t1, sp, 1204
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1208
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t1, t1, t2
	bne	t1, zero, .fib.bb1126
.fib.bb1127:
	li	t1, 0
	addi	t2, sp, 616
	sw	t1, 0(t2)
.fib.bb1128:
	li	t1, 0
	subw	t0, t0, t1
	sltu	t0, zero, t0
	ld	t5, 392(sp)
	or	t1, t5, t0
	bne	t1, zero, .fib.bb1129
.fib.bb1130:
	li	t1, 0
	addi	t2, sp, 1216
	sw	t1, 0(t2)
.fib.bb1131:
	ld	t5, 392(sp)
	and	t0, t5, t0
	bne	t0, zero, .fib.bb1132
.fib.bb1133:
	li	t1, 0
	addi	t2, sp, 1220
	sw	t1, 0(t2)
.fib.bb1134:
	addi	t1, sp, 1220
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1135
.fib.bb1136:
	li	t1, 0
	addi	t2, sp, 1224
	sw	t1, 0(t2)
.fib.bb1137:
	addi	t1, sp, 1216
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1224
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1138
.fib.bb1139:
	li	t1, 0
	addi	t2, sp, 1212
	sw	t1, 0(t2)
.fib.bb1140:
	addi	t1, sp, 1212
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 616
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1141
.fib.bb1142:
	li	t3, 0
	addi	a2, sp, 1228
	sw	t3, 0(a2)
.fib.bb1143:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1144
.fib.bb1145:
	li	t2, 0
	addi	t3, sp, 1232
	sw	t2, 0(t3)
.fib.bb1146:
	addi	t2, sp, 1232
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1147
.fib.bb1148:
	li	t2, 0
	addi	t3, sp, 1236
	sw	t2, 0(t3)
.fib.bb1149:
	addi	t2, sp, 1228
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1236
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1150
.fib.bb1151:
	li	t2, 0
	addi	t3, sp, 680
	sw	t2, 0(t3)
.fib.bb1152:
	bne	t0, zero, .fib.bb1153
.fib.bb1154:
	li	t0, 0
	addi	t2, sp, 1240
	sw	t0, 0(t2)
.fib.bb1155:
	bne	t1, zero, .fib.bb1156
.fib.bb1157:
	li	t0, 0
	addi	t1, sp, 1244
	sw	t0, 0(t1)
.fib.bb1158:
	addi	t0, sp, 1240
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1244
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1161
.fib.bb1160:
.fib.bb1161:
	addi	t0, sp, 680
	lw	t0, 0(t0)
	slliw	t0, t0, 1
	addi	t1, sp, 676
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 672
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 668
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 664
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 660
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 656
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 652
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 648
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 644
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 640
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 636
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 632
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 628
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 624
	lw	t1, 0(t1)
	addw	t0, t0, t1
	slliw	t0, t0, 1
	addi	t1, sp, 620
	lw	t1, 0(t1)
	addw	a0, t0, t1
	call	fib
	mv	t0, a0
	ld	t5, 344(sp)
	srliw	t1, t5, 31	# const-div.pow2.bias-fold
	ld	t5, 344(sp)
	addw	t2, t5, t1
	andi	t3, t2, 1
	subw	t1, t3, t1
	li	t3, 0
	slt	t3, t1, t3
	subw	a2, zero, t1
	li	a3, 0
	subw	t3, a3, t3
	xor	a2, t1, a2
	and	t3, a2, t3
	xor	t1, t1, t3
.fib.bb1162:
	sraiw	t2, t2, 1
	li	a4, 0
	srliw	t3, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, t3
	andi	a2, t2, 1
	subw	t3, a2, t3
	li	a2, 0
	subw	a3, zero, t3
	slt	a2, t3, a2
	xor	a3, t3, a3
	subw	a2, a4, a2
	and	a2, a3, a2
	xor	t3, t3, a2
.fib.bb1163:
	sraiw	t2, t2, 1
	li	a5, 0
	srliw	a2, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a2
	andi	a3, t2, 1
	subw	a2, a3, a2
	li	a3, 0
	subw	a4, zero, a2
	slt	a3, a2, a3
	xor	a4, a2, a4
	subw	a3, a5, a3
	and	a3, a4, a3
	xor	a2, a2, a3
.fib.bb1164:
	sraiw	t2, t2, 1
	li	a6, 0
	srliw	a3, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a3
	andi	a4, t2, 1
	subw	a3, a4, a3
	li	a4, 0
	subw	a5, zero, a3
	slt	a4, a3, a4
	xor	a5, a3, a5
	subw	a4, a6, a4
	and	a4, a5, a4
	xor	a3, a3, a4
.fib.bb1165:
	sraiw	t2, t2, 1
	li	a7, 0
	srliw	a4, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a4
	andi	a5, t2, 1
	subw	a4, a5, a4
	li	a5, 0
	subw	a6, zero, a4
	slt	a5, a4, a5
	xor	a6, a4, a6
	subw	a5, a7, a5
	and	a5, a6, a5
	xor	a4, a4, a5
.fib.bb1166:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1248
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1167
.fib.bb1168:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1252
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1169
.fib.bb1170:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1256
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1171
.fib.bb1172:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1260
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1173
.fib.bb1174:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1264
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1175
.fib.bb1176:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1268
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1177
.fib.bb1178:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1272
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1179
.fib.bb1180:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1276
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1181
.fib.bb1182:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1280
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1183
.fib.bb1184:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	a6, t2, 1
	subw	a5, a6, a5
	addi	a6, sp, 1284
	sw	a5, 0(a6)
	blt	a5, zero, .fib.bb1185
.fib.bb1186:
	sraiw	t2, t2, 1
	srliw	a5, t2, 31	# const-div.pow2.bias-fold
	addw	t2, t2, a5
	andi	t2, t2, 1
	subw	t2, t2, a5
	addi	a5, sp, 1288
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1187
.fib.bb1188:
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1292
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1189
.fib.bb1190:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1296
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1191
.fib.bb1192:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1300
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1193
.fib.bb1194:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1304
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1195
.fib.bb1196:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1308
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1197
.fib.bb1198:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1312
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1199
.fib.bb1200:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1316
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1201
.fib.bb1202:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1320
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1203
.fib.bb1204:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1324
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1205
.fib.bb1206:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1328
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1207
.fib.bb1208:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1332
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1209
.fib.bb1210:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1336
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1211
.fib.bb1212:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1340
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1213
.fib.bb1214:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1344
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1215
.fib.bb1216:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	a5, t0, 1
	subw	t2, a5, t2
	addi	a5, sp, 1348
	sw	t2, 0(a5)
	blt	t2, zero, .fib.bb1217
.fib.bb1218:
	sraiw	t0, t0, 1
	srliw	t2, t0, 31	# const-div.pow2.bias-fold
	addw	t0, t0, t2
	andi	t0, t0, 1
	subw	t0, t0, t2
	addi	t2, sp, 1352
	sw	t0, 0(t2)
	blt	t0, zero, .fib.bb1219
.fib.bb1220:
	li	t0, 0
	li	t2, 0
	subw	t0, t1, t0
	addi	t1, sp, 1292
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1221
.fib.bb1222:
	li	t2, 0
	addi	a5, sp, 1484
	sw	t2, 0(a5)
.fib.bb1223:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1224
.fib.bb1225:
	li	t1, 0
	addi	t2, sp, 1488
	sw	t1, 0(t2)
.fib.bb1226:
	addi	t1, sp, 1488
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1227
.fib.bb1228:
	li	t1, 0
	addi	t2, sp, 1492
	sw	t1, 0(t2)
.fib.bb1229:
	addi	t1, sp, 1484
	li	t2, 0
	lw	t1, 0(t1)
	li	a5, 0
	subw	t1, t1, t2
	addi	t2, sp, 1492
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, a5
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1230
.fib.bb1231:
	li	t1, 0
	addi	t2, sp, 1480
	sw	t1, 0(t2)
.fib.bb1232:
	addi	t1, sp, 1480
	lw	t1, 0(t1)
	bne	t1, zero, .fib.bb1233
.fib.bb1234:
	li	t1, 0
	addi	t2, sp, 1496
	sw	t1, 0(t2)
.fib.bb1235:
	addi	t1, sp, 1496
	li	t2, 0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	bne	t1, zero, .fib.bb1236
.fib.bb1237:
	li	t1, 0
	addi	t2, sp, 1416
	sw	t1, 0(t2)
.fib.bb1238:
	bne	t0, zero, .fib.bb1239
.fib.bb1240:
	li	t0, 0
	addi	t1, sp, 1500
	sw	t0, 0(t1)
.fib.bb1241:
	addi	t0, sp, 1500
	lw	t0, 0(t0)
	bne	t0, zero, .fib.bb1242
.fib.bb1243:
	li	t0, 0
	addi	t1, sp, 1356
	sw	t0, 0(t1)
.fib.bb1244:
	addi	t1, sp, 1296
	li	t0, 0
	lw	t1, 0(t1)
	subw	t0, t3, t0
	li	t2, 0
	sltu	t0, zero, t0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1245
.fib.bb1246:
	li	t2, 0
	addi	t3, sp, 1508
	sw	t2, 0(t3)
.fib.bb1247:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1248
.fib.bb1249:
	li	t1, 0
	addi	t2, sp, 1512
	sw	t1, 0(t2)
.fib.bb1250:
	addi	t1, sp, 1512
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1251
.fib.bb1252:
	li	t1, 0
	addi	t2, sp, 1516
	sw	t1, 0(t2)
.fib.bb1253:
	addi	t1, sp, 1508
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1516
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1254
.fib.bb1255:
	li	t1, 0
	addi	t2, sp, 1504
	sw	t1, 0(t2)
.fib.bb1256:
	addi	t1, sp, 1504
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1356
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1257
.fib.bb1258:
	li	t3, 0
	addi	a5, sp, 1520
	sw	t3, 0(a5)
.fib.bb1259:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1260
.fib.bb1261:
	li	t2, 0
	addi	t3, sp, 1524
	sw	t2, 0(t3)
.fib.bb1262:
	addi	t2, sp, 1524
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1263
.fib.bb1264:
	li	t2, 0
	addi	t3, sp, 1528
	sw	t2, 0(t3)
.fib.bb1265:
	addi	t2, sp, 1520
	li	t3, 0
	lw	t2, 0(t2)
	li	a5, 0
	subw	t2, t2, t3
	addi	t3, sp, 1528
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a5
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1266
.fib.bb1267:
	li	t2, 0
	addi	t3, sp, 1420
	sw	t2, 0(t3)
.fib.bb1268:
	bne	t0, zero, .fib.bb1269
.fib.bb1270:
	li	t0, 0
	addi	t2, sp, 1532
	sw	t0, 0(t2)
.fib.bb1271:
	bne	t1, zero, .fib.bb1272
.fib.bb1273:
	li	t0, 0
	addi	t1, sp, 1536
	sw	t0, 0(t1)
.fib.bb1274:
	addi	t0, sp, 1532
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1536
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1275
.fib.bb1276:
	li	t0, 0
	addi	t1, sp, 1360
	sw	t0, 0(t1)
.fib.bb1277:
	addi	t1, sp, 1300
	li	t0, 0
	lw	t1, 0(t1)
	subw	t0, a2, t0
	li	t2, 0
	sltu	t0, zero, t0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1278
.fib.bb1279:
	li	t2, 0
	addi	t3, sp, 1544
	sw	t2, 0(t3)
.fib.bb1280:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1281
.fib.bb1282:
	li	t1, 0
	addi	t2, sp, 1548
	sw	t1, 0(t2)
.fib.bb1283:
	addi	t1, sp, 1548
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1284
.fib.bb1285:
	li	t1, 0
	addi	t2, sp, 1552
	sw	t1, 0(t2)
.fib.bb1286:
	addi	t1, sp, 1544
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1552
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1287
.fib.bb1288:
	li	t1, 0
	addi	t2, sp, 1540
	sw	t1, 0(t2)
.fib.bb1289:
	addi	t1, sp, 1540
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1360
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1290
.fib.bb1291:
	li	t3, 0
	addi	a2, sp, 1556
	sw	t3, 0(a2)
.fib.bb1292:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1293
.fib.bb1294:
	li	t2, 0
	addi	t3, sp, 1560
	sw	t2, 0(t3)
.fib.bb1295:
	addi	t2, sp, 1560
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1296
.fib.bb1297:
	li	t2, 0
	addi	t3, sp, 1564
	sw	t2, 0(t3)
.fib.bb1298:
	addi	t2, sp, 1556
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1564
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1299
.fib.bb1300:
	li	t2, 0
	addi	t3, sp, 1424
	sw	t2, 0(t3)
.fib.bb1301:
	bne	t0, zero, .fib.bb1302
.fib.bb1303:
	li	t0, 0
	addi	t2, sp, 1568
	sw	t0, 0(t2)
.fib.bb1304:
	bne	t1, zero, .fib.bb1305
.fib.bb1306:
	li	t0, 0
	addi	t1, sp, 1572
	sw	t0, 0(t1)
.fib.bb1307:
	addi	t0, sp, 1568
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1572
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1308
.fib.bb1309:
	li	t0, 0
	addi	t1, sp, 1364
	sw	t0, 0(t1)
.fib.bb1310:
	addi	t1, sp, 1304
	li	t0, 0
	lw	t1, 0(t1)
	subw	t0, a3, t0
	li	t2, 0
	sltu	t0, zero, t0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1311
.fib.bb1312:
	li	t2, 0
	addi	t3, sp, 1580
	sw	t2, 0(t3)
.fib.bb1313:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1314
.fib.bb1315:
	li	t1, 0
	addi	t2, sp, 1584
	sw	t1, 0(t2)
.fib.bb1316:
	addi	t1, sp, 1584
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1317
.fib.bb1318:
	li	t1, 0
	addi	t2, sp, 1588
	sw	t1, 0(t2)
.fib.bb1319:
	addi	t1, sp, 1580
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1588
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1320
.fib.bb1321:
	li	t1, 0
	addi	t2, sp, 1576
	sw	t1, 0(t2)
.fib.bb1322:
	addi	t1, sp, 1576
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1364
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1323
.fib.bb1324:
	li	t3, 0
	addi	a2, sp, 1592
	sw	t3, 0(a2)
.fib.bb1325:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1326
.fib.bb1327:
	li	t2, 0
	addi	t3, sp, 1596
	sw	t2, 0(t3)
.fib.bb1328:
	addi	t2, sp, 1596
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1329
.fib.bb1330:
	li	t2, 0
	addi	t3, sp, 1600
	sw	t2, 0(t3)
.fib.bb1331:
	addi	t2, sp, 1592
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1600
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1332
.fib.bb1333:
	li	t2, 0
	addi	t3, sp, 1428
	sw	t2, 0(t3)
.fib.bb1334:
	bne	t0, zero, .fib.bb1335
.fib.bb1336:
	li	t0, 0
	addi	t2, sp, 1604
	sw	t0, 0(t2)
.fib.bb1337:
	bne	t1, zero, .fib.bb1338
.fib.bb1339:
	li	t0, 0
	addi	t1, sp, 1608
	sw	t0, 0(t1)
.fib.bb1340:
	addi	t0, sp, 1604
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1608
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1341
.fib.bb1342:
	li	t0, 0
	addi	t1, sp, 1368
	sw	t0, 0(t1)
.fib.bb1343:
	addi	t1, sp, 1308
	li	t0, 0
	lw	t1, 0(t1)
	subw	t0, a4, t0
	li	t2, 0
	sltu	t0, zero, t0
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1344
.fib.bb1345:
	li	t2, 0
	addi	t3, sp, 1616
	sw	t2, 0(t3)
.fib.bb1346:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1347
.fib.bb1348:
	li	t1, 0
	addi	t2, sp, 1620
	sw	t1, 0(t2)
.fib.bb1349:
	addi	t1, sp, 1620
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1350
.fib.bb1351:
	li	t1, 0
	addi	t2, sp, 1624
	sw	t1, 0(t2)
.fib.bb1352:
	addi	t1, sp, 1616
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1624
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1353
.fib.bb1354:
	li	t1, 0
	addi	t2, sp, 1612
	sw	t1, 0(t2)
.fib.bb1355:
	addi	t1, sp, 1612
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1368
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1356
.fib.bb1357:
	li	t3, 0
	addi	a2, sp, 1628
	sw	t3, 0(a2)
.fib.bb1358:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1359
.fib.bb1360:
	li	t2, 0
	addi	t3, sp, 1632
	sw	t2, 0(t3)
.fib.bb1361:
	addi	t2, sp, 1632
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1362
.fib.bb1363:
	li	t2, 0
	addi	t3, sp, 1636
	sw	t2, 0(t3)
.fib.bb1364:
	addi	t2, sp, 1628
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1636
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1365
.fib.bb1366:
	li	t2, 0
	addi	t3, sp, 1432
	sw	t2, 0(t3)
.fib.bb1367:
	bne	t0, zero, .fib.bb1368
.fib.bb1369:
	li	t0, 0
	addi	t2, sp, 1640
	sw	t0, 0(t2)
.fib.bb1370:
	bne	t1, zero, .fib.bb1371
.fib.bb1372:
	li	t0, 0
	addi	t1, sp, 1644
	sw	t0, 0(t1)
.fib.bb1373:
	addi	t0, sp, 1640
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1644
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1374
.fib.bb1375:
	li	t0, 0
	addi	t1, sp, 1372
	sw	t0, 0(t1)
.fib.bb1376:
	addi	t0, sp, 1248
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1312
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1377
.fib.bb1378:
	li	t2, 0
	addi	t3, sp, 1652
	sw	t2, 0(t3)
.fib.bb1379:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1380
.fib.bb1381:
	li	t1, 0
	addi	t2, sp, 1656
	sw	t1, 0(t2)
.fib.bb1382:
	addi	t1, sp, 1656
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1383
.fib.bb1384:
	li	t1, 0
	addi	t2, sp, 1660
	sw	t1, 0(t2)
.fib.bb1385:
	addi	t1, sp, 1652
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1660
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1386
.fib.bb1387:
	li	t1, 0
	addi	t2, sp, 1648
	sw	t1, 0(t2)
.fib.bb1388:
	addi	t1, sp, 1648
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1372
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1389
.fib.bb1390:
	li	t3, 0
	addi	a2, sp, 1664
	sw	t3, 0(a2)
.fib.bb1391:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1392
.fib.bb1393:
	li	t2, 0
	addi	t3, sp, 1668
	sw	t2, 0(t3)
.fib.bb1394:
	addi	t2, sp, 1668
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1395
.fib.bb1396:
	li	t2, 0
	addi	t3, sp, 1672
	sw	t2, 0(t3)
.fib.bb1397:
	addi	t2, sp, 1664
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1672
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1398
.fib.bb1399:
	li	t2, 0
	addi	t3, sp, 1436
	sw	t2, 0(t3)
.fib.bb1400:
	bne	t0, zero, .fib.bb1401
.fib.bb1402:
	li	t0, 0
	addi	t2, sp, 1676
	sw	t0, 0(t2)
.fib.bb1403:
	bne	t1, zero, .fib.bb1404
.fib.bb1405:
	li	t0, 0
	addi	t1, sp, 1680
	sw	t0, 0(t1)
.fib.bb1406:
	addi	t0, sp, 1676
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1680
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1407
.fib.bb1408:
	li	t0, 0
	addi	t1, sp, 1376
	sw	t0, 0(t1)
.fib.bb1409:
	addi	t0, sp, 1252
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1316
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1410
.fib.bb1411:
	li	t2, 0
	addi	t3, sp, 1688
	sw	t2, 0(t3)
.fib.bb1412:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1413
.fib.bb1414:
	li	t1, 0
	addi	t2, sp, 1692
	sw	t1, 0(t2)
.fib.bb1415:
	addi	t1, sp, 1692
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1416
.fib.bb1417:
	li	t1, 0
	addi	t2, sp, 1696
	sw	t1, 0(t2)
.fib.bb1418:
	addi	t1, sp, 1688
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1696
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1419
.fib.bb1420:
	li	t1, 0
	addi	t2, sp, 1684
	sw	t1, 0(t2)
.fib.bb1421:
	addi	t1, sp, 1684
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1376
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1422
.fib.bb1423:
	li	t3, 0
	addi	a2, sp, 1700
	sw	t3, 0(a2)
.fib.bb1424:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1425
.fib.bb1426:
	li	t2, 0
	addi	t3, sp, 1704
	sw	t2, 0(t3)
.fib.bb1427:
	addi	t2, sp, 1704
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1428
.fib.bb1429:
	li	t2, 0
	addi	t3, sp, 1708
	sw	t2, 0(t3)
.fib.bb1430:
	addi	t2, sp, 1700
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1708
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1431
.fib.bb1432:
	li	t2, 0
	addi	t3, sp, 1440
	sw	t2, 0(t3)
.fib.bb1433:
	bne	t0, zero, .fib.bb1434
.fib.bb1435:
	li	t0, 0
	addi	t2, sp, 1712
	sw	t0, 0(t2)
.fib.bb1436:
	bne	t1, zero, .fib.bb1437
.fib.bb1438:
	li	t0, 0
	addi	t1, sp, 1716
	sw	t0, 0(t1)
.fib.bb1439:
	addi	t0, sp, 1712
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1716
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1440
.fib.bb1441:
	li	t0, 0
	addi	t1, sp, 1380
	sw	t0, 0(t1)
.fib.bb1442:
	addi	t0, sp, 1256
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1320
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1443
.fib.bb1444:
	li	t2, 0
	addi	t3, sp, 1724
	sw	t2, 0(t3)
.fib.bb1445:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1446
.fib.bb1447:
	li	t1, 0
	addi	t2, sp, 1728
	sw	t1, 0(t2)
.fib.bb1448:
	addi	t1, sp, 1728
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1449
.fib.bb1450:
	li	t1, 0
	addi	t2, sp, 1732
	sw	t1, 0(t2)
.fib.bb1451:
	addi	t1, sp, 1724
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1732
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1452
.fib.bb1453:
	li	t1, 0
	addi	t2, sp, 1720
	sw	t1, 0(t2)
.fib.bb1454:
	addi	t1, sp, 1720
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1380
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1455
.fib.bb1456:
	li	t3, 0
	addi	a2, sp, 1736
	sw	t3, 0(a2)
.fib.bb1457:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1458
.fib.bb1459:
	li	t2, 0
	addi	t3, sp, 1740
	sw	t2, 0(t3)
.fib.bb1460:
	addi	t2, sp, 1740
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1461
.fib.bb1462:
	li	t2, 0
	addi	t3, sp, 1744
	sw	t2, 0(t3)
.fib.bb1463:
	addi	t2, sp, 1736
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1744
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1464
.fib.bb1465:
	li	t2, 0
	addi	t3, sp, 1444
	sw	t2, 0(t3)
.fib.bb1466:
	bne	t0, zero, .fib.bb1467
.fib.bb1468:
	li	t0, 0
	addi	t2, sp, 1748
	sw	t0, 0(t2)
.fib.bb1469:
	bne	t1, zero, .fib.bb1470
.fib.bb1471:
	li	t0, 0
	addi	t1, sp, 1752
	sw	t0, 0(t1)
.fib.bb1472:
	addi	t0, sp, 1748
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1752
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1473
.fib.bb1474:
	li	t0, 0
	addi	t1, sp, 1384
	sw	t0, 0(t1)
.fib.bb1475:
	addi	t0, sp, 1260
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1324
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1476
.fib.bb1477:
	li	t2, 0
	addi	t3, sp, 1760
	sw	t2, 0(t3)
.fib.bb1478:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1479
.fib.bb1480:
	li	t1, 0
	addi	t2, sp, 1764
	sw	t1, 0(t2)
.fib.bb1481:
	addi	t1, sp, 1764
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1482
.fib.bb1483:
	li	t1, 0
	addi	t2, sp, 1768
	sw	t1, 0(t2)
.fib.bb1484:
	addi	t1, sp, 1760
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1768
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1485
.fib.bb1486:
	li	t1, 0
	addi	t2, sp, 1756
	sw	t1, 0(t2)
.fib.bb1487:
	addi	t1, sp, 1756
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1384
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1488
.fib.bb1489:
	li	t3, 0
	addi	a2, sp, 1772
	sw	t3, 0(a2)
.fib.bb1490:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1491
.fib.bb1492:
	li	t2, 0
	addi	t3, sp, 1776
	sw	t2, 0(t3)
.fib.bb1493:
	addi	t2, sp, 1776
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1494
.fib.bb1495:
	li	t2, 0
	addi	t3, sp, 1780
	sw	t2, 0(t3)
.fib.bb1496:
	addi	t2, sp, 1772
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1780
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1497
.fib.bb1498:
	li	t2, 0
	addi	t3, sp, 1448
	sw	t2, 0(t3)
.fib.bb1499:
	bne	t0, zero, .fib.bb1500
.fib.bb1501:
	li	t0, 0
	addi	t2, sp, 1784
	sw	t0, 0(t2)
.fib.bb1502:
	bne	t1, zero, .fib.bb1503
.fib.bb1504:
	li	t0, 0
	addi	t1, sp, 1788
	sw	t0, 0(t1)
.fib.bb1505:
	addi	t0, sp, 1784
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1788
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1506
.fib.bb1507:
	li	t0, 0
	addi	t1, sp, 1388
	sw	t0, 0(t1)
.fib.bb1508:
	addi	t0, sp, 1264
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1328
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1509
.fib.bb1510:
	li	t2, 0
	addi	t3, sp, 1796
	sw	t2, 0(t3)
.fib.bb1511:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1512
.fib.bb1513:
	li	t1, 0
	addi	t2, sp, 1800
	sw	t1, 0(t2)
.fib.bb1514:
	addi	t1, sp, 1800
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1515
.fib.bb1516:
	li	t1, 0
	addi	t2, sp, 1804
	sw	t1, 0(t2)
.fib.bb1517:
	addi	t1, sp, 1796
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1804
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1518
.fib.bb1519:
	li	t1, 0
	addi	t2, sp, 1792
	sw	t1, 0(t2)
.fib.bb1520:
	addi	t1, sp, 1792
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1388
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1521
.fib.bb1522:
	li	t3, 0
	addi	a2, sp, 1808
	sw	t3, 0(a2)
.fib.bb1523:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1524
.fib.bb1525:
	li	t2, 0
	addi	t3, sp, 1812
	sw	t2, 0(t3)
.fib.bb1526:
	addi	t2, sp, 1812
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1527
.fib.bb1528:
	li	t2, 0
	addi	t3, sp, 1816
	sw	t2, 0(t3)
.fib.bb1529:
	addi	t2, sp, 1808
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1816
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1530
.fib.bb1531:
	li	t2, 0
	addi	t3, sp, 1452
	sw	t2, 0(t3)
.fib.bb1532:
	bne	t0, zero, .fib.bb1533
.fib.bb1534:
	li	t0, 0
	addi	t2, sp, 1820
	sw	t0, 0(t2)
.fib.bb1535:
	bne	t1, zero, .fib.bb1536
.fib.bb1537:
	li	t0, 0
	addi	t1, sp, 1824
	sw	t0, 0(t1)
.fib.bb1538:
	addi	t0, sp, 1820
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1824
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1539
.fib.bb1540:
	li	t0, 0
	addi	t1, sp, 1392
	sw	t0, 0(t1)
.fib.bb1541:
	addi	t0, sp, 1268
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1332
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1542
.fib.bb1543:
	li	t2, 0
	addi	t3, sp, 1832
	sw	t2, 0(t3)
.fib.bb1544:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1545
.fib.bb1546:
	li	t1, 0
	addi	t2, sp, 1836
	sw	t1, 0(t2)
.fib.bb1547:
	addi	t1, sp, 1836
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1548
.fib.bb1549:
	li	t1, 0
	addi	t2, sp, 1840
	sw	t1, 0(t2)
.fib.bb1550:
	addi	t1, sp, 1832
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1840
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1551
.fib.bb1552:
	li	t1, 0
	addi	t2, sp, 1828
	sw	t1, 0(t2)
.fib.bb1553:
	addi	t1, sp, 1828
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1392
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1554
.fib.bb1555:
	li	t3, 0
	addi	a2, sp, 1844
	sw	t3, 0(a2)
.fib.bb1556:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1557
.fib.bb1558:
	li	t2, 0
	addi	t3, sp, 1848
	sw	t2, 0(t3)
.fib.bb1559:
	addi	t2, sp, 1848
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1560
.fib.bb1561:
	li	t2, 0
	addi	t3, sp, 1852
	sw	t2, 0(t3)
.fib.bb1562:
	addi	t2, sp, 1844
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1852
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1563
.fib.bb1564:
	li	t2, 0
	addi	t3, sp, 1456
	sw	t2, 0(t3)
.fib.bb1565:
	bne	t0, zero, .fib.bb1566
.fib.bb1567:
	li	t0, 0
	addi	t2, sp, 1856
	sw	t0, 0(t2)
.fib.bb1568:
	bne	t1, zero, .fib.bb1569
.fib.bb1570:
	li	t0, 0
	addi	t1, sp, 1860
	sw	t0, 0(t1)
.fib.bb1571:
	addi	t0, sp, 1856
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1860
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1572
.fib.bb1573:
	li	t0, 0
	addi	t1, sp, 1396
	sw	t0, 0(t1)
.fib.bb1574:
	addi	t0, sp, 1272
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1336
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1575
.fib.bb1576:
	li	t2, 0
	addi	t3, sp, 1868
	sw	t2, 0(t3)
.fib.bb1577:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1578
.fib.bb1579:
	li	t1, 0
	addi	t2, sp, 1872
	sw	t1, 0(t2)
.fib.bb1580:
	addi	t1, sp, 1872
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1581
.fib.bb1582:
	li	t1, 0
	addi	t2, sp, 1876
	sw	t1, 0(t2)
.fib.bb1583:
	addi	t1, sp, 1868
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1876
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1584
.fib.bb1585:
	li	t1, 0
	addi	t2, sp, 1864
	sw	t1, 0(t2)
.fib.bb1586:
	addi	t1, sp, 1864
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1396
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1587
.fib.bb1588:
	li	t3, 0
	addi	a2, sp, 1880
	sw	t3, 0(a2)
.fib.bb1589:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1590
.fib.bb1591:
	li	t2, 0
	addi	t3, sp, 1884
	sw	t2, 0(t3)
.fib.bb1592:
	addi	t2, sp, 1884
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1593
.fib.bb1594:
	li	t2, 0
	addi	t3, sp, 1888
	sw	t2, 0(t3)
.fib.bb1595:
	addi	t2, sp, 1880
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1888
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1596
.fib.bb1597:
	li	t2, 0
	addi	t3, sp, 1460
	sw	t2, 0(t3)
.fib.bb1598:
	bne	t0, zero, .fib.bb1599
.fib.bb1600:
	li	t0, 0
	addi	t2, sp, 1892
	sw	t0, 0(t2)
.fib.bb1601:
	bne	t1, zero, .fib.bb1602
.fib.bb1603:
	li	t0, 0
	addi	t1, sp, 1896
	sw	t0, 0(t1)
.fib.bb1604:
	addi	t0, sp, 1892
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1896
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1605
.fib.bb1606:
	li	t0, 0
	addi	t1, sp, 1400
	sw	t0, 0(t1)
.fib.bb1607:
	addi	t0, sp, 1276
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1340
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1608
.fib.bb1609:
	li	t2, 0
	addi	t3, sp, 1904
	sw	t2, 0(t3)
.fib.bb1610:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1611
.fib.bb1612:
	li	t1, 0
	addi	t2, sp, 1908
	sw	t1, 0(t2)
.fib.bb1613:
	addi	t1, sp, 1908
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1614
.fib.bb1615:
	li	t1, 0
	addi	t2, sp, 1912
	sw	t1, 0(t2)
.fib.bb1616:
	addi	t1, sp, 1904
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1912
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1617
.fib.bb1618:
	li	t1, 0
	addi	t2, sp, 1900
	sw	t1, 0(t2)
.fib.bb1619:
	addi	t1, sp, 1900
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1400
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1620
.fib.bb1621:
	li	t3, 0
	addi	a2, sp, 1916
	sw	t3, 0(a2)
.fib.bb1622:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1623
.fib.bb1624:
	li	t2, 0
	addi	t3, sp, 1920
	sw	t2, 0(t3)
.fib.bb1625:
	addi	t2, sp, 1920
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1626
.fib.bb1627:
	li	t2, 0
	addi	t3, sp, 1924
	sw	t2, 0(t3)
.fib.bb1628:
	addi	t2, sp, 1916
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1924
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1629
.fib.bb1630:
	li	t2, 0
	addi	t3, sp, 1464
	sw	t2, 0(t3)
.fib.bb1631:
	bne	t0, zero, .fib.bb1632
.fib.bb1633:
	li	t0, 0
	addi	t2, sp, 1928
	sw	t0, 0(t2)
.fib.bb1634:
	bne	t1, zero, .fib.bb1635
.fib.bb1636:
	li	t0, 0
	addi	t1, sp, 1932
	sw	t0, 0(t1)
.fib.bb1637:
	addi	t0, sp, 1928
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1932
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1638
.fib.bb1639:
	li	t0, 0
	addi	t1, sp, 1404
	sw	t0, 0(t1)
.fib.bb1640:
	addi	t0, sp, 1280
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1344
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1641
.fib.bb1642:
	li	t2, 0
	addi	t3, sp, 1940
	sw	t2, 0(t3)
.fib.bb1643:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1644
.fib.bb1645:
	li	t1, 0
	addi	t2, sp, 1944
	sw	t1, 0(t2)
.fib.bb1646:
	addi	t1, sp, 1944
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1647
.fib.bb1648:
	li	t1, 0
	addi	t2, sp, 1948
	sw	t1, 0(t2)
.fib.bb1649:
	addi	t1, sp, 1940
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1948
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1650
.fib.bb1651:
	li	t1, 0
	addi	t2, sp, 1936
	sw	t1, 0(t2)
.fib.bb1652:
	addi	t1, sp, 1936
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1404
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1653
.fib.bb1654:
	li	t3, 0
	addi	a2, sp, 1952
	sw	t3, 0(a2)
.fib.bb1655:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1656
.fib.bb1657:
	li	t2, 0
	addi	t3, sp, 1956
	sw	t2, 0(t3)
.fib.bb1658:
	addi	t2, sp, 1956
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1659
.fib.bb1660:
	li	t2, 0
	addi	t3, sp, 1960
	sw	t2, 0(t3)
.fib.bb1661:
	addi	t2, sp, 1952
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1960
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1662
.fib.bb1663:
	li	t2, 0
	addi	t3, sp, 1468
	sw	t2, 0(t3)
.fib.bb1664:
	bne	t0, zero, .fib.bb1665
.fib.bb1666:
	li	t0, 0
	addi	t2, sp, 1964
	sw	t0, 0(t2)
.fib.bb1667:
	bne	t1, zero, .fib.bb1668
.fib.bb1669:
	li	t0, 0
	addi	t1, sp, 1968
	sw	t0, 0(t1)
.fib.bb1670:
	addi	t0, sp, 1964
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1968
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1671
.fib.bb1672:
	li	t0, 0
	addi	t1, sp, 1408
	sw	t0, 0(t1)
.fib.bb1673:
	addi	t0, sp, 1284
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1348
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1674
.fib.bb1675:
	li	t2, 0
	addi	t3, sp, 1976
	sw	t2, 0(t3)
.fib.bb1676:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1677
.fib.bb1678:
	li	t1, 0
	addi	t2, sp, 1980
	sw	t1, 0(t2)
.fib.bb1679:
	addi	t1, sp, 1980
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1680
.fib.bb1681:
	li	t1, 0
	addi	t2, sp, 1984
	sw	t1, 0(t2)
.fib.bb1682:
	addi	t1, sp, 1976
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1984
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1683
.fib.bb1684:
	li	t1, 0
	addi	t2, sp, 1972
	sw	t1, 0(t2)
.fib.bb1685:
	addi	t1, sp, 1972
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1408
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1686
.fib.bb1687:
	li	t3, 0
	addi	a2, sp, 1988
	sw	t3, 0(a2)
.fib.bb1688:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1689
.fib.bb1690:
	li	t2, 0
	addi	t3, sp, 1992
	sw	t2, 0(t3)
.fib.bb1691:
	addi	t2, sp, 1992
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1692
.fib.bb1693:
	li	t2, 0
	addi	t3, sp, 1996
	sw	t2, 0(t3)
.fib.bb1694:
	addi	t2, sp, 1988
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 1996
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1695
.fib.bb1696:
	li	t2, 0
	addi	t3, sp, 1472
	sw	t2, 0(t3)
.fib.bb1697:
	bne	t0, zero, .fib.bb1698
.fib.bb1699:
	li	t0, 0
	addi	t2, sp, 2000
	sw	t0, 0(t2)
.fib.bb1700:
	bne	t1, zero, .fib.bb1701
.fib.bb1702:
	li	t0, 0
	addi	t1, sp, 2004
	sw	t0, 0(t1)
.fib.bb1703:
	addi	t0, sp, 2000
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 2004
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1704
.fib.bb1705:
	li	t0, 0
	addi	t1, sp, 1412
	sw	t0, 0(t1)
.fib.bb1706:
	addi	t0, sp, 1288
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 1352
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t2, t0, t1
	bne	t2, zero, .fib.bb1707
.fib.bb1708:
	li	t2, 0
	addi	t3, sp, 2012
	sw	t2, 0(t3)
.fib.bb1709:
	and	t0, t0, t1
	bne	t0, zero, .fib.bb1710
.fib.bb1711:
	li	t1, 0
	addi	t2, sp, 2016
	sw	t1, 0(t2)
.fib.bb1712:
	addi	t1, sp, 2016
	lw	t1, 0(t1)
	beq	t1, zero, .fib.bb1713
.fib.bb1714:
	li	t1, 0
	addi	t2, sp, 2020
	sw	t1, 0(t2)
.fib.bb1715:
	addi	t1, sp, 2012
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 2020
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1716
.fib.bb1717:
	li	t1, 0
	addi	t2, sp, 2008
	sw	t1, 0(t2)
.fib.bb1718:
	addi	t1, sp, 2008
	li	t2, 0
	lw	t1, 0(t1)
	li	t3, 0
	subw	t1, t1, t2
	addi	t2, sp, 1412
	sltu	t1, zero, t1
	lw	t2, 0(t2)
	subw	t2, t2, t3
	sltu	t2, zero, t2
	or	t3, t1, t2
	bne	t3, zero, .fib.bb1719
.fib.bb1720:
	li	t3, 0
	addi	a2, sp, 2024
	sw	t3, 0(a2)
.fib.bb1721:
	and	t1, t1, t2
	bne	t1, zero, .fib.bb1722
.fib.bb1723:
	li	t2, 0
	addi	t3, sp, 2028
	sw	t2, 0(t3)
.fib.bb1724:
	addi	t2, sp, 2028
	lw	t2, 0(t2)
	beq	t2, zero, .fib.bb1725
.fib.bb1726:
	li	t2, 0
	addi	t3, sp, 2032
	sw	t2, 0(t3)
.fib.bb1727:
	addi	t2, sp, 2024
	li	t3, 0
	lw	t2, 0(t2)
	li	a2, 0
	subw	t2, t2, t3
	addi	t3, sp, 2032
	sltu	t2, zero, t2
	lw	t3, 0(t3)
	subw	t3, t3, a2
	sltu	t3, zero, t3
	and	t2, t2, t3
	bne	t2, zero, .fib.bb1728
.fib.bb1729:
	li	t2, 0
	addi	t3, sp, 1476
	sw	t2, 0(t3)
.fib.bb1730:
	bne	t0, zero, .fib.bb1731
.fib.bb1732:
	li	t0, 0
	addi	t2, sp, 2036
	sw	t0, 0(t2)
.fib.bb1733:
	bne	t1, zero, .fib.bb1734
.fib.bb1735:
	li	t0, 0
	addi	t1, sp, 2040
	sw	t0, 0(t1)
.fib.bb1736:
	addi	t0, sp, 2036
	li	t1, 0
	lw	t0, 0(t0)
	li	t2, 0
	subw	t0, t0, t1
	addi	t1, sp, 2040
	sltu	t0, zero, t0
	lw	t1, 0(t1)
	subw	t1, t1, t2
	sltu	t1, zero, t1
	or	t0, t0, t1
	bne	t0, zero, .fib.bb1739
.fib.bb1738:
.fib.bb1739:
	addi	t0, sp, 1476
	addi	t1, sp, 1472
	lw	t0, 0(t0)
	lw	t1, 0(t1)
	slliw	t0, t0, 1
	addw	t0, t0, t1
	addi	t1, sp, 1468
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1464
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1460
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1456
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1452
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1448
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1444
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1440
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1436
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1432
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1428
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1424
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1420
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	addi	t1, sp, 1416
	slliw	t0, t0, 1
	lw	t1, 0(t1)
	addw	t0, t0, t1
	j	.fib.bb1
.fib.bb532:
	li	a0, 1
	addi	a1, sp, 464
	sw	a0, 0(a1)
	j	.fib.bb534
.fib.bb535:
	li	t3, 1
	addi	a1, sp, 472
	sw	t3, 0(a1)
	j	.fib.bb537
.fib.bb538:
	li	t3, 1
	addi	a0, sp, 476
	sw	t3, 0(a0)
	j	.fib.bb540
.fib.bb541:
	li	t3, 1
	addi	a0, sp, 480
	sw	t3, 0(a0)
	j	.fib.bb543
.fib.bb552:
	li	a0, 1
	addi	a1, sp, 488
	sw	a0, 0(a1)
	j	.fib.bb554
.fib.bb555:
	li	a2, 1
	addi	a1, sp, 496
	sw	a2, 0(a1)
	j	.fib.bb557
.fib.bb558:
	li	a2, 1
	addi	a0, sp, 500
	sw	a2, 0(a0)
	j	.fib.bb560
.fib.bb561:
	li	a2, 1
	addi	a0, sp, 504
	sw	a2, 0(a0)
	j	.fib.bb563
.fib.bb572:
	li	a0, 1
	addi	a1, sp, 512
	sw	a0, 0(a1)
	j	.fib.bb574
.fib.bb575:
	li	a3, 1
	addi	a1, sp, 520
	sw	a3, 0(a1)
	j	.fib.bb577
.fib.bb578:
	li	a3, 1
	addi	a0, sp, 524
	sw	a3, 0(a0)
	j	.fib.bb580
.fib.bb581:
	li	a3, 1
	addi	a0, sp, 528
	sw	a3, 0(a0)
	j	.fib.bb583
.fib.bb592:
	li	a0, 1
	addi	a1, sp, 536
	sw	a0, 0(a1)
	j	.fib.bb594
.fib.bb595:
	li	a1, 1
	addi	s10, sp, 544
	sw	a1, 0(s10)
	j	.fib.bb597
.fib.bb598:
	li	a0, 1
	addi	a1, sp, 548
	sw	a0, 0(a1)
	j	.fib.bb600
.fib.bb601:
	li	a0, 1
	addi	a1, sp, 552
	sw	a0, 0(a1)
	j	.fib.bb603
.fib.bb608:
	j	.fib.bb626
.fib.bb646:
	li	t4, 1
	addi	a0, sp, 692
	sw	t4, 0(a0)
	j	.fib.bb648
.fib.bb649:
	li	t4, 1
	addi	a0, sp, 696
	sw	t4, 0(a0)
	j	.fib.bb651
.fib.bb652:
	li	t4, 1
	addi	a0, sp, 684
	sw	t4, 0(a0)
	j	.fib.bb654
.fib.bb655:
	li	t4, 1
	addi	a0, sp, 700
	sw	t4, 0(a0)
	j	.fib.bb657
.fib.bb658:
	li	t4, 1
	addi	a0, sp, 620
	sw	t4, 0(a0)
	j	.fib.bb660
.fib.bb661:
	li	t1, 1
	addi	t4, sp, 704
	sw	t1, 0(t4)
	j	.fib.bb663
.fib.bb664:
	li	t1, 1
	addi	t4, sp, 560
	sw	t1, 0(t4)
	j	.fib.bb666
.fib.bb667:
	li	t2, 1
	addi	t4, sp, 712
	sw	t2, 0(t4)
	j	.fib.bb669
.fib.bb670:
	li	t2, 1
	addi	t4, sp, 716
	sw	t2, 0(t4)
	j	.fib.bb672
.fib.bb673:
	li	t2, 1
	addi	t4, sp, 720
	sw	t2, 0(t4)
	j	.fib.bb675
.fib.bb676:
	li	t2, 1
	addi	t4, sp, 708
	sw	t2, 0(t4)
	j	.fib.bb678
.fib.bb679:
	li	a0, 1
	addi	a1, sp, 724
	sw	a0, 0(a1)
	j	.fib.bb681
.fib.bb682:
	li	t4, 1
	addi	a0, sp, 728
	sw	t4, 0(a0)
	j	.fib.bb684
.fib.bb685:
	li	t4, 1
	addi	a0, sp, 732
	sw	t4, 0(a0)
	j	.fib.bb687
.fib.bb688:
	li	t4, 1
	addi	a0, sp, 624
	sw	t4, 0(a0)
	j	.fib.bb690
.fib.bb691:
	li	t1, 1
	addi	t4, sp, 736
	sw	t1, 0(t4)
	j	.fib.bb693
.fib.bb694:
	li	t1, 1
	addi	t2, sp, 740
	sw	t1, 0(t2)
	j	.fib.bb696
.fib.bb697:
	li	t1, 1
	addi	t2, sp, 564
	sw	t1, 0(t2)
	j	.fib.bb699
.fib.bb700:
	li	t2, 1
	addi	t3, sp, 748
	sw	t2, 0(t3)
	j	.fib.bb702
.fib.bb703:
	li	t2, 1
	addi	t3, sp, 752
	sw	t2, 0(t3)
	j	.fib.bb705
.fib.bb706:
	li	t2, 1
	addi	t3, sp, 756
	sw	t2, 0(t3)
	j	.fib.bb708
.fib.bb709:
	li	t2, 1
	addi	t3, sp, 744
	sw	t2, 0(t3)
	j	.fib.bb711
.fib.bb712:
	li	t4, 1
	addi	a0, sp, 760
	sw	t4, 0(a0)
	j	.fib.bb714
.fib.bb715:
	li	t3, 1
	addi	t4, sp, 764
	sw	t3, 0(t4)
	j	.fib.bb717
.fib.bb718:
	li	t3, 1
	addi	t4, sp, 768
	sw	t3, 0(t4)
	j	.fib.bb720
.fib.bb721:
	li	t3, 1
	addi	t4, sp, 628
	sw	t3, 0(t4)
	j	.fib.bb723
.fib.bb724:
	li	t1, 1
	addi	t3, sp, 772
	sw	t1, 0(t3)
	j	.fib.bb726
.fib.bb727:
	li	t1, 1
	addi	t2, sp, 776
	sw	t1, 0(t2)
	j	.fib.bb729
.fib.bb730:
	li	t1, 1
	addi	t2, sp, 568
	sw	t1, 0(t2)
	j	.fib.bb732
.fib.bb733:
	li	t2, 1
	addi	t3, sp, 784
	sw	t2, 0(t3)
	j	.fib.bb735
.fib.bb736:
	li	t2, 1
	addi	t3, sp, 788
	sw	t2, 0(t3)
	j	.fib.bb738
.fib.bb739:
	li	t2, 1
	addi	t3, sp, 792
	sw	t2, 0(t3)
	j	.fib.bb741
.fib.bb742:
	li	t2, 1
	addi	t3, sp, 780
	sw	t2, 0(t3)
	j	.fib.bb744
.fib.bb745:
	li	a2, 1
	addi	t4, sp, 796
	sw	a2, 0(t4)
	j	.fib.bb747
.fib.bb748:
	li	t3, 1
	addi	a2, sp, 800
	sw	t3, 0(a2)
	j	.fib.bb750
.fib.bb751:
	li	t3, 1
	addi	a2, sp, 804
	sw	t3, 0(a2)
	j	.fib.bb753
.fib.bb754:
	li	t3, 1
	addi	a2, sp, 632
	sw	t3, 0(a2)
	j	.fib.bb756
.fib.bb757:
	li	t1, 1
	addi	t3, sp, 808
	sw	t1, 0(t3)
	j	.fib.bb759
.fib.bb760:
	li	t1, 1
	addi	t2, sp, 812
	sw	t1, 0(t2)
	j	.fib.bb762
.fib.bb763:
	li	t1, 1
	addi	t2, sp, 572
	sw	t1, 0(t2)
	j	.fib.bb765
.fib.bb766:
	li	t2, 1
	addi	t3, sp, 820
	sw	t2, 0(t3)
	j	.fib.bb768
.fib.bb769:
	li	t2, 1
	addi	t3, sp, 824
	sw	t2, 0(t3)
	j	.fib.bb771
.fib.bb772:
	li	t2, 1
	addi	t3, sp, 828
	sw	t2, 0(t3)
	j	.fib.bb774
.fib.bb775:
	li	t2, 1
	addi	t3, sp, 816
	sw	t2, 0(t3)
	j	.fib.bb777
.fib.bb778:
	li	a2, 1
	addi	a3, sp, 832
	sw	a2, 0(a3)
	j	.fib.bb780
.fib.bb781:
	li	t3, 1
	addi	a2, sp, 836
	sw	t3, 0(a2)
	j	.fib.bb783
.fib.bb784:
	li	t3, 1
	addi	a2, sp, 840
	sw	t3, 0(a2)
	j	.fib.bb786
.fib.bb787:
	li	t3, 1
	addi	a2, sp, 636
	sw	t3, 0(a2)
	j	.fib.bb789
.fib.bb790:
	li	t1, 1
	addi	t3, sp, 844
	sw	t1, 0(t3)
	j	.fib.bb792
.fib.bb793:
	li	t1, 1
	addi	t2, sp, 848
	sw	t1, 0(t2)
	j	.fib.bb795
.fib.bb796:
	li	t1, 1
	addi	t2, sp, 576
	sw	t1, 0(t2)
	j	.fib.bb798
.fib.bb799:
	li	t2, 1
	addi	t3, sp, 856
	sw	t2, 0(t3)
	j	.fib.bb801
.fib.bb802:
	li	t2, 1
	addi	t3, sp, 860
	sw	t2, 0(t3)
	j	.fib.bb804
.fib.bb805:
	li	t2, 1
	addi	t3, sp, 864
	sw	t2, 0(t3)
	j	.fib.bb807
.fib.bb808:
	li	t2, 1
	addi	t3, sp, 852
	sw	t2, 0(t3)
	j	.fib.bb810
.fib.bb811:
	li	a2, 1
	addi	a3, sp, 868
	sw	a2, 0(a3)
	j	.fib.bb813
.fib.bb814:
	li	t3, 1
	addi	a2, sp, 872
	sw	t3, 0(a2)
	j	.fib.bb816
.fib.bb817:
	li	t3, 1
	addi	a2, sp, 876
	sw	t3, 0(a2)
	j	.fib.bb819
.fib.bb820:
	li	t3, 1
	addi	a2, sp, 640
	sw	t3, 0(a2)
	j	.fib.bb822
.fib.bb823:
	li	t1, 1
	addi	t3, sp, 880
	sw	t1, 0(t3)
	j	.fib.bb825
.fib.bb826:
	li	t1, 1
	addi	t2, sp, 884
	sw	t1, 0(t2)
	j	.fib.bb828
.fib.bb829:
	li	t1, 1
	addi	t2, sp, 580
	sw	t1, 0(t2)
	j	.fib.bb831
.fib.bb832:
	li	t2, 1
	addi	t3, sp, 892
	sw	t2, 0(t3)
	j	.fib.bb834
.fib.bb835:
	li	t2, 1
	addi	t3, sp, 896
	sw	t2, 0(t3)
	j	.fib.bb837
.fib.bb838:
	li	t2, 1
	addi	t3, sp, 900
	sw	t2, 0(t3)
	j	.fib.bb840
.fib.bb841:
	li	t2, 1
	addi	t3, sp, 888
	sw	t2, 0(t3)
	j	.fib.bb843
.fib.bb844:
	li	a2, 1
	addi	a3, sp, 904
	sw	a2, 0(a3)
	j	.fib.bb846
.fib.bb847:
	li	t3, 1
	addi	a2, sp, 908
	sw	t3, 0(a2)
	j	.fib.bb849
.fib.bb850:
	li	t3, 1
	addi	a2, sp, 912
	sw	t3, 0(a2)
	j	.fib.bb852
.fib.bb853:
	li	t3, 1
	addi	a2, sp, 644
	sw	t3, 0(a2)
	j	.fib.bb855
.fib.bb856:
	li	t1, 1
	addi	t3, sp, 916
	sw	t1, 0(t3)
	j	.fib.bb858
.fib.bb859:
	li	t1, 1
	addi	t2, sp, 920
	sw	t1, 0(t2)
	j	.fib.bb861
.fib.bb862:
	li	t1, 1
	addi	t2, sp, 584
	sw	t1, 0(t2)
	j	.fib.bb864
.fib.bb865:
	li	t2, 1
	addi	t3, sp, 928
	sw	t2, 0(t3)
	j	.fib.bb867
.fib.bb868:
	li	t2, 1
	addi	t3, sp, 932
	sw	t2, 0(t3)
	j	.fib.bb870
.fib.bb871:
	li	t2, 1
	addi	t3, sp, 936
	sw	t2, 0(t3)
	j	.fib.bb873
.fib.bb874:
	li	t2, 1
	addi	t3, sp, 924
	sw	t2, 0(t3)
	j	.fib.bb876
.fib.bb877:
	li	a2, 1
	addi	a3, sp, 940
	sw	a2, 0(a3)
	j	.fib.bb879
.fib.bb880:
	li	t3, 1
	addi	a2, sp, 944
	sw	t3, 0(a2)
	j	.fib.bb882
.fib.bb883:
	li	t3, 1
	addi	a2, sp, 948
	sw	t3, 0(a2)
	j	.fib.bb885
.fib.bb886:
	li	t3, 1
	addi	a2, sp, 648
	sw	t3, 0(a2)
	j	.fib.bb888
.fib.bb889:
	li	t1, 1
	addi	t3, sp, 952
	sw	t1, 0(t3)
	j	.fib.bb891
.fib.bb892:
	li	t1, 1
	addi	t2, sp, 956
	sw	t1, 0(t2)
	j	.fib.bb894
.fib.bb895:
	li	t1, 1
	addi	t2, sp, 588
	sw	t1, 0(t2)
	j	.fib.bb897
.fib.bb898:
	li	t2, 1
	addi	t3, sp, 964
	sw	t2, 0(t3)
	j	.fib.bb900
.fib.bb901:
	li	t2, 1
	addi	t3, sp, 968
	sw	t2, 0(t3)
	j	.fib.bb903
.fib.bb904:
	li	t2, 1
	addi	t3, sp, 972
	sw	t2, 0(t3)
	j	.fib.bb906
.fib.bb907:
	li	t2, 1
	addi	t3, sp, 960
	sw	t2, 0(t3)
	j	.fib.bb909
.fib.bb910:
	li	a2, 1
	addi	a3, sp, 976
	sw	a2, 0(a3)
	j	.fib.bb912
.fib.bb913:
	li	t3, 1
	addi	a2, sp, 980
	sw	t3, 0(a2)
	j	.fib.bb915
.fib.bb916:
	li	t3, 1
	addi	a2, sp, 984
	sw	t3, 0(a2)
	j	.fib.bb918
.fib.bb919:
	li	t3, 1
	addi	a2, sp, 652
	sw	t3, 0(a2)
	j	.fib.bb921
.fib.bb922:
	li	t1, 1
	addi	t3, sp, 988
	sw	t1, 0(t3)
	j	.fib.bb924
.fib.bb925:
	li	t1, 1
	addi	t2, sp, 992
	sw	t1, 0(t2)
	j	.fib.bb927
.fib.bb928:
	li	t1, 1
	addi	t2, sp, 592
	sw	t1, 0(t2)
	j	.fib.bb930
.fib.bb931:
	li	t2, 1
	addi	t3, sp, 1000
	sw	t2, 0(t3)
	j	.fib.bb933
.fib.bb934:
	li	t2, 1
	addi	t3, sp, 1004
	sw	t2, 0(t3)
	j	.fib.bb936
.fib.bb937:
	li	t2, 1
	addi	t3, sp, 1008
	sw	t2, 0(t3)
	j	.fib.bb939
.fib.bb940:
	li	t2, 1
	addi	t3, sp, 996
	sw	t2, 0(t3)
	j	.fib.bb942
.fib.bb943:
	li	a2, 1
	addi	a3, sp, 1012
	sw	a2, 0(a3)
	j	.fib.bb945
.fib.bb946:
	li	t3, 1
	addi	a2, sp, 1016
	sw	t3, 0(a2)
	j	.fib.bb948
.fib.bb949:
	li	t3, 1
	addi	a2, sp, 1020
	sw	t3, 0(a2)
	j	.fib.bb951
.fib.bb952:
	li	t3, 1
	addi	a2, sp, 656
	sw	t3, 0(a2)
	j	.fib.bb954
.fib.bb955:
	li	t1, 1
	addi	t3, sp, 1024
	sw	t1, 0(t3)
	j	.fib.bb957
.fib.bb958:
	li	t1, 1
	addi	t2, sp, 1028
	sw	t1, 0(t2)
	j	.fib.bb960
.fib.bb961:
	li	t1, 1
	addi	t2, sp, 596
	sw	t1, 0(t2)
	j	.fib.bb963
.fib.bb964:
	li	t2, 1
	addi	t3, sp, 1036
	sw	t2, 0(t3)
	j	.fib.bb966
.fib.bb967:
	li	t2, 1
	addi	t3, sp, 1040
	sw	t2, 0(t3)
	j	.fib.bb969
.fib.bb970:
	li	t2, 1
	addi	t3, sp, 1044
	sw	t2, 0(t3)
	j	.fib.bb972
.fib.bb973:
	li	t2, 1
	addi	t3, sp, 1032
	sw	t2, 0(t3)
	j	.fib.bb975
.fib.bb976:
	li	a2, 1
	addi	a3, sp, 1048
	sw	a2, 0(a3)
	j	.fib.bb978
.fib.bb979:
	li	t3, 1
	addi	a2, sp, 1052
	sw	t3, 0(a2)
	j	.fib.bb981
.fib.bb982:
	li	t3, 1
	addi	a2, sp, 1056
	sw	t3, 0(a2)
	j	.fib.bb984
.fib.bb985:
	li	t3, 1
	addi	a2, sp, 660
	sw	t3, 0(a2)
	j	.fib.bb987
.fib.bb988:
	li	t1, 1
	addi	t3, sp, 1060
	sw	t1, 0(t3)
	j	.fib.bb990
.fib.bb991:
	li	t1, 1
	addi	t2, sp, 1064
	sw	t1, 0(t2)
	j	.fib.bb993
.fib.bb994:
	li	t1, 1
	addi	t2, sp, 600
	sw	t1, 0(t2)
	j	.fib.bb996
.fib.bb997:
	li	t2, 1
	addi	t3, sp, 1072
	sw	t2, 0(t3)
	j	.fib.bb999
.fib.bb1000:
	li	t2, 1
	addi	t3, sp, 1076
	sw	t2, 0(t3)
	j	.fib.bb1002
.fib.bb1003:
	li	t2, 1
	addi	t3, sp, 1080
	sw	t2, 0(t3)
	j	.fib.bb1005
.fib.bb1006:
	li	t2, 1
	addi	t3, sp, 1068
	sw	t2, 0(t3)
	j	.fib.bb1008
.fib.bb1009:
	li	a2, 1
	addi	a3, sp, 1084
	sw	a2, 0(a3)
	j	.fib.bb1011
.fib.bb1012:
	li	t3, 1
	addi	a2, sp, 1088
	sw	t3, 0(a2)
	j	.fib.bb1014
.fib.bb1015:
	li	t3, 1
	addi	a2, sp, 1092
	sw	t3, 0(a2)
	j	.fib.bb1017
.fib.bb1018:
	li	t3, 1
	addi	a2, sp, 664
	sw	t3, 0(a2)
	j	.fib.bb1020
.fib.bb1021:
	li	t1, 1
	addi	t3, sp, 1096
	sw	t1, 0(t3)
	j	.fib.bb1023
.fib.bb1024:
	li	t1, 1
	addi	t2, sp, 1100
	sw	t1, 0(t2)
	j	.fib.bb1026
.fib.bb1027:
	li	t1, 1
	addi	t2, sp, 604
	sw	t1, 0(t2)
	j	.fib.bb1029
.fib.bb1030:
	li	t2, 1
	addi	t3, sp, 1108
	sw	t2, 0(t3)
	j	.fib.bb1032
.fib.bb1033:
	li	t2, 1
	addi	t3, sp, 1112
	sw	t2, 0(t3)
	j	.fib.bb1035
.fib.bb1036:
	li	t2, 1
	addi	t3, sp, 1116
	sw	t2, 0(t3)
	j	.fib.bb1038
.fib.bb1039:
	li	t2, 1
	addi	t3, sp, 1104
	sw	t2, 0(t3)
	j	.fib.bb1041
.fib.bb1042:
	li	a2, 1
	addi	a3, sp, 1120
	sw	a2, 0(a3)
	j	.fib.bb1044
.fib.bb1045:
	li	t3, 1
	addi	a2, sp, 1124
	sw	t3, 0(a2)
	j	.fib.bb1047
.fib.bb1048:
	li	t3, 1
	addi	a2, sp, 1128
	sw	t3, 0(a2)
	j	.fib.bb1050
.fib.bb1051:
	li	t3, 1
	addi	a2, sp, 668
	sw	t3, 0(a2)
	j	.fib.bb1053
.fib.bb1054:
	li	t1, 1
	addi	t3, sp, 1132
	sw	t1, 0(t3)
	j	.fib.bb1056
.fib.bb1057:
	li	t1, 1
	addi	t2, sp, 1136
	sw	t1, 0(t2)
	j	.fib.bb1059
.fib.bb1060:
	li	t1, 1
	addi	t2, sp, 608
	sw	t1, 0(t2)
	j	.fib.bb1062
.fib.bb1063:
	li	t2, 1
	addi	t3, sp, 1144
	sw	t2, 0(t3)
	j	.fib.bb1065
.fib.bb1066:
	li	t2, 1
	addi	t3, sp, 1148
	sw	t2, 0(t3)
	j	.fib.bb1068
.fib.bb1069:
	li	t2, 1
	addi	t3, sp, 1152
	sw	t2, 0(t3)
	j	.fib.bb1071
.fib.bb1072:
	li	t2, 1
	addi	t3, sp, 1140
	sw	t2, 0(t3)
	j	.fib.bb1074
.fib.bb1075:
	li	a2, 1
	addi	a3, sp, 1156
	sw	a2, 0(a3)
	j	.fib.bb1077
.fib.bb1078:
	li	t3, 1
	addi	a2, sp, 1160
	sw	t3, 0(a2)
	j	.fib.bb1080
.fib.bb1081:
	li	t3, 1
	addi	a2, sp, 1164
	sw	t3, 0(a2)
	j	.fib.bb1083
.fib.bb1084:
	li	t3, 1
	addi	a2, sp, 672
	sw	t3, 0(a2)
	j	.fib.bb1086
.fib.bb1087:
	li	t1, 1
	addi	t3, sp, 1168
	sw	t1, 0(t3)
	j	.fib.bb1089
.fib.bb1090:
	li	t1, 1
	addi	t2, sp, 1172
	sw	t1, 0(t2)
	j	.fib.bb1092
.fib.bb1093:
	li	t1, 1
	addi	t2, sp, 612
	sw	t1, 0(t2)
	j	.fib.bb1095
.fib.bb1096:
	li	t2, 1
	addi	t3, sp, 1180
	sw	t2, 0(t3)
	j	.fib.bb1098
.fib.bb1099:
	li	t2, 1
	addi	t3, sp, 1184
	sw	t2, 0(t3)
	j	.fib.bb1101
.fib.bb1102:
	li	t2, 1
	addi	t3, sp, 1188
	sw	t2, 0(t3)
	j	.fib.bb1104
.fib.bb1105:
	li	t2, 1
	addi	t3, sp, 1176
	sw	t2, 0(t3)
	j	.fib.bb1107
.fib.bb1108:
	li	a2, 1
	addi	a3, sp, 1192
	sw	a2, 0(a3)
	j	.fib.bb1110
.fib.bb1111:
	li	t3, 1
	addi	a2, sp, 1196
	sw	t3, 0(a2)
	j	.fib.bb1113
.fib.bb1114:
	li	t3, 1
	addi	a2, sp, 1200
	sw	t3, 0(a2)
	j	.fib.bb1116
.fib.bb1117:
	li	t3, 1
	addi	a2, sp, 676
	sw	t3, 0(a2)
	j	.fib.bb1119
.fib.bb1120:
	li	t1, 1
	addi	t3, sp, 1204
	sw	t1, 0(t3)
	j	.fib.bb1122
.fib.bb1123:
	li	t1, 1
	addi	t2, sp, 1208
	sw	t1, 0(t2)
	j	.fib.bb1125
.fib.bb1126:
	li	t1, 1
	addi	t2, sp, 616
	sw	t1, 0(t2)
	j	.fib.bb1128
.fib.bb1129:
	li	t1, 1
	addi	t2, sp, 1216
	sw	t1, 0(t2)
	j	.fib.bb1131
.fib.bb1132:
	li	t1, 1
	addi	t2, sp, 1220
	sw	t1, 0(t2)
	j	.fib.bb1134
.fib.bb1135:
	li	t1, 1
	addi	t2, sp, 1224
	sw	t1, 0(t2)
	j	.fib.bb1137
.fib.bb1138:
	li	t1, 1
	addi	t2, sp, 1212
	sw	t1, 0(t2)
	j	.fib.bb1140
.fib.bb1141:
	li	t3, 1
	addi	a2, sp, 1228
	sw	t3, 0(a2)
	j	.fib.bb1143
.fib.bb1144:
	li	t2, 1
	addi	t3, sp, 1232
	sw	t2, 0(t3)
	j	.fib.bb1146
.fib.bb1147:
	li	t2, 1
	addi	t3, sp, 1236
	sw	t2, 0(t3)
	j	.fib.bb1149
.fib.bb1150:
	li	t2, 1
	addi	t3, sp, 680
	sw	t2, 0(t3)
	j	.fib.bb1152
.fib.bb1153:
	li	t0, 1
	addi	t2, sp, 1240
	sw	t0, 0(t2)
	j	.fib.bb1155
.fib.bb1156:
	li	t0, 1
	addi	t1, sp, 1244
	sw	t0, 0(t1)
	j	.fib.bb1158
.fib.bb1159:
	j	.fib.bb1161
.fib.bb1169:
	subw	a5, zero, a5
	addi	a6, sp, 1252
	sw	a5, 0(a6)
	j	.fib.bb1170
.fib.bb1171:
	subw	a5, zero, a5
	addi	a6, sp, 1256
	sw	a5, 0(a6)
	j	.fib.bb1172
.fib.bb1173:
	subw	a5, zero, a5
	addi	a6, sp, 1260
	sw	a5, 0(a6)
	j	.fib.bb1174
.fib.bb1175:
	subw	a5, zero, a5
	addi	a6, sp, 1264
	sw	a5, 0(a6)
	j	.fib.bb1176
.fib.bb1177:
	subw	a5, zero, a5
	addi	a6, sp, 1268
	sw	a5, 0(a6)
	j	.fib.bb1178
.fib.bb1179:
	subw	a5, zero, a5
	addi	a6, sp, 1272
	sw	a5, 0(a6)
	j	.fib.bb1180
.fib.bb1181:
	subw	a5, zero, a5
	addi	a6, sp, 1276
	sw	a5, 0(a6)
	j	.fib.bb1182
.fib.bb1183:
	subw	a5, zero, a5
	addi	a6, sp, 1280
	sw	a5, 0(a6)
	j	.fib.bb1184
.fib.bb1185:
	subw	a5, zero, a5
	addi	a6, sp, 1284
	sw	a5, 0(a6)
	j	.fib.bb1186
.fib.bb1187:
	subw	t2, zero, t2
	addi	a5, sp, 1288
	sw	t2, 0(a5)
	j	.fib.bb1188
.fib.bb1189:
	subw	t2, zero, t2
	addi	a5, sp, 1292
	sw	t2, 0(a5)
	j	.fib.bb1190
.fib.bb1191:
	subw	t2, zero, t2
	addi	a5, sp, 1296
	sw	t2, 0(a5)
	j	.fib.bb1192
.fib.bb1193:
	subw	t2, zero, t2
	addi	a5, sp, 1300
	sw	t2, 0(a5)
	j	.fib.bb1194
.fib.bb1195:
	subw	t2, zero, t2
	addi	a5, sp, 1304
	sw	t2, 0(a5)
	j	.fib.bb1196
.fib.bb1197:
	subw	t2, zero, t2
	addi	a5, sp, 1308
	sw	t2, 0(a5)
	j	.fib.bb1198
.fib.bb1199:
	subw	t2, zero, t2
	addi	a5, sp, 1312
	sw	t2, 0(a5)
	j	.fib.bb1200
.fib.bb1201:
	subw	t2, zero, t2
	addi	a5, sp, 1316
	sw	t2, 0(a5)
	j	.fib.bb1202
.fib.bb1203:
	subw	t2, zero, t2
	addi	a5, sp, 1320
	sw	t2, 0(a5)
	j	.fib.bb1204
.fib.bb1205:
	subw	t2, zero, t2
	addi	a5, sp, 1324
	sw	t2, 0(a5)
	j	.fib.bb1206
.fib.bb1207:
	subw	t2, zero, t2
	addi	a5, sp, 1328
	sw	t2, 0(a5)
	j	.fib.bb1208
.fib.bb1209:
	subw	t2, zero, t2
	addi	a5, sp, 1332
	sw	t2, 0(a5)
	j	.fib.bb1210
.fib.bb1211:
	subw	t2, zero, t2
	addi	a5, sp, 1336
	sw	t2, 0(a5)
	j	.fib.bb1212
.fib.bb1213:
	subw	t2, zero, t2
	addi	a5, sp, 1340
	sw	t2, 0(a5)
	j	.fib.bb1214
.fib.bb1215:
	subw	t2, zero, t2
	addi	a5, sp, 1344
	sw	t2, 0(a5)
	j	.fib.bb1216
.fib.bb1217:
	subw	t2, zero, t2
	addi	a5, sp, 1348
	sw	t2, 0(a5)
	j	.fib.bb1218
.fib.bb1219:
	subw	t0, zero, t0
	addi	t2, sp, 1352
	sw	t0, 0(t2)
	j	.fib.bb1220
.fib.bb1221:
	li	t2, 1
	addi	a5, sp, 1484
	sw	t2, 0(a5)
	j	.fib.bb1223
.fib.bb1224:
	li	t1, 1
	addi	t2, sp, 1488
	sw	t1, 0(t2)
	j	.fib.bb1226
.fib.bb1227:
	li	t1, 1
	addi	t2, sp, 1492
	sw	t1, 0(t2)
	j	.fib.bb1229
.fib.bb1230:
	li	t1, 1
	addi	t2, sp, 1480
	sw	t1, 0(t2)
	j	.fib.bb1232
.fib.bb1233:
	li	t1, 1
	addi	t2, sp, 1496
	sw	t1, 0(t2)
	j	.fib.bb1235
.fib.bb1236:
	li	t1, 1
	addi	t2, sp, 1416
	sw	t1, 0(t2)
	j	.fib.bb1238
.fib.bb1239:
	li	t0, 1
	addi	t1, sp, 1500
	sw	t0, 0(t1)
	j	.fib.bb1241
.fib.bb1242:
	li	t0, 1
	addi	t1, sp, 1356
	sw	t0, 0(t1)
	j	.fib.bb1244
.fib.bb1245:
	li	t2, 1
	addi	t3, sp, 1508
	sw	t2, 0(t3)
	j	.fib.bb1247
.fib.bb1248:
	li	t1, 1
	addi	t2, sp, 1512
	sw	t1, 0(t2)
	j	.fib.bb1250
.fib.bb1251:
	li	t1, 1
	addi	t2, sp, 1516
	sw	t1, 0(t2)
	j	.fib.bb1253
.fib.bb1254:
	li	t1, 1
	addi	t2, sp, 1504
	sw	t1, 0(t2)
	j	.fib.bb1256
.fib.bb1257:
	li	t3, 1
	addi	a5, sp, 1520
	sw	t3, 0(a5)
	j	.fib.bb1259
.fib.bb1260:
	li	t2, 1
	addi	t3, sp, 1524
	sw	t2, 0(t3)
	j	.fib.bb1262
.fib.bb1263:
	li	t2, 1
	addi	t3, sp, 1528
	sw	t2, 0(t3)
	j	.fib.bb1265
.fib.bb1266:
	li	t2, 1
	addi	t3, sp, 1420
	sw	t2, 0(t3)
	j	.fib.bb1268
.fib.bb1269:
	li	t0, 1
	addi	t2, sp, 1532
	sw	t0, 0(t2)
	j	.fib.bb1271
.fib.bb1272:
	li	t0, 1
	addi	t1, sp, 1536
	sw	t0, 0(t1)
	j	.fib.bb1274
.fib.bb1275:
	li	t0, 1
	addi	t1, sp, 1360
	sw	t0, 0(t1)
	j	.fib.bb1277
.fib.bb1278:
	li	t2, 1
	addi	t3, sp, 1544
	sw	t2, 0(t3)
	j	.fib.bb1280
.fib.bb1281:
	li	t1, 1
	addi	t2, sp, 1548
	sw	t1, 0(t2)
	j	.fib.bb1283
.fib.bb1284:
	li	t1, 1
	addi	t2, sp, 1552
	sw	t1, 0(t2)
	j	.fib.bb1286
.fib.bb1287:
	li	t1, 1
	addi	t2, sp, 1540
	sw	t1, 0(t2)
	j	.fib.bb1289
.fib.bb1290:
	li	t3, 1
	addi	a2, sp, 1556
	sw	t3, 0(a2)
	j	.fib.bb1292
.fib.bb1293:
	li	t2, 1
	addi	t3, sp, 1560
	sw	t2, 0(t3)
	j	.fib.bb1295
.fib.bb1296:
	li	t2, 1
	addi	t3, sp, 1564
	sw	t2, 0(t3)
	j	.fib.bb1298
.fib.bb1299:
	li	t2, 1
	addi	t3, sp, 1424
	sw	t2, 0(t3)
	j	.fib.bb1301
.fib.bb1302:
	li	t0, 1
	addi	t2, sp, 1568
	sw	t0, 0(t2)
	j	.fib.bb1304
.fib.bb1305:
	li	t0, 1
	addi	t1, sp, 1572
	sw	t0, 0(t1)
	j	.fib.bb1307
.fib.bb1308:
	li	t0, 1
	addi	t1, sp, 1364
	sw	t0, 0(t1)
	j	.fib.bb1310
.fib.bb1311:
	li	t2, 1
	addi	t3, sp, 1580
	sw	t2, 0(t3)
	j	.fib.bb1313
.fib.bb1314:
	li	t1, 1
	addi	t2, sp, 1584
	sw	t1, 0(t2)
	j	.fib.bb1316
.fib.bb1317:
	li	t1, 1
	addi	t2, sp, 1588
	sw	t1, 0(t2)
	j	.fib.bb1319
.fib.bb1320:
	li	t1, 1
	addi	t2, sp, 1576
	sw	t1, 0(t2)
	j	.fib.bb1322
.fib.bb1323:
	li	t3, 1
	addi	a2, sp, 1592
	sw	t3, 0(a2)
	j	.fib.bb1325
.fib.bb1326:
	li	t2, 1
	addi	t3, sp, 1596
	sw	t2, 0(t3)
	j	.fib.bb1328
.fib.bb1329:
	li	t2, 1
	addi	t3, sp, 1600
	sw	t2, 0(t3)
	j	.fib.bb1331
.fib.bb1332:
	li	t2, 1
	addi	t3, sp, 1428
	sw	t2, 0(t3)
	j	.fib.bb1334
.fib.bb1335:
	li	t0, 1
	addi	t2, sp, 1604
	sw	t0, 0(t2)
	j	.fib.bb1337
.fib.bb1338:
	li	t0, 1
	addi	t1, sp, 1608
	sw	t0, 0(t1)
	j	.fib.bb1340
.fib.bb1341:
	li	t0, 1
	addi	t1, sp, 1368
	sw	t0, 0(t1)
	j	.fib.bb1343
.fib.bb1344:
	li	t2, 1
	addi	t3, sp, 1616
	sw	t2, 0(t3)
	j	.fib.bb1346
.fib.bb1347:
	li	t1, 1
	addi	t2, sp, 1620
	sw	t1, 0(t2)
	j	.fib.bb1349
.fib.bb1350:
	li	t1, 1
	addi	t2, sp, 1624
	sw	t1, 0(t2)
	j	.fib.bb1352
.fib.bb1353:
	li	t1, 1
	addi	t2, sp, 1612
	sw	t1, 0(t2)
	j	.fib.bb1355
.fib.bb1356:
	li	t3, 1
	addi	a2, sp, 1628
	sw	t3, 0(a2)
	j	.fib.bb1358
.fib.bb1359:
	li	t2, 1
	addi	t3, sp, 1632
	sw	t2, 0(t3)
	j	.fib.bb1361
.fib.bb1362:
	li	t2, 1
	addi	t3, sp, 1636
	sw	t2, 0(t3)
	j	.fib.bb1364
.fib.bb1365:
	li	t2, 1
	addi	t3, sp, 1432
	sw	t2, 0(t3)
	j	.fib.bb1367
.fib.bb1368:
	li	t0, 1
	addi	t2, sp, 1640
	sw	t0, 0(t2)
	j	.fib.bb1370
.fib.bb1371:
	li	t0, 1
	addi	t1, sp, 1644
	sw	t0, 0(t1)
	j	.fib.bb1373
.fib.bb1374:
	li	t0, 1
	addi	t1, sp, 1372
	sw	t0, 0(t1)
	j	.fib.bb1376
.fib.bb1377:
	li	t2, 1
	addi	t3, sp, 1652
	sw	t2, 0(t3)
	j	.fib.bb1379
.fib.bb1380:
	li	t1, 1
	addi	t2, sp, 1656
	sw	t1, 0(t2)
	j	.fib.bb1382
.fib.bb1383:
	li	t1, 1
	addi	t2, sp, 1660
	sw	t1, 0(t2)
	j	.fib.bb1385
.fib.bb1386:
	li	t1, 1
	addi	t2, sp, 1648
	sw	t1, 0(t2)
	j	.fib.bb1388
.fib.bb1389:
	li	t3, 1
	addi	a2, sp, 1664
	sw	t3, 0(a2)
	j	.fib.bb1391
.fib.bb1392:
	li	t2, 1
	addi	t3, sp, 1668
	sw	t2, 0(t3)
	j	.fib.bb1394
.fib.bb1395:
	li	t2, 1
	addi	t3, sp, 1672
	sw	t2, 0(t3)
	j	.fib.bb1397
.fib.bb1398:
	li	t2, 1
	addi	t3, sp, 1436
	sw	t2, 0(t3)
	j	.fib.bb1400
.fib.bb1401:
	li	t0, 1
	addi	t2, sp, 1676
	sw	t0, 0(t2)
	j	.fib.bb1403
.fib.bb1404:
	li	t0, 1
	addi	t1, sp, 1680
	sw	t0, 0(t1)
	j	.fib.bb1406
.fib.bb1407:
	li	t0, 1
	addi	t1, sp, 1376
	sw	t0, 0(t1)
	j	.fib.bb1409
.fib.bb1410:
	li	t2, 1
	addi	t3, sp, 1688
	sw	t2, 0(t3)
	j	.fib.bb1412
.fib.bb1413:
	li	t1, 1
	addi	t2, sp, 1692
	sw	t1, 0(t2)
	j	.fib.bb1415
.fib.bb1416:
	li	t1, 1
	addi	t2, sp, 1696
	sw	t1, 0(t2)
	j	.fib.bb1418
.fib.bb1419:
	li	t1, 1
	addi	t2, sp, 1684
	sw	t1, 0(t2)
	j	.fib.bb1421
.fib.bb1422:
	li	t3, 1
	addi	a2, sp, 1700
	sw	t3, 0(a2)
	j	.fib.bb1424
.fib.bb1425:
	li	t2, 1
	addi	t3, sp, 1704
	sw	t2, 0(t3)
	j	.fib.bb1427
.fib.bb1428:
	li	t2, 1
	addi	t3, sp, 1708
	sw	t2, 0(t3)
	j	.fib.bb1430
.fib.bb1431:
	li	t2, 1
	addi	t3, sp, 1440
	sw	t2, 0(t3)
	j	.fib.bb1433
.fib.bb1434:
	li	t0, 1
	addi	t2, sp, 1712
	sw	t0, 0(t2)
	j	.fib.bb1436
.fib.bb1437:
	li	t0, 1
	addi	t1, sp, 1716
	sw	t0, 0(t1)
	j	.fib.bb1439
.fib.bb1440:
	li	t0, 1
	addi	t1, sp, 1380
	sw	t0, 0(t1)
	j	.fib.bb1442
.fib.bb1443:
	li	t2, 1
	addi	t3, sp, 1724
	sw	t2, 0(t3)
	j	.fib.bb1445
.fib.bb1446:
	li	t1, 1
	addi	t2, sp, 1728
	sw	t1, 0(t2)
	j	.fib.bb1448
.fib.bb1449:
	li	t1, 1
	addi	t2, sp, 1732
	sw	t1, 0(t2)
	j	.fib.bb1451
.fib.bb1452:
	li	t1, 1
	addi	t2, sp, 1720
	sw	t1, 0(t2)
	j	.fib.bb1454
.fib.bb1455:
	li	t3, 1
	addi	a2, sp, 1736
	sw	t3, 0(a2)
	j	.fib.bb1457
.fib.bb1458:
	li	t2, 1
	addi	t3, sp, 1740
	sw	t2, 0(t3)
	j	.fib.bb1460
.fib.bb1461:
	li	t2, 1
	addi	t3, sp, 1744
	sw	t2, 0(t3)
	j	.fib.bb1463
.fib.bb1464:
	li	t2, 1
	addi	t3, sp, 1444
	sw	t2, 0(t3)
	j	.fib.bb1466
.fib.bb1467:
	li	t0, 1
	addi	t2, sp, 1748
	sw	t0, 0(t2)
	j	.fib.bb1469
.fib.bb1470:
	li	t0, 1
	addi	t1, sp, 1752
	sw	t0, 0(t1)
	j	.fib.bb1472
.fib.bb1473:
	li	t0, 1
	addi	t1, sp, 1384
	sw	t0, 0(t1)
	j	.fib.bb1475
.fib.bb1476:
	li	t2, 1
	addi	t3, sp, 1760
	sw	t2, 0(t3)
	j	.fib.bb1478
.fib.bb1479:
	li	t1, 1
	addi	t2, sp, 1764
	sw	t1, 0(t2)
	j	.fib.bb1481
.fib.bb1482:
	li	t1, 1
	addi	t2, sp, 1768
	sw	t1, 0(t2)
	j	.fib.bb1484
.fib.bb1485:
	li	t1, 1
	addi	t2, sp, 1756
	sw	t1, 0(t2)
	j	.fib.bb1487
.fib.bb1488:
	li	t3, 1
	addi	a2, sp, 1772
	sw	t3, 0(a2)
	j	.fib.bb1490
.fib.bb1491:
	li	t2, 1
	addi	t3, sp, 1776
	sw	t2, 0(t3)
	j	.fib.bb1493
.fib.bb1494:
	li	t2, 1
	addi	t3, sp, 1780
	sw	t2, 0(t3)
	j	.fib.bb1496
.fib.bb1497:
	li	t2, 1
	addi	t3, sp, 1448
	sw	t2, 0(t3)
	j	.fib.bb1499
.fib.bb1500:
	li	t0, 1
	addi	t2, sp, 1784
	sw	t0, 0(t2)
	j	.fib.bb1502
.fib.bb1503:
	li	t0, 1
	addi	t1, sp, 1788
	sw	t0, 0(t1)
	j	.fib.bb1505
.fib.bb1506:
	li	t0, 1
	addi	t1, sp, 1388
	sw	t0, 0(t1)
	j	.fib.bb1508
.fib.bb1509:
	li	t2, 1
	addi	t3, sp, 1796
	sw	t2, 0(t3)
	j	.fib.bb1511
.fib.bb1512:
	li	t1, 1
	addi	t2, sp, 1800
	sw	t1, 0(t2)
	j	.fib.bb1514
.fib.bb1515:
	li	t1, 1
	addi	t2, sp, 1804
	sw	t1, 0(t2)
	j	.fib.bb1517
.fib.bb1518:
	li	t1, 1
	addi	t2, sp, 1792
	sw	t1, 0(t2)
	j	.fib.bb1520
.fib.bb1521:
	li	t3, 1
	addi	a2, sp, 1808
	sw	t3, 0(a2)
	j	.fib.bb1523
.fib.bb1524:
	li	t2, 1
	addi	t3, sp, 1812
	sw	t2, 0(t3)
	j	.fib.bb1526
.fib.bb1527:
	li	t2, 1
	addi	t3, sp, 1816
	sw	t2, 0(t3)
	j	.fib.bb1529
.fib.bb1530:
	li	t2, 1
	addi	t3, sp, 1452
	sw	t2, 0(t3)
	j	.fib.bb1532
.fib.bb1533:
	li	t0, 1
	addi	t2, sp, 1820
	sw	t0, 0(t2)
	j	.fib.bb1535
.fib.bb1536:
	li	t0, 1
	addi	t1, sp, 1824
	sw	t0, 0(t1)
	j	.fib.bb1538
.fib.bb1539:
	li	t0, 1
	addi	t1, sp, 1392
	sw	t0, 0(t1)
	j	.fib.bb1541
.fib.bb1542:
	li	t2, 1
	addi	t3, sp, 1832
	sw	t2, 0(t3)
	j	.fib.bb1544
.fib.bb1545:
	li	t1, 1
	addi	t2, sp, 1836
	sw	t1, 0(t2)
	j	.fib.bb1547
.fib.bb1548:
	li	t1, 1
	addi	t2, sp, 1840
	sw	t1, 0(t2)
	j	.fib.bb1550
.fib.bb1551:
	li	t1, 1
	addi	t2, sp, 1828
	sw	t1, 0(t2)
	j	.fib.bb1553
.fib.bb1554:
	li	t3, 1
	addi	a2, sp, 1844
	sw	t3, 0(a2)
	j	.fib.bb1556
.fib.bb1557:
	li	t2, 1
	addi	t3, sp, 1848
	sw	t2, 0(t3)
	j	.fib.bb1559
.fib.bb1560:
	li	t2, 1
	addi	t3, sp, 1852
	sw	t2, 0(t3)
	j	.fib.bb1562
.fib.bb1563:
	li	t2, 1
	addi	t3, sp, 1456
	sw	t2, 0(t3)
	j	.fib.bb1565
.fib.bb1566:
	li	t0, 1
	addi	t2, sp, 1856
	sw	t0, 0(t2)
	j	.fib.bb1568
.fib.bb1569:
	li	t0, 1
	addi	t1, sp, 1860
	sw	t0, 0(t1)
	j	.fib.bb1571
.fib.bb1572:
	li	t0, 1
	addi	t1, sp, 1396
	sw	t0, 0(t1)
	j	.fib.bb1574
.fib.bb1575:
	li	t2, 1
	addi	t3, sp, 1868
	sw	t2, 0(t3)
	j	.fib.bb1577
.fib.bb1578:
	li	t1, 1
	addi	t2, sp, 1872
	sw	t1, 0(t2)
	j	.fib.bb1580
.fib.bb1581:
	li	t1, 1
	addi	t2, sp, 1876
	sw	t1, 0(t2)
	j	.fib.bb1583
.fib.bb1584:
	li	t1, 1
	addi	t2, sp, 1864
	sw	t1, 0(t2)
	j	.fib.bb1586
.fib.bb1587:
	li	t3, 1
	addi	a2, sp, 1880
	sw	t3, 0(a2)
	j	.fib.bb1589
.fib.bb1590:
	li	t2, 1
	addi	t3, sp, 1884
	sw	t2, 0(t3)
	j	.fib.bb1592
.fib.bb1593:
	li	t2, 1
	addi	t3, sp, 1888
	sw	t2, 0(t3)
	j	.fib.bb1595
.fib.bb1596:
	li	t2, 1
	addi	t3, sp, 1460
	sw	t2, 0(t3)
	j	.fib.bb1598
.fib.bb1599:
	li	t0, 1
	addi	t2, sp, 1892
	sw	t0, 0(t2)
	j	.fib.bb1601
.fib.bb1602:
	li	t0, 1
	addi	t1, sp, 1896
	sw	t0, 0(t1)
	j	.fib.bb1604
.fib.bb1605:
	li	t0, 1
	addi	t1, sp, 1400
	sw	t0, 0(t1)
	j	.fib.bb1607
.fib.bb1608:
	li	t2, 1
	addi	t3, sp, 1904
	sw	t2, 0(t3)
	j	.fib.bb1610
.fib.bb1611:
	li	t1, 1
	addi	t2, sp, 1908
	sw	t1, 0(t2)
	j	.fib.bb1613
.fib.bb1614:
	li	t1, 1
	addi	t2, sp, 1912
	sw	t1, 0(t2)
	j	.fib.bb1616
.fib.bb1617:
	li	t1, 1
	addi	t2, sp, 1900
	sw	t1, 0(t2)
	j	.fib.bb1619
.fib.bb1620:
	li	t3, 1
	addi	a2, sp, 1916
	sw	t3, 0(a2)
	j	.fib.bb1622
.fib.bb1623:
	li	t2, 1
	addi	t3, sp, 1920
	sw	t2, 0(t3)
	j	.fib.bb1625
.fib.bb1626:
	li	t2, 1
	addi	t3, sp, 1924
	sw	t2, 0(t3)
	j	.fib.bb1628
.fib.bb1629:
	li	t2, 1
	addi	t3, sp, 1464
	sw	t2, 0(t3)
	j	.fib.bb1631
.fib.bb1632:
	li	t0, 1
	addi	t2, sp, 1928
	sw	t0, 0(t2)
	j	.fib.bb1634
.fib.bb1635:
	li	t0, 1
	addi	t1, sp, 1932
	sw	t0, 0(t1)
	j	.fib.bb1637
.fib.bb1638:
	li	t0, 1
	addi	t1, sp, 1404
	sw	t0, 0(t1)
	j	.fib.bb1640
.fib.bb1641:
	li	t2, 1
	addi	t3, sp, 1940
	sw	t2, 0(t3)
	j	.fib.bb1643
.fib.bb1644:
	li	t1, 1
	addi	t2, sp, 1944
	sw	t1, 0(t2)
	j	.fib.bb1646
.fib.bb1647:
	li	t1, 1
	addi	t2, sp, 1948
	sw	t1, 0(t2)
	j	.fib.bb1649
.fib.bb1650:
	li	t1, 1
	addi	t2, sp, 1936
	sw	t1, 0(t2)
	j	.fib.bb1652
.fib.bb1653:
	li	t3, 1
	addi	a2, sp, 1952
	sw	t3, 0(a2)
	j	.fib.bb1655
.fib.bb1656:
	li	t2, 1
	addi	t3, sp, 1956
	sw	t2, 0(t3)
	j	.fib.bb1658
.fib.bb1659:
	li	t2, 1
	addi	t3, sp, 1960
	sw	t2, 0(t3)
	j	.fib.bb1661
.fib.bb1662:
	li	t2, 1
	addi	t3, sp, 1468
	sw	t2, 0(t3)
	j	.fib.bb1664
.fib.bb1665:
	li	t0, 1
	addi	t2, sp, 1964
	sw	t0, 0(t2)
	j	.fib.bb1667
.fib.bb1668:
	li	t0, 1
	addi	t1, sp, 1968
	sw	t0, 0(t1)
	j	.fib.bb1670
.fib.bb1671:
	li	t0, 1
	addi	t1, sp, 1408
	sw	t0, 0(t1)
	j	.fib.bb1673
.fib.bb1674:
	li	t2, 1
	addi	t3, sp, 1976
	sw	t2, 0(t3)
	j	.fib.bb1676
.fib.bb1677:
	li	t1, 1
	addi	t2, sp, 1980
	sw	t1, 0(t2)
	j	.fib.bb1679
.fib.bb1680:
	li	t1, 1
	addi	t2, sp, 1984
	sw	t1, 0(t2)
	j	.fib.bb1682
.fib.bb1683:
	li	t1, 1
	addi	t2, sp, 1972
	sw	t1, 0(t2)
	j	.fib.bb1685
.fib.bb1686:
	li	t3, 1
	addi	a2, sp, 1988
	sw	t3, 0(a2)
	j	.fib.bb1688
.fib.bb1689:
	li	t2, 1
	addi	t3, sp, 1992
	sw	t2, 0(t3)
	j	.fib.bb1691
.fib.bb1692:
	li	t2, 1
	addi	t3, sp, 1996
	sw	t2, 0(t3)
	j	.fib.bb1694
.fib.bb1695:
	li	t2, 1
	addi	t3, sp, 1472
	sw	t2, 0(t3)
	j	.fib.bb1697
.fib.bb1698:
	li	t0, 1
	addi	t2, sp, 2000
	sw	t0, 0(t2)
	j	.fib.bb1700
.fib.bb1701:
	li	t0, 1
	addi	t1, sp, 2004
	sw	t0, 0(t1)
	j	.fib.bb1703
.fib.bb1704:
	li	t0, 1
	addi	t1, sp, 1412
	sw	t0, 0(t1)
	j	.fib.bb1706
.fib.bb1707:
	li	t2, 1
	addi	t3, sp, 2012
	sw	t2, 0(t3)
	j	.fib.bb1709
.fib.bb1710:
	li	t1, 1
	addi	t2, sp, 2016
	sw	t1, 0(t2)
	j	.fib.bb1712
.fib.bb1713:
	li	t1, 1
	addi	t2, sp, 2020
	sw	t1, 0(t2)
	j	.fib.bb1715
.fib.bb1716:
	li	t1, 1
	addi	t2, sp, 2008
	sw	t1, 0(t2)
	j	.fib.bb1718
.fib.bb1719:
	li	t3, 1
	addi	a2, sp, 2024
	sw	t3, 0(a2)
	j	.fib.bb1721
.fib.bb1722:
	li	t2, 1
	addi	t3, sp, 2028
	sw	t2, 0(t3)
	j	.fib.bb1724
.fib.bb1725:
	li	t2, 1
	addi	t3, sp, 2032
	sw	t2, 0(t3)
	j	.fib.bb1727
.fib.bb1728:
	li	t2, 1
	addi	t3, sp, 1476
	sw	t2, 0(t3)
	j	.fib.bb1730
.fib.bb1731:
	li	t0, 1
	addi	t2, sp, 2036
	sw	t0, 0(t2)
	j	.fib.bb1733
.fib.bb1734:
	li	t0, 1
	addi	t1, sp, 2040
	sw	t0, 0(t1)
	j	.fib.bb1736
.fib.bb1737:
	j	.fib.bb1739
.fib.bb207:
	j	.fib.bb209
.fib.bb414:
	j	.fib.bb416
.fib.bb529:
	li	a0, 1
	addi	a1, sp, 468
	sw	a0, 0(a1)
	j	.fib.bb531
.fib.bb545:
	li	t0, 1
	addi	a0, sp, 484
	sw	t0, 0(a0)
	j	.fib.bb547
.fib.bb549:
	li	a0, 1
	addi	a1, sp, 492
	sw	a0, 0(a1)
	j	.fib.bb551
.fib.bb565:
	li	t0, 1
	addi	a0, sp, 508
	sw	t0, 0(a0)
	j	.fib.bb567
.fib.bb569:
	li	a0, 1
	addi	a1, sp, 516
	sw	a0, 0(a1)
	j	.fib.bb571
.fib.bb585:
	li	t0, 1
	addi	a0, sp, 532
	sw	t0, 0(a0)
	j	.fib.bb587
.fib.bb589:
	li	a0, 1
	addi	a1, sp, 540
	sw	a0, 0(a1)
	j	.fib.bb591
.fib.bb605:
	li	t0, 1
	addi	a0, sp, 556
	sw	t0, 0(a0)
	j	.fib.bb607
.fib.bb643:
	li	t4, 1
	addi	a0, sp, 688
	sw	t4, 0(a0)
	j	.fib.bb645
.fib.bb1167:
	subw	a5, zero, a5
	addi	a6, sp, 1248
	sw	a5, 0(a6)
	j	.fib.bb1168
	.size fib, .-fib

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -32
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	li	s2, 1
	li	s1, 20
.main.bb1:
	blt	s1, s2, .main.bb3
.main.bb2:
	li	t5, 102
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 105
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 98
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 40
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 41
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 61
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	li	t5, 32
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	mv	t5, s2
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	fib
	mv	t0, a0
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	s2, s2, 1
	bge	s1, s2, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 0
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 32
	ret
	.size main, .-main
