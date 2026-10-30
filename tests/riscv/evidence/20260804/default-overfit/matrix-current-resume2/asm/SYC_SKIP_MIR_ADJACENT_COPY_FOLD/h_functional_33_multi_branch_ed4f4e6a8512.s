
  .text

  .globl main
  .type  main, @function
main:
.main.bb0:
	addi	sp, sp, -48
	sd	ra, 8(sp)
	sd	s1, 16(sp)
	sd	s2, 24(sp)
	sd	s3, 32(sp)
	call	getint
	mv	s1, a0
	li	t0, 0
	mv	s2, t0	# phi-copy
.main.bb1:
	bge	s2, s1, .main.bb3
.main.bb2:
	call	getint
	mv	s3, a0
	li	t0, 0
	slt	t0, t0, s3
	li	t1, 100
	slt	t1, s3, t1
	and	t0, t0, t1
	beq	t0, zero, .main.bb5
.main.bb4:
	li	t0, 99
	slt	t0, s3, t0
	beq	t0, zero, .main.bb8
.main.bb7:
	li	t0, 98
	slt	t0, s3, t0
	beq	t0, zero, .main.bb11
.main.bb10:
	li	t0, 97
	slt	t0, s3, t0
	beq	t0, zero, .main.bb14
.main.bb13:
	li	t0, 96
	slt	t0, s3, t0
	beq	t0, zero, .main.bb17
.main.bb16:
	li	t0, 95
	slt	t0, s3, t0
	beq	t0, zero, .main.bb20
.main.bb19:
	li	t0, 94
	slt	t0, s3, t0
	beq	t0, zero, .main.bb23
.main.bb22:
	li	t0, 93
	slt	t0, s3, t0
	beq	t0, zero, .main.bb26
.main.bb25:
	li	t0, 92
	slt	t0, s3, t0
	beq	t0, zero, .main.bb29
.main.bb28:
	li	t0, 91
	slt	t0, s3, t0
	beq	t0, zero, .main.bb32
.main.bb31:
	li	t0, 90
	slt	t0, s3, t0
	beq	t0, zero, .main.bb35
.main.bb34:
	li	t0, 89
	slt	t0, s3, t0
	beq	t0, zero, .main.bb38
.main.bb37:
	li	t0, 88
	slt	t0, s3, t0
	beq	t0, zero, .main.bb41
.main.bb40:
	li	t0, 87
	slt	t0, s3, t0
	beq	t0, zero, .main.bb44
.main.bb43:
	li	t0, 86
	slt	t0, s3, t0
	beq	t0, zero, .main.bb47
.main.bb46:
	li	t0, 85
	slt	t0, s3, t0
	beq	t0, zero, .main.bb50
.main.bb49:
	li	t0, 84
	slt	t0, s3, t0
	beq	t0, zero, .main.bb53
.main.bb52:
	li	t0, 83
	slt	t0, s3, t0
	beq	t0, zero, .main.bb56
.main.bb55:
	li	t0, 82
	slt	t0, s3, t0
	beq	t0, zero, .main.bb59
.main.bb58:
	li	t0, 81
	slt	t0, s3, t0
	beq	t0, zero, .main.bb62
.main.bb61:
	li	t0, 80
	slt	t0, s3, t0
	beq	t0, zero, .main.bb65
.main.bb64:
	li	t0, 79
	slt	t0, s3, t0
	beq	t0, zero, .main.bb68
.main.bb67:
	li	t0, 78
	slt	t0, s3, t0
	beq	t0, zero, .main.bb71
.main.bb70:
	li	t0, 77
	slt	t0, s3, t0
	beq	t0, zero, .main.bb74
.main.bb73:
	li	t0, 76
	slt	t0, s3, t0
	beq	t0, zero, .main.bb77
.main.bb76:
	li	t0, 75
	slt	t0, s3, t0
	beq	t0, zero, .main.bb80
.main.bb79:
	li	t0, 74
	slt	t0, s3, t0
	beq	t0, zero, .main.bb83
.main.bb82:
	li	t0, 73
	slt	t0, s3, t0
	beq	t0, zero, .main.bb86
.main.bb85:
	li	t0, 72
	slt	t0, s3, t0
	beq	t0, zero, .main.bb89
.main.bb88:
	li	t0, 71
	slt	t0, s3, t0
	beq	t0, zero, .main.bb92
.main.bb91:
	li	t0, 70
	slt	t0, s3, t0
	beq	t0, zero, .main.bb95
.main.bb94:
	li	t0, 69
	slt	t0, s3, t0
	beq	t0, zero, .main.bb98
.main.bb97:
	li	t0, 68
	slt	t0, s3, t0
	beq	t0, zero, .main.bb101
.main.bb100:
	li	t0, 67
	slt	t0, s3, t0
	beq	t0, zero, .main.bb104
.main.bb103:
	li	t0, 66
	slt	t0, s3, t0
	beq	t0, zero, .main.bb107
.main.bb106:
	li	t0, 65
	slt	t0, s3, t0
	beq	t0, zero, .main.bb110
.main.bb109:
	li	t0, 64
	slt	t0, s3, t0
	beq	t0, zero, .main.bb113
.main.bb112:
	li	t0, 63
	slt	t0, s3, t0
	beq	t0, zero, .main.bb116
.main.bb115:
	li	t0, 62
	slt	t0, s3, t0
	beq	t0, zero, .main.bb119
.main.bb118:
	li	t0, 61
	slt	t0, s3, t0
	beq	t0, zero, .main.bb122
.main.bb121:
	li	t0, 60
	slt	t0, s3, t0
	beq	t0, zero, .main.bb125
.main.bb124:
	li	t0, 59
	slt	t0, s3, t0
	beq	t0, zero, .main.bb128
.main.bb127:
	li	t0, 58
	slt	t0, s3, t0
	beq	t0, zero, .main.bb131
.main.bb130:
	li	t0, 57
	slt	t0, s3, t0
	beq	t0, zero, .main.bb134
.main.bb133:
	li	t0, 56
	slt	t0, s3, t0
	beq	t0, zero, .main.bb137
.main.bb136:
	li	t0, 55
	slt	t0, s3, t0
	beq	t0, zero, .main.bb140
.main.bb139:
	li	t0, 54
	slt	t0, s3, t0
	beq	t0, zero, .main.bb143
.main.bb142:
	li	t0, 53
	slt	t0, s3, t0
	beq	t0, zero, .main.bb146
.main.bb145:
	li	t0, 52
	slt	t0, s3, t0
	beq	t0, zero, .main.bb149
.main.bb148:
	li	t0, 51
	slt	t0, s3, t0
	beq	t0, zero, .main.bb152
.main.bb151:
	li	t0, 50
	slt	t0, s3, t0
	beq	t0, zero, .main.bb155
.main.bb154:
	li	t0, 49
	slt	t0, s3, t0
	beq	t0, zero, .main.bb158
.main.bb157:
	li	t0, 48
	slt	t0, s3, t0
	beq	t0, zero, .main.bb161
.main.bb160:
	li	t0, 47
	slt	t0, s3, t0
	beq	t0, zero, .main.bb164
.main.bb163:
	li	t0, 46
	slt	t0, s3, t0
	beq	t0, zero, .main.bb167
.main.bb166:
	li	t0, 45
	slt	t0, s3, t0
	beq	t0, zero, .main.bb170
.main.bb169:
	li	t0, 44
	slt	t0, s3, t0
	beq	t0, zero, .main.bb173
.main.bb172:
	li	t0, 43
	slt	t0, s3, t0
	beq	t0, zero, .main.bb176
.main.bb175:
	li	t0, 42
	slt	t0, s3, t0
	beq	t0, zero, .main.bb179
.main.bb178:
	li	t0, 41
	slt	t0, s3, t0
	beq	t0, zero, .main.bb182
.main.bb181:
	li	t0, 40
	slt	t0, s3, t0
	beq	t0, zero, .main.bb185
.main.bb184:
	li	t0, 39
	slt	t0, s3, t0
	beq	t0, zero, .main.bb188
.main.bb187:
	li	t0, 38
	slt	t0, s3, t0
	beq	t0, zero, .main.bb191
.main.bb190:
	li	t0, 37
	slt	t0, s3, t0
	beq	t0, zero, .main.bb194
.main.bb193:
	li	t0, 36
	slt	t0, s3, t0
	beq	t0, zero, .main.bb197
.main.bb196:
	li	t0, 35
	slt	t0, s3, t0
	beq	t0, zero, .main.bb200
.main.bb199:
	li	t0, 34
	slt	t0, s3, t0
	beq	t0, zero, .main.bb203
.main.bb202:
	li	t0, 33
	slt	t0, s3, t0
	beq	t0, zero, .main.bb206
.main.bb205:
	li	t0, 32
	slt	t0, s3, t0
	beq	t0, zero, .main.bb209
.main.bb208:
	li	t0, 31
	slt	t0, s3, t0
	beq	t0, zero, .main.bb212
.main.bb211:
	li	t0, 30
	slt	t0, s3, t0
	beq	t0, zero, .main.bb215
.main.bb214:
	li	t0, 29
	slt	t0, s3, t0
	beq	t0, zero, .main.bb218
.main.bb217:
	li	t0, 28
	slt	t0, s3, t0
	beq	t0, zero, .main.bb221
.main.bb220:
	li	t0, 27
	slt	t0, s3, t0
	beq	t0, zero, .main.bb224
.main.bb223:
	li	t0, 26
	slt	t0, s3, t0
	beq	t0, zero, .main.bb227
.main.bb226:
	li	t0, 25
	slt	t0, s3, t0
	beq	t0, zero, .main.bb230
.main.bb229:
	li	t0, 24
	slt	t0, s3, t0
	beq	t0, zero, .main.bb233
.main.bb232:
	li	t0, 23
	slt	t0, s3, t0
	beq	t0, zero, .main.bb236
.main.bb235:
	li	t0, 22
	slt	t0, s3, t0
	beq	t0, zero, .main.bb239
.main.bb238:
	li	t0, 21
	slt	t0, s3, t0
	beq	t0, zero, .main.bb242
.main.bb241:
	li	t0, 20
	slt	t0, s3, t0
	beq	t0, zero, .main.bb245
.main.bb244:
	li	t0, 19
	slt	t0, s3, t0
	beq	t0, zero, .main.bb248
.main.bb247:
	li	t0, 18
	slt	t0, s3, t0
	beq	t0, zero, .main.bb251
.main.bb250:
	li	t0, 17
	slt	t0, s3, t0
	beq	t0, zero, .main.bb254
.main.bb253:
	li	t0, 16
	slt	t0, s3, t0
	beq	t0, zero, .main.bb257
.main.bb256:
	li	t0, 15
	slt	t0, s3, t0
	beq	t0, zero, .main.bb260
.main.bb259:
	li	t0, 14
	slt	t0, s3, t0
	beq	t0, zero, .main.bb263
.main.bb262:
	li	t0, 13
	slt	t0, s3, t0
	beq	t0, zero, .main.bb266
.main.bb265:
	li	t0, 12
	slt	t0, s3, t0
	beq	t0, zero, .main.bb269
.main.bb268:
	li	t0, 11
	slt	t0, s3, t0
	beq	t0, zero, .main.bb272
.main.bb271:
	li	t0, 10
	slt	t0, s3, t0
	beq	t0, zero, .main.bb275
.main.bb274:
	li	t0, 9
	slt	t0, s3, t0
	beq	t0, zero, .main.bb278
.main.bb277:
	li	t0, 8
	slt	t0, s3, t0
	beq	t0, zero, .main.bb281
.main.bb280:
	li	t0, 7
	slt	t0, s3, t0
	beq	t0, zero, .main.bb284
.main.bb283:
	li	t0, 6
	slt	t0, s3, t0
	beq	t0, zero, .main.bb287
.main.bb286:
	li	t0, 5
	slt	t0, s3, t0
	beq	t0, zero, .main.bb290
.main.bb289:
	li	t0, 4
	slt	t0, s3, t0
	beq	t0, zero, .main.bb293
.main.bb292:
	li	t0, 3
	slt	t0, s3, t0
	beq	t0, zero, .main.bb296
.main.bb295:
	li	t0, 2
	slt	t0, s3, t0
	beq	t0, zero, .main.bb299
.main.bb298:
	li	t0, 1
	j	.main.bb6
.main.bb300:
	j	.main.bb6
.main.bb297:
	j	.main.bb6
.main.bb294:
	j	.main.bb6
.main.bb291:
	j	.main.bb6
.main.bb288:
	j	.main.bb6
.main.bb285:
	j	.main.bb6
.main.bb282:
	j	.main.bb6
.main.bb279:
	j	.main.bb6
.main.bb276:
	j	.main.bb6
.main.bb273:
	j	.main.bb6
.main.bb270:
	j	.main.bb6
.main.bb267:
	j	.main.bb6
.main.bb264:
	j	.main.bb6
.main.bb261:
	j	.main.bb6
.main.bb258:
	j	.main.bb6
.main.bb255:
	j	.main.bb6
.main.bb252:
	j	.main.bb6
.main.bb249:
	j	.main.bb6
.main.bb246:
	j	.main.bb6
.main.bb243:
	j	.main.bb6
.main.bb240:
	j	.main.bb6
.main.bb237:
	j	.main.bb6
.main.bb234:
	j	.main.bb6
.main.bb231:
	j	.main.bb6
.main.bb228:
	j	.main.bb6
.main.bb225:
	j	.main.bb6
.main.bb222:
	j	.main.bb6
.main.bb219:
	j	.main.bb6
.main.bb216:
	j	.main.bb6
.main.bb213:
	j	.main.bb6
.main.bb210:
	j	.main.bb6
.main.bb207:
	j	.main.bb6
.main.bb204:
	j	.main.bb6
.main.bb201:
	j	.main.bb6
.main.bb198:
	j	.main.bb6
.main.bb195:
	j	.main.bb6
.main.bb192:
	j	.main.bb6
.main.bb189:
	j	.main.bb6
.main.bb186:
	j	.main.bb6
.main.bb183:
	j	.main.bb6
.main.bb180:
	j	.main.bb6
.main.bb177:
	j	.main.bb6
.main.bb174:
	j	.main.bb6
.main.bb171:
	j	.main.bb6
.main.bb168:
	j	.main.bb6
.main.bb165:
	j	.main.bb6
.main.bb162:
	j	.main.bb6
.main.bb159:
	j	.main.bb6
.main.bb156:
	j	.main.bb6
.main.bb153:
	j	.main.bb6
.main.bb150:
	j	.main.bb6
.main.bb147:
	j	.main.bb6
.main.bb144:
	j	.main.bb6
.main.bb141:
	j	.main.bb6
.main.bb138:
	j	.main.bb6
.main.bb135:
	j	.main.bb6
.main.bb132:
	j	.main.bb6
.main.bb129:
	j	.main.bb6
.main.bb126:
	j	.main.bb6
.main.bb123:
	j	.main.bb6
.main.bb120:
	j	.main.bb6
.main.bb117:
	j	.main.bb6
.main.bb114:
	j	.main.bb6
.main.bb111:
	j	.main.bb6
.main.bb108:
	j	.main.bb6
.main.bb105:
	j	.main.bb6
.main.bb102:
	j	.main.bb6
.main.bb99:
	j	.main.bb6
.main.bb96:
	j	.main.bb6
.main.bb93:
	j	.main.bb6
.main.bb90:
	j	.main.bb6
.main.bb87:
	j	.main.bb6
.main.bb84:
	j	.main.bb6
.main.bb81:
	j	.main.bb6
.main.bb78:
	j	.main.bb6
.main.bb75:
	j	.main.bb6
.main.bb72:
	j	.main.bb6
.main.bb69:
	j	.main.bb6
.main.bb66:
	j	.main.bb6
.main.bb63:
	j	.main.bb6
.main.bb60:
	j	.main.bb6
.main.bb57:
	j	.main.bb6
.main.bb54:
	j	.main.bb6
.main.bb51:
	j	.main.bb6
.main.bb48:
	j	.main.bb6
.main.bb45:
	j	.main.bb6
.main.bb42:
	j	.main.bb6
.main.bb39:
	j	.main.bb6
.main.bb36:
	j	.main.bb6
.main.bb33:
	j	.main.bb6
.main.bb30:
	j	.main.bb6
.main.bb27:
	j	.main.bb6
.main.bb24:
	j	.main.bb6
.main.bb21:
	j	.main.bb6
.main.bb18:
	j	.main.bb6
.main.bb15:
	j	.main.bb6
.main.bb12:
	j	.main.bb6
.main.bb9:
.main.bb6:
	mv	t5, t0
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putint
	li	t5, 10
	sd	t5, 0(sp)
	ld	a0, 0(sp)
	call	putch
	addiw	t0, s2, 1
	mv	s2, t0	# phi-copy
	blt	s2, s1, .main.bb2	# rotated-loop-branch
.main.bb3:
	li	a0, 0
	ld	s3, 32(sp)
	ld	s2, 24(sp)
	ld	s1, 16(sp)
	ld	ra, 8(sp)
	addi	sp, sp, 48
	ret
.main.bb5:
	li	t0, 100
	j	.main.bb6
.main.bb8:
	li	t0, 99
	j	.main.bb6
.main.bb11:
	li	t0, 98
	j	.main.bb6
.main.bb14:
	li	t0, 97
	j	.main.bb6
.main.bb17:
	li	t0, 96
	j	.main.bb6
.main.bb20:
	li	t0, 95
	j	.main.bb6
.main.bb23:
	li	t0, 94
	j	.main.bb6
.main.bb26:
	li	t0, 93
	j	.main.bb6
.main.bb29:
	li	t0, 92
	j	.main.bb6
.main.bb32:
	li	t0, 91
	j	.main.bb6
.main.bb35:
	li	t0, 90
	j	.main.bb6
.main.bb38:
	li	t0, 89
	j	.main.bb6
.main.bb41:
	li	t0, 88
	j	.main.bb6
.main.bb44:
	li	t0, 87
	j	.main.bb6
.main.bb47:
	li	t0, 86
	j	.main.bb6
.main.bb50:
	li	t0, 85
	j	.main.bb6
.main.bb53:
	li	t0, 84
	j	.main.bb6
.main.bb56:
	li	t0, 83
	j	.main.bb6
.main.bb59:
	li	t0, 82
	j	.main.bb6
.main.bb62:
	li	t0, 81
	j	.main.bb6
.main.bb65:
	li	t0, 80
	j	.main.bb6
.main.bb68:
	li	t0, 79
	j	.main.bb6
.main.bb71:
	li	t0, 78
	j	.main.bb6
.main.bb74:
	li	t0, 77
	j	.main.bb6
.main.bb77:
	li	t0, 76
	j	.main.bb6
.main.bb80:
	li	t0, 75
	j	.main.bb6
.main.bb83:
	li	t0, 74
	j	.main.bb6
.main.bb86:
	li	t0, 73
	j	.main.bb6
.main.bb89:
	li	t0, 72
	j	.main.bb6
.main.bb92:
	li	t0, 71
	j	.main.bb6
.main.bb95:
	li	t0, 70
	j	.main.bb6
.main.bb98:
	li	t0, 69
	j	.main.bb6
.main.bb101:
	li	t0, 68
	j	.main.bb6
.main.bb104:
	li	t0, 67
	j	.main.bb6
.main.bb107:
	li	t0, 66
	j	.main.bb6
.main.bb110:
	li	t0, 65
	j	.main.bb6
.main.bb113:
	li	t0, 64
	j	.main.bb6
.main.bb116:
	li	t0, 63
	j	.main.bb6
.main.bb119:
	li	t0, 62
	j	.main.bb6
.main.bb122:
	li	t0, 61
	j	.main.bb6
.main.bb125:
	li	t0, 60
	j	.main.bb6
.main.bb128:
	li	t0, 59
	j	.main.bb6
.main.bb131:
	li	t0, 58
	j	.main.bb6
.main.bb134:
	li	t0, 57
	j	.main.bb6
.main.bb137:
	li	t0, 56
	j	.main.bb6
.main.bb140:
	li	t0, 55
	j	.main.bb6
.main.bb143:
	li	t0, 54
	j	.main.bb6
.main.bb146:
	li	t0, 53
	j	.main.bb6
.main.bb149:
	li	t0, 52
	j	.main.bb6
.main.bb152:
	li	t0, 51
	j	.main.bb6
.main.bb155:
	li	t0, 50
	j	.main.bb6
.main.bb158:
	li	t0, 49
	j	.main.bb6
.main.bb161:
	li	t0, 48
	j	.main.bb6
.main.bb164:
	li	t0, 47
	j	.main.bb6
.main.bb167:
	li	t0, 46
	j	.main.bb6
.main.bb170:
	li	t0, 45
	j	.main.bb6
.main.bb173:
	li	t0, 44
	j	.main.bb6
.main.bb176:
	li	t0, 43
	j	.main.bb6
.main.bb179:
	li	t0, 42
	j	.main.bb6
.main.bb182:
	li	t0, 41
	j	.main.bb6
.main.bb185:
	li	t0, 40
	j	.main.bb6
.main.bb188:
	li	t0, 39
	j	.main.bb6
.main.bb191:
	li	t0, 38
	j	.main.bb6
.main.bb194:
	li	t0, 37
	j	.main.bb6
.main.bb197:
	li	t0, 36
	j	.main.bb6
.main.bb200:
	li	t0, 35
	j	.main.bb6
.main.bb203:
	li	t0, 34
	j	.main.bb6
.main.bb206:
	li	t0, 33
	j	.main.bb6
.main.bb209:
	li	t0, 32
	j	.main.bb6
.main.bb212:
	li	t0, 31
	j	.main.bb6
.main.bb215:
	li	t0, 30
	j	.main.bb6
.main.bb218:
	li	t0, 29
	j	.main.bb6
.main.bb221:
	li	t0, 28
	j	.main.bb6
.main.bb224:
	li	t0, 27
	j	.main.bb6
.main.bb227:
	li	t0, 26
	j	.main.bb6
.main.bb230:
	li	t0, 25
	j	.main.bb6
.main.bb233:
	li	t0, 24
	j	.main.bb6
.main.bb236:
	li	t0, 23
	j	.main.bb6
.main.bb239:
	li	t0, 22
	j	.main.bb6
.main.bb242:
	li	t0, 21
	j	.main.bb6
.main.bb245:
	li	t0, 20
	j	.main.bb6
.main.bb248:
	li	t0, 19
	j	.main.bb6
.main.bb251:
	li	t0, 18
	j	.main.bb6
.main.bb254:
	li	t0, 17
	j	.main.bb6
.main.bb257:
	li	t0, 16
	j	.main.bb6
.main.bb260:
	li	t0, 15
	j	.main.bb6
.main.bb263:
	li	t0, 14
	j	.main.bb6
.main.bb266:
	li	t0, 13
	j	.main.bb6
.main.bb269:
	li	t0, 12
	j	.main.bb6
.main.bb272:
	li	t0, 11
	j	.main.bb6
.main.bb275:
	li	t0, 10
	j	.main.bb6
.main.bb278:
	li	t0, 9
	j	.main.bb6
.main.bb281:
	li	t0, 8
	j	.main.bb6
.main.bb284:
	li	t0, 7
	j	.main.bb6
.main.bb287:
	li	t0, 6
	j	.main.bb6
.main.bb290:
	li	t0, 5
	j	.main.bb6
.main.bb293:
	li	t0, 4
	j	.main.bb6
.main.bb296:
	li	t0, 3
	j	.main.bb6
.main.bb299:
	li	t0, 2
	j	.main.bb6
	.size main, .-main
