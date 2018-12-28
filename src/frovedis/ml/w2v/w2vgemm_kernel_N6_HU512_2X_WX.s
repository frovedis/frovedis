	.text
	.file	"w2vgemm_kernel_N6_HU512_2X_WX.c"
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI0_0:
	.4byte	1065353216
.LCPI0_1:
	.4byte	0
.LCPI0_2:
	.4byte	3212836864
.LCPI0_3:
	.4byte	1086324736
.LCPI0_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI0_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W01
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W01,@function
w2v_kernel_N6_HU512_2X_W01:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB0_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB0_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s35, -8(,%s9)
	adds.w.sx %s43, %s34, (0)1
	lvl %s43
	ldl.sx %s34, (,%s4)
	ld %s36, 240(,%s9)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s36, %s36, %s34
	vld %v7,8,%s36
	ldl.sx %s34, (,%s5)
	ldl.sx %s38, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s37, %s7, %s34
	muls.l %s34, %s38, %s1
	ldl.sx %s39, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s38, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s39, %s39, %s1
	sll %s39, %s39, 2
	adds.l %s39, %s7, %s39
	muls.l %s34, %s34, %s1
	ldl.sx %s41, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s40, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s41, %s41, %s1
	sll %s41, %s41, 2
	adds.l %s41, %s7, %s41
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s42, %s7, %s34
	vld %v8,8,%s37
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	lvl %s43
	pvfmul %v0,%v8,%v7
	vsll %v1,%v0,32
	vfadd.s %v0,%v0,%v1
	vfsum.s %v0,%v0
	or %s44, 1, (0)1
	lvl %s44
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lvl %s43
	pvfmul %v0,%v9,%v7
	vsll %v1,%v0,32
	vfadd.s %v0,%v0,%v1
	vfsum.s %v0,%v0
	lvl %s44
	lea %s45, 4(%s6)
	vstu %v0,4,%s45
	lvl %s43
	pvfmul %v0,%v10,%v7
	vsll %v1,%v0,32
	vfadd.s %v0,%v0,%v1
	vfsum.s %v0,%v0
	lvl %s44
	lea %s45, 8(%s6)
	vstu %v0,4,%s45
	lvl %s43
	pvfmul %v0,%v11,%v7
	vsll %v1,%v0,32
	vfadd.s %v0,%v0,%v1
	vfsum.s %v0,%v0
	lvl %s44
	lea %s45, 12(%s6)
	vstu %v0,4,%s45
	lvl %s43
	pvfmul %v0,%v12,%v7
	vsll %v1,%v0,32
	vfadd.s %v0,%v0,%v1
	vfsum.s %v0,%v0
	lvl %s44
	lea %s45, 16(%s6)
	vstu %v0,4,%s45
	lvl %s43
	pvfmul %v0,%v13,%v7
	vsll %v1,%v0,32
	vfadd.s %v0,%v0,%v1
	vfsum.s %v0,%v0
	lvl %s44
	lea %s44, 20(%s6)
	vstu %v0,4,%s44
	muls.l %s44, %s2, %s0
	adds.w.sx %s44, %s44, (0)1
	lvl %s44
	lea.sl %s44, .LCPI0_0@hi
	ldu %s44, .LCPI0_0@lo(,%s44)
	vseq %v0
	vbrdu %v14,%s44
	adds.w.sx %s45, %s2, (0)1
	vsubs.w.sx %v0,%s45,%v0
	vldu %v15,4,%s6
	lea.sl %s45, .LCPI0_2@hi
	ldu %s45, .LCPI0_2@lo(,%s45)
	vfmk.w.gt %vm1,%v0
	lea.sl %s46, .LCPI0_1@hi
	ldu %s46, .LCPI0_1@lo(,%s46)
	vfmul.s %v0,%s45,%v15
	lea %s45, __vec_expf@lo
	and %s45, %s45, (32)0
	lea.sl %s45, __vec_expf@hi(%s45)
	vbrdu %v14,%s46,%vm1
	bsic %lr, (,%s45)
	lea.sl %s45, .LCPI0_3@hi
	ldu %s45, .LCPI0_3@lo(,%s45)
	vfadd.s %v0,%s44,%v0
	vfsub.s %v1,%s45,%v15
	lea.sl %s45, .LCPI0_4@hi
	ldu %s45, .LCPI0_4@lo(,%s45)
	vfdiv.s %v0,%s44,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s44,%vm1
	vfsub.s %v1,%s45,%v15
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s46,%vm1
	vfsub.s %v0,%v14,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s43
	lea.sl %s43, .LCPI0_5@hi
	ld %s43, .LCPI0_5@lo(,%s43)
	ldl.zx %s44, (,%s34)
	sll %s45, %s2, 2
	adds.l %s45, %s34, %s45
	ldl.zx %s46, (,%s45)
	vbrd %v0,%s43
	muls.l %s43, %s35, %s44
	muls.l %s44, %s35, %s46
	sll %s46, %s2, 3
	adds.l %s46, %s34, %s46
	ldl.zx %s47, (,%s46)
	muls.l %s48, 12, %s2
	adds.l %s48, %s34, %s48
	ldl.zx %s49, (,%s48)
	sll %s50, %s2, 4
	adds.l %s50, %s34, %s50
	ldl.zx %s51, (,%s50)
	muls.l %s52, 20, %s2
	adds.l %s52, %s34, %s52
	ldl.zx %s53, (,%s52)
	muls.l %s47, %s35, %s47
	muls.l %s49, %s35, %s49
	muls.l %s51, %s35, %s51
	muls.l %s53, %s35, %s53
	pvfmad %v1,%v0,%s43,%v7
	pvfmad %v2,%v0,%s44,%v7
	pvfmad %v3,%v0,%s47,%v7
	pvfmad %v4,%v0,%s49,%v7
	pvfmad %v5,%v0,%s51,%v7
	pvfmad %v6,%v0,%s53,%v7
	pvfadd %v1,%v8,%v1
	pvfadd %v2,%v9,%v2
	pvfadd %v3,%v10,%v3
	pvfadd %v4,%v11,%v4
	pvfadd %v5,%v12,%v5
	pvfadd %v6,%v13,%v6
	vst %v1,8,%s37
	vst %v2,8,%s38
	vst %v3,8,%s39
	vst %v4,8,%s40
	vst %v5,8,%s41
	vst %v6,8,%s42
	ldl.zx %s34, (,%s34)
	ldl.zx %s37, (,%s45)
	muls.l %s34, %s35, %s34
	muls.l %s37, %s35, %s37
	ldl.zx %s38, (,%s46)
	ldl.zx %s39, (,%s48)
	ldl.zx %s40, (,%s50)
	ldl.zx %s41, (,%s52)
	muls.l %s38, %s35, %s38
	muls.l %s39, %s35, %s39
	muls.l %s40, %s35, %s40
	muls.l %s35, %s35, %s41
	pvfmad %v0,%v0,%s34,%v8
	pvfmad %v0,%v0,%s37,%v9
	pvfmad %v0,%v0,%s38,%v10
	pvfmad %v0,%v0,%s39,%v11
	pvfmad %v0,%v0,%s40,%v12
	pvfmad %v0,%v0,%s35,%v13
	pvfadd %v0,%v7,%v0
	vst %v0,8,%s36
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end0:
	.size	w2v_kernel_N6_HU512_2X_W01, .Lfunc_end0-w2v_kernel_N6_HU512_2X_W01

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI1_0:
	.4byte	1065353216
.LCPI1_1:
	.4byte	0
.LCPI1_2:
	.4byte	3212836864
.LCPI1_3:
	.4byte	1086324736
.LCPI1_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI1_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W02
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W02,@function
w2v_kernel_N6_HU512_2X_W02:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB1_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB1_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s35, -8(,%s9)
	adds.w.sx %s44, %s34, (0)1
	lvl %s44
	ldl.sx %s34, (,%s4)
	ld %s37, 240(,%s9)
	ldl.sx %s38, 4(,%s4)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s36, %s37, %s34
	muls.l %s34, %s38, %s1
	sll %s34, %s34, 2
	adds.l %s37, %s37, %s34
	vld %v7,8,%s36
	vld %v8,8,%s37
	ldl.sx %s34, (,%s5)
	ldl.sx %s39, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s38, %s7, %s34
	muls.l %s34, %s39, %s1
	ldl.sx %s40, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s39, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s40, %s40, %s1
	sll %s40, %s40, 2
	adds.l %s40, %s7, %s40
	muls.l %s34, %s34, %s1
	ldl.sx %s42, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s41, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s42, %s42, %s1
	sll %s42, %s42, 2
	adds.l %s42, %s7, %s42
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s43, %s7, %s34
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	lvl %s44
	pvfmul %v0,%v9,%v7
	pvfmul %v1,%v9,%v8
	vsll %v2,%v0,32
	vsll %v3,%v1,32
	vfadd.s %v0,%v0,%v2
	vfadd.s %v1,%v1,%v3
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	or %s45, 1, (0)1
	lvl %s45
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s46, 4(%s6)
	vstu %v1,4,%s46
	lvl %s44
	pvfmul %v0,%v10,%v7
	pvfmul %v1,%v10,%v8
	vsll %v2,%v0,32
	vsll %v3,%v1,32
	vfadd.s %v0,%v0,%v2
	vfadd.s %v1,%v1,%v3
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	lvl %s45
	lea %s46, 8(%s6)
	vstu %v0,4,%s46
	lea %s46, 12(%s6)
	vstu %v1,4,%s46
	lvl %s44
	pvfmul %v0,%v11,%v7
	pvfmul %v1,%v11,%v8
	vsll %v2,%v0,32
	vsll %v3,%v1,32
	vfadd.s %v0,%v0,%v2
	vfadd.s %v1,%v1,%v3
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	lvl %s45
	lea %s46, 16(%s6)
	vstu %v0,4,%s46
	lea %s46, 20(%s6)
	vstu %v1,4,%s46
	lvl %s44
	pvfmul %v0,%v12,%v7
	pvfmul %v1,%v12,%v8
	vsll %v2,%v0,32
	vsll %v3,%v1,32
	vfadd.s %v0,%v0,%v2
	vfadd.s %v1,%v1,%v3
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	lvl %s45
	lea %s46, 24(%s6)
	vstu %v0,4,%s46
	lea %s46, 28(%s6)
	vstu %v1,4,%s46
	lvl %s44
	pvfmul %v0,%v13,%v7
	pvfmul %v1,%v13,%v8
	vsll %v2,%v0,32
	vsll %v3,%v1,32
	vfadd.s %v0,%v0,%v2
	vfadd.s %v1,%v1,%v3
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	lvl %s45
	lea %s46, 32(%s6)
	vstu %v0,4,%s46
	lea %s46, 36(%s6)
	vstu %v1,4,%s46
	lvl %s44
	pvfmul %v0,%v14,%v7
	pvfmul %v1,%v14,%v8
	vsll %v2,%v0,32
	vsll %v3,%v1,32
	vfadd.s %v0,%v0,%v2
	vfadd.s %v1,%v1,%v3
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	lvl %s45
	lea %s45, 40(%s6)
	vstu %v0,4,%s45
	lea %s45, 44(%s6)
	vstu %v1,4,%s45
	muls.l %s45, %s2, %s0
	adds.w.sx %s45, %s45, (0)1
	lvl %s45
	lea.sl %s45, .LCPI1_0@hi
	ldu %s45, .LCPI1_0@lo(,%s45)
	vseq %v0
	vbrdu %v15,%s45
	adds.w.sx %s46, %s2, (0)1
	vsubs.w.sx %v0,%s46,%v0
	vldu %v16,4,%s6
	lea.sl %s46, .LCPI1_2@hi
	ldu %s46, .LCPI1_2@lo(,%s46)
	vfmk.w.gt %vm1,%v0
	lea.sl %s47, .LCPI1_1@hi
	ldu %s47, .LCPI1_1@lo(,%s47)
	vfmul.s %v0,%s46,%v16
	lea %s46, __vec_expf@lo
	and %s46, %s46, (32)0
	lea.sl %s46, __vec_expf@hi(%s46)
	vbrdu %v15,%s47,%vm1
	bsic %lr, (,%s46)
	lea.sl %s46, .LCPI1_3@hi
	ldu %s46, .LCPI1_3@lo(,%s46)
	vfadd.s %v0,%s45,%v0
	vfsub.s %v1,%s46,%v16
	lea.sl %s46, .LCPI1_4@hi
	ldu %s46, .LCPI1_4@lo(,%s46)
	vfdiv.s %v0,%s45,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s45,%vm1
	vfsub.s %v1,%s46,%v16
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s47,%vm1
	vfsub.s %v0,%v15,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s44
	lea.sl %s44, .LCPI1_5@hi
	ld %s44, .LCPI1_5@lo(,%s44)
	ldl.zx %s45, (,%s34)
	sll %s46, %s2, 2
	adds.l %s47, %s34, %s46
	ldl.zx %s48, (,%s47)
	vbrd %v0,%s44
	muls.l %s44, %s35, %s45
	muls.l %s45, %s35, %s48
	sll %s48, %s2, 3
	adds.l %s49, %s34, %s48
	ldl.zx %s50, (,%s49)
	muls.l %s51, 12, %s2
	adds.l %s52, %s34, %s51
	ldl.zx %s53, (,%s52)
	sll %s54, %s2, 4
	adds.l %s55, %s34, %s54
	ldl.zx %s56, (,%s55)
	muls.l %s57, 20, %s2
	adds.l %s58, %s34, %s57
	ldl.zx %s59, (,%s58)
	muls.l %s50, %s35, %s50
	muls.l %s53, %s35, %s53
	muls.l %s56, %s35, %s56
	muls.l %s59, %s35, %s59
	pvfmad %v1,%v0,%s44,%v7
	pvfmad %v2,%v0,%s45,%v7
	pvfmad %v3,%v0,%s50,%v7
	pvfmad %v4,%v0,%s53,%v7
	ldl.zx %s44, 4(,%s34)
	lea %s45, 4(%s34, %s46)
	ldl.zx %s45, (,%s45)
	pvfmad %v5,%v0,%s56,%v7
	pvfmad %v6,%v0,%s59,%v7
	muls.l %s44, %s35, %s44
	muls.l %s45, %s35, %s45
	lea %s46, 4(%s34, %s48)
	ldl.zx %s46, (,%s46)
	lea %s50, 4(%s34, %s51)
	ldl.zx %s50, (,%s50)
	lea %s51, 4(%s34, %s54)
	ldl.zx %s51, (,%s51)
	lea %s53, 4(%s34, %s57)
	ldl.zx %s53, (,%s53)
	muls.l %s46, %s35, %s46
	muls.l %s50, %s35, %s50
	muls.l %s51, %s35, %s51
	muls.l %s53, %s35, %s53
	pvfmad %v1,%v1,%s44,%v8
	pvfmad %v2,%v2,%s45,%v8
	pvfmad %v3,%v3,%s46,%v8
	pvfmad %v4,%v4,%s50,%v8
	pvfmad %v5,%v5,%s51,%v8
	pvfmad %v6,%v6,%s53,%v8
	pvfadd %v1,%v9,%v1
	pvfadd %v2,%v10,%v2
	pvfadd %v3,%v11,%v3
	pvfadd %v4,%v12,%v4
	pvfadd %v5,%v13,%v5
	pvfadd %v6,%v14,%v6
	vst %v1,8,%s38
	vst %v2,8,%s39
	vst %v3,8,%s40
	vst %v4,8,%s41
	vst %v5,8,%s42
	vst %v6,8,%s43
	ldl.zx %s38, (,%s34)
	ldl.zx %s39, (,%s47)
	muls.l %s38, %s35, %s38
	muls.l %s39, %s35, %s39
	ldl.zx %s40, (,%s49)
	ldl.zx %s41, (,%s52)
	ldl.zx %s42, (,%s55)
	ldl.zx %s43, (,%s58)
	muls.l %s40, %s35, %s40
	muls.l %s41, %s35, %s41
	muls.l %s42, %s35, %s42
	muls.l %s43, %s35, %s43
	pvfmad %v1,%v0,%s38,%v9
	pvfmad %v1,%v1,%s39,%v10
	pvfmad %v1,%v1,%s40,%v11
	pvfmad %v1,%v1,%s41,%v12
	ldl.zx %s38, 4(,%s34)
	ldl.zx %s39, 4(,%s47)
	pvfmad %v1,%v1,%s42,%v13
	pvfmad %v1,%v1,%s43,%v14
	muls.l %s38, %s35, %s38
	muls.l %s39, %s35, %s39
	or %s40, 4, %s48
	adds.l %s40, %s34, %s40
	ldl.zx %s40, (,%s40)
	ldl.zx %s41, 4(,%s52)
	or %s42, 4, %s54
	adds.l %s34, %s34, %s42
	ldl.zx %s34, (,%s34)
	ldl.zx %s42, 4(,%s58)
	muls.l %s40, %s35, %s40
	muls.l %s41, %s35, %s41
	muls.l %s34, %s35, %s34
	muls.l %s35, %s35, %s42
	pvfmad %v0,%v0,%s38,%v9
	pvfmad %v0,%v0,%s39,%v10
	pvfmad %v0,%v0,%s40,%v11
	pvfmad %v0,%v0,%s41,%v12
	pvfmad %v0,%v0,%s34,%v13
	pvfmad %v0,%v0,%s35,%v14
	pvfadd %v1,%v7,%v1
	pvfadd %v0,%v8,%v0
	vst %v1,8,%s36
	vst %v0,8,%s37
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end1:
	.size	w2v_kernel_N6_HU512_2X_W02, .Lfunc_end1-w2v_kernel_N6_HU512_2X_W02

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI2_0:
	.4byte	1065353216
.LCPI2_1:
	.4byte	0
.LCPI2_2:
	.4byte	3212836864
.LCPI2_3:
	.4byte	1086324736
.LCPI2_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI2_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W03
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W03,@function
w2v_kernel_N6_HU512_2X_W03:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB2_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB2_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s35, -8(,%s9)
	adds.w.sx %s45, %s34, (0)1
	lvl %s45
	ldl.sx %s34, (,%s4)
	ld %s38, 240(,%s9)
	muls.l %s34, %s34, %s1
	ldl.sx %s37, 4(,%s4)
	sll %s34, %s34, 2
	adds.l %s36, %s38, %s34
	ldl.sx %s34, 8(,%s4)
	muls.l %s37, %s37, %s1
	sll %s37, %s37, 2
	adds.l %s37, %s38, %s37
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s38, %s38, %s34
	vld %v7,8,%s36
	vld %v8,8,%s37
	vld %v9,8,%s38
	ldl.sx %s34, (,%s5)
	ldl.sx %s40, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s39, %s7, %s34
	muls.l %s34, %s40, %s1
	ldl.sx %s41, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s40, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s41, %s41, %s1
	sll %s41, %s41, 2
	adds.l %s41, %s7, %s41
	muls.l %s34, %s34, %s1
	ldl.sx %s43, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s42, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s43, %s43, %s1
	sll %s43, %s43, 2
	adds.l %s43, %s7, %s43
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s44, %s7, %s34
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	lvl %s45
	pvfmul %v0,%v10,%v7
	pvfmul %v1,%v10,%v8
	pvfmul %v2,%v10,%v9
	vsll %v3,%v0,32
	vsll %v4,%v1,32
	vsll %v5,%v2,32
	vfadd.s %v0,%v0,%v3
	vfadd.s %v1,%v1,%v4
	vfadd.s %v2,%v2,%v5
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	or %s46, 1, (0)1
	lvl %s46
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s47, 4(%s6)
	vstu %v1,4,%s47
	lea %s47, 8(%s6)
	vstu %v2,4,%s47
	lvl %s45
	pvfmul %v0,%v11,%v7
	pvfmul %v1,%v11,%v8
	pvfmul %v2,%v11,%v9
	vsll %v3,%v0,32
	vsll %v4,%v1,32
	vsll %v5,%v2,32
	vfadd.s %v0,%v0,%v3
	vfadd.s %v1,%v1,%v4
	vfadd.s %v2,%v2,%v5
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	lvl %s46
	lea %s47, 12(%s6)
	vstu %v0,4,%s47
	lea %s47, 16(%s6)
	vstu %v1,4,%s47
	lea %s47, 20(%s6)
	vstu %v2,4,%s47
	lvl %s45
	pvfmul %v0,%v12,%v7
	pvfmul %v1,%v12,%v8
	pvfmul %v2,%v12,%v9
	vsll %v3,%v0,32
	vsll %v4,%v1,32
	vsll %v5,%v2,32
	vfadd.s %v0,%v0,%v3
	vfadd.s %v1,%v1,%v4
	vfadd.s %v2,%v2,%v5
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	lvl %s46
	lea %s47, 24(%s6)
	vstu %v0,4,%s47
	lea %s47, 28(%s6)
	vstu %v1,4,%s47
	lea %s47, 32(%s6)
	vstu %v2,4,%s47
	lvl %s45
	pvfmul %v0,%v13,%v7
	pvfmul %v1,%v13,%v8
	pvfmul %v2,%v13,%v9
	vsll %v3,%v0,32
	vsll %v4,%v1,32
	vsll %v5,%v2,32
	vfadd.s %v0,%v0,%v3
	vfadd.s %v1,%v1,%v4
	vfadd.s %v2,%v2,%v5
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	lvl %s46
	lea %s47, 36(%s6)
	vstu %v0,4,%s47
	lea %s47, 40(%s6)
	vstu %v1,4,%s47
	lea %s47, 44(%s6)
	vstu %v2,4,%s47
	lvl %s45
	pvfmul %v0,%v14,%v7
	pvfmul %v1,%v14,%v8
	pvfmul %v2,%v14,%v9
	vsll %v3,%v0,32
	vsll %v4,%v1,32
	vsll %v5,%v2,32
	vfadd.s %v0,%v0,%v3
	vfadd.s %v1,%v1,%v4
	vfadd.s %v2,%v2,%v5
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	lvl %s46
	lea %s47, 48(%s6)
	vstu %v0,4,%s47
	lea %s47, 52(%s6)
	vstu %v1,4,%s47
	lea %s47, 56(%s6)
	vstu %v2,4,%s47
	lvl %s45
	pvfmul %v0,%v15,%v7
	pvfmul %v1,%v15,%v8
	pvfmul %v2,%v15,%v9
	vsll %v3,%v0,32
	vsll %v4,%v1,32
	vsll %v5,%v2,32
	vfadd.s %v0,%v0,%v3
	vfadd.s %v1,%v1,%v4
	vfadd.s %v2,%v2,%v5
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	lvl %s46
	lea %s46, 60(%s6)
	vstu %v0,4,%s46
	lea %s46, 64(%s6)
	vstu %v1,4,%s46
	lea %s46, 68(%s6)
	vstu %v2,4,%s46
	muls.l %s46, %s2, %s0
	adds.w.sx %s46, %s46, (0)1
	lvl %s46
	lea.sl %s46, .LCPI2_0@hi
	ldu %s46, .LCPI2_0@lo(,%s46)
	vseq %v0
	vbrdu %v16,%s46
	adds.w.sx %s47, %s2, (0)1
	vsubs.w.sx %v0,%s47,%v0
	vldu %v17,4,%s6
	lea.sl %s47, .LCPI2_2@hi
	ldu %s47, .LCPI2_2@lo(,%s47)
	vfmk.w.gt %vm1,%v0
	lea.sl %s48, .LCPI2_1@hi
	ldu %s48, .LCPI2_1@lo(,%s48)
	vfmul.s %v0,%s47,%v17
	lea %s47, __vec_expf@lo
	and %s47, %s47, (32)0
	lea.sl %s47, __vec_expf@hi(%s47)
	vbrdu %v16,%s48,%vm1
	bsic %lr, (,%s47)
	lea.sl %s47, .LCPI2_3@hi
	ldu %s47, .LCPI2_3@lo(,%s47)
	vfadd.s %v0,%s46,%v0
	vfsub.s %v1,%s47,%v17
	lea.sl %s47, .LCPI2_4@hi
	ldu %s47, .LCPI2_4@lo(,%s47)
	vfdiv.s %v0,%s46,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s46,%vm1
	vfsub.s %v1,%s47,%v17
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s48,%vm1
	vfsub.s %v0,%v16,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s45
	lea.sl %s45, .LCPI2_5@hi
	ld %s46, .LCPI2_5@lo(,%s45)
	ldl.zx %s47, (,%s34)
	sll %s50, %s2, 2
	adds.l %s45, %s34, %s50
	ldl.zx %s48, (,%s45)
	vbrd %v0,%s46
	muls.l %s51, %s35, %s47
	muls.l %s52, %s35, %s48
	sll %s53, %s2, 3
	adds.l %s46, %s34, %s53
	ldl.zx %s54, (,%s46)
	muls.l %s55, 12, %s2
	adds.l %s47, %s34, %s55
	ldl.zx %s56, (,%s47)
	sll %s48, %s2, 4
	adds.l %s57, %s34, %s48
	ldl.zx %s58, (,%s57)
	muls.l %s59, 20, %s2
	adds.l %s49, %s34, %s59
	ldl.zx %s60, (,%s49)
	muls.l %s54, %s35, %s54
	muls.l %s56, %s35, %s56
	muls.l %s58, %s35, %s58
	muls.l %s60, %s35, %s60
	pvfmad %v1,%v0,%s51,%v7
	pvfmad %v2,%v0,%s52,%v7
	pvfmad %v3,%v0,%s54,%v7
	pvfmad %v4,%v0,%s56,%v7
	ldl.zx %s51, 4(,%s34)
	lea %s52, 4(%s34, %s50)
	ldl.zx %s52, (,%s52)
	pvfmad %v5,%v0,%s58,%v7
	pvfmad %v6,%v0,%s60,%v7
	muls.l %s51, %s35, %s51
	muls.l %s52, %s35, %s52
	lea %s54, 4(%s34, %s53)
	ldl.zx %s54, (,%s54)
	lea %s56, 4(%s34, %s55)
	ldl.zx %s56, (,%s56)
	lea %s58, 4(%s34, %s48)
	ldl.zx %s58, (,%s58)
	lea %s60, 4(%s34, %s59)
	ldl.zx %s60, (,%s60)
	muls.l %s54, %s35, %s54
	muls.l %s56, %s35, %s56
	muls.l %s58, %s35, %s58
	muls.l %s60, %s35, %s60
	pvfmad %v1,%v1,%s51,%v8
	pvfmad %v2,%v2,%s52,%v8
	pvfmad %v3,%v3,%s54,%v8
	pvfmad %v4,%v4,%s56,%v8
	ldl.zx %s51, 8(,%s34)
	lea %s50, 8(%s34, %s50)
	ldl.zx %s50, (,%s50)
	pvfmad %v5,%v5,%s58,%v8
	pvfmad %v6,%v6,%s60,%v8
	muls.l %s51, %s35, %s51
	muls.l %s50, %s35, %s50
	lea %s52, 8(%s34, %s53)
	ldl.zx %s52, (,%s52)
	lea %s54, 8(%s34, %s55)
	ldl.zx %s54, (,%s54)
	lea %s55, 8(%s34, %s48)
	ldl.zx %s55, (,%s55)
	lea %s56, 8(%s34, %s59)
	ldl.zx %s56, (,%s56)
	muls.l %s52, %s35, %s52
	muls.l %s54, %s35, %s54
	muls.l %s55, %s35, %s55
	muls.l %s56, %s35, %s56
	pvfmad %v1,%v1,%s51,%v9
	pvfmad %v2,%v2,%s50,%v9
	pvfmad %v3,%v3,%s52,%v9
	pvfmad %v4,%v4,%s54,%v9
	pvfmad %v5,%v5,%s55,%v9
	pvfmad %v6,%v6,%s56,%v9
	pvfadd %v1,%v10,%v1
	pvfadd %v2,%v11,%v2
	pvfadd %v3,%v12,%v3
	pvfadd %v4,%v13,%v4
	pvfadd %v5,%v14,%v5
	pvfadd %v6,%v15,%v6
	vst %v1,8,%s39
	vst %v2,8,%s40
	vst %v3,8,%s41
	vst %v4,8,%s42
	vst %v5,8,%s43
	vst %v6,8,%s44
	ldl.zx %s39, (,%s34)
	ldl.zx %s40, (,%s45)
	muls.l %s39, %s35, %s39
	muls.l %s40, %s35, %s40
	ldl.zx %s41, (,%s46)
	ldl.zx %s42, (,%s47)
	ldl.zx %s43, (,%s57)
	ldl.zx %s44, (,%s49)
	muls.l %s41, %s35, %s41
	muls.l %s42, %s35, %s42
	muls.l %s43, %s35, %s43
	muls.l %s44, %s35, %s44
	pvfmad %v1,%v0,%s39,%v10
	pvfmad %v1,%v1,%s40,%v11
	pvfmad %v1,%v1,%s41,%v12
	pvfmad %v1,%v1,%s42,%v13
	ldl.zx %s39, 4(,%s34)
	ldl.zx %s40, 4(,%s45)
	pvfmad %v1,%v1,%s43,%v14
	pvfmad %v1,%v1,%s44,%v15
	muls.l %s39, %s35, %s39
	muls.l %s40, %s35, %s40
	or %s41, 4, %s53
	adds.l %s41, %s34, %s41
	ldl.zx %s41, (,%s41)
	ldl.zx %s42, 4(,%s47)
	or %s43, 4, %s48
	adds.l %s43, %s34, %s43
	ldl.zx %s43, (,%s43)
	ldl.zx %s44, 4(,%s49)
	muls.l %s41, %s35, %s41
	muls.l %s42, %s35, %s42
	muls.l %s43, %s35, %s43
	muls.l %s44, %s35, %s44
	pvfmad %v2,%v0,%s39,%v10
	pvfmad %v2,%v2,%s40,%v11
	pvfmad %v2,%v2,%s41,%v12
	pvfmad %v2,%v2,%s42,%v13
	ldl.zx %s39, 8(,%s34)
	ldl.zx %s40, 8(,%s45)
	pvfmad %v2,%v2,%s43,%v14
	pvfmad %v2,%v2,%s44,%v15
	muls.l %s39, %s35, %s39
	muls.l %s40, %s35, %s40
	ldl.zx %s41, 8(,%s46)
	ldl.zx %s42, 8(,%s47)
	or %s43, 8, %s48
	adds.l %s34, %s34, %s43
	ldl.zx %s34, (,%s34)
	ldl.zx %s43, 8(,%s49)
	muls.l %s41, %s35, %s41
	muls.l %s42, %s35, %s42
	muls.l %s34, %s35, %s34
	muls.l %s35, %s35, %s43
	pvfmad %v0,%v0,%s39,%v10
	pvfmad %v0,%v0,%s40,%v11
	pvfmad %v0,%v0,%s41,%v12
	pvfmad %v0,%v0,%s42,%v13
	pvfmad %v0,%v0,%s34,%v14
	pvfmad %v0,%v0,%s35,%v15
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v0,%v9,%v0
	vst %v1,8,%s36
	vst %v2,8,%s37
	vst %v0,8,%s38
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end2:
	.size	w2v_kernel_N6_HU512_2X_W03, .Lfunc_end2-w2v_kernel_N6_HU512_2X_W03

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI3_0:
	.4byte	1065353216
.LCPI3_1:
	.4byte	0
.LCPI3_2:
	.4byte	3212836864
.LCPI3_3:
	.4byte	1086324736
.LCPI3_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI3_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W04
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W04,@function
w2v_kernel_N6_HU512_2X_W04:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB3_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB3_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s35, -8(,%s9)
	adds.w.sx %s46, %s34, (0)1
	lvl %s46
	ldl.sx %s34, (,%s4)
	ld %s39, 240(,%s9)
	ldl.sx %s37, 4(,%s4)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s36, %s39, %s34
	muls.l %s34, %s37, %s1
	ldl.sx %s38, 8(,%s4)
	sll %s34, %s34, 2
	adds.l %s37, %s39, %s34
	ldl.sx %s34, 12(,%s4)
	muls.l %s38, %s38, %s1
	sll %s38, %s38, 2
	adds.l %s38, %s39, %s38
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s39, %s39, %s34
	vld %v7,8,%s36
	vld %v8,8,%s37
	vld %v9,8,%s38
	vld %v10,8,%s39
	ldl.sx %s34, (,%s5)
	ldl.sx %s41, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s40, %s7, %s34
	muls.l %s34, %s41, %s1
	ldl.sx %s42, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s41, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s42, %s42, %s1
	sll %s42, %s42, 2
	adds.l %s42, %s7, %s42
	muls.l %s34, %s34, %s1
	ldl.sx %s44, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s43, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s44, %s44, %s1
	sll %s44, %s44, 2
	adds.l %s44, %s7, %s44
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s45, %s7, %s34
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	lvl %s46
	pvfmul %v0,%v11,%v7
	pvfmul %v1,%v11,%v8
	pvfmul %v2,%v11,%v9
	pvfmul %v3,%v11,%v10
	vsll %v4,%v0,32
	vsll %v5,%v1,32
	vsll %v6,%v2,32
	vsll %v17,%v3,32
	vfadd.s %v0,%v0,%v4
	vfadd.s %v1,%v1,%v5
	vfadd.s %v2,%v2,%v6
	vfadd.s %v3,%v3,%v17
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	or %s47, 1, (0)1
	lvl %s47
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s48, 4(%s6)
	vstu %v1,4,%s48
	lea %s48, 8(%s6)
	vstu %v2,4,%s48
	lea %s48, 12(%s6)
	vstu %v3,4,%s48
	lvl %s46
	pvfmul %v0,%v12,%v7
	pvfmul %v1,%v12,%v8
	pvfmul %v2,%v12,%v9
	pvfmul %v3,%v12,%v10
	vsll %v4,%v0,32
	vsll %v5,%v1,32
	vsll %v6,%v2,32
	vsll %v17,%v3,32
	vfadd.s %v0,%v0,%v4
	vfadd.s %v1,%v1,%v5
	vfadd.s %v2,%v2,%v6
	vfadd.s %v3,%v3,%v17
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	lvl %s47
	lea %s48, 16(%s6)
	vstu %v0,4,%s48
	lea %s48, 20(%s6)
	vstu %v1,4,%s48
	lea %s48, 24(%s6)
	vstu %v2,4,%s48
	lea %s48, 28(%s6)
	vstu %v3,4,%s48
	lvl %s46
	pvfmul %v0,%v13,%v7
	pvfmul %v1,%v13,%v8
	pvfmul %v2,%v13,%v9
	pvfmul %v3,%v13,%v10
	vsll %v4,%v0,32
	vsll %v5,%v1,32
	vsll %v6,%v2,32
	vsll %v17,%v3,32
	vfadd.s %v0,%v0,%v4
	vfadd.s %v1,%v1,%v5
	vfadd.s %v2,%v2,%v6
	vfadd.s %v3,%v3,%v17
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	lvl %s47
	lea %s48, 32(%s6)
	vstu %v0,4,%s48
	lea %s48, 36(%s6)
	vstu %v1,4,%s48
	lea %s48, 40(%s6)
	vstu %v2,4,%s48
	lea %s48, 44(%s6)
	vstu %v3,4,%s48
	lvl %s46
	pvfmul %v0,%v14,%v7
	pvfmul %v1,%v14,%v8
	pvfmul %v2,%v14,%v9
	pvfmul %v3,%v14,%v10
	vsll %v4,%v0,32
	vsll %v5,%v1,32
	vsll %v6,%v2,32
	vsll %v17,%v3,32
	vfadd.s %v0,%v0,%v4
	vfadd.s %v1,%v1,%v5
	vfadd.s %v2,%v2,%v6
	vfadd.s %v3,%v3,%v17
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	lvl %s47
	lea %s48, 48(%s6)
	vstu %v0,4,%s48
	lea %s48, 52(%s6)
	vstu %v1,4,%s48
	lea %s48, 56(%s6)
	vstu %v2,4,%s48
	lea %s48, 60(%s6)
	vstu %v3,4,%s48
	lvl %s46
	pvfmul %v0,%v15,%v7
	pvfmul %v1,%v15,%v8
	pvfmul %v2,%v15,%v9
	pvfmul %v3,%v15,%v10
	vsll %v4,%v0,32
	vsll %v5,%v1,32
	vsll %v6,%v2,32
	vsll %v17,%v3,32
	vfadd.s %v0,%v0,%v4
	vfadd.s %v1,%v1,%v5
	vfadd.s %v2,%v2,%v6
	vfadd.s %v3,%v3,%v17
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	lvl %s47
	lea %s48, 64(%s6)
	vstu %v0,4,%s48
	lea %s48, 68(%s6)
	vstu %v1,4,%s48
	lea %s48, 72(%s6)
	vstu %v2,4,%s48
	lea %s48, 76(%s6)
	vstu %v3,4,%s48
	lvl %s46
	pvfmul %v0,%v16,%v7
	pvfmul %v1,%v16,%v8
	pvfmul %v2,%v16,%v9
	pvfmul %v3,%v16,%v10
	vsll %v4,%v0,32
	vsll %v5,%v1,32
	vsll %v6,%v2,32
	vsll %v17,%v3,32
	vfadd.s %v0,%v0,%v4
	vfadd.s %v1,%v1,%v5
	vfadd.s %v2,%v2,%v6
	vfadd.s %v3,%v3,%v17
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	lvl %s47
	lea %s47, 80(%s6)
	vstu %v0,4,%s47
	lea %s47, 84(%s6)
	vstu %v1,4,%s47
	lea %s47, 88(%s6)
	vstu %v2,4,%s47
	lea %s47, 92(%s6)
	vstu %v3,4,%s47
	muls.l %s47, %s2, %s0
	adds.w.sx %s47, %s47, (0)1
	lvl %s47
	lea.sl %s47, .LCPI3_0@hi
	ldu %s47, .LCPI3_0@lo(,%s47)
	vseq %v0
	vbrdu %v17,%s47
	adds.w.sx %s48, %s2, (0)1
	vsubs.w.sx %v0,%s48,%v0
	vldu %v18,4,%s6
	lea.sl %s48, .LCPI3_2@hi
	ldu %s48, .LCPI3_2@lo(,%s48)
	vfmk.w.gt %vm1,%v0
	lea.sl %s49, .LCPI3_1@hi
	ldu %s49, .LCPI3_1@lo(,%s49)
	vfmul.s %v0,%s48,%v18
	lea %s48, __vec_expf@lo
	and %s48, %s48, (32)0
	lea.sl %s48, __vec_expf@hi(%s48)
	vbrdu %v17,%s49,%vm1
	bsic %lr, (,%s48)
	lea.sl %s48, .LCPI3_3@hi
	ldu %s48, .LCPI3_3@lo(,%s48)
	vfadd.s %v0,%s47,%v0
	vfsub.s %v1,%s48,%v18
	lea.sl %s48, .LCPI3_4@hi
	ldu %s48, .LCPI3_4@lo(,%s48)
	vfdiv.s %v0,%s47,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s47,%vm1
	vfsub.s %v1,%s48,%v18
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s49,%vm1
	vfsub.s %v0,%v17,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s46
	lea.sl %s46, .LCPI3_5@hi
	ld %s47, .LCPI3_5@lo(,%s46)
	ldl.zx %s48, (,%s34)
	sll %s52, %s2, 2
	adds.l %s46, %s34, %s52
	ldl.zx %s49, (,%s46)
	vbrd %v0,%s47
	muls.l %s53, %s35, %s48
	muls.l %s54, %s35, %s49
	sll %s51, %s2, 3
	adds.l %s47, %s34, %s51
	ldl.zx %s55, (,%s47)
	muls.l %s56, 12, %s2
	adds.l %s48, %s34, %s56
	ldl.zx %s57, (,%s48)
	sll %s49, %s2, 4
	adds.l %s58, %s34, %s49
	ldl.zx %s59, (,%s58)
	muls.l %s60, 20, %s2
	adds.l %s50, %s34, %s60
	ldl.zx %s61, (,%s50)
	muls.l %s55, %s35, %s55
	muls.l %s57, %s35, %s57
	muls.l %s59, %s35, %s59
	muls.l %s61, %s35, %s61
	pvfmad %v1,%v0,%s53,%v7
	pvfmad %v2,%v0,%s54,%v7
	pvfmad %v3,%v0,%s55,%v7
	pvfmad %v4,%v0,%s57,%v7
	ldl.zx %s53, 4(,%s34)
	lea %s54, 4(%s34, %s52)
	ldl.zx %s54, (,%s54)
	pvfmad %v5,%v0,%s59,%v7
	pvfmad %v6,%v0,%s61,%v7
	muls.l %s53, %s35, %s53
	muls.l %s54, %s35, %s54
	lea %s55, 4(%s34, %s51)
	ldl.zx %s55, (,%s55)
	lea %s57, 4(%s34, %s56)
	ldl.zx %s57, (,%s57)
	lea %s59, 4(%s34, %s49)
	ldl.zx %s59, (,%s59)
	lea %s61, 4(%s34, %s60)
	ldl.zx %s61, (,%s61)
	muls.l %s55, %s35, %s55
	muls.l %s57, %s35, %s57
	muls.l %s59, %s35, %s59
	muls.l %s61, %s35, %s61
	pvfmad %v1,%v1,%s53,%v8
	pvfmad %v2,%v2,%s54,%v8
	pvfmad %v3,%v3,%s55,%v8
	pvfmad %v4,%v4,%s57,%v8
	ldl.zx %s53, 8(,%s34)
	lea %s54, 8(%s34, %s52)
	ldl.zx %s54, (,%s54)
	pvfmad %v5,%v5,%s59,%v8
	pvfmad %v6,%v6,%s61,%v8
	muls.l %s53, %s35, %s53
	muls.l %s54, %s35, %s54
	lea %s55, 8(%s34, %s51)
	ldl.zx %s55, (,%s55)
	lea %s57, 8(%s34, %s56)
	ldl.zx %s57, (,%s57)
	lea %s59, 8(%s34, %s49)
	ldl.zx %s59, (,%s59)
	lea %s61, 8(%s34, %s60)
	ldl.zx %s61, (,%s61)
	muls.l %s55, %s35, %s55
	muls.l %s57, %s35, %s57
	muls.l %s59, %s35, %s59
	muls.l %s61, %s35, %s61
	pvfmad %v1,%v1,%s53,%v9
	pvfmad %v2,%v2,%s54,%v9
	pvfmad %v3,%v3,%s55,%v9
	pvfmad %v4,%v4,%s57,%v9
	ldl.zx %s53, 12(,%s34)
	lea %s52, 12(%s34, %s52)
	ldl.zx %s52, (,%s52)
	pvfmad %v5,%v5,%s59,%v9
	pvfmad %v6,%v6,%s61,%v9
	muls.l %s53, %s35, %s53
	muls.l %s52, %s35, %s52
	lea %s54, 12(%s34, %s51)
	ldl.zx %s54, (,%s54)
	lea %s55, 12(%s34, %s56)
	ldl.zx %s55, (,%s55)
	lea %s56, 12(%s34, %s49)
	ldl.zx %s56, (,%s56)
	lea %s57, 12(%s34, %s60)
	ldl.zx %s57, (,%s57)
	muls.l %s54, %s35, %s54
	muls.l %s55, %s35, %s55
	muls.l %s56, %s35, %s56
	muls.l %s57, %s35, %s57
	pvfmad %v1,%v1,%s53,%v10
	pvfmad %v2,%v2,%s52,%v10
	pvfmad %v3,%v3,%s54,%v10
	pvfmad %v4,%v4,%s55,%v10
	pvfmad %v5,%v5,%s56,%v10
	pvfmad %v6,%v6,%s57,%v10
	pvfadd %v1,%v11,%v1
	pvfadd %v2,%v12,%v2
	pvfadd %v3,%v13,%v3
	pvfadd %v4,%v14,%v4
	pvfadd %v5,%v15,%v5
	pvfadd %v6,%v16,%v6
	vst %v1,8,%s40
	vst %v2,8,%s41
	vst %v3,8,%s42
	vst %v4,8,%s43
	vst %v5,8,%s44
	vst %v6,8,%s45
	ldl.zx %s40, (,%s34)
	ldl.zx %s41, (,%s46)
	muls.l %s40, %s35, %s40
	muls.l %s41, %s35, %s41
	ldl.zx %s42, (,%s47)
	ldl.zx %s43, (,%s48)
	ldl.zx %s44, (,%s58)
	ldl.zx %s45, (,%s50)
	muls.l %s42, %s35, %s42
	muls.l %s43, %s35, %s43
	muls.l %s44, %s35, %s44
	muls.l %s45, %s35, %s45
	pvfmad %v1,%v0,%s40,%v11
	pvfmad %v1,%v1,%s41,%v12
	pvfmad %v1,%v1,%s42,%v13
	pvfmad %v1,%v1,%s43,%v14
	ldl.zx %s40, 4(,%s34)
	ldl.zx %s41, 4(,%s46)
	pvfmad %v1,%v1,%s44,%v15
	pvfmad %v1,%v1,%s45,%v16
	muls.l %s40, %s35, %s40
	muls.l %s41, %s35, %s41
	or %s42, 4, %s51
	adds.l %s42, %s34, %s42
	ldl.zx %s42, (,%s42)
	ldl.zx %s43, 4(,%s48)
	or %s44, 4, %s49
	adds.l %s44, %s34, %s44
	ldl.zx %s44, (,%s44)
	ldl.zx %s45, 4(,%s50)
	muls.l %s42, %s35, %s42
	muls.l %s43, %s35, %s43
	muls.l %s44, %s35, %s44
	muls.l %s45, %s35, %s45
	pvfmad %v2,%v0,%s40,%v11
	pvfmad %v2,%v2,%s41,%v12
	pvfmad %v2,%v2,%s42,%v13
	pvfmad %v2,%v2,%s43,%v14
	ldl.zx %s40, 8(,%s34)
	ldl.zx %s41, 8(,%s46)
	pvfmad %v2,%v2,%s44,%v15
	pvfmad %v2,%v2,%s45,%v16
	muls.l %s40, %s35, %s40
	muls.l %s41, %s35, %s41
	ldl.zx %s42, 8(,%s47)
	ldl.zx %s43, 8(,%s48)
	or %s44, 8, %s49
	adds.l %s44, %s34, %s44
	ldl.zx %s44, (,%s44)
	ldl.zx %s45, 8(,%s50)
	muls.l %s42, %s35, %s42
	muls.l %s43, %s35, %s43
	muls.l %s44, %s35, %s44
	muls.l %s45, %s35, %s45
	pvfmad %v3,%v0,%s40,%v11
	pvfmad %v3,%v3,%s41,%v12
	pvfmad %v3,%v3,%s42,%v13
	pvfmad %v3,%v3,%s43,%v14
	ldl.zx %s40, 12(,%s34)
	ldl.zx %s41, 12(,%s46)
	pvfmad %v3,%v3,%s44,%v15
	pvfmad %v3,%v3,%s45,%v16
	muls.l %s40, %s35, %s40
	muls.l %s41, %s35, %s41
	ldl.zx %s42, 12(,%s47)
	ldl.zx %s43, 12(,%s48)
	or %s44, 12, %s49
	adds.l %s34, %s34, %s44
	ldl.zx %s34, (,%s34)
	ldl.zx %s44, 12(,%s50)
	muls.l %s42, %s35, %s42
	muls.l %s43, %s35, %s43
	muls.l %s34, %s35, %s34
	muls.l %s35, %s35, %s44
	pvfmad %v0,%v0,%s40,%v11
	pvfmad %v0,%v0,%s41,%v12
	pvfmad %v0,%v0,%s42,%v13
	pvfmad %v0,%v0,%s43,%v14
	pvfmad %v0,%v0,%s34,%v15
	pvfmad %v0,%v0,%s35,%v16
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v0,%v10,%v0
	vst %v1,8,%s36
	vst %v2,8,%s37
	vst %v3,8,%s38
	vst %v0,8,%s39
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end3:
	.size	w2v_kernel_N6_HU512_2X_W04, .Lfunc_end3-w2v_kernel_N6_HU512_2X_W04

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI4_0:
	.4byte	1065353216
.LCPI4_1:
	.4byte	0
.LCPI4_2:
	.4byte	3212836864
.LCPI4_3:
	.4byte	1086324736
.LCPI4_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI4_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W05
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W05,@function
w2v_kernel_N6_HU512_2X_W05:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB4_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB4_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s36, -8(,%s9)
	adds.w.sx %s47, %s34, (0)1
	lvl %s47
	ldl.sx %s34, (,%s4)
	ld %s40, 240(,%s9)
	muls.l %s34, %s34, %s1
	ldl.sx %s37, 4(,%s4)
	sll %s34, %s34, 2
	adds.l %s35, %s40, %s34
	ldl.sx %s34, 8(,%s4)
	muls.l %s37, %s37, %s1
	sll %s37, %s37, 2
	adds.l %s37, %s40, %s37
	muls.l %s34, %s34, %s1
	ldl.sx %s39, 12(,%s4)
	sll %s34, %s34, 2
	adds.l %s38, %s40, %s34
	ldl.sx %s34, 16(,%s4)
	muls.l %s39, %s39, %s1
	sll %s39, %s39, 2
	adds.l %s39, %s40, %s39
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s40, %s40, %s34
	vld %v7,8,%s35
	vld %v8,8,%s37
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	ldl.sx %s34, (,%s5)
	ldl.sx %s42, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s41, %s7, %s34
	muls.l %s34, %s42, %s1
	ldl.sx %s43, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s42, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s43, %s43, %s1
	sll %s43, %s43, 2
	adds.l %s43, %s7, %s43
	muls.l %s34, %s34, %s1
	ldl.sx %s45, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s44, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s45, %s45, %s1
	sll %s45, %s45, 2
	adds.l %s45, %s7, %s45
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s46, %s7, %s34
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	lvl %s47
	pvfmul %v0,%v12,%v7
	pvfmul %v1,%v12,%v8
	pvfmul %v2,%v12,%v9
	pvfmul %v3,%v12,%v10
	pvfmul %v4,%v12,%v11
	vsll %v5,%v0,32
	vsll %v6,%v1,32
	vsll %v18,%v2,32
	vsll %v19,%v3,32
	vsll %v20,%v4,32
	vfadd.s %v0,%v0,%v5
	vfadd.s %v1,%v1,%v6
	vfadd.s %v2,%v2,%v18
	vfadd.s %v3,%v3,%v19
	vfadd.s %v4,%v4,%v20
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	or %s48, 1, (0)1
	lvl %s48
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s49, 4(%s6)
	vstu %v1,4,%s49
	lea %s49, 8(%s6)
	vstu %v2,4,%s49
	lea %s49, 12(%s6)
	vstu %v3,4,%s49
	lea %s49, 16(%s6)
	vstu %v4,4,%s49
	lvl %s47
	pvfmul %v0,%v13,%v7
	pvfmul %v1,%v13,%v8
	pvfmul %v2,%v13,%v9
	pvfmul %v3,%v13,%v10
	pvfmul %v4,%v13,%v11
	vsll %v5,%v0,32
	vsll %v6,%v1,32
	vsll %v18,%v2,32
	vsll %v19,%v3,32
	vsll %v20,%v4,32
	vfadd.s %v0,%v0,%v5
	vfadd.s %v1,%v1,%v6
	vfadd.s %v2,%v2,%v18
	vfadd.s %v3,%v3,%v19
	vfadd.s %v4,%v4,%v20
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	lvl %s48
	lea %s49, 20(%s6)
	vstu %v0,4,%s49
	lea %s49, 24(%s6)
	vstu %v1,4,%s49
	lea %s49, 28(%s6)
	vstu %v2,4,%s49
	lea %s49, 32(%s6)
	vstu %v3,4,%s49
	lea %s49, 36(%s6)
	vstu %v4,4,%s49
	lvl %s47
	pvfmul %v0,%v14,%v7
	pvfmul %v1,%v14,%v8
	pvfmul %v2,%v14,%v9
	pvfmul %v3,%v14,%v10
	pvfmul %v4,%v14,%v11
	vsll %v5,%v0,32
	vsll %v6,%v1,32
	vsll %v18,%v2,32
	vsll %v19,%v3,32
	vsll %v20,%v4,32
	vfadd.s %v0,%v0,%v5
	vfadd.s %v1,%v1,%v6
	vfadd.s %v2,%v2,%v18
	vfadd.s %v3,%v3,%v19
	vfadd.s %v4,%v4,%v20
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	lvl %s48
	lea %s49, 40(%s6)
	vstu %v0,4,%s49
	lea %s49, 44(%s6)
	vstu %v1,4,%s49
	lea %s49, 48(%s6)
	vstu %v2,4,%s49
	lea %s49, 52(%s6)
	vstu %v3,4,%s49
	lea %s49, 56(%s6)
	vstu %v4,4,%s49
	lvl %s47
	pvfmul %v0,%v15,%v7
	pvfmul %v1,%v15,%v8
	pvfmul %v2,%v15,%v9
	pvfmul %v3,%v15,%v10
	pvfmul %v4,%v15,%v11
	vsll %v5,%v0,32
	vsll %v6,%v1,32
	vsll %v18,%v2,32
	vsll %v19,%v3,32
	vsll %v20,%v4,32
	vfadd.s %v0,%v0,%v5
	vfadd.s %v1,%v1,%v6
	vfadd.s %v2,%v2,%v18
	vfadd.s %v3,%v3,%v19
	vfadd.s %v4,%v4,%v20
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	lvl %s48
	lea %s49, 60(%s6)
	vstu %v0,4,%s49
	lea %s49, 64(%s6)
	vstu %v1,4,%s49
	lea %s49, 68(%s6)
	vstu %v2,4,%s49
	lea %s49, 72(%s6)
	vstu %v3,4,%s49
	lea %s49, 76(%s6)
	vstu %v4,4,%s49
	lvl %s47
	pvfmul %v0,%v16,%v7
	pvfmul %v1,%v16,%v8
	pvfmul %v2,%v16,%v9
	pvfmul %v3,%v16,%v10
	pvfmul %v4,%v16,%v11
	vsll %v5,%v0,32
	vsll %v6,%v1,32
	vsll %v18,%v2,32
	vsll %v19,%v3,32
	vsll %v20,%v4,32
	vfadd.s %v0,%v0,%v5
	vfadd.s %v1,%v1,%v6
	vfadd.s %v2,%v2,%v18
	vfadd.s %v3,%v3,%v19
	vfadd.s %v4,%v4,%v20
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	lvl %s48
	lea %s49, 80(%s6)
	vstu %v0,4,%s49
	lea %s49, 84(%s6)
	vstu %v1,4,%s49
	lea %s49, 88(%s6)
	vstu %v2,4,%s49
	lea %s49, 92(%s6)
	vstu %v3,4,%s49
	lea %s49, 96(%s6)
	vstu %v4,4,%s49
	lvl %s47
	pvfmul %v0,%v17,%v7
	pvfmul %v1,%v17,%v8
	pvfmul %v2,%v17,%v9
	pvfmul %v3,%v17,%v10
	pvfmul %v4,%v17,%v11
	vsll %v5,%v0,32
	vsll %v6,%v1,32
	vsll %v18,%v2,32
	vsll %v19,%v3,32
	vsll %v20,%v4,32
	vfadd.s %v0,%v0,%v5
	vfadd.s %v1,%v1,%v6
	vfadd.s %v2,%v2,%v18
	vfadd.s %v3,%v3,%v19
	vfadd.s %v4,%v4,%v20
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	lvl %s48
	lea %s48, 100(%s6)
	vstu %v0,4,%s48
	lea %s48, 104(%s6)
	vstu %v1,4,%s48
	lea %s48, 108(%s6)
	vstu %v2,4,%s48
	lea %s48, 112(%s6)
	vstu %v3,4,%s48
	lea %s48, 116(%s6)
	vstu %v4,4,%s48
	muls.l %s48, %s2, %s0
	adds.w.sx %s48, %s48, (0)1
	lvl %s48
	lea.sl %s48, .LCPI4_0@hi
	ldu %s48, .LCPI4_0@lo(,%s48)
	vseq %v0
	vbrdu %v18,%s48
	adds.w.sx %s49, %s2, (0)1
	vsubs.w.sx %v0,%s49,%v0
	vldu %v19,4,%s6
	lea.sl %s49, .LCPI4_2@hi
	ldu %s49, .LCPI4_2@lo(,%s49)
	vfmk.w.gt %vm1,%v0
	lea.sl %s50, .LCPI4_1@hi
	ldu %s50, .LCPI4_1@lo(,%s50)
	vfmul.s %v0,%s49,%v19
	lea %s49, __vec_expf@lo
	and %s49, %s49, (32)0
	lea.sl %s49, __vec_expf@hi(%s49)
	vbrdu %v18,%s50,%vm1
	bsic %lr, (,%s49)
	lea.sl %s49, .LCPI4_3@hi
	ldu %s49, .LCPI4_3@lo(,%s49)
	vfadd.s %v0,%s48,%v0
	vfsub.s %v1,%s49,%v19
	lea.sl %s49, .LCPI4_4@hi
	ldu %s49, .LCPI4_4@lo(,%s49)
	vfdiv.s %v0,%s48,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s48,%vm1
	vfsub.s %v1,%s49,%v19
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s50,%vm1
	vfsub.s %v0,%v18,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s47
	lea.sl %s47, .LCPI4_5@hi
	ld %s48, .LCPI4_5@lo(,%s47)
	ldl.zx %s49, (,%s34)
	sll %s54, %s2, 2
	adds.l %s47, %s34, %s54
	ldl.zx %s50, (,%s47)
	vbrd %v0,%s48
	muls.l %s55, %s36, %s49
	muls.l %s56, %s36, %s50
	sll %s53, %s2, 3
	adds.l %s48, %s34, %s53
	ldl.zx %s57, (,%s48)
	muls.l %s58, 12, %s2
	adds.l %s49, %s34, %s58
	ldl.zx %s59, (,%s49)
	sll %s52, %s2, 4
	adds.l %s50, %s34, %s52
	ldl.zx %s60, (,%s50)
	muls.l %s61, 20, %s2
	adds.l %s51, %s34, %s61
	ldl.zx %s62, (,%s51)
	muls.l %s57, %s36, %s57
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	muls.l %s62, %s36, %s62
	pvfmad %v1,%v0,%s55,%v7
	pvfmad %v2,%v0,%s56,%v7
	pvfmad %v3,%v0,%s57,%v7
	pvfmad %v4,%v0,%s59,%v7
	ldl.zx %s55, 4(,%s34)
	lea %s56, 4(%s34, %s54)
	ldl.zx %s56, (,%s56)
	pvfmad %v5,%v0,%s60,%v7
	pvfmad %v6,%v0,%s62,%v7
	muls.l %s55, %s36, %s55
	muls.l %s56, %s36, %s56
	lea %s57, 4(%s34, %s53)
	ldl.zx %s57, (,%s57)
	lea %s59, 4(%s34, %s58)
	ldl.zx %s59, (,%s59)
	lea %s60, 4(%s34, %s52)
	ldl.zx %s60, (,%s60)
	lea %s62, 4(%s34, %s61)
	ldl.zx %s62, (,%s62)
	muls.l %s57, %s36, %s57
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	muls.l %s62, %s36, %s62
	pvfmad %v1,%v1,%s55,%v8
	pvfmad %v2,%v2,%s56,%v8
	pvfmad %v3,%v3,%s57,%v8
	pvfmad %v4,%v4,%s59,%v8
	ldl.zx %s55, 8(,%s34)
	lea %s56, 8(%s34, %s54)
	ldl.zx %s56, (,%s56)
	pvfmad %v5,%v5,%s60,%v8
	pvfmad %v6,%v6,%s62,%v8
	muls.l %s55, %s36, %s55
	muls.l %s56, %s36, %s56
	lea %s57, 8(%s34, %s53)
	ldl.zx %s57, (,%s57)
	lea %s59, 8(%s34, %s58)
	ldl.zx %s59, (,%s59)
	lea %s60, 8(%s34, %s52)
	ldl.zx %s60, (,%s60)
	lea %s62, 8(%s34, %s61)
	ldl.zx %s62, (,%s62)
	muls.l %s57, %s36, %s57
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	muls.l %s62, %s36, %s62
	pvfmad %v1,%v1,%s55,%v9
	pvfmad %v2,%v2,%s56,%v9
	pvfmad %v3,%v3,%s57,%v9
	pvfmad %v4,%v4,%s59,%v9
	ldl.zx %s55, 12(,%s34)
	lea %s56, 12(%s34, %s54)
	ldl.zx %s56, (,%s56)
	pvfmad %v5,%v5,%s60,%v9
	pvfmad %v6,%v6,%s62,%v9
	muls.l %s55, %s36, %s55
	muls.l %s56, %s36, %s56
	lea %s57, 12(%s34, %s53)
	ldl.zx %s57, (,%s57)
	lea %s59, 12(%s34, %s58)
	ldl.zx %s59, (,%s59)
	lea %s60, 12(%s34, %s52)
	ldl.zx %s60, (,%s60)
	lea %s62, 12(%s34, %s61)
	ldl.zx %s62, (,%s62)
	muls.l %s57, %s36, %s57
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	muls.l %s62, %s36, %s62
	pvfmad %v1,%v1,%s55,%v10
	pvfmad %v2,%v2,%s56,%v10
	pvfmad %v3,%v3,%s57,%v10
	pvfmad %v4,%v4,%s59,%v10
	ldl.zx %s55, 16(,%s34)
	lea %s54, 16(%s34, %s54)
	ldl.zx %s54, (,%s54)
	pvfmad %v5,%v5,%s60,%v10
	pvfmad %v6,%v6,%s62,%v10
	muls.l %s55, %s36, %s55
	muls.l %s54, %s36, %s54
	lea %s56, 16(%s34, %s53)
	ldl.zx %s56, (,%s56)
	lea %s57, 16(%s34, %s58)
	ldl.zx %s57, (,%s57)
	lea %s58, 16(%s34, %s52)
	ldl.zx %s58, (,%s58)
	lea %s59, 16(%s34, %s61)
	ldl.zx %s59, (,%s59)
	muls.l %s56, %s36, %s56
	muls.l %s57, %s36, %s57
	muls.l %s58, %s36, %s58
	muls.l %s59, %s36, %s59
	pvfmad %v1,%v1,%s55,%v11
	pvfmad %v2,%v2,%s54,%v11
	pvfmad %v3,%v3,%s56,%v11
	pvfmad %v4,%v4,%s57,%v11
	pvfmad %v5,%v5,%s58,%v11
	pvfmad %v6,%v6,%s59,%v11
	pvfadd %v1,%v12,%v1
	pvfadd %v2,%v13,%v2
	pvfadd %v3,%v14,%v3
	pvfadd %v4,%v15,%v4
	pvfadd %v5,%v16,%v5
	pvfadd %v6,%v17,%v6
	vst %v1,8,%s41
	vst %v2,8,%s42
	vst %v3,8,%s43
	vst %v4,8,%s44
	vst %v5,8,%s45
	vst %v6,8,%s46
	ldl.zx %s41, (,%s34)
	ldl.zx %s42, (,%s47)
	muls.l %s41, %s36, %s41
	muls.l %s42, %s36, %s42
	ldl.zx %s43, (,%s48)
	ldl.zx %s44, (,%s49)
	ldl.zx %s45, (,%s50)
	ldl.zx %s46, (,%s51)
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	pvfmad %v1,%v0,%s41,%v12
	pvfmad %v1,%v1,%s42,%v13
	pvfmad %v1,%v1,%s43,%v14
	pvfmad %v1,%v1,%s44,%v15
	ldl.zx %s41, 4(,%s34)
	ldl.zx %s42, 4(,%s47)
	pvfmad %v1,%v1,%s45,%v16
	pvfmad %v1,%v1,%s46,%v17
	muls.l %s41, %s36, %s41
	muls.l %s42, %s36, %s42
	or %s43, 4, %s53
	adds.l %s43, %s34, %s43
	ldl.zx %s43, (,%s43)
	ldl.zx %s44, 4(,%s49)
	or %s45, 4, %s52
	adds.l %s45, %s34, %s45
	ldl.zx %s45, (,%s45)
	ldl.zx %s46, 4(,%s51)
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	pvfmad %v2,%v0,%s41,%v12
	pvfmad %v2,%v2,%s42,%v13
	pvfmad %v2,%v2,%s43,%v14
	pvfmad %v2,%v2,%s44,%v15
	ldl.zx %s41, 8(,%s34)
	ldl.zx %s42, 8(,%s47)
	pvfmad %v2,%v2,%s45,%v16
	pvfmad %v2,%v2,%s46,%v17
	muls.l %s41, %s36, %s41
	muls.l %s42, %s36, %s42
	ldl.zx %s43, 8(,%s48)
	ldl.zx %s44, 8(,%s49)
	or %s45, 8, %s52
	adds.l %s45, %s34, %s45
	ldl.zx %s45, (,%s45)
	ldl.zx %s46, 8(,%s51)
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	pvfmad %v3,%v0,%s41,%v12
	pvfmad %v3,%v3,%s42,%v13
	pvfmad %v3,%v3,%s43,%v14
	pvfmad %v3,%v3,%s44,%v15
	ldl.zx %s41, 12(,%s34)
	ldl.zx %s42, 12(,%s47)
	pvfmad %v3,%v3,%s45,%v16
	pvfmad %v3,%v3,%s46,%v17
	muls.l %s41, %s36, %s41
	muls.l %s42, %s36, %s42
	ldl.zx %s43, 12(,%s48)
	ldl.zx %s44, 12(,%s49)
	or %s45, 12, %s52
	adds.l %s45, %s34, %s45
	ldl.zx %s45, (,%s45)
	ldl.zx %s46, 12(,%s51)
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	pvfmad %v4,%v0,%s41,%v12
	pvfmad %v4,%v4,%s42,%v13
	pvfmad %v4,%v4,%s43,%v14
	pvfmad %v4,%v4,%s44,%v15
	ldl.zx %s34, 16(,%s34)
	ldl.zx %s41, 16(,%s47)
	pvfmad %v4,%v4,%s45,%v16
	pvfmad %v4,%v4,%s46,%v17
	muls.l %s34, %s36, %s34
	muls.l %s41, %s36, %s41
	ldl.zx %s42, 16(,%s48)
	ldl.zx %s43, 16(,%s49)
	ldl.zx %s44, 16(,%s50)
	ldl.zx %s45, 16(,%s51)
	muls.l %s42, %s36, %s42
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	muls.l %s36, %s36, %s45
	pvfmad %v0,%v0,%s34,%v12
	pvfmad %v0,%v0,%s41,%v13
	pvfmad %v0,%v0,%s42,%v14
	pvfmad %v0,%v0,%s43,%v15
	pvfmad %v0,%v0,%s44,%v16
	pvfmad %v0,%v0,%s36,%v17
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v0,%v11,%v0
	vst %v1,8,%s35
	vst %v2,8,%s37
	vst %v3,8,%s38
	vst %v4,8,%s39
	vst %v0,8,%s40
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end4:
	.size	w2v_kernel_N6_HU512_2X_W05, .Lfunc_end4-w2v_kernel_N6_HU512_2X_W05

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI5_0:
	.4byte	1065353216
.LCPI5_1:
	.4byte	0
.LCPI5_2:
	.4byte	3212836864
.LCPI5_3:
	.4byte	1086324736
.LCPI5_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI5_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W06
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W06,@function
w2v_kernel_N6_HU512_2X_W06:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB5_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB5_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s36, -8(,%s9)
	adds.w.sx %s48, %s34, (0)1
	lvl %s48
	ldl.sx %s34, (,%s4)
	ld %s41, 240(,%s9)
	ldl.sx %s37, 4(,%s4)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s35, %s41, %s34
	muls.l %s34, %s37, %s1
	ldl.sx %s38, 8(,%s4)
	sll %s34, %s34, 2
	adds.l %s37, %s41, %s34
	ldl.sx %s34, 12(,%s4)
	muls.l %s38, %s38, %s1
	sll %s38, %s38, 2
	adds.l %s38, %s41, %s38
	muls.l %s34, %s34, %s1
	ldl.sx %s40, 16(,%s4)
	sll %s34, %s34, 2
	adds.l %s39, %s41, %s34
	ldl.sx %s34, 20(,%s4)
	muls.l %s40, %s40, %s1
	sll %s40, %s40, 2
	adds.l %s40, %s41, %s40
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s41, %s41, %s34
	vld %v7,8,%s35
	vld %v8,8,%s37
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	ldl.sx %s34, (,%s5)
	ldl.sx %s43, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s42, %s7, %s34
	muls.l %s34, %s43, %s1
	ldl.sx %s44, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s43, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s44, %s44, %s1
	sll %s44, %s44, 2
	adds.l %s44, %s7, %s44
	muls.l %s34, %s34, %s1
	ldl.sx %s46, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s45, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s46, %s46, %s1
	sll %s46, %s46, 2
	adds.l %s46, %s7, %s46
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s47, %s7, %s34
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	lvl %s48
	pvfmul %v0,%v13,%v7
	pvfmul %v1,%v13,%v8
	pvfmul %v2,%v13,%v9
	pvfmul %v3,%v13,%v10
	pvfmul %v4,%v13,%v11
	pvfmul %v5,%v13,%v12
	vsll %v6,%v0,32
	vsll %v19,%v1,32
	vsll %v20,%v2,32
	vsll %v21,%v3,32
	vsll %v22,%v4,32
	vsll %v23,%v5,32
	vfadd.s %v0,%v0,%v6
	vfadd.s %v1,%v1,%v19
	vfadd.s %v2,%v2,%v20
	vfadd.s %v3,%v3,%v21
	vfadd.s %v4,%v4,%v22
	vfadd.s %v5,%v5,%v23
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	or %s49, 1, (0)1
	lvl %s49
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s50, 4(%s6)
	vstu %v1,4,%s50
	lea %s50, 8(%s6)
	vstu %v2,4,%s50
	lea %s50, 12(%s6)
	vstu %v3,4,%s50
	lea %s50, 16(%s6)
	vstu %v4,4,%s50
	lea %s50, 20(%s6)
	vstu %v5,4,%s50
	lvl %s48
	pvfmul %v0,%v14,%v7
	pvfmul %v1,%v14,%v8
	pvfmul %v2,%v14,%v9
	pvfmul %v3,%v14,%v10
	pvfmul %v4,%v14,%v11
	pvfmul %v5,%v14,%v12
	vsll %v6,%v0,32
	vsll %v19,%v1,32
	vsll %v20,%v2,32
	vsll %v21,%v3,32
	vsll %v22,%v4,32
	vsll %v23,%v5,32
	vfadd.s %v0,%v0,%v6
	vfadd.s %v1,%v1,%v19
	vfadd.s %v2,%v2,%v20
	vfadd.s %v3,%v3,%v21
	vfadd.s %v4,%v4,%v22
	vfadd.s %v5,%v5,%v23
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	lvl %s49
	lea %s50, 24(%s6)
	vstu %v0,4,%s50
	lea %s50, 28(%s6)
	vstu %v1,4,%s50
	lea %s50, 32(%s6)
	vstu %v2,4,%s50
	lea %s50, 36(%s6)
	vstu %v3,4,%s50
	lea %s50, 40(%s6)
	vstu %v4,4,%s50
	lea %s50, 44(%s6)
	vstu %v5,4,%s50
	lvl %s48
	pvfmul %v0,%v15,%v7
	pvfmul %v1,%v15,%v8
	pvfmul %v2,%v15,%v9
	pvfmul %v3,%v15,%v10
	pvfmul %v4,%v15,%v11
	pvfmul %v5,%v15,%v12
	vsll %v6,%v0,32
	vsll %v19,%v1,32
	vsll %v20,%v2,32
	vsll %v21,%v3,32
	vsll %v22,%v4,32
	vsll %v23,%v5,32
	vfadd.s %v0,%v0,%v6
	vfadd.s %v1,%v1,%v19
	vfadd.s %v2,%v2,%v20
	vfadd.s %v3,%v3,%v21
	vfadd.s %v4,%v4,%v22
	vfadd.s %v5,%v5,%v23
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	lvl %s49
	lea %s50, 48(%s6)
	vstu %v0,4,%s50
	lea %s50, 52(%s6)
	vstu %v1,4,%s50
	lea %s50, 56(%s6)
	vstu %v2,4,%s50
	lea %s50, 60(%s6)
	vstu %v3,4,%s50
	lea %s50, 64(%s6)
	vstu %v4,4,%s50
	lea %s50, 68(%s6)
	vstu %v5,4,%s50
	lvl %s48
	pvfmul %v0,%v16,%v7
	pvfmul %v1,%v16,%v8
	pvfmul %v2,%v16,%v9
	pvfmul %v3,%v16,%v10
	pvfmul %v4,%v16,%v11
	pvfmul %v5,%v16,%v12
	vsll %v6,%v0,32
	vsll %v19,%v1,32
	vsll %v20,%v2,32
	vsll %v21,%v3,32
	vsll %v22,%v4,32
	vsll %v23,%v5,32
	vfadd.s %v0,%v0,%v6
	vfadd.s %v1,%v1,%v19
	vfadd.s %v2,%v2,%v20
	vfadd.s %v3,%v3,%v21
	vfadd.s %v4,%v4,%v22
	vfadd.s %v5,%v5,%v23
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	lvl %s49
	lea %s50, 72(%s6)
	vstu %v0,4,%s50
	lea %s50, 76(%s6)
	vstu %v1,4,%s50
	lea %s50, 80(%s6)
	vstu %v2,4,%s50
	lea %s50, 84(%s6)
	vstu %v3,4,%s50
	lea %s50, 88(%s6)
	vstu %v4,4,%s50
	lea %s50, 92(%s6)
	vstu %v5,4,%s50
	lvl %s48
	pvfmul %v0,%v17,%v7
	pvfmul %v1,%v17,%v8
	pvfmul %v2,%v17,%v9
	pvfmul %v3,%v17,%v10
	pvfmul %v4,%v17,%v11
	pvfmul %v5,%v17,%v12
	vsll %v6,%v0,32
	vsll %v19,%v1,32
	vsll %v20,%v2,32
	vsll %v21,%v3,32
	vsll %v22,%v4,32
	vsll %v23,%v5,32
	vfadd.s %v0,%v0,%v6
	vfadd.s %v1,%v1,%v19
	vfadd.s %v2,%v2,%v20
	vfadd.s %v3,%v3,%v21
	vfadd.s %v4,%v4,%v22
	vfadd.s %v5,%v5,%v23
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	lvl %s49
	lea %s50, 96(%s6)
	vstu %v0,4,%s50
	lea %s50, 100(%s6)
	vstu %v1,4,%s50
	lea %s50, 104(%s6)
	vstu %v2,4,%s50
	lea %s50, 108(%s6)
	vstu %v3,4,%s50
	lea %s50, 112(%s6)
	vstu %v4,4,%s50
	lea %s50, 116(%s6)
	vstu %v5,4,%s50
	lvl %s48
	pvfmul %v0,%v18,%v7
	pvfmul %v1,%v18,%v8
	pvfmul %v2,%v18,%v9
	pvfmul %v3,%v18,%v10
	pvfmul %v4,%v18,%v11
	pvfmul %v5,%v18,%v12
	vsll %v6,%v0,32
	vsll %v19,%v1,32
	vsll %v20,%v2,32
	vsll %v21,%v3,32
	vsll %v22,%v4,32
	vsll %v23,%v5,32
	vfadd.s %v0,%v0,%v6
	vfadd.s %v1,%v1,%v19
	vfadd.s %v2,%v2,%v20
	vfadd.s %v3,%v3,%v21
	vfadd.s %v4,%v4,%v22
	vfadd.s %v5,%v5,%v23
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	lvl %s49
	lea %s49, 120(%s6)
	vstu %v0,4,%s49
	lea %s49, 124(%s6)
	vstu %v1,4,%s49
	lea %s49, 128(%s6)
	vstu %v2,4,%s49
	lea %s49, 132(%s6)
	vstu %v3,4,%s49
	lea %s49, 136(%s6)
	vstu %v4,4,%s49
	lea %s49, 140(%s6)
	vstu %v5,4,%s49
	muls.l %s49, %s2, %s0
	adds.w.sx %s49, %s49, (0)1
	lvl %s49
	lea.sl %s49, .LCPI5_0@hi
	ldu %s49, .LCPI5_0@lo(,%s49)
	vseq %v0
	vbrdu %v19,%s49
	adds.w.sx %s50, %s2, (0)1
	vsubs.w.sx %v0,%s50,%v0
	vldu %v20,4,%s6
	lea.sl %s50, .LCPI5_2@hi
	ldu %s50, .LCPI5_2@lo(,%s50)
	vfmk.w.gt %vm1,%v0
	lea.sl %s51, .LCPI5_1@hi
	ldu %s51, .LCPI5_1@lo(,%s51)
	vfmul.s %v0,%s50,%v20
	lea %s50, __vec_expf@lo
	and %s50, %s50, (32)0
	lea.sl %s50, __vec_expf@hi(%s50)
	vbrdu %v19,%s51,%vm1
	bsic %lr, (,%s50)
	lea.sl %s50, .LCPI5_3@hi
	ldu %s50, .LCPI5_3@lo(,%s50)
	vfadd.s %v0,%s49,%v0
	vfsub.s %v1,%s50,%v20
	lea.sl %s50, .LCPI5_4@hi
	ldu %s50, .LCPI5_4@lo(,%s50)
	vfdiv.s %v0,%s49,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s49,%vm1
	vfsub.s %v1,%s50,%v20
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s51,%vm1
	vfsub.s %v0,%v19,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s48
	lea.sl %s48, .LCPI5_5@hi
	ld %s49, .LCPI5_5@lo(,%s48)
	ldl.zx %s50, (,%s34)
	sll %s55, %s2, 2
	adds.l %s48, %s34, %s55
	ldl.zx %s51, (,%s48)
	vbrd %v0,%s49
	muls.l %s58, %s36, %s50
	muls.l %s59, %s36, %s51
	sll %s54, %s2, 3
	adds.l %s49, %s34, %s54
	ldl.zx %s60, (,%s49)
	muls.l %s56, 12, %s2
	adds.l %s50, %s34, %s56
	ldl.zx %s61, (,%s50)
	sll %s53, %s2, 4
	adds.l %s51, %s34, %s53
	ldl.zx %s62, (,%s51)
	muls.l %s57, 20, %s2
	adds.l %s52, %s34, %s57
	ldl.zx %s63, (,%s52)
	muls.l %s60, %s36, %s60
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	pvfmad %v1,%v0,%s58,%v7
	pvfmad %v2,%v0,%s59,%v7
	pvfmad %v3,%v0,%s60,%v7
	pvfmad %v4,%v0,%s61,%v7
	ldl.zx %s58, 4(,%s34)
	lea %s59, 4(%s34, %s55)
	ldl.zx %s59, (,%s59)
	pvfmad %v5,%v0,%s62,%v7
	pvfmad %v6,%v0,%s63,%v7
	muls.l %s58, %s36, %s58
	muls.l %s59, %s36, %s59
	lea %s60, 4(%s34, %s54)
	ldl.zx %s60, (,%s60)
	lea %s61, 4(%s34, %s56)
	ldl.zx %s61, (,%s61)
	lea %s62, 4(%s34, %s53)
	ldl.zx %s62, (,%s62)
	lea %s63, 4(%s34, %s57)
	ldl.zx %s63, (,%s63)
	muls.l %s60, %s36, %s60
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	pvfmad %v1,%v1,%s58,%v8
	pvfmad %v2,%v2,%s59,%v8
	pvfmad %v3,%v3,%s60,%v8
	pvfmad %v4,%v4,%s61,%v8
	ldl.zx %s58, 8(,%s34)
	lea %s59, 8(%s34, %s55)
	ldl.zx %s59, (,%s59)
	pvfmad %v5,%v5,%s62,%v8
	pvfmad %v6,%v6,%s63,%v8
	muls.l %s58, %s36, %s58
	muls.l %s59, %s36, %s59
	lea %s60, 8(%s34, %s54)
	ldl.zx %s60, (,%s60)
	lea %s61, 8(%s34, %s56)
	ldl.zx %s61, (,%s61)
	lea %s62, 8(%s34, %s53)
	ldl.zx %s62, (,%s62)
	lea %s63, 8(%s34, %s57)
	ldl.zx %s63, (,%s63)
	muls.l %s60, %s36, %s60
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	pvfmad %v1,%v1,%s58,%v9
	pvfmad %v2,%v2,%s59,%v9
	pvfmad %v3,%v3,%s60,%v9
	pvfmad %v4,%v4,%s61,%v9
	ldl.zx %s58, 12(,%s34)
	lea %s59, 12(%s34, %s55)
	ldl.zx %s59, (,%s59)
	pvfmad %v5,%v5,%s62,%v9
	pvfmad %v6,%v6,%s63,%v9
	muls.l %s58, %s36, %s58
	muls.l %s59, %s36, %s59
	lea %s60, 12(%s34, %s54)
	ldl.zx %s60, (,%s60)
	lea %s61, 12(%s34, %s56)
	ldl.zx %s61, (,%s61)
	lea %s62, 12(%s34, %s53)
	ldl.zx %s62, (,%s62)
	lea %s63, 12(%s34, %s57)
	ldl.zx %s63, (,%s63)
	muls.l %s60, %s36, %s60
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	pvfmad %v1,%v1,%s58,%v10
	pvfmad %v2,%v2,%s59,%v10
	pvfmad %v3,%v3,%s60,%v10
	pvfmad %v4,%v4,%s61,%v10
	ldl.zx %s58, 16(,%s34)
	lea %s59, 16(%s34, %s55)
	ldl.zx %s59, (,%s59)
	pvfmad %v5,%v5,%s62,%v10
	pvfmad %v6,%v6,%s63,%v10
	muls.l %s58, %s36, %s58
	muls.l %s59, %s36, %s59
	lea %s60, 16(%s34, %s54)
	ldl.zx %s60, (,%s60)
	lea %s61, 16(%s34, %s56)
	ldl.zx %s61, (,%s61)
	lea %s62, 16(%s34, %s53)
	ldl.zx %s62, (,%s62)
	lea %s63, 16(%s34, %s57)
	ldl.zx %s63, (,%s63)
	muls.l %s60, %s36, %s60
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	pvfmad %v1,%v1,%s58,%v11
	pvfmad %v2,%v2,%s59,%v11
	pvfmad %v3,%v3,%s60,%v11
	pvfmad %v4,%v4,%s61,%v11
	ldl.zx %s58, 20(,%s34)
	lea %s55, 20(%s34, %s55)
	ldl.zx %s55, (,%s55)
	pvfmad %v5,%v5,%s62,%v11
	pvfmad %v6,%v6,%s63,%v11
	muls.l %s58, %s36, %s58
	muls.l %s55, %s36, %s55
	lea %s59, 20(%s34, %s54)
	ldl.zx %s59, (,%s59)
	lea %s56, 20(%s34, %s56)
	ldl.zx %s56, (,%s56)
	lea %s60, 20(%s34, %s53)
	ldl.zx %s60, (,%s60)
	lea %s57, 20(%s34, %s57)
	ldl.zx %s57, (,%s57)
	muls.l %s59, %s36, %s59
	muls.l %s56, %s36, %s56
	muls.l %s60, %s36, %s60
	muls.l %s57, %s36, %s57
	pvfmad %v1,%v1,%s58,%v12
	pvfmad %v2,%v2,%s55,%v12
	pvfmad %v3,%v3,%s59,%v12
	pvfmad %v4,%v4,%s56,%v12
	pvfmad %v5,%v5,%s60,%v12
	pvfmad %v6,%v6,%s57,%v12
	pvfadd %v1,%v13,%v1
	pvfadd %v2,%v14,%v2
	pvfadd %v3,%v15,%v3
	pvfadd %v4,%v16,%v4
	pvfadd %v5,%v17,%v5
	pvfadd %v6,%v18,%v6
	vst %v1,8,%s42
	vst %v2,8,%s43
	vst %v3,8,%s44
	vst %v4,8,%s45
	vst %v5,8,%s46
	vst %v6,8,%s47
	ldl.zx %s42, (,%s34)
	ldl.zx %s43, (,%s48)
	muls.l %s42, %s36, %s42
	muls.l %s43, %s36, %s43
	ldl.zx %s44, (,%s49)
	ldl.zx %s45, (,%s50)
	ldl.zx %s46, (,%s51)
	ldl.zx %s47, (,%s52)
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	pvfmad %v1,%v0,%s42,%v13
	pvfmad %v1,%v1,%s43,%v14
	pvfmad %v1,%v1,%s44,%v15
	pvfmad %v1,%v1,%s45,%v16
	ldl.zx %s42, 4(,%s34)
	ldl.zx %s43, 4(,%s48)
	pvfmad %v1,%v1,%s46,%v17
	pvfmad %v1,%v1,%s47,%v18
	muls.l %s42, %s36, %s42
	muls.l %s43, %s36, %s43
	or %s44, 4, %s54
	adds.l %s44, %s34, %s44
	ldl.zx %s44, (,%s44)
	ldl.zx %s45, 4(,%s50)
	or %s46, 4, %s53
	adds.l %s46, %s34, %s46
	ldl.zx %s46, (,%s46)
	ldl.zx %s47, 4(,%s52)
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	pvfmad %v2,%v0,%s42,%v13
	pvfmad %v2,%v2,%s43,%v14
	pvfmad %v2,%v2,%s44,%v15
	pvfmad %v2,%v2,%s45,%v16
	ldl.zx %s42, 8(,%s34)
	ldl.zx %s43, 8(,%s48)
	pvfmad %v2,%v2,%s46,%v17
	pvfmad %v2,%v2,%s47,%v18
	muls.l %s42, %s36, %s42
	muls.l %s43, %s36, %s43
	ldl.zx %s44, 8(,%s49)
	ldl.zx %s45, 8(,%s50)
	or %s46, 8, %s53
	adds.l %s46, %s34, %s46
	ldl.zx %s46, (,%s46)
	ldl.zx %s47, 8(,%s52)
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	pvfmad %v3,%v0,%s42,%v13
	pvfmad %v3,%v3,%s43,%v14
	pvfmad %v3,%v3,%s44,%v15
	pvfmad %v3,%v3,%s45,%v16
	ldl.zx %s42, 12(,%s34)
	ldl.zx %s43, 12(,%s48)
	pvfmad %v3,%v3,%s46,%v17
	pvfmad %v3,%v3,%s47,%v18
	muls.l %s42, %s36, %s42
	muls.l %s43, %s36, %s43
	ldl.zx %s44, 12(,%s49)
	ldl.zx %s45, 12(,%s50)
	or %s46, 12, %s53
	adds.l %s46, %s34, %s46
	ldl.zx %s46, (,%s46)
	ldl.zx %s47, 12(,%s52)
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	pvfmad %v4,%v0,%s42,%v13
	pvfmad %v4,%v4,%s43,%v14
	pvfmad %v4,%v4,%s44,%v15
	pvfmad %v4,%v4,%s45,%v16
	ldl.zx %s42, 16(,%s34)
	ldl.zx %s43, 16(,%s48)
	pvfmad %v4,%v4,%s46,%v17
	pvfmad %v4,%v4,%s47,%v18
	muls.l %s42, %s36, %s42
	muls.l %s43, %s36, %s43
	ldl.zx %s44, 16(,%s49)
	ldl.zx %s45, 16(,%s50)
	ldl.zx %s46, 16(,%s51)
	ldl.zx %s47, 16(,%s52)
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	pvfmad %v5,%v0,%s42,%v13
	pvfmad %v5,%v5,%s43,%v14
	pvfmad %v5,%v5,%s44,%v15
	pvfmad %v5,%v5,%s45,%v16
	ldl.zx %s34, 20(,%s34)
	ldl.zx %s42, 20(,%s48)
	pvfmad %v5,%v5,%s46,%v17
	pvfmad %v5,%v5,%s47,%v18
	muls.l %s34, %s36, %s34
	muls.l %s42, %s36, %s42
	ldl.zx %s43, 20(,%s49)
	ldl.zx %s44, 20(,%s50)
	ldl.zx %s45, 20(,%s51)
	ldl.zx %s46, 20(,%s52)
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s36, %s36, %s46
	pvfmad %v0,%v0,%s34,%v13
	pvfmad %v0,%v0,%s42,%v14
	pvfmad %v0,%v0,%s43,%v15
	pvfmad %v0,%v0,%s44,%v16
	pvfmad %v0,%v0,%s45,%v17
	pvfmad %v0,%v0,%s36,%v18
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v0,%v12,%v0
	vst %v1,8,%s35
	vst %v2,8,%s37
	vst %v3,8,%s38
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v0,8,%s41
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end5:
	.size	w2v_kernel_N6_HU512_2X_W06, .Lfunc_end5-w2v_kernel_N6_HU512_2X_W06

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI6_0:
	.4byte	1065353216
.LCPI6_1:
	.4byte	0
.LCPI6_2:
	.4byte	3212836864
.LCPI6_3:
	.4byte	1086324736
.LCPI6_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI6_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W07
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W07,@function
w2v_kernel_N6_HU512_2X_W07:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB6_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB6_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s36, -8(,%s9)
	adds.w.sx %s49, %s34, (0)1
	lvl %s49
	ldl.sx %s34, (,%s4)
	ld %s42, 240(,%s9)
	muls.l %s34, %s34, %s1
	ldl.sx %s37, 4(,%s4)
	sll %s34, %s34, 2
	adds.l %s35, %s42, %s34
	ldl.sx %s34, 8(,%s4)
	muls.l %s37, %s37, %s1
	sll %s37, %s37, 2
	adds.l %s37, %s42, %s37
	muls.l %s34, %s34, %s1
	ldl.sx %s39, 12(,%s4)
	sll %s34, %s34, 2
	adds.l %s38, %s42, %s34
	ldl.sx %s34, 16(,%s4)
	muls.l %s39, %s39, %s1
	sll %s39, %s39, 2
	adds.l %s39, %s42, %s39
	muls.l %s34, %s34, %s1
	ldl.sx %s41, 20(,%s4)
	sll %s34, %s34, 2
	adds.l %s40, %s42, %s34
	ldl.sx %s34, 24(,%s4)
	muls.l %s41, %s41, %s1
	sll %s41, %s41, 2
	adds.l %s41, %s42, %s41
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s42, %s42, %s34
	vld %v7,8,%s35
	vld %v8,8,%s37
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	ldl.sx %s34, (,%s5)
	ldl.sx %s44, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s43, %s7, %s34
	muls.l %s34, %s44, %s1
	ldl.sx %s45, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s44, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s45, %s45, %s1
	sll %s45, %s45, 2
	adds.l %s45, %s7, %s45
	muls.l %s34, %s34, %s1
	ldl.sx %s47, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s46, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s47, %s47, %s1
	sll %s47, %s47, 2
	adds.l %s47, %s7, %s47
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s48, %s7, %s34
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	vld %v19,8,%s48
	lvl %s49
	pvfmul %v0,%v14,%v7
	pvfmul %v1,%v14,%v8
	pvfmul %v2,%v14,%v9
	pvfmul %v3,%v14,%v10
	pvfmul %v4,%v14,%v11
	pvfmul %v5,%v14,%v12
	pvfmul %v6,%v14,%v13
	vsll %v20,%v0,32
	vsll %v21,%v1,32
	vsll %v22,%v2,32
	vsll %v23,%v3,32
	vsll %v24,%v4,32
	vsll %v25,%v5,32
	vsll %v26,%v6,32
	vfadd.s %v0,%v0,%v20
	vfadd.s %v1,%v1,%v21
	vfadd.s %v2,%v2,%v22
	vfadd.s %v3,%v3,%v23
	vfadd.s %v4,%v4,%v24
	vfadd.s %v5,%v5,%v25
	vfadd.s %v6,%v6,%v26
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	or %s50, 1, (0)1
	lvl %s50
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s51, 4(%s6)
	vstu %v1,4,%s51
	lea %s51, 8(%s6)
	vstu %v2,4,%s51
	lea %s51, 12(%s6)
	vstu %v3,4,%s51
	lea %s51, 16(%s6)
	vstu %v4,4,%s51
	lea %s51, 20(%s6)
	vstu %v5,4,%s51
	lea %s51, 24(%s6)
	vstu %v6,4,%s51
	lvl %s49
	pvfmul %v0,%v15,%v7
	pvfmul %v1,%v15,%v8
	pvfmul %v2,%v15,%v9
	pvfmul %v3,%v15,%v10
	pvfmul %v4,%v15,%v11
	pvfmul %v5,%v15,%v12
	pvfmul %v6,%v15,%v13
	vsll %v20,%v0,32
	vsll %v21,%v1,32
	vsll %v22,%v2,32
	vsll %v23,%v3,32
	vsll %v24,%v4,32
	vsll %v25,%v5,32
	vsll %v26,%v6,32
	vfadd.s %v0,%v0,%v20
	vfadd.s %v1,%v1,%v21
	vfadd.s %v2,%v2,%v22
	vfadd.s %v3,%v3,%v23
	vfadd.s %v4,%v4,%v24
	vfadd.s %v5,%v5,%v25
	vfadd.s %v6,%v6,%v26
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	lvl %s50
	lea %s51, 28(%s6)
	vstu %v0,4,%s51
	lea %s51, 32(%s6)
	vstu %v1,4,%s51
	lea %s51, 36(%s6)
	vstu %v2,4,%s51
	lea %s51, 40(%s6)
	vstu %v3,4,%s51
	lea %s51, 44(%s6)
	vstu %v4,4,%s51
	lea %s51, 48(%s6)
	vstu %v5,4,%s51
	lea %s51, 52(%s6)
	vstu %v6,4,%s51
	lvl %s49
	pvfmul %v0,%v16,%v7
	pvfmul %v1,%v16,%v8
	pvfmul %v2,%v16,%v9
	pvfmul %v3,%v16,%v10
	pvfmul %v4,%v16,%v11
	pvfmul %v5,%v16,%v12
	pvfmul %v6,%v16,%v13
	vsll %v20,%v0,32
	vsll %v21,%v1,32
	vsll %v22,%v2,32
	vsll %v23,%v3,32
	vsll %v24,%v4,32
	vsll %v25,%v5,32
	vsll %v26,%v6,32
	vfadd.s %v0,%v0,%v20
	vfadd.s %v1,%v1,%v21
	vfadd.s %v2,%v2,%v22
	vfadd.s %v3,%v3,%v23
	vfadd.s %v4,%v4,%v24
	vfadd.s %v5,%v5,%v25
	vfadd.s %v6,%v6,%v26
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	lvl %s50
	lea %s51, 56(%s6)
	vstu %v0,4,%s51
	lea %s51, 60(%s6)
	vstu %v1,4,%s51
	lea %s51, 64(%s6)
	vstu %v2,4,%s51
	lea %s51, 68(%s6)
	vstu %v3,4,%s51
	lea %s51, 72(%s6)
	vstu %v4,4,%s51
	lea %s51, 76(%s6)
	vstu %v5,4,%s51
	lea %s51, 80(%s6)
	vstu %v6,4,%s51
	lvl %s49
	pvfmul %v0,%v17,%v7
	pvfmul %v1,%v17,%v8
	pvfmul %v2,%v17,%v9
	pvfmul %v3,%v17,%v10
	pvfmul %v4,%v17,%v11
	pvfmul %v5,%v17,%v12
	pvfmul %v6,%v17,%v13
	vsll %v20,%v0,32
	vsll %v21,%v1,32
	vsll %v22,%v2,32
	vsll %v23,%v3,32
	vsll %v24,%v4,32
	vsll %v25,%v5,32
	vsll %v26,%v6,32
	vfadd.s %v0,%v0,%v20
	vfadd.s %v1,%v1,%v21
	vfadd.s %v2,%v2,%v22
	vfadd.s %v3,%v3,%v23
	vfadd.s %v4,%v4,%v24
	vfadd.s %v5,%v5,%v25
	vfadd.s %v6,%v6,%v26
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	lvl %s50
	lea %s51, 84(%s6)
	vstu %v0,4,%s51
	lea %s51, 88(%s6)
	vstu %v1,4,%s51
	lea %s51, 92(%s6)
	vstu %v2,4,%s51
	lea %s51, 96(%s6)
	vstu %v3,4,%s51
	lea %s51, 100(%s6)
	vstu %v4,4,%s51
	lea %s51, 104(%s6)
	vstu %v5,4,%s51
	lea %s51, 108(%s6)
	vstu %v6,4,%s51
	lvl %s49
	pvfmul %v0,%v18,%v7
	pvfmul %v1,%v18,%v8
	pvfmul %v2,%v18,%v9
	pvfmul %v3,%v18,%v10
	pvfmul %v4,%v18,%v11
	pvfmul %v5,%v18,%v12
	pvfmul %v6,%v18,%v13
	vsll %v20,%v0,32
	vsll %v21,%v1,32
	vsll %v22,%v2,32
	vsll %v23,%v3,32
	vsll %v24,%v4,32
	vsll %v25,%v5,32
	vsll %v26,%v6,32
	vfadd.s %v0,%v0,%v20
	vfadd.s %v1,%v1,%v21
	vfadd.s %v2,%v2,%v22
	vfadd.s %v3,%v3,%v23
	vfadd.s %v4,%v4,%v24
	vfadd.s %v5,%v5,%v25
	vfadd.s %v6,%v6,%v26
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	lvl %s50
	lea %s51, 112(%s6)
	vstu %v0,4,%s51
	lea %s51, 116(%s6)
	vstu %v1,4,%s51
	lea %s51, 120(%s6)
	vstu %v2,4,%s51
	lea %s51, 124(%s6)
	vstu %v3,4,%s51
	lea %s51, 128(%s6)
	vstu %v4,4,%s51
	lea %s51, 132(%s6)
	vstu %v5,4,%s51
	lea %s51, 136(%s6)
	vstu %v6,4,%s51
	lvl %s49
	pvfmul %v0,%v19,%v7
	pvfmul %v1,%v19,%v8
	pvfmul %v2,%v19,%v9
	pvfmul %v3,%v19,%v10
	pvfmul %v4,%v19,%v11
	pvfmul %v5,%v19,%v12
	pvfmul %v6,%v19,%v13
	vsll %v20,%v0,32
	vsll %v21,%v1,32
	vsll %v22,%v2,32
	vsll %v23,%v3,32
	vsll %v24,%v4,32
	vsll %v25,%v5,32
	vsll %v26,%v6,32
	vfadd.s %v0,%v0,%v20
	vfadd.s %v1,%v1,%v21
	vfadd.s %v2,%v2,%v22
	vfadd.s %v3,%v3,%v23
	vfadd.s %v4,%v4,%v24
	vfadd.s %v5,%v5,%v25
	vfadd.s %v6,%v6,%v26
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	lvl %s50
	lea %s50, 140(%s6)
	vstu %v0,4,%s50
	lea %s50, 144(%s6)
	vstu %v1,4,%s50
	lea %s50, 148(%s6)
	vstu %v2,4,%s50
	lea %s50, 152(%s6)
	vstu %v3,4,%s50
	lea %s50, 156(%s6)
	vstu %v4,4,%s50
	lea %s50, 160(%s6)
	vstu %v5,4,%s50
	lea %s50, 164(%s6)
	vstu %v6,4,%s50
	muls.l %s50, %s2, %s0
	adds.w.sx %s50, %s50, (0)1
	lvl %s50
	lea.sl %s50, .LCPI6_0@hi
	ldu %s50, .LCPI6_0@lo(,%s50)
	vseq %v0
	vbrdu %v20,%s50
	adds.w.sx %s51, %s2, (0)1
	vsubs.w.sx %v0,%s51,%v0
	vldu %v21,4,%s6
	lea.sl %s51, .LCPI6_2@hi
	ldu %s51, .LCPI6_2@lo(,%s51)
	vfmk.w.gt %vm1,%v0
	lea.sl %s52, .LCPI6_1@hi
	ldu %s52, .LCPI6_1@lo(,%s52)
	vfmul.s %v0,%s51,%v21
	lea %s51, __vec_expf@lo
	and %s51, %s51, (32)0
	lea.sl %s51, __vec_expf@hi(%s51)
	vbrdu %v20,%s52,%vm1
	bsic %lr, (,%s51)
	lea.sl %s51, .LCPI6_3@hi
	ldu %s51, .LCPI6_3@lo(,%s51)
	vfadd.s %v0,%s50,%v0
	vfsub.s %v1,%s51,%v21
	lea.sl %s51, .LCPI6_4@hi
	ldu %s51, .LCPI6_4@lo(,%s51)
	vfdiv.s %v0,%s50,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s50,%vm1
	vfsub.s %v1,%s51,%v21
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s52,%vm1
	vfsub.s %v0,%v20,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s49
	lea.sl %s49, .LCPI6_5@hi
	ld %s50, .LCPI6_5@lo(,%s49)
	ldl.zx %s51, (,%s34)
	sll %s56, %s2, 2
	adds.l %s49, %s34, %s56
	ldl.zx %s52, (,%s49)
	vbrd %v0,%s50
	muls.l %s59, %s36, %s51
	muls.l %s60, %s36, %s52
	sll %s55, %s2, 3
	adds.l %s50, %s34, %s55
	ldl.zx %s61, (,%s50)
	muls.l %s57, 12, %s2
	adds.l %s51, %s34, %s57
	ldl.zx %s62, (,%s51)
	sll %s54, %s2, 4
	adds.l %s52, %s34, %s54
	ldl.zx %s63, (,%s52)
	muls.l %s58, 20, %s2
	adds.l %s53, %s34, %s58
	ldl.zx %s0, (,%s53)
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	muls.l %s0, %s36, %s0
	pvfmad %v1,%v0,%s59,%v7
	pvfmad %v2,%v0,%s60,%v7
	pvfmad %v3,%v0,%s61,%v7
	pvfmad %v4,%v0,%s62,%v7
	ldl.zx %s59, 4(,%s34)
	lea %s60, 4(%s34, %s56)
	ldl.zx %s60, (,%s60)
	pvfmad %v5,%v0,%s63,%v7
	pvfmad %v6,%v0,%s0,%v7
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	lea %s61, 4(%s34, %s55)
	ldl.zx %s61, (,%s61)
	lea %s62, 4(%s34, %s57)
	ldl.zx %s62, (,%s62)
	lea %s63, 4(%s34, %s54)
	ldl.zx %s63, (,%s63)
	lea %s0, 4(%s34, %s58)
	ldl.zx %s0, (,%s0)
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	muls.l %s0, %s36, %s0
	pvfmad %v1,%v1,%s59,%v8
	pvfmad %v2,%v2,%s60,%v8
	pvfmad %v3,%v3,%s61,%v8
	pvfmad %v4,%v4,%s62,%v8
	ldl.zx %s59, 8(,%s34)
	lea %s60, 8(%s34, %s56)
	ldl.zx %s60, (,%s60)
	pvfmad %v5,%v5,%s63,%v8
	pvfmad %v6,%v6,%s0,%v8
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	lea %s61, 8(%s34, %s55)
	ldl.zx %s61, (,%s61)
	lea %s62, 8(%s34, %s57)
	ldl.zx %s62, (,%s62)
	lea %s63, 8(%s34, %s54)
	ldl.zx %s63, (,%s63)
	lea %s0, 8(%s34, %s58)
	ldl.zx %s0, (,%s0)
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	muls.l %s0, %s36, %s0
	pvfmad %v1,%v1,%s59,%v9
	pvfmad %v2,%v2,%s60,%v9
	pvfmad %v3,%v3,%s61,%v9
	pvfmad %v4,%v4,%s62,%v9
	ldl.zx %s59, 12(,%s34)
	lea %s60, 12(%s34, %s56)
	ldl.zx %s60, (,%s60)
	pvfmad %v5,%v5,%s63,%v9
	pvfmad %v6,%v6,%s0,%v9
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	lea %s61, 12(%s34, %s55)
	ldl.zx %s61, (,%s61)
	lea %s62, 12(%s34, %s57)
	ldl.zx %s62, (,%s62)
	lea %s63, 12(%s34, %s54)
	ldl.zx %s63, (,%s63)
	lea %s0, 12(%s34, %s58)
	ldl.zx %s0, (,%s0)
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	muls.l %s0, %s36, %s0
	pvfmad %v1,%v1,%s59,%v10
	pvfmad %v2,%v2,%s60,%v10
	pvfmad %v3,%v3,%s61,%v10
	pvfmad %v4,%v4,%s62,%v10
	ldl.zx %s59, 16(,%s34)
	lea %s60, 16(%s34, %s56)
	ldl.zx %s60, (,%s60)
	pvfmad %v5,%v5,%s63,%v10
	pvfmad %v6,%v6,%s0,%v10
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	lea %s61, 16(%s34, %s55)
	ldl.zx %s61, (,%s61)
	lea %s62, 16(%s34, %s57)
	ldl.zx %s62, (,%s62)
	lea %s63, 16(%s34, %s54)
	ldl.zx %s63, (,%s63)
	lea %s0, 16(%s34, %s58)
	ldl.zx %s0, (,%s0)
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	muls.l %s0, %s36, %s0
	pvfmad %v1,%v1,%s59,%v11
	pvfmad %v2,%v2,%s60,%v11
	pvfmad %v3,%v3,%s61,%v11
	pvfmad %v4,%v4,%s62,%v11
	ldl.zx %s59, 20(,%s34)
	lea %s60, 20(%s34, %s56)
	ldl.zx %s60, (,%s60)
	pvfmad %v5,%v5,%s63,%v11
	pvfmad %v6,%v6,%s0,%v11
	muls.l %s59, %s36, %s59
	muls.l %s60, %s36, %s60
	lea %s61, 20(%s34, %s55)
	ldl.zx %s61, (,%s61)
	lea %s62, 20(%s34, %s57)
	ldl.zx %s62, (,%s62)
	lea %s63, 20(%s34, %s54)
	ldl.zx %s63, (,%s63)
	lea %s0, 20(%s34, %s58)
	ldl.zx %s0, (,%s0)
	muls.l %s61, %s36, %s61
	muls.l %s62, %s36, %s62
	muls.l %s63, %s36, %s63
	muls.l %s0, %s36, %s0
	pvfmad %v1,%v1,%s59,%v12
	pvfmad %v2,%v2,%s60,%v12
	pvfmad %v3,%v3,%s61,%v12
	pvfmad %v4,%v4,%s62,%v12
	ldl.zx %s59, 24(,%s34)
	lea %s56, 24(%s34, %s56)
	ldl.zx %s56, (,%s56)
	pvfmad %v5,%v5,%s63,%v12
	pvfmad %v6,%v6,%s0,%v12
	muls.l %s59, %s36, %s59
	muls.l %s56, %s36, %s56
	lea %s60, 24(%s34, %s55)
	ldl.zx %s60, (,%s60)
	lea %s57, 24(%s34, %s57)
	ldl.zx %s57, (,%s57)
	lea %s61, 24(%s34, %s54)
	ldl.zx %s61, (,%s61)
	lea %s58, 24(%s34, %s58)
	ldl.zx %s58, (,%s58)
	muls.l %s60, %s36, %s60
	muls.l %s57, %s36, %s57
	muls.l %s61, %s36, %s61
	muls.l %s58, %s36, %s58
	pvfmad %v1,%v1,%s59,%v13
	pvfmad %v2,%v2,%s56,%v13
	pvfmad %v3,%v3,%s60,%v13
	pvfmad %v4,%v4,%s57,%v13
	pvfmad %v5,%v5,%s61,%v13
	pvfmad %v6,%v6,%s58,%v13
	pvfadd %v1,%v14,%v1
	pvfadd %v2,%v15,%v2
	pvfadd %v3,%v16,%v3
	pvfadd %v4,%v17,%v4
	pvfadd %v5,%v18,%v5
	pvfadd %v6,%v19,%v6
	vst %v1,8,%s43
	vst %v2,8,%s44
	vst %v3,8,%s45
	vst %v4,8,%s46
	vst %v5,8,%s47
	vst %v6,8,%s48
	ldl.zx %s43, (,%s34)
	ldl.zx %s44, (,%s49)
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	ldl.zx %s45, (,%s50)
	ldl.zx %s46, (,%s51)
	ldl.zx %s47, (,%s52)
	ldl.zx %s48, (,%s53)
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	muls.l %s48, %s36, %s48
	pvfmad %v1,%v0,%s43,%v14
	pvfmad %v1,%v1,%s44,%v15
	pvfmad %v1,%v1,%s45,%v16
	pvfmad %v1,%v1,%s46,%v17
	ldl.zx %s43, 4(,%s34)
	ldl.zx %s44, 4(,%s49)
	pvfmad %v1,%v1,%s47,%v18
	pvfmad %v1,%v1,%s48,%v19
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	or %s45, 4, %s55
	adds.l %s45, %s34, %s45
	ldl.zx %s45, (,%s45)
	ldl.zx %s46, 4(,%s51)
	or %s47, 4, %s54
	adds.l %s47, %s34, %s47
	ldl.zx %s47, (,%s47)
	ldl.zx %s48, 4(,%s53)
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	muls.l %s48, %s36, %s48
	pvfmad %v2,%v0,%s43,%v14
	pvfmad %v2,%v2,%s44,%v15
	pvfmad %v2,%v2,%s45,%v16
	pvfmad %v2,%v2,%s46,%v17
	ldl.zx %s43, 8(,%s34)
	ldl.zx %s44, 8(,%s49)
	pvfmad %v2,%v2,%s47,%v18
	pvfmad %v2,%v2,%s48,%v19
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	ldl.zx %s45, 8(,%s50)
	ldl.zx %s46, 8(,%s51)
	or %s47, 8, %s54
	adds.l %s47, %s34, %s47
	ldl.zx %s47, (,%s47)
	ldl.zx %s48, 8(,%s53)
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	muls.l %s48, %s36, %s48
	pvfmad %v3,%v0,%s43,%v14
	pvfmad %v3,%v3,%s44,%v15
	pvfmad %v3,%v3,%s45,%v16
	pvfmad %v3,%v3,%s46,%v17
	ldl.zx %s43, 12(,%s34)
	ldl.zx %s44, 12(,%s49)
	pvfmad %v3,%v3,%s47,%v18
	pvfmad %v3,%v3,%s48,%v19
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	ldl.zx %s45, 12(,%s50)
	ldl.zx %s46, 12(,%s51)
	or %s47, 12, %s54
	adds.l %s47, %s34, %s47
	ldl.zx %s47, (,%s47)
	ldl.zx %s48, 12(,%s53)
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	muls.l %s48, %s36, %s48
	pvfmad %v4,%v0,%s43,%v14
	pvfmad %v4,%v4,%s44,%v15
	pvfmad %v4,%v4,%s45,%v16
	pvfmad %v4,%v4,%s46,%v17
	ldl.zx %s43, 16(,%s34)
	ldl.zx %s44, 16(,%s49)
	pvfmad %v4,%v4,%s47,%v18
	pvfmad %v4,%v4,%s48,%v19
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	ldl.zx %s45, 16(,%s50)
	ldl.zx %s46, 16(,%s51)
	ldl.zx %s47, 16(,%s52)
	ldl.zx %s48, 16(,%s53)
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	muls.l %s48, %s36, %s48
	pvfmad %v5,%v0,%s43,%v14
	pvfmad %v5,%v5,%s44,%v15
	pvfmad %v5,%v5,%s45,%v16
	pvfmad %v5,%v5,%s46,%v17
	ldl.zx %s43, 20(,%s34)
	ldl.zx %s44, 20(,%s49)
	pvfmad %v5,%v5,%s47,%v18
	pvfmad %v5,%v5,%s48,%v19
	muls.l %s43, %s36, %s43
	muls.l %s44, %s36, %s44
	ldl.zx %s45, 20(,%s50)
	ldl.zx %s46, 20(,%s51)
	ldl.zx %s47, 20(,%s52)
	ldl.zx %s48, 20(,%s53)
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s47, %s36, %s47
	muls.l %s48, %s36, %s48
	pvfmad %v6,%v0,%s43,%v14
	pvfmad %v6,%v6,%s44,%v15
	pvfmad %v6,%v6,%s45,%v16
	pvfmad %v6,%v6,%s46,%v17
	ldl.zx %s34, 24(,%s34)
	ldl.zx %s43, 24(,%s49)
	pvfmad %v6,%v6,%s47,%v18
	pvfmad %v6,%v6,%s48,%v19
	muls.l %s34, %s36, %s34
	muls.l %s43, %s36, %s43
	ldl.zx %s44, 24(,%s50)
	ldl.zx %s45, 24(,%s51)
	ldl.zx %s46, 24(,%s52)
	ldl.zx %s47, 24(,%s53)
	muls.l %s44, %s36, %s44
	muls.l %s45, %s36, %s45
	muls.l %s46, %s36, %s46
	muls.l %s36, %s36, %s47
	pvfmad %v0,%v0,%s34,%v14
	pvfmad %v0,%v0,%s43,%v15
	pvfmad %v0,%v0,%s44,%v16
	pvfmad %v0,%v0,%s45,%v17
	pvfmad %v0,%v0,%s46,%v18
	pvfmad %v0,%v0,%s36,%v19
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v0,%v13,%v0
	vst %v1,8,%s35
	vst %v2,8,%s37
	vst %v3,8,%s38
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v0,8,%s42
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end6:
	.size	w2v_kernel_N6_HU512_2X_W07, .Lfunc_end6-w2v_kernel_N6_HU512_2X_W07

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI7_0:
	.4byte	1065353216
.LCPI7_1:
	.4byte	0
.LCPI7_2:
	.4byte	3212836864
.LCPI7_3:
	.4byte	1086324736
.LCPI7_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI7_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W08
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W08,@function
w2v_kernel_N6_HU512_2X_W08:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB7_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB7_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s37, -8(,%s9)
	adds.w.sx %s50, %s34, (0)1
	lvl %s50
	ldl.sx %s34, (,%s4)
	ld %s43, 240(,%s9)
	ldl.sx %s36, 4(,%s4)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s35, %s43, %s34
	muls.l %s34, %s36, %s1
	ldl.sx %s38, 8(,%s4)
	sll %s34, %s34, 2
	adds.l %s36, %s43, %s34
	ldl.sx %s34, 12(,%s4)
	muls.l %s38, %s38, %s1
	sll %s38, %s38, 2
	adds.l %s38, %s43, %s38
	muls.l %s34, %s34, %s1
	ldl.sx %s40, 16(,%s4)
	sll %s34, %s34, 2
	adds.l %s39, %s43, %s34
	ldl.sx %s34, 20(,%s4)
	muls.l %s40, %s40, %s1
	sll %s40, %s40, 2
	adds.l %s40, %s43, %s40
	muls.l %s34, %s34, %s1
	ldl.sx %s42, 24(,%s4)
	sll %s34, %s34, 2
	adds.l %s41, %s43, %s34
	ldl.sx %s34, 28(,%s4)
	muls.l %s42, %s42, %s1
	sll %s42, %s42, 2
	adds.l %s42, %s43, %s42
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s43, %s43, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	ldl.sx %s34, (,%s5)
	ldl.sx %s45, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s44, %s7, %s34
	muls.l %s34, %s45, %s1
	ldl.sx %s46, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s45, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s46, %s46, %s1
	sll %s46, %s46, 2
	adds.l %s46, %s7, %s46
	muls.l %s34, %s34, %s1
	ldl.sx %s48, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s47, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s48, %s48, %s1
	sll %s48, %s48, 2
	adds.l %s48, %s7, %s48
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s49, %s7, %s34
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	vld %v19,8,%s48
	vld %v20,8,%s49
	lvl %s50
	pvfmul %v0,%v15,%v7
	pvfmul %v1,%v15,%v8
	pvfmul %v2,%v15,%v9
	pvfmul %v3,%v15,%v10
	pvfmul %v4,%v15,%v11
	pvfmul %v5,%v15,%v12
	pvfmul %v6,%v15,%v13
	pvfmul %v21,%v15,%v14
	vsll %v22,%v0,32
	vsll %v23,%v1,32
	vsll %v24,%v2,32
	vsll %v25,%v3,32
	vsll %v26,%v4,32
	vsll %v27,%v5,32
	vsll %v28,%v6,32
	vsll %v29,%v21,32
	vfadd.s %v0,%v0,%v22
	vfadd.s %v1,%v1,%v23
	vfadd.s %v2,%v2,%v24
	vfadd.s %v3,%v3,%v25
	vfadd.s %v4,%v4,%v26
	vfadd.s %v5,%v5,%v27
	vfadd.s %v6,%v6,%v28
	vfadd.s %v21,%v21,%v29
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v21,%v21
	or %s51, 1, (0)1
	lvl %s51
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s52, 4(%s6)
	vstu %v1,4,%s52
	lea %s52, 8(%s6)
	vstu %v2,4,%s52
	lea %s52, 12(%s6)
	vstu %v3,4,%s52
	lea %s52, 16(%s6)
	vstu %v4,4,%s52
	lea %s52, 20(%s6)
	vstu %v5,4,%s52
	lea %s52, 24(%s6)
	vstu %v6,4,%s52
	lea %s52, 28(%s6)
	vstu %v21,4,%s52
	lvl %s50
	pvfmul %v0,%v16,%v7
	pvfmul %v1,%v16,%v8
	pvfmul %v2,%v16,%v9
	pvfmul %v3,%v16,%v10
	pvfmul %v4,%v16,%v11
	pvfmul %v5,%v16,%v12
	pvfmul %v6,%v16,%v13
	pvfmul %v21,%v16,%v14
	vsll %v22,%v0,32
	vsll %v23,%v1,32
	vsll %v24,%v2,32
	vsll %v25,%v3,32
	vsll %v26,%v4,32
	vsll %v27,%v5,32
	vsll %v28,%v6,32
	vsll %v29,%v21,32
	vfadd.s %v0,%v0,%v22
	vfadd.s %v1,%v1,%v23
	vfadd.s %v2,%v2,%v24
	vfadd.s %v3,%v3,%v25
	vfadd.s %v4,%v4,%v26
	vfadd.s %v5,%v5,%v27
	vfadd.s %v6,%v6,%v28
	vfadd.s %v21,%v21,%v29
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v21,%v21
	lvl %s51
	lea %s52, 32(%s6)
	vstu %v0,4,%s52
	lea %s52, 36(%s6)
	vstu %v1,4,%s52
	lea %s52, 40(%s6)
	vstu %v2,4,%s52
	lea %s52, 44(%s6)
	vstu %v3,4,%s52
	lea %s52, 48(%s6)
	vstu %v4,4,%s52
	lea %s52, 52(%s6)
	vstu %v5,4,%s52
	lea %s52, 56(%s6)
	vstu %v6,4,%s52
	lea %s52, 60(%s6)
	vstu %v21,4,%s52
	lvl %s50
	pvfmul %v0,%v17,%v7
	pvfmul %v1,%v17,%v8
	pvfmul %v2,%v17,%v9
	pvfmul %v3,%v17,%v10
	pvfmul %v4,%v17,%v11
	pvfmul %v5,%v17,%v12
	pvfmul %v6,%v17,%v13
	pvfmul %v21,%v17,%v14
	vsll %v22,%v0,32
	vsll %v23,%v1,32
	vsll %v24,%v2,32
	vsll %v25,%v3,32
	vsll %v26,%v4,32
	vsll %v27,%v5,32
	vsll %v28,%v6,32
	vsll %v29,%v21,32
	vfadd.s %v0,%v0,%v22
	vfadd.s %v1,%v1,%v23
	vfadd.s %v2,%v2,%v24
	vfadd.s %v3,%v3,%v25
	vfadd.s %v4,%v4,%v26
	vfadd.s %v5,%v5,%v27
	vfadd.s %v6,%v6,%v28
	vfadd.s %v21,%v21,%v29
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v21,%v21
	lvl %s51
	lea %s52, 64(%s6)
	vstu %v0,4,%s52
	lea %s52, 68(%s6)
	vstu %v1,4,%s52
	lea %s52, 72(%s6)
	vstu %v2,4,%s52
	lea %s52, 76(%s6)
	vstu %v3,4,%s52
	lea %s52, 80(%s6)
	vstu %v4,4,%s52
	lea %s52, 84(%s6)
	vstu %v5,4,%s52
	lea %s52, 88(%s6)
	vstu %v6,4,%s52
	lea %s52, 92(%s6)
	vstu %v21,4,%s52
	lvl %s50
	pvfmul %v0,%v18,%v7
	pvfmul %v1,%v18,%v8
	pvfmul %v2,%v18,%v9
	pvfmul %v3,%v18,%v10
	pvfmul %v4,%v18,%v11
	pvfmul %v5,%v18,%v12
	pvfmul %v6,%v18,%v13
	pvfmul %v21,%v18,%v14
	vsll %v22,%v0,32
	vsll %v23,%v1,32
	vsll %v24,%v2,32
	vsll %v25,%v3,32
	vsll %v26,%v4,32
	vsll %v27,%v5,32
	vsll %v28,%v6,32
	vsll %v29,%v21,32
	vfadd.s %v0,%v0,%v22
	vfadd.s %v1,%v1,%v23
	vfadd.s %v2,%v2,%v24
	vfadd.s %v3,%v3,%v25
	vfadd.s %v4,%v4,%v26
	vfadd.s %v5,%v5,%v27
	vfadd.s %v6,%v6,%v28
	vfadd.s %v21,%v21,%v29
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v21,%v21
	lvl %s51
	lea %s52, 96(%s6)
	vstu %v0,4,%s52
	lea %s52, 100(%s6)
	vstu %v1,4,%s52
	lea %s52, 104(%s6)
	vstu %v2,4,%s52
	lea %s52, 108(%s6)
	vstu %v3,4,%s52
	lea %s52, 112(%s6)
	vstu %v4,4,%s52
	lea %s52, 116(%s6)
	vstu %v5,4,%s52
	lea %s52, 120(%s6)
	vstu %v6,4,%s52
	lea %s52, 124(%s6)
	vstu %v21,4,%s52
	lvl %s50
	pvfmul %v0,%v19,%v7
	pvfmul %v1,%v19,%v8
	pvfmul %v2,%v19,%v9
	pvfmul %v3,%v19,%v10
	pvfmul %v4,%v19,%v11
	pvfmul %v5,%v19,%v12
	pvfmul %v6,%v19,%v13
	pvfmul %v21,%v19,%v14
	vsll %v22,%v0,32
	vsll %v23,%v1,32
	vsll %v24,%v2,32
	vsll %v25,%v3,32
	vsll %v26,%v4,32
	vsll %v27,%v5,32
	vsll %v28,%v6,32
	vsll %v29,%v21,32
	vfadd.s %v0,%v0,%v22
	vfadd.s %v1,%v1,%v23
	vfadd.s %v2,%v2,%v24
	vfadd.s %v3,%v3,%v25
	vfadd.s %v4,%v4,%v26
	vfadd.s %v5,%v5,%v27
	vfadd.s %v6,%v6,%v28
	vfadd.s %v21,%v21,%v29
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v21,%v21
	lvl %s51
	lea %s52, 128(%s6)
	vstu %v0,4,%s52
	lea %s52, 132(%s6)
	vstu %v1,4,%s52
	lea %s52, 136(%s6)
	vstu %v2,4,%s52
	lea %s52, 140(%s6)
	vstu %v3,4,%s52
	lea %s52, 144(%s6)
	vstu %v4,4,%s52
	lea %s52, 148(%s6)
	vstu %v5,4,%s52
	lea %s52, 152(%s6)
	vstu %v6,4,%s52
	lea %s52, 156(%s6)
	vstu %v21,4,%s52
	lvl %s50
	pvfmul %v0,%v20,%v7
	pvfmul %v1,%v20,%v8
	pvfmul %v2,%v20,%v9
	pvfmul %v3,%v20,%v10
	pvfmul %v4,%v20,%v11
	pvfmul %v5,%v20,%v12
	pvfmul %v6,%v20,%v13
	pvfmul %v21,%v20,%v14
	vsll %v22,%v0,32
	vsll %v23,%v1,32
	vsll %v24,%v2,32
	vsll %v25,%v3,32
	vsll %v26,%v4,32
	vsll %v27,%v5,32
	vsll %v28,%v6,32
	vsll %v29,%v21,32
	vfadd.s %v0,%v0,%v22
	vfadd.s %v1,%v1,%v23
	vfadd.s %v2,%v2,%v24
	vfadd.s %v3,%v3,%v25
	vfadd.s %v4,%v4,%v26
	vfadd.s %v5,%v5,%v27
	vfadd.s %v6,%v6,%v28
	vfadd.s %v21,%v21,%v29
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v21,%v21
	lvl %s51
	lea %s51, 160(%s6)
	vstu %v0,4,%s51
	lea %s51, 164(%s6)
	vstu %v1,4,%s51
	lea %s51, 168(%s6)
	vstu %v2,4,%s51
	lea %s51, 172(%s6)
	vstu %v3,4,%s51
	lea %s51, 176(%s6)
	vstu %v4,4,%s51
	lea %s51, 180(%s6)
	vstu %v5,4,%s51
	lea %s51, 184(%s6)
	vstu %v6,4,%s51
	lea %s51, 188(%s6)
	vstu %v21,4,%s51
	muls.l %s51, %s2, %s0
	adds.w.sx %s51, %s51, (0)1
	lvl %s51
	lea.sl %s51, .LCPI7_0@hi
	ldu %s51, .LCPI7_0@lo(,%s51)
	vseq %v0
	vbrdu %v21,%s51
	adds.w.sx %s52, %s2, (0)1
	vsubs.w.sx %v0,%s52,%v0
	vldu %v22,4,%s6
	lea.sl %s52, .LCPI7_2@hi
	ldu %s52, .LCPI7_2@lo(,%s52)
	vfmk.w.gt %vm1,%v0
	lea.sl %s53, .LCPI7_1@hi
	ldu %s53, .LCPI7_1@lo(,%s53)
	vfmul.s %v0,%s52,%v22
	lea %s52, __vec_expf@lo
	and %s52, %s52, (32)0
	lea.sl %s52, __vec_expf@hi(%s52)
	vbrdu %v21,%s53,%vm1
	bsic %lr, (,%s52)
	lea.sl %s52, .LCPI7_3@hi
	ldu %s52, .LCPI7_3@lo(,%s52)
	vfadd.s %v0,%s51,%v0
	vfsub.s %v1,%s52,%v22
	lea.sl %s52, .LCPI7_4@hi
	ldu %s52, .LCPI7_4@lo(,%s52)
	vfdiv.s %v0,%s51,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s51,%vm1
	vfsub.s %v1,%s52,%v22
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s53,%vm1
	vfsub.s %v0,%v21,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s50
	lea.sl %s50, .LCPI7_5@hi
	ld %s51, .LCPI7_5@lo(,%s50)
	ldl.zx %s52, (,%s34)
	sll %s57, %s2, 2
	adds.l %s50, %s34, %s57
	ldl.zx %s53, (,%s50)
	vbrd %v0,%s51
	muls.l %s60, %s37, %s52
	muls.l %s61, %s37, %s53
	sll %s56, %s2, 3
	adds.l %s51, %s34, %s56
	ldl.zx %s62, (,%s51)
	muls.l %s58, 12, %s2
	adds.l %s52, %s34, %s58
	ldl.zx %s63, (,%s52)
	sll %s55, %s2, 4
	adds.l %s53, %s34, %s55
	ldl.zx %s0, (,%s53)
	muls.l %s59, 20, %s2
	adds.l %s54, %s34, %s59
	ldl.zx %s1, (,%s54)
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	pvfmad %v1,%v0,%s60,%v7
	pvfmad %v2,%v0,%s61,%v7
	pvfmad %v3,%v0,%s62,%v7
	pvfmad %v4,%v0,%s63,%v7
	ldl.zx %s60, 4(,%s34)
	lea %s61, 4(%s34, %s57)
	ldl.zx %s61, (,%s61)
	pvfmad %v5,%v0,%s0,%v7
	pvfmad %v6,%v0,%s1,%v7
	muls.l %s60, %s37, %s60
	muls.l %s61, %s37, %s61
	lea %s62, 4(%s34, %s56)
	ldl.zx %s62, (,%s62)
	lea %s63, 4(%s34, %s58)
	ldl.zx %s63, (,%s63)
	lea %s0, 4(%s34, %s55)
	ldl.zx %s0, (,%s0)
	lea %s1, 4(%s34, %s59)
	ldl.zx %s1, (,%s1)
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	pvfmad %v1,%v1,%s60,%v8
	pvfmad %v2,%v2,%s61,%v8
	pvfmad %v3,%v3,%s62,%v8
	pvfmad %v4,%v4,%s63,%v8
	ldl.zx %s60, 8(,%s34)
	lea %s61, 8(%s34, %s57)
	ldl.zx %s61, (,%s61)
	pvfmad %v5,%v5,%s0,%v8
	pvfmad %v6,%v6,%s1,%v8
	muls.l %s60, %s37, %s60
	muls.l %s61, %s37, %s61
	lea %s62, 8(%s34, %s56)
	ldl.zx %s62, (,%s62)
	lea %s63, 8(%s34, %s58)
	ldl.zx %s63, (,%s63)
	lea %s0, 8(%s34, %s55)
	ldl.zx %s0, (,%s0)
	lea %s1, 8(%s34, %s59)
	ldl.zx %s1, (,%s1)
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	pvfmad %v1,%v1,%s60,%v9
	pvfmad %v2,%v2,%s61,%v9
	pvfmad %v3,%v3,%s62,%v9
	pvfmad %v4,%v4,%s63,%v9
	ldl.zx %s60, 12(,%s34)
	lea %s61, 12(%s34, %s57)
	ldl.zx %s61, (,%s61)
	pvfmad %v5,%v5,%s0,%v9
	pvfmad %v6,%v6,%s1,%v9
	muls.l %s60, %s37, %s60
	muls.l %s61, %s37, %s61
	lea %s62, 12(%s34, %s56)
	ldl.zx %s62, (,%s62)
	lea %s63, 12(%s34, %s58)
	ldl.zx %s63, (,%s63)
	lea %s0, 12(%s34, %s55)
	ldl.zx %s0, (,%s0)
	lea %s1, 12(%s34, %s59)
	ldl.zx %s1, (,%s1)
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	pvfmad %v1,%v1,%s60,%v10
	pvfmad %v2,%v2,%s61,%v10
	pvfmad %v3,%v3,%s62,%v10
	pvfmad %v4,%v4,%s63,%v10
	ldl.zx %s60, 16(,%s34)
	lea %s61, 16(%s34, %s57)
	ldl.zx %s61, (,%s61)
	pvfmad %v5,%v5,%s0,%v10
	pvfmad %v6,%v6,%s1,%v10
	muls.l %s60, %s37, %s60
	muls.l %s61, %s37, %s61
	lea %s62, 16(%s34, %s56)
	ldl.zx %s62, (,%s62)
	lea %s63, 16(%s34, %s58)
	ldl.zx %s63, (,%s63)
	lea %s0, 16(%s34, %s55)
	ldl.zx %s0, (,%s0)
	lea %s1, 16(%s34, %s59)
	ldl.zx %s1, (,%s1)
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	pvfmad %v1,%v1,%s60,%v11
	pvfmad %v2,%v2,%s61,%v11
	pvfmad %v3,%v3,%s62,%v11
	pvfmad %v4,%v4,%s63,%v11
	ldl.zx %s60, 20(,%s34)
	lea %s61, 20(%s34, %s57)
	ldl.zx %s61, (,%s61)
	pvfmad %v5,%v5,%s0,%v11
	pvfmad %v6,%v6,%s1,%v11
	muls.l %s60, %s37, %s60
	muls.l %s61, %s37, %s61
	lea %s62, 20(%s34, %s56)
	ldl.zx %s62, (,%s62)
	lea %s63, 20(%s34, %s58)
	ldl.zx %s63, (,%s63)
	lea %s0, 20(%s34, %s55)
	ldl.zx %s0, (,%s0)
	lea %s1, 20(%s34, %s59)
	ldl.zx %s1, (,%s1)
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	pvfmad %v1,%v1,%s60,%v12
	pvfmad %v2,%v2,%s61,%v12
	pvfmad %v3,%v3,%s62,%v12
	pvfmad %v4,%v4,%s63,%v12
	ldl.zx %s60, 24(,%s34)
	lea %s61, 24(%s34, %s57)
	ldl.zx %s61, (,%s61)
	pvfmad %v5,%v5,%s0,%v12
	pvfmad %v6,%v6,%s1,%v12
	muls.l %s60, %s37, %s60
	muls.l %s61, %s37, %s61
	lea %s62, 24(%s34, %s56)
	ldl.zx %s62, (,%s62)
	lea %s63, 24(%s34, %s58)
	ldl.zx %s63, (,%s63)
	lea %s0, 24(%s34, %s55)
	ldl.zx %s0, (,%s0)
	lea %s1, 24(%s34, %s59)
	ldl.zx %s1, (,%s1)
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	pvfmad %v1,%v1,%s60,%v13
	pvfmad %v2,%v2,%s61,%v13
	pvfmad %v3,%v3,%s62,%v13
	pvfmad %v4,%v4,%s63,%v13
	ldl.zx %s60, 28(,%s34)
	lea %s57, 28(%s34, %s57)
	ldl.zx %s57, (,%s57)
	pvfmad %v5,%v5,%s0,%v13
	pvfmad %v6,%v6,%s1,%v13
	muls.l %s60, %s37, %s60
	muls.l %s57, %s37, %s57
	lea %s61, 28(%s34, %s56)
	ldl.zx %s61, (,%s61)
	lea %s58, 28(%s34, %s58)
	ldl.zx %s58, (,%s58)
	lea %s62, 28(%s34, %s55)
	ldl.zx %s62, (,%s62)
	lea %s59, 28(%s34, %s59)
	ldl.zx %s59, (,%s59)
	muls.l %s61, %s37, %s61
	muls.l %s58, %s37, %s58
	muls.l %s62, %s37, %s62
	muls.l %s59, %s37, %s59
	pvfmad %v1,%v1,%s60,%v14
	pvfmad %v2,%v2,%s57,%v14
	pvfmad %v3,%v3,%s61,%v14
	pvfmad %v4,%v4,%s58,%v14
	pvfmad %v5,%v5,%s62,%v14
	pvfmad %v6,%v6,%s59,%v14
	pvfadd %v1,%v15,%v1
	pvfadd %v2,%v16,%v2
	pvfadd %v3,%v17,%v3
	pvfadd %v4,%v18,%v4
	pvfadd %v5,%v19,%v5
	pvfadd %v6,%v20,%v6
	vst %v1,8,%s44
	vst %v2,8,%s45
	vst %v3,8,%s46
	vst %v4,8,%s47
	vst %v5,8,%s48
	vst %v6,8,%s49
	ldl.zx %s44, (,%s34)
	ldl.zx %s45, (,%s50)
	muls.l %s44, %s37, %s44
	muls.l %s45, %s37, %s45
	ldl.zx %s46, (,%s51)
	ldl.zx %s47, (,%s52)
	ldl.zx %s48, (,%s53)
	ldl.zx %s49, (,%s54)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	pvfmad %v1,%v0,%s44,%v15
	pvfmad %v1,%v1,%s45,%v16
	pvfmad %v1,%v1,%s46,%v17
	pvfmad %v1,%v1,%s47,%v18
	ldl.zx %s44, 4(,%s34)
	ldl.zx %s45, 4(,%s50)
	pvfmad %v1,%v1,%s48,%v19
	pvfmad %v1,%v1,%s49,%v20
	muls.l %s44, %s37, %s44
	muls.l %s45, %s37, %s45
	or %s46, 4, %s56
	adds.l %s46, %s34, %s46
	ldl.zx %s46, (,%s46)
	ldl.zx %s47, 4(,%s52)
	or %s48, 4, %s55
	adds.l %s48, %s34, %s48
	ldl.zx %s48, (,%s48)
	ldl.zx %s49, 4(,%s54)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	pvfmad %v2,%v0,%s44,%v15
	pvfmad %v2,%v2,%s45,%v16
	pvfmad %v2,%v2,%s46,%v17
	pvfmad %v2,%v2,%s47,%v18
	ldl.zx %s44, 8(,%s34)
	ldl.zx %s45, 8(,%s50)
	pvfmad %v2,%v2,%s48,%v19
	pvfmad %v2,%v2,%s49,%v20
	muls.l %s44, %s37, %s44
	muls.l %s45, %s37, %s45
	ldl.zx %s46, 8(,%s51)
	ldl.zx %s47, 8(,%s52)
	or %s48, 8, %s55
	adds.l %s48, %s34, %s48
	ldl.zx %s48, (,%s48)
	ldl.zx %s49, 8(,%s54)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	pvfmad %v3,%v0,%s44,%v15
	pvfmad %v3,%v3,%s45,%v16
	pvfmad %v3,%v3,%s46,%v17
	pvfmad %v3,%v3,%s47,%v18
	ldl.zx %s44, 12(,%s34)
	ldl.zx %s45, 12(,%s50)
	pvfmad %v3,%v3,%s48,%v19
	pvfmad %v3,%v3,%s49,%v20
	muls.l %s44, %s37, %s44
	muls.l %s45, %s37, %s45
	ldl.zx %s46, 12(,%s51)
	ldl.zx %s47, 12(,%s52)
	or %s48, 12, %s55
	adds.l %s48, %s34, %s48
	ldl.zx %s48, (,%s48)
	ldl.zx %s49, 12(,%s54)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	pvfmad %v4,%v0,%s44,%v15
	pvfmad %v4,%v4,%s45,%v16
	pvfmad %v4,%v4,%s46,%v17
	pvfmad %v4,%v4,%s47,%v18
	ldl.zx %s44, 16(,%s34)
	ldl.zx %s45, 16(,%s50)
	pvfmad %v4,%v4,%s48,%v19
	pvfmad %v4,%v4,%s49,%v20
	muls.l %s44, %s37, %s44
	muls.l %s45, %s37, %s45
	ldl.zx %s46, 16(,%s51)
	ldl.zx %s47, 16(,%s52)
	ldl.zx %s48, 16(,%s53)
	ldl.zx %s49, 16(,%s54)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	pvfmad %v5,%v0,%s44,%v15
	pvfmad %v5,%v5,%s45,%v16
	pvfmad %v5,%v5,%s46,%v17
	pvfmad %v5,%v5,%s47,%v18
	ldl.zx %s44, 20(,%s34)
	ldl.zx %s45, 20(,%s50)
	pvfmad %v5,%v5,%s48,%v19
	pvfmad %v5,%v5,%s49,%v20
	muls.l %s44, %s37, %s44
	muls.l %s45, %s37, %s45
	ldl.zx %s46, 20(,%s51)
	ldl.zx %s47, 20(,%s52)
	ldl.zx %s48, 20(,%s53)
	ldl.zx %s49, 20(,%s54)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	pvfmad %v6,%v0,%s44,%v15
	pvfmad %v6,%v6,%s45,%v16
	pvfmad %v6,%v6,%s46,%v17
	pvfmad %v6,%v6,%s47,%v18
	ldl.zx %s44, 24(,%s34)
	ldl.zx %s45, 24(,%s50)
	pvfmad %v6,%v6,%s48,%v19
	pvfmad %v6,%v6,%s49,%v20
	muls.l %s44, %s37, %s44
	muls.l %s45, %s37, %s45
	ldl.zx %s46, 24(,%s51)
	ldl.zx %s47, 24(,%s52)
	ldl.zx %s48, 24(,%s53)
	ldl.zx %s49, 24(,%s54)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	pvfmad %v21,%v0,%s44,%v15
	pvfmad %v21,%v21,%s45,%v16
	pvfmad %v21,%v21,%s46,%v17
	pvfmad %v21,%v21,%s47,%v18
	ldl.zx %s34, 28(,%s34)
	ldl.zx %s44, 28(,%s50)
	pvfmad %v21,%v21,%s48,%v19
	pvfmad %v21,%v21,%s49,%v20
	muls.l %s34, %s37, %s34
	muls.l %s44, %s37, %s44
	ldl.zx %s45, 28(,%s51)
	ldl.zx %s46, 28(,%s52)
	ldl.zx %s47, 28(,%s53)
	ldl.zx %s48, 28(,%s54)
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s37, %s37, %s48
	pvfmad %v0,%v0,%s34,%v15
	pvfmad %v0,%v0,%s44,%v16
	pvfmad %v0,%v0,%s45,%v17
	pvfmad %v0,%v0,%s46,%v18
	pvfmad %v0,%v0,%s47,%v19
	pvfmad %v0,%v0,%s37,%v20
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v21
	pvfadd %v0,%v14,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s38
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v0,8,%s43
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end7:
	.size	w2v_kernel_N6_HU512_2X_W08, .Lfunc_end7-w2v_kernel_N6_HU512_2X_W08

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI8_0:
	.4byte	1065353216
.LCPI8_1:
	.4byte	0
.LCPI8_2:
	.4byte	3212836864
.LCPI8_3:
	.4byte	1086324736
.LCPI8_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI8_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W09
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W09,@function
w2v_kernel_N6_HU512_2X_W09:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB8_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB8_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s37, -8(,%s9)
	adds.w.sx %s51, %s34, (0)1
	lvl %s51
	ldl.sx %s34, (,%s4)
	ld %s44, 240(,%s9)
	muls.l %s34, %s34, %s1
	ldl.sx %s36, 4(,%s4)
	sll %s34, %s34, 2
	adds.l %s35, %s44, %s34
	ldl.sx %s34, 8(,%s4)
	muls.l %s36, %s36, %s1
	sll %s36, %s36, 2
	adds.l %s36, %s44, %s36
	muls.l %s34, %s34, %s1
	ldl.sx %s39, 12(,%s4)
	sll %s34, %s34, 2
	adds.l %s38, %s44, %s34
	ldl.sx %s34, 16(,%s4)
	muls.l %s39, %s39, %s1
	sll %s39, %s39, 2
	adds.l %s39, %s44, %s39
	muls.l %s34, %s34, %s1
	ldl.sx %s41, 20(,%s4)
	sll %s34, %s34, 2
	adds.l %s40, %s44, %s34
	ldl.sx %s34, 24(,%s4)
	muls.l %s41, %s41, %s1
	sll %s41, %s41, 2
	adds.l %s41, %s44, %s41
	muls.l %s34, %s34, %s1
	ldl.sx %s43, 28(,%s4)
	sll %s34, %s34, 2
	adds.l %s42, %s44, %s34
	ldl.sx %s34, 32(,%s4)
	muls.l %s43, %s43, %s1
	sll %s43, %s43, 2
	adds.l %s43, %s44, %s43
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s44, %s44, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	ldl.sx %s34, (,%s5)
	ldl.sx %s46, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s45, %s7, %s34
	muls.l %s34, %s46, %s1
	ldl.sx %s47, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s46, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s47, %s47, %s1
	sll %s47, %s47, 2
	adds.l %s47, %s7, %s47
	muls.l %s34, %s34, %s1
	ldl.sx %s49, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s48, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s49, %s49, %s1
	sll %s49, %s49, 2
	adds.l %s49, %s7, %s49
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s50, %s7, %s34
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	vld %v19,8,%s48
	vld %v20,8,%s49
	vld %v21,8,%s50
	lvl %s51
	pvfmul %v0,%v16,%v7
	pvfmul %v1,%v16,%v8
	pvfmul %v2,%v16,%v9
	pvfmul %v3,%v16,%v10
	pvfmul %v4,%v16,%v11
	pvfmul %v5,%v16,%v12
	pvfmul %v6,%v16,%v13
	pvfmul %v22,%v16,%v14
	pvfmul %v23,%v16,%v15
	vsll %v24,%v0,32
	vsll %v25,%v1,32
	vsll %v26,%v2,32
	vsll %v27,%v3,32
	vsll %v28,%v4,32
	vsll %v29,%v5,32
	vsll %v30,%v6,32
	vsll %v31,%v22,32
	vsll %v32,%v23,32
	vfadd.s %v0,%v0,%v24
	vfadd.s %v1,%v1,%v25
	vfadd.s %v2,%v2,%v26
	vfadd.s %v3,%v3,%v27
	vfadd.s %v4,%v4,%v28
	vfadd.s %v5,%v5,%v29
	vfadd.s %v6,%v6,%v30
	vfadd.s %v22,%v22,%v31
	vfadd.s %v23,%v23,%v32
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v22,%v22
	vfsum.s %v23,%v23
	or %s52, 1, (0)1
	lvl %s52
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s53, 4(%s6)
	vstu %v1,4,%s53
	lea %s53, 8(%s6)
	vstu %v2,4,%s53
	lea %s53, 12(%s6)
	vstu %v3,4,%s53
	lea %s53, 16(%s6)
	vstu %v4,4,%s53
	lea %s53, 20(%s6)
	vstu %v5,4,%s53
	lea %s53, 24(%s6)
	vstu %v6,4,%s53
	lea %s53, 28(%s6)
	vstu %v22,4,%s53
	lea %s53, 32(%s6)
	vstu %v23,4,%s53
	lvl %s51
	pvfmul %v0,%v17,%v7
	pvfmul %v1,%v17,%v8
	pvfmul %v2,%v17,%v9
	pvfmul %v3,%v17,%v10
	pvfmul %v4,%v17,%v11
	pvfmul %v5,%v17,%v12
	pvfmul %v6,%v17,%v13
	pvfmul %v22,%v17,%v14
	pvfmul %v23,%v17,%v15
	vsll %v24,%v0,32
	vsll %v25,%v1,32
	vsll %v26,%v2,32
	vsll %v27,%v3,32
	vsll %v28,%v4,32
	vsll %v29,%v5,32
	vsll %v30,%v6,32
	vsll %v31,%v22,32
	vsll %v32,%v23,32
	vfadd.s %v0,%v0,%v24
	vfadd.s %v1,%v1,%v25
	vfadd.s %v2,%v2,%v26
	vfadd.s %v3,%v3,%v27
	vfadd.s %v4,%v4,%v28
	vfadd.s %v5,%v5,%v29
	vfadd.s %v6,%v6,%v30
	vfadd.s %v22,%v22,%v31
	vfadd.s %v23,%v23,%v32
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v22,%v22
	vfsum.s %v23,%v23
	lvl %s52
	lea %s53, 36(%s6)
	vstu %v0,4,%s53
	lea %s53, 40(%s6)
	vstu %v1,4,%s53
	lea %s53, 44(%s6)
	vstu %v2,4,%s53
	lea %s53, 48(%s6)
	vstu %v3,4,%s53
	lea %s53, 52(%s6)
	vstu %v4,4,%s53
	lea %s53, 56(%s6)
	vstu %v5,4,%s53
	lea %s53, 60(%s6)
	vstu %v6,4,%s53
	lea %s53, 64(%s6)
	vstu %v22,4,%s53
	lea %s53, 68(%s6)
	vstu %v23,4,%s53
	lvl %s51
	pvfmul %v0,%v18,%v7
	pvfmul %v1,%v18,%v8
	pvfmul %v2,%v18,%v9
	pvfmul %v3,%v18,%v10
	pvfmul %v4,%v18,%v11
	pvfmul %v5,%v18,%v12
	pvfmul %v6,%v18,%v13
	pvfmul %v22,%v18,%v14
	pvfmul %v23,%v18,%v15
	vsll %v24,%v0,32
	vsll %v25,%v1,32
	vsll %v26,%v2,32
	vsll %v27,%v3,32
	vsll %v28,%v4,32
	vsll %v29,%v5,32
	vsll %v30,%v6,32
	vsll %v31,%v22,32
	vsll %v32,%v23,32
	vfadd.s %v0,%v0,%v24
	vfadd.s %v1,%v1,%v25
	vfadd.s %v2,%v2,%v26
	vfadd.s %v3,%v3,%v27
	vfadd.s %v4,%v4,%v28
	vfadd.s %v5,%v5,%v29
	vfadd.s %v6,%v6,%v30
	vfadd.s %v22,%v22,%v31
	vfadd.s %v23,%v23,%v32
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v22,%v22
	vfsum.s %v23,%v23
	lvl %s52
	lea %s53, 72(%s6)
	vstu %v0,4,%s53
	lea %s53, 76(%s6)
	vstu %v1,4,%s53
	lea %s53, 80(%s6)
	vstu %v2,4,%s53
	lea %s53, 84(%s6)
	vstu %v3,4,%s53
	lea %s53, 88(%s6)
	vstu %v4,4,%s53
	lea %s53, 92(%s6)
	vstu %v5,4,%s53
	lea %s53, 96(%s6)
	vstu %v6,4,%s53
	lea %s53, 100(%s6)
	vstu %v22,4,%s53
	lea %s53, 104(%s6)
	vstu %v23,4,%s53
	lvl %s51
	pvfmul %v0,%v19,%v7
	pvfmul %v1,%v19,%v8
	pvfmul %v2,%v19,%v9
	pvfmul %v3,%v19,%v10
	pvfmul %v4,%v19,%v11
	pvfmul %v5,%v19,%v12
	pvfmul %v6,%v19,%v13
	pvfmul %v22,%v19,%v14
	pvfmul %v23,%v19,%v15
	vsll %v24,%v0,32
	vsll %v25,%v1,32
	vsll %v26,%v2,32
	vsll %v27,%v3,32
	vsll %v28,%v4,32
	vsll %v29,%v5,32
	vsll %v30,%v6,32
	vsll %v31,%v22,32
	vsll %v32,%v23,32
	vfadd.s %v0,%v0,%v24
	vfadd.s %v1,%v1,%v25
	vfadd.s %v2,%v2,%v26
	vfadd.s %v3,%v3,%v27
	vfadd.s %v4,%v4,%v28
	vfadd.s %v5,%v5,%v29
	vfadd.s %v6,%v6,%v30
	vfadd.s %v22,%v22,%v31
	vfadd.s %v23,%v23,%v32
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v22,%v22
	vfsum.s %v23,%v23
	lvl %s52
	lea %s53, 108(%s6)
	vstu %v0,4,%s53
	lea %s53, 112(%s6)
	vstu %v1,4,%s53
	lea %s53, 116(%s6)
	vstu %v2,4,%s53
	lea %s53, 120(%s6)
	vstu %v3,4,%s53
	lea %s53, 124(%s6)
	vstu %v4,4,%s53
	lea %s53, 128(%s6)
	vstu %v5,4,%s53
	lea %s53, 132(%s6)
	vstu %v6,4,%s53
	lea %s53, 136(%s6)
	vstu %v22,4,%s53
	lea %s53, 140(%s6)
	vstu %v23,4,%s53
	lvl %s51
	pvfmul %v0,%v20,%v7
	pvfmul %v1,%v20,%v8
	pvfmul %v2,%v20,%v9
	pvfmul %v3,%v20,%v10
	pvfmul %v4,%v20,%v11
	pvfmul %v5,%v20,%v12
	pvfmul %v6,%v20,%v13
	pvfmul %v22,%v20,%v14
	pvfmul %v23,%v20,%v15
	vsll %v24,%v0,32
	vsll %v25,%v1,32
	vsll %v26,%v2,32
	vsll %v27,%v3,32
	vsll %v28,%v4,32
	vsll %v29,%v5,32
	vsll %v30,%v6,32
	vsll %v31,%v22,32
	vsll %v32,%v23,32
	vfadd.s %v0,%v0,%v24
	vfadd.s %v1,%v1,%v25
	vfadd.s %v2,%v2,%v26
	vfadd.s %v3,%v3,%v27
	vfadd.s %v4,%v4,%v28
	vfadd.s %v5,%v5,%v29
	vfadd.s %v6,%v6,%v30
	vfadd.s %v22,%v22,%v31
	vfadd.s %v23,%v23,%v32
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v22,%v22
	vfsum.s %v23,%v23
	lvl %s52
	lea %s53, 144(%s6)
	vstu %v0,4,%s53
	lea %s53, 148(%s6)
	vstu %v1,4,%s53
	lea %s53, 152(%s6)
	vstu %v2,4,%s53
	lea %s53, 156(%s6)
	vstu %v3,4,%s53
	lea %s53, 160(%s6)
	vstu %v4,4,%s53
	lea %s53, 164(%s6)
	vstu %v5,4,%s53
	lea %s53, 168(%s6)
	vstu %v6,4,%s53
	lea %s53, 172(%s6)
	vstu %v22,4,%s53
	lea %s53, 176(%s6)
	vstu %v23,4,%s53
	lvl %s51
	pvfmul %v0,%v21,%v7
	pvfmul %v1,%v21,%v8
	pvfmul %v2,%v21,%v9
	pvfmul %v3,%v21,%v10
	pvfmul %v4,%v21,%v11
	pvfmul %v5,%v21,%v12
	pvfmul %v6,%v21,%v13
	pvfmul %v22,%v21,%v14
	pvfmul %v23,%v21,%v15
	vsll %v24,%v0,32
	vsll %v25,%v1,32
	vsll %v26,%v2,32
	vsll %v27,%v3,32
	vsll %v28,%v4,32
	vsll %v29,%v5,32
	vsll %v30,%v6,32
	vsll %v31,%v22,32
	vsll %v32,%v23,32
	vfadd.s %v0,%v0,%v24
	vfadd.s %v1,%v1,%v25
	vfadd.s %v2,%v2,%v26
	vfadd.s %v3,%v3,%v27
	vfadd.s %v4,%v4,%v28
	vfadd.s %v5,%v5,%v29
	vfadd.s %v6,%v6,%v30
	vfadd.s %v22,%v22,%v31
	vfadd.s %v23,%v23,%v32
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v22,%v22
	vfsum.s %v23,%v23
	lvl %s52
	lea %s52, 180(%s6)
	vstu %v0,4,%s52
	lea %s52, 184(%s6)
	vstu %v1,4,%s52
	lea %s52, 188(%s6)
	vstu %v2,4,%s52
	lea %s52, 192(%s6)
	vstu %v3,4,%s52
	lea %s52, 196(%s6)
	vstu %v4,4,%s52
	lea %s52, 200(%s6)
	vstu %v5,4,%s52
	lea %s52, 204(%s6)
	vstu %v6,4,%s52
	lea %s52, 208(%s6)
	vstu %v22,4,%s52
	lea %s52, 212(%s6)
	vstu %v23,4,%s52
	muls.l %s52, %s2, %s0
	adds.w.sx %s52, %s52, (0)1
	lvl %s52
	lea.sl %s52, .LCPI8_0@hi
	ldu %s52, .LCPI8_0@lo(,%s52)
	vseq %v0
	vbrdu %v22,%s52
	adds.w.sx %s53, %s2, (0)1
	vsubs.w.sx %v0,%s53,%v0
	vldu %v23,4,%s6
	lea.sl %s53, .LCPI8_2@hi
	ldu %s53, .LCPI8_2@lo(,%s53)
	vfmk.w.gt %vm1,%v0
	lea.sl %s54, .LCPI8_1@hi
	ldu %s54, .LCPI8_1@lo(,%s54)
	vfmul.s %v0,%s53,%v23
	lea %s53, __vec_expf@lo
	and %s53, %s53, (32)0
	lea.sl %s53, __vec_expf@hi(%s53)
	vbrdu %v22,%s54,%vm1
	bsic %lr, (,%s53)
	lea.sl %s53, .LCPI8_3@hi
	ldu %s53, .LCPI8_3@lo(,%s53)
	vfadd.s %v0,%s52,%v0
	vfsub.s %v1,%s53,%v23
	lea.sl %s53, .LCPI8_4@hi
	ldu %s53, .LCPI8_4@lo(,%s53)
	vfdiv.s %v0,%s52,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s52,%vm1
	vfsub.s %v1,%s53,%v23
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s54,%vm1
	vfsub.s %v0,%v22,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s51
	lea.sl %s51, .LCPI8_5@hi
	ld %s52, .LCPI8_5@lo(,%s51)
	ldl.zx %s53, (,%s34)
	sll %s58, %s2, 2
	adds.l %s51, %s34, %s58
	ldl.zx %s54, (,%s51)
	vbrd %v0,%s52
	muls.l %s61, %s37, %s53
	muls.l %s62, %s37, %s54
	sll %s57, %s2, 3
	adds.l %s52, %s34, %s57
	ldl.zx %s63, (,%s52)
	muls.l %s59, 12, %s2
	adds.l %s53, %s34, %s59
	ldl.zx %s0, (,%s53)
	sll %s56, %s2, 4
	adds.l %s54, %s34, %s56
	ldl.zx %s1, (,%s54)
	muls.l %s60, 20, %s2
	adds.l %s55, %s34, %s60
	ldl.zx %s2, (,%s55)
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v0,%s61,%v7
	pvfmad %v2,%v0,%s62,%v7
	pvfmad %v3,%v0,%s63,%v7
	pvfmad %v4,%v0,%s0,%v7
	ldl.zx %s61, 4(,%s34)
	lea %s62, 4(%s34, %s58)
	ldl.zx %s62, (,%s62)
	pvfmad %v5,%v0,%s1,%v7
	pvfmad %v6,%v0,%s2,%v7
	muls.l %s61, %s37, %s61
	muls.l %s62, %s37, %s62
	lea %s63, 4(%s34, %s57)
	ldl.zx %s63, (,%s63)
	lea %s0, 4(%s34, %s59)
	ldl.zx %s0, (,%s0)
	lea %s1, 4(%s34, %s56)
	ldl.zx %s1, (,%s1)
	lea %s2, 4(%s34, %s60)
	ldl.zx %s2, (,%s2)
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v1,%s61,%v8
	pvfmad %v2,%v2,%s62,%v8
	pvfmad %v3,%v3,%s63,%v8
	pvfmad %v4,%v4,%s0,%v8
	ldl.zx %s61, 8(,%s34)
	lea %s62, 8(%s34, %s58)
	ldl.zx %s62, (,%s62)
	pvfmad %v5,%v5,%s1,%v8
	pvfmad %v6,%v6,%s2,%v8
	muls.l %s61, %s37, %s61
	muls.l %s62, %s37, %s62
	lea %s63, 8(%s34, %s57)
	ldl.zx %s63, (,%s63)
	lea %s0, 8(%s34, %s59)
	ldl.zx %s0, (,%s0)
	lea %s1, 8(%s34, %s56)
	ldl.zx %s1, (,%s1)
	lea %s2, 8(%s34, %s60)
	ldl.zx %s2, (,%s2)
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v1,%s61,%v9
	pvfmad %v2,%v2,%s62,%v9
	pvfmad %v3,%v3,%s63,%v9
	pvfmad %v4,%v4,%s0,%v9
	ldl.zx %s61, 12(,%s34)
	lea %s62, 12(%s34, %s58)
	ldl.zx %s62, (,%s62)
	pvfmad %v5,%v5,%s1,%v9
	pvfmad %v6,%v6,%s2,%v9
	muls.l %s61, %s37, %s61
	muls.l %s62, %s37, %s62
	lea %s63, 12(%s34, %s57)
	ldl.zx %s63, (,%s63)
	lea %s0, 12(%s34, %s59)
	ldl.zx %s0, (,%s0)
	lea %s1, 12(%s34, %s56)
	ldl.zx %s1, (,%s1)
	lea %s2, 12(%s34, %s60)
	ldl.zx %s2, (,%s2)
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v1,%s61,%v10
	pvfmad %v2,%v2,%s62,%v10
	pvfmad %v3,%v3,%s63,%v10
	pvfmad %v4,%v4,%s0,%v10
	ldl.zx %s61, 16(,%s34)
	lea %s62, 16(%s34, %s58)
	ldl.zx %s62, (,%s62)
	pvfmad %v5,%v5,%s1,%v10
	pvfmad %v6,%v6,%s2,%v10
	muls.l %s61, %s37, %s61
	muls.l %s62, %s37, %s62
	lea %s63, 16(%s34, %s57)
	ldl.zx %s63, (,%s63)
	lea %s0, 16(%s34, %s59)
	ldl.zx %s0, (,%s0)
	lea %s1, 16(%s34, %s56)
	ldl.zx %s1, (,%s1)
	lea %s2, 16(%s34, %s60)
	ldl.zx %s2, (,%s2)
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v1,%s61,%v11
	pvfmad %v2,%v2,%s62,%v11
	pvfmad %v3,%v3,%s63,%v11
	pvfmad %v4,%v4,%s0,%v11
	ldl.zx %s61, 20(,%s34)
	lea %s62, 20(%s34, %s58)
	ldl.zx %s62, (,%s62)
	pvfmad %v5,%v5,%s1,%v11
	pvfmad %v6,%v6,%s2,%v11
	muls.l %s61, %s37, %s61
	muls.l %s62, %s37, %s62
	lea %s63, 20(%s34, %s57)
	ldl.zx %s63, (,%s63)
	lea %s0, 20(%s34, %s59)
	ldl.zx %s0, (,%s0)
	lea %s1, 20(%s34, %s56)
	ldl.zx %s1, (,%s1)
	lea %s2, 20(%s34, %s60)
	ldl.zx %s2, (,%s2)
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v1,%s61,%v12
	pvfmad %v2,%v2,%s62,%v12
	pvfmad %v3,%v3,%s63,%v12
	pvfmad %v4,%v4,%s0,%v12
	ldl.zx %s61, 24(,%s34)
	lea %s62, 24(%s34, %s58)
	ldl.zx %s62, (,%s62)
	pvfmad %v5,%v5,%s1,%v12
	pvfmad %v6,%v6,%s2,%v12
	muls.l %s61, %s37, %s61
	muls.l %s62, %s37, %s62
	lea %s63, 24(%s34, %s57)
	ldl.zx %s63, (,%s63)
	lea %s0, 24(%s34, %s59)
	ldl.zx %s0, (,%s0)
	lea %s1, 24(%s34, %s56)
	ldl.zx %s1, (,%s1)
	lea %s2, 24(%s34, %s60)
	ldl.zx %s2, (,%s2)
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v1,%s61,%v13
	pvfmad %v2,%v2,%s62,%v13
	pvfmad %v3,%v3,%s63,%v13
	pvfmad %v4,%v4,%s0,%v13
	ldl.zx %s61, 28(,%s34)
	lea %s62, 28(%s34, %s58)
	ldl.zx %s62, (,%s62)
	pvfmad %v5,%v5,%s1,%v13
	pvfmad %v6,%v6,%s2,%v13
	muls.l %s61, %s37, %s61
	muls.l %s62, %s37, %s62
	lea %s63, 28(%s34, %s57)
	ldl.zx %s63, (,%s63)
	lea %s0, 28(%s34, %s59)
	ldl.zx %s0, (,%s0)
	lea %s1, 28(%s34, %s56)
	ldl.zx %s1, (,%s1)
	lea %s2, 28(%s34, %s60)
	ldl.zx %s2, (,%s2)
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v1,%s61,%v14
	pvfmad %v2,%v2,%s62,%v14
	pvfmad %v3,%v3,%s63,%v14
	pvfmad %v4,%v4,%s0,%v14
	ldl.zx %s61, 32(,%s34)
	lea %s58, 32(%s34, %s58)
	ldl.zx %s58, (,%s58)
	pvfmad %v5,%v5,%s1,%v14
	pvfmad %v6,%v6,%s2,%v14
	muls.l %s61, %s37, %s61
	muls.l %s58, %s37, %s58
	lea %s62, 32(%s34, %s57)
	ldl.zx %s62, (,%s62)
	lea %s59, 32(%s34, %s59)
	ldl.zx %s59, (,%s59)
	lea %s63, 32(%s34, %s56)
	ldl.zx %s63, (,%s63)
	lea %s60, 32(%s34, %s60)
	ldl.zx %s60, (,%s60)
	muls.l %s62, %s37, %s62
	muls.l %s59, %s37, %s59
	muls.l %s63, %s37, %s63
	muls.l %s60, %s37, %s60
	pvfmad %v1,%v1,%s61,%v15
	pvfmad %v2,%v2,%s58,%v15
	pvfmad %v3,%v3,%s62,%v15
	pvfmad %v4,%v4,%s59,%v15
	pvfmad %v5,%v5,%s63,%v15
	pvfmad %v6,%v6,%s60,%v15
	pvfadd %v1,%v16,%v1
	pvfadd %v2,%v17,%v2
	pvfadd %v3,%v18,%v3
	pvfadd %v4,%v19,%v4
	pvfadd %v5,%v20,%v5
	pvfadd %v6,%v21,%v6
	vst %v1,8,%s45
	vst %v2,8,%s46
	vst %v3,8,%s47
	vst %v4,8,%s48
	vst %v5,8,%s49
	vst %v6,8,%s50
	ldl.zx %s45, (,%s34)
	ldl.zx %s46, (,%s51)
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	ldl.zx %s47, (,%s52)
	ldl.zx %s48, (,%s53)
	ldl.zx %s49, (,%s54)
	ldl.zx %s50, (,%s55)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	pvfmad %v1,%v0,%s45,%v16
	pvfmad %v1,%v1,%s46,%v17
	pvfmad %v1,%v1,%s47,%v18
	pvfmad %v1,%v1,%s48,%v19
	ldl.zx %s45, 4(,%s34)
	ldl.zx %s46, 4(,%s51)
	pvfmad %v1,%v1,%s49,%v20
	pvfmad %v1,%v1,%s50,%v21
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	or %s47, 4, %s57
	adds.l %s47, %s34, %s47
	ldl.zx %s47, (,%s47)
	ldl.zx %s48, 4(,%s53)
	or %s49, 4, %s56
	adds.l %s49, %s34, %s49
	ldl.zx %s49, (,%s49)
	ldl.zx %s50, 4(,%s55)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	pvfmad %v2,%v0,%s45,%v16
	pvfmad %v2,%v2,%s46,%v17
	pvfmad %v2,%v2,%s47,%v18
	pvfmad %v2,%v2,%s48,%v19
	ldl.zx %s45, 8(,%s34)
	ldl.zx %s46, 8(,%s51)
	pvfmad %v2,%v2,%s49,%v20
	pvfmad %v2,%v2,%s50,%v21
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	ldl.zx %s47, 8(,%s52)
	ldl.zx %s48, 8(,%s53)
	or %s49, 8, %s56
	adds.l %s49, %s34, %s49
	ldl.zx %s49, (,%s49)
	ldl.zx %s50, 8(,%s55)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	pvfmad %v3,%v0,%s45,%v16
	pvfmad %v3,%v3,%s46,%v17
	pvfmad %v3,%v3,%s47,%v18
	pvfmad %v3,%v3,%s48,%v19
	ldl.zx %s45, 12(,%s34)
	ldl.zx %s46, 12(,%s51)
	pvfmad %v3,%v3,%s49,%v20
	pvfmad %v3,%v3,%s50,%v21
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	ldl.zx %s47, 12(,%s52)
	ldl.zx %s48, 12(,%s53)
	or %s49, 12, %s56
	adds.l %s49, %s34, %s49
	ldl.zx %s49, (,%s49)
	ldl.zx %s50, 12(,%s55)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	pvfmad %v4,%v0,%s45,%v16
	pvfmad %v4,%v4,%s46,%v17
	pvfmad %v4,%v4,%s47,%v18
	pvfmad %v4,%v4,%s48,%v19
	ldl.zx %s45, 16(,%s34)
	ldl.zx %s46, 16(,%s51)
	pvfmad %v4,%v4,%s49,%v20
	pvfmad %v4,%v4,%s50,%v21
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	ldl.zx %s47, 16(,%s52)
	ldl.zx %s48, 16(,%s53)
	ldl.zx %s49, 16(,%s54)
	ldl.zx %s50, 16(,%s55)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	pvfmad %v5,%v0,%s45,%v16
	pvfmad %v5,%v5,%s46,%v17
	pvfmad %v5,%v5,%s47,%v18
	pvfmad %v5,%v5,%s48,%v19
	ldl.zx %s45, 20(,%s34)
	ldl.zx %s46, 20(,%s51)
	pvfmad %v5,%v5,%s49,%v20
	pvfmad %v5,%v5,%s50,%v21
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	ldl.zx %s47, 20(,%s52)
	ldl.zx %s48, 20(,%s53)
	ldl.zx %s49, 20(,%s54)
	ldl.zx %s50, 20(,%s55)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	pvfmad %v6,%v0,%s45,%v16
	pvfmad %v6,%v6,%s46,%v17
	pvfmad %v6,%v6,%s47,%v18
	pvfmad %v6,%v6,%s48,%v19
	ldl.zx %s45, 24(,%s34)
	ldl.zx %s46, 24(,%s51)
	pvfmad %v6,%v6,%s49,%v20
	pvfmad %v6,%v6,%s50,%v21
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	ldl.zx %s47, 24(,%s52)
	ldl.zx %s48, 24(,%s53)
	ldl.zx %s49, 24(,%s54)
	ldl.zx %s50, 24(,%s55)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	pvfmad %v22,%v0,%s45,%v16
	pvfmad %v22,%v22,%s46,%v17
	pvfmad %v22,%v22,%s47,%v18
	pvfmad %v22,%v22,%s48,%v19
	ldl.zx %s45, 28(,%s34)
	ldl.zx %s46, 28(,%s51)
	pvfmad %v22,%v22,%s49,%v20
	pvfmad %v22,%v22,%s50,%v21
	muls.l %s45, %s37, %s45
	muls.l %s46, %s37, %s46
	ldl.zx %s47, 28(,%s52)
	ldl.zx %s48, 28(,%s53)
	ldl.zx %s49, 28(,%s54)
	ldl.zx %s50, 28(,%s55)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	pvfmad %v23,%v0,%s45,%v16
	pvfmad %v23,%v23,%s46,%v17
	pvfmad %v23,%v23,%s47,%v18
	pvfmad %v23,%v23,%s48,%v19
	ldl.zx %s34, 32(,%s34)
	ldl.zx %s45, 32(,%s51)
	pvfmad %v23,%v23,%s49,%v20
	pvfmad %v23,%v23,%s50,%v21
	muls.l %s34, %s37, %s34
	muls.l %s45, %s37, %s45
	ldl.zx %s46, 32(,%s52)
	ldl.zx %s47, 32(,%s53)
	ldl.zx %s48, 32(,%s54)
	ldl.zx %s49, 32(,%s55)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s37, %s37, %s49
	pvfmad %v0,%v0,%s34,%v16
	pvfmad %v0,%v0,%s45,%v17
	pvfmad %v0,%v0,%s46,%v18
	pvfmad %v0,%v0,%s47,%v19
	pvfmad %v0,%v0,%s48,%v20
	pvfmad %v0,%v0,%s37,%v21
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v22
	pvfadd %v8,%v14,%v23
	pvfadd %v0,%v15,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s38
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v8,8,%s43
	vst %v0,8,%s44
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end8:
	.size	w2v_kernel_N6_HU512_2X_W09, .Lfunc_end8-w2v_kernel_N6_HU512_2X_W09

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI9_0:
	.4byte	1065353216
.LCPI9_1:
	.4byte	0
.LCPI9_2:
	.4byte	3212836864
.LCPI9_3:
	.4byte	1086324736
.LCPI9_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI9_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W10
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W10,@function
w2v_kernel_N6_HU512_2X_W10:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB9_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB9_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s37, -8(,%s9)
	adds.w.sx %s52, %s34, (0)1
	lvl %s52
	ldl.sx %s34, (,%s4)
	ld %s45, 240(,%s9)
	ldl.sx %s36, 4(,%s4)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s35, %s45, %s34
	muls.l %s34, %s36, %s1
	ldl.sx %s38, 8(,%s4)
	sll %s34, %s34, 2
	adds.l %s36, %s45, %s34
	ldl.sx %s34, 12(,%s4)
	muls.l %s38, %s38, %s1
	sll %s38, %s38, 2
	adds.l %s38, %s45, %s38
	muls.l %s34, %s34, %s1
	ldl.sx %s40, 16(,%s4)
	sll %s34, %s34, 2
	adds.l %s39, %s45, %s34
	ldl.sx %s34, 20(,%s4)
	muls.l %s40, %s40, %s1
	sll %s40, %s40, 2
	adds.l %s40, %s45, %s40
	muls.l %s34, %s34, %s1
	ldl.sx %s42, 24(,%s4)
	sll %s34, %s34, 2
	adds.l %s41, %s45, %s34
	ldl.sx %s34, 28(,%s4)
	muls.l %s42, %s42, %s1
	sll %s42, %s42, 2
	adds.l %s42, %s45, %s42
	muls.l %s34, %s34, %s1
	ldl.sx %s44, 32(,%s4)
	sll %s34, %s34, 2
	adds.l %s43, %s45, %s34
	ldl.sx %s34, 36(,%s4)
	muls.l %s44, %s44, %s1
	sll %s44, %s44, 2
	adds.l %s44, %s45, %s44
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s45, %s45, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	ldl.sx %s34, (,%s5)
	ldl.sx %s47, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s46, %s7, %s34
	muls.l %s34, %s47, %s1
	ldl.sx %s48, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s47, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s48, %s48, %s1
	sll %s48, %s48, 2
	adds.l %s48, %s7, %s48
	muls.l %s34, %s34, %s1
	ldl.sx %s50, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s49, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s50, %s50, %s1
	sll %s50, %s50, 2
	adds.l %s50, %s7, %s50
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s51, %s7, %s34
	vld %v17,8,%s46
	vld %v18,8,%s47
	vld %v19,8,%s48
	vld %v20,8,%s49
	vld %v21,8,%s50
	vld %v22,8,%s51
	lvl %s52
	pvfmul %v0,%v17,%v7
	pvfmul %v1,%v17,%v8
	pvfmul %v2,%v17,%v9
	pvfmul %v3,%v17,%v10
	pvfmul %v4,%v17,%v11
	pvfmul %v5,%v17,%v12
	pvfmul %v6,%v17,%v13
	pvfmul %v23,%v17,%v14
	pvfmul %v24,%v17,%v15
	pvfmul %v25,%v17,%v16
	vsll %v26,%v0,32
	vsll %v27,%v1,32
	vsll %v28,%v2,32
	vsll %v29,%v3,32
	vsll %v30,%v4,32
	vsll %v31,%v5,32
	vsll %v32,%v6,32
	vsll %v33,%v23,32
	vsll %v34,%v24,32
	vsll %v35,%v25,32
	vfadd.s %v0,%v0,%v26
	vfadd.s %v1,%v1,%v27
	vfadd.s %v2,%v2,%v28
	vfadd.s %v3,%v3,%v29
	vfadd.s %v4,%v4,%v30
	vfadd.s %v5,%v5,%v31
	vfadd.s %v6,%v6,%v32
	vfadd.s %v23,%v23,%v33
	vfadd.s %v24,%v24,%v34
	vfadd.s %v25,%v25,%v35
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v23,%v23
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	or %s53, 1, (0)1
	lvl %s53
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s54, 4(%s6)
	vstu %v1,4,%s54
	lea %s54, 8(%s6)
	vstu %v2,4,%s54
	lea %s54, 12(%s6)
	vstu %v3,4,%s54
	lea %s54, 16(%s6)
	vstu %v4,4,%s54
	lea %s54, 20(%s6)
	vstu %v5,4,%s54
	lea %s54, 24(%s6)
	vstu %v6,4,%s54
	lea %s54, 28(%s6)
	vstu %v23,4,%s54
	lea %s54, 32(%s6)
	vstu %v24,4,%s54
	lea %s54, 36(%s6)
	vstu %v25,4,%s54
	lvl %s52
	pvfmul %v0,%v18,%v7
	pvfmul %v1,%v18,%v8
	pvfmul %v2,%v18,%v9
	pvfmul %v3,%v18,%v10
	pvfmul %v4,%v18,%v11
	pvfmul %v5,%v18,%v12
	pvfmul %v6,%v18,%v13
	pvfmul %v23,%v18,%v14
	pvfmul %v24,%v18,%v15
	pvfmul %v25,%v18,%v16
	vsll %v26,%v0,32
	vsll %v27,%v1,32
	vsll %v28,%v2,32
	vsll %v29,%v3,32
	vsll %v30,%v4,32
	vsll %v31,%v5,32
	vsll %v32,%v6,32
	vsll %v33,%v23,32
	vsll %v34,%v24,32
	vsll %v35,%v25,32
	vfadd.s %v0,%v0,%v26
	vfadd.s %v1,%v1,%v27
	vfadd.s %v2,%v2,%v28
	vfadd.s %v3,%v3,%v29
	vfadd.s %v4,%v4,%v30
	vfadd.s %v5,%v5,%v31
	vfadd.s %v6,%v6,%v32
	vfadd.s %v23,%v23,%v33
	vfadd.s %v24,%v24,%v34
	vfadd.s %v25,%v25,%v35
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v23,%v23
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	lvl %s53
	lea %s54, 40(%s6)
	vstu %v0,4,%s54
	lea %s54, 44(%s6)
	vstu %v1,4,%s54
	lea %s54, 48(%s6)
	vstu %v2,4,%s54
	lea %s54, 52(%s6)
	vstu %v3,4,%s54
	lea %s54, 56(%s6)
	vstu %v4,4,%s54
	lea %s54, 60(%s6)
	vstu %v5,4,%s54
	lea %s54, 64(%s6)
	vstu %v6,4,%s54
	lea %s54, 68(%s6)
	vstu %v23,4,%s54
	lea %s54, 72(%s6)
	vstu %v24,4,%s54
	lea %s54, 76(%s6)
	vstu %v25,4,%s54
	lvl %s52
	pvfmul %v0,%v19,%v7
	pvfmul %v1,%v19,%v8
	pvfmul %v2,%v19,%v9
	pvfmul %v3,%v19,%v10
	pvfmul %v4,%v19,%v11
	pvfmul %v5,%v19,%v12
	pvfmul %v6,%v19,%v13
	pvfmul %v23,%v19,%v14
	pvfmul %v24,%v19,%v15
	pvfmul %v25,%v19,%v16
	vsll %v26,%v0,32
	vsll %v27,%v1,32
	vsll %v28,%v2,32
	vsll %v29,%v3,32
	vsll %v30,%v4,32
	vsll %v31,%v5,32
	vsll %v32,%v6,32
	vsll %v33,%v23,32
	vsll %v34,%v24,32
	vsll %v35,%v25,32
	vfadd.s %v0,%v0,%v26
	vfadd.s %v1,%v1,%v27
	vfadd.s %v2,%v2,%v28
	vfadd.s %v3,%v3,%v29
	vfadd.s %v4,%v4,%v30
	vfadd.s %v5,%v5,%v31
	vfadd.s %v6,%v6,%v32
	vfadd.s %v23,%v23,%v33
	vfadd.s %v24,%v24,%v34
	vfadd.s %v25,%v25,%v35
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v23,%v23
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	lvl %s53
	lea %s54, 80(%s6)
	vstu %v0,4,%s54
	lea %s54, 84(%s6)
	vstu %v1,4,%s54
	lea %s54, 88(%s6)
	vstu %v2,4,%s54
	lea %s54, 92(%s6)
	vstu %v3,4,%s54
	lea %s54, 96(%s6)
	vstu %v4,4,%s54
	lea %s54, 100(%s6)
	vstu %v5,4,%s54
	lea %s54, 104(%s6)
	vstu %v6,4,%s54
	lea %s54, 108(%s6)
	vstu %v23,4,%s54
	lea %s54, 112(%s6)
	vstu %v24,4,%s54
	lea %s54, 116(%s6)
	vstu %v25,4,%s54
	lvl %s52
	pvfmul %v0,%v20,%v7
	pvfmul %v1,%v20,%v8
	pvfmul %v2,%v20,%v9
	pvfmul %v3,%v20,%v10
	pvfmul %v4,%v20,%v11
	pvfmul %v5,%v20,%v12
	pvfmul %v6,%v20,%v13
	pvfmul %v23,%v20,%v14
	pvfmul %v24,%v20,%v15
	pvfmul %v25,%v20,%v16
	vsll %v26,%v0,32
	vsll %v27,%v1,32
	vsll %v28,%v2,32
	vsll %v29,%v3,32
	vsll %v30,%v4,32
	vsll %v31,%v5,32
	vsll %v32,%v6,32
	vsll %v33,%v23,32
	vsll %v34,%v24,32
	vsll %v35,%v25,32
	vfadd.s %v0,%v0,%v26
	vfadd.s %v1,%v1,%v27
	vfadd.s %v2,%v2,%v28
	vfadd.s %v3,%v3,%v29
	vfadd.s %v4,%v4,%v30
	vfadd.s %v5,%v5,%v31
	vfadd.s %v6,%v6,%v32
	vfadd.s %v23,%v23,%v33
	vfadd.s %v24,%v24,%v34
	vfadd.s %v25,%v25,%v35
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v23,%v23
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	lvl %s53
	lea %s54, 120(%s6)
	vstu %v0,4,%s54
	lea %s54, 124(%s6)
	vstu %v1,4,%s54
	lea %s54, 128(%s6)
	vstu %v2,4,%s54
	lea %s54, 132(%s6)
	vstu %v3,4,%s54
	lea %s54, 136(%s6)
	vstu %v4,4,%s54
	lea %s54, 140(%s6)
	vstu %v5,4,%s54
	lea %s54, 144(%s6)
	vstu %v6,4,%s54
	lea %s54, 148(%s6)
	vstu %v23,4,%s54
	lea %s54, 152(%s6)
	vstu %v24,4,%s54
	lea %s54, 156(%s6)
	vstu %v25,4,%s54
	lvl %s52
	pvfmul %v0,%v21,%v7
	pvfmul %v1,%v21,%v8
	pvfmul %v2,%v21,%v9
	pvfmul %v3,%v21,%v10
	pvfmul %v4,%v21,%v11
	pvfmul %v5,%v21,%v12
	pvfmul %v6,%v21,%v13
	pvfmul %v23,%v21,%v14
	pvfmul %v24,%v21,%v15
	pvfmul %v25,%v21,%v16
	vsll %v26,%v0,32
	vsll %v27,%v1,32
	vsll %v28,%v2,32
	vsll %v29,%v3,32
	vsll %v30,%v4,32
	vsll %v31,%v5,32
	vsll %v32,%v6,32
	vsll %v33,%v23,32
	vsll %v34,%v24,32
	vsll %v35,%v25,32
	vfadd.s %v0,%v0,%v26
	vfadd.s %v1,%v1,%v27
	vfadd.s %v2,%v2,%v28
	vfadd.s %v3,%v3,%v29
	vfadd.s %v4,%v4,%v30
	vfadd.s %v5,%v5,%v31
	vfadd.s %v6,%v6,%v32
	vfadd.s %v23,%v23,%v33
	vfadd.s %v24,%v24,%v34
	vfadd.s %v25,%v25,%v35
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v23,%v23
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	lvl %s53
	lea %s54, 160(%s6)
	vstu %v0,4,%s54
	lea %s54, 164(%s6)
	vstu %v1,4,%s54
	lea %s54, 168(%s6)
	vstu %v2,4,%s54
	lea %s54, 172(%s6)
	vstu %v3,4,%s54
	lea %s54, 176(%s6)
	vstu %v4,4,%s54
	lea %s54, 180(%s6)
	vstu %v5,4,%s54
	lea %s54, 184(%s6)
	vstu %v6,4,%s54
	lea %s54, 188(%s6)
	vstu %v23,4,%s54
	lea %s54, 192(%s6)
	vstu %v24,4,%s54
	lea %s54, 196(%s6)
	vstu %v25,4,%s54
	lvl %s52
	pvfmul %v0,%v22,%v7
	pvfmul %v1,%v22,%v8
	pvfmul %v2,%v22,%v9
	pvfmul %v3,%v22,%v10
	pvfmul %v4,%v22,%v11
	pvfmul %v5,%v22,%v12
	pvfmul %v6,%v22,%v13
	pvfmul %v23,%v22,%v14
	pvfmul %v24,%v22,%v15
	pvfmul %v25,%v22,%v16
	vsll %v26,%v0,32
	vsll %v27,%v1,32
	vsll %v28,%v2,32
	vsll %v29,%v3,32
	vsll %v30,%v4,32
	vsll %v31,%v5,32
	vsll %v32,%v6,32
	vsll %v33,%v23,32
	vsll %v34,%v24,32
	vsll %v35,%v25,32
	vfadd.s %v0,%v0,%v26
	vfadd.s %v1,%v1,%v27
	vfadd.s %v2,%v2,%v28
	vfadd.s %v3,%v3,%v29
	vfadd.s %v4,%v4,%v30
	vfadd.s %v5,%v5,%v31
	vfadd.s %v6,%v6,%v32
	vfadd.s %v23,%v23,%v33
	vfadd.s %v24,%v24,%v34
	vfadd.s %v25,%v25,%v35
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v23,%v23
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	lvl %s53
	lea %s53, 200(%s6)
	vstu %v0,4,%s53
	lea %s53, 204(%s6)
	vstu %v1,4,%s53
	lea %s53, 208(%s6)
	vstu %v2,4,%s53
	lea %s53, 212(%s6)
	vstu %v3,4,%s53
	lea %s53, 216(%s6)
	vstu %v4,4,%s53
	lea %s53, 220(%s6)
	vstu %v5,4,%s53
	lea %s53, 224(%s6)
	vstu %v6,4,%s53
	lea %s53, 228(%s6)
	vstu %v23,4,%s53
	lea %s53, 232(%s6)
	vstu %v24,4,%s53
	lea %s53, 236(%s6)
	vstu %v25,4,%s53
	muls.l %s53, %s2, %s0
	adds.w.sx %s53, %s53, (0)1
	lvl %s53
	lea.sl %s53, .LCPI9_0@hi
	ldu %s53, .LCPI9_0@lo(,%s53)
	vseq %v0
	vbrdu %v23,%s53
	adds.w.sx %s54, %s2, (0)1
	vsubs.w.sx %v0,%s54,%v0
	vldu %v24,4,%s6
	lea.sl %s54, .LCPI9_2@hi
	ldu %s54, .LCPI9_2@lo(,%s54)
	vfmk.w.gt %vm1,%v0
	lea.sl %s55, .LCPI9_1@hi
	ldu %s55, .LCPI9_1@lo(,%s55)
	vfmul.s %v0,%s54,%v24
	lea %s54, __vec_expf@lo
	and %s54, %s54, (32)0
	lea.sl %s54, __vec_expf@hi(%s54)
	vbrdu %v23,%s55,%vm1
	bsic %lr, (,%s54)
	lea.sl %s54, .LCPI9_3@hi
	ldu %s54, .LCPI9_3@lo(,%s54)
	vfadd.s %v0,%s53,%v0
	vfsub.s %v1,%s54,%v24
	lea.sl %s54, .LCPI9_4@hi
	ldu %s54, .LCPI9_4@lo(,%s54)
	vfdiv.s %v0,%s53,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s53,%vm1
	vfsub.s %v1,%s54,%v24
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s55,%vm1
	vfsub.s %v0,%v23,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s52
	lea.sl %s52, .LCPI9_5@hi
	ld %s53, .LCPI9_5@lo(,%s52)
	ldl.zx %s54, (,%s34)
	sll %s59, %s2, 2
	adds.l %s52, %s34, %s59
	ldl.zx %s55, (,%s52)
	vbrd %v0,%s53
	muls.l %s62, %s37, %s54
	muls.l %s63, %s37, %s55
	sll %s58, %s2, 3
	adds.l %s53, %s34, %s58
	ldl.zx %s0, (,%s53)
	muls.l %s60, 12, %s2
	adds.l %s54, %s34, %s60
	ldl.zx %s1, (,%s54)
	sll %s57, %s2, 4
	adds.l %s55, %s34, %s57
	ldl.zx %s3, (,%s55)
	muls.l %s61, 20, %s2
	adds.l %s56, %s34, %s61
	ldl.zx %s2, (,%s56)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s3, %s37, %s3
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v0,%s62,%v7
	pvfmad %v2,%v0,%s63,%v7
	pvfmad %v3,%v0,%s0,%v7
	pvfmad %v4,%v0,%s1,%v7
	ldl.zx %s62, 4(,%s34)
	lea %s63, 4(%s34, %s59)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v0,%s3,%v7
	pvfmad %v6,%v0,%s2,%v7
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	lea %s0, 4(%s34, %s58)
	ldl.zx %s0, (,%s0)
	lea %s1, 4(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s2, 4(%s34, %s57)
	ldl.zx %s2, (,%s2)
	lea %s3, 4(%s34, %s61)
	ldl.zx %s3, (,%s3)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	pvfmad %v1,%v1,%s62,%v8
	pvfmad %v2,%v2,%s63,%v8
	pvfmad %v3,%v3,%s0,%v8
	pvfmad %v4,%v4,%s1,%v8
	ldl.zx %s62, 8(,%s34)
	lea %s63, 8(%s34, %s59)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v5,%s2,%v8
	pvfmad %v6,%v6,%s3,%v8
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	lea %s0, 8(%s34, %s58)
	ldl.zx %s0, (,%s0)
	lea %s1, 8(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s2, 8(%s34, %s57)
	ldl.zx %s2, (,%s2)
	lea %s3, 8(%s34, %s61)
	ldl.zx %s3, (,%s3)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	pvfmad %v1,%v1,%s62,%v9
	pvfmad %v2,%v2,%s63,%v9
	pvfmad %v3,%v3,%s0,%v9
	pvfmad %v4,%v4,%s1,%v9
	ldl.zx %s62, 12(,%s34)
	lea %s63, 12(%s34, %s59)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v5,%s2,%v9
	pvfmad %v6,%v6,%s3,%v9
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	lea %s0, 12(%s34, %s58)
	ldl.zx %s0, (,%s0)
	lea %s1, 12(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s2, 12(%s34, %s57)
	ldl.zx %s2, (,%s2)
	lea %s3, 12(%s34, %s61)
	ldl.zx %s3, (,%s3)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	pvfmad %v1,%v1,%s62,%v10
	pvfmad %v2,%v2,%s63,%v10
	pvfmad %v3,%v3,%s0,%v10
	pvfmad %v4,%v4,%s1,%v10
	ldl.zx %s62, 16(,%s34)
	lea %s63, 16(%s34, %s59)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v5,%s2,%v10
	pvfmad %v6,%v6,%s3,%v10
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	lea %s0, 16(%s34, %s58)
	ldl.zx %s0, (,%s0)
	lea %s1, 16(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s2, 16(%s34, %s57)
	ldl.zx %s2, (,%s2)
	lea %s3, 16(%s34, %s61)
	ldl.zx %s3, (,%s3)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	pvfmad %v1,%v1,%s62,%v11
	pvfmad %v2,%v2,%s63,%v11
	pvfmad %v3,%v3,%s0,%v11
	pvfmad %v4,%v4,%s1,%v11
	ldl.zx %s62, 20(,%s34)
	lea %s63, 20(%s34, %s59)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v5,%s2,%v11
	pvfmad %v6,%v6,%s3,%v11
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	lea %s0, 20(%s34, %s58)
	ldl.zx %s0, (,%s0)
	lea %s1, 20(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s2, 20(%s34, %s57)
	ldl.zx %s2, (,%s2)
	lea %s3, 20(%s34, %s61)
	ldl.zx %s3, (,%s3)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	pvfmad %v1,%v1,%s62,%v12
	pvfmad %v2,%v2,%s63,%v12
	pvfmad %v3,%v3,%s0,%v12
	pvfmad %v4,%v4,%s1,%v12
	ldl.zx %s62, 24(,%s34)
	lea %s63, 24(%s34, %s59)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v5,%s2,%v12
	pvfmad %v6,%v6,%s3,%v12
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	lea %s0, 24(%s34, %s58)
	ldl.zx %s0, (,%s0)
	lea %s1, 24(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s2, 24(%s34, %s57)
	ldl.zx %s2, (,%s2)
	lea %s3, 24(%s34, %s61)
	ldl.zx %s3, (,%s3)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	pvfmad %v1,%v1,%s62,%v13
	pvfmad %v2,%v2,%s63,%v13
	pvfmad %v3,%v3,%s0,%v13
	pvfmad %v4,%v4,%s1,%v13
	ldl.zx %s62, 28(,%s34)
	lea %s63, 28(%s34, %s59)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v5,%s2,%v13
	pvfmad %v6,%v6,%s3,%v13
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	lea %s0, 28(%s34, %s58)
	ldl.zx %s0, (,%s0)
	lea %s1, 28(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s2, 28(%s34, %s57)
	ldl.zx %s2, (,%s2)
	lea %s3, 28(%s34, %s61)
	ldl.zx %s3, (,%s3)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	pvfmad %v1,%v1,%s62,%v14
	pvfmad %v2,%v2,%s63,%v14
	pvfmad %v3,%v3,%s0,%v14
	pvfmad %v4,%v4,%s1,%v14
	ldl.zx %s62, 32(,%s34)
	lea %s63, 32(%s34, %s59)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v5,%s2,%v14
	pvfmad %v6,%v6,%s3,%v14
	muls.l %s62, %s37, %s62
	muls.l %s63, %s37, %s63
	lea %s0, 32(%s34, %s58)
	ldl.zx %s0, (,%s0)
	lea %s1, 32(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s2, 32(%s34, %s57)
	ldl.zx %s2, (,%s2)
	lea %s3, 32(%s34, %s61)
	ldl.zx %s3, (,%s3)
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	pvfmad %v1,%v1,%s62,%v15
	pvfmad %v2,%v2,%s63,%v15
	pvfmad %v3,%v3,%s0,%v15
	pvfmad %v4,%v4,%s1,%v15
	ldl.zx %s62, 36(,%s34)
	lea %s59, 36(%s34, %s59)
	ldl.zx %s59, (,%s59)
	pvfmad %v5,%v5,%s2,%v15
	pvfmad %v6,%v6,%s3,%v15
	muls.l %s62, %s37, %s62
	muls.l %s59, %s37, %s59
	lea %s63, 36(%s34, %s58)
	ldl.zx %s63, (,%s63)
	lea %s60, 36(%s34, %s60)
	ldl.zx %s60, (,%s60)
	lea %s0, 36(%s34, %s57)
	ldl.zx %s0, (,%s0)
	lea %s61, 36(%s34, %s61)
	ldl.zx %s61, (,%s61)
	muls.l %s63, %s37, %s63
	muls.l %s60, %s37, %s60
	muls.l %s0, %s37, %s0
	muls.l %s61, %s37, %s61
	pvfmad %v1,%v1,%s62,%v16
	pvfmad %v2,%v2,%s59,%v16
	pvfmad %v3,%v3,%s63,%v16
	pvfmad %v4,%v4,%s60,%v16
	pvfmad %v5,%v5,%s0,%v16
	pvfmad %v6,%v6,%s61,%v16
	pvfadd %v1,%v17,%v1
	pvfadd %v2,%v18,%v2
	pvfadd %v3,%v19,%v3
	pvfadd %v4,%v20,%v4
	pvfadd %v5,%v21,%v5
	pvfadd %v6,%v22,%v6
	vst %v1,8,%s46
	vst %v2,8,%s47
	vst %v3,8,%s48
	vst %v4,8,%s49
	vst %v5,8,%s50
	vst %v6,8,%s51
	ldl.zx %s46, (,%s34)
	ldl.zx %s47, (,%s52)
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	ldl.zx %s48, (,%s53)
	ldl.zx %s49, (,%s54)
	ldl.zx %s50, (,%s55)
	ldl.zx %s51, (,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v1,%v0,%s46,%v17
	pvfmad %v1,%v1,%s47,%v18
	pvfmad %v1,%v1,%s48,%v19
	pvfmad %v1,%v1,%s49,%v20
	ldl.zx %s46, 4(,%s34)
	ldl.zx %s47, 4(,%s52)
	pvfmad %v1,%v1,%s50,%v21
	pvfmad %v1,%v1,%s51,%v22
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	or %s48, 4, %s58
	adds.l %s48, %s34, %s48
	ldl.zx %s48, (,%s48)
	ldl.zx %s49, 4(,%s54)
	or %s50, 4, %s57
	adds.l %s50, %s34, %s50
	ldl.zx %s50, (,%s50)
	ldl.zx %s51, 4(,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v2,%v0,%s46,%v17
	pvfmad %v2,%v2,%s47,%v18
	pvfmad %v2,%v2,%s48,%v19
	pvfmad %v2,%v2,%s49,%v20
	ldl.zx %s46, 8(,%s34)
	ldl.zx %s47, 8(,%s52)
	pvfmad %v2,%v2,%s50,%v21
	pvfmad %v2,%v2,%s51,%v22
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	ldl.zx %s48, 8(,%s53)
	ldl.zx %s49, 8(,%s54)
	or %s50, 8, %s57
	adds.l %s50, %s34, %s50
	ldl.zx %s50, (,%s50)
	ldl.zx %s51, 8(,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v3,%v0,%s46,%v17
	pvfmad %v3,%v3,%s47,%v18
	pvfmad %v3,%v3,%s48,%v19
	pvfmad %v3,%v3,%s49,%v20
	ldl.zx %s46, 12(,%s34)
	ldl.zx %s47, 12(,%s52)
	pvfmad %v3,%v3,%s50,%v21
	pvfmad %v3,%v3,%s51,%v22
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	ldl.zx %s48, 12(,%s53)
	ldl.zx %s49, 12(,%s54)
	or %s50, 12, %s57
	adds.l %s50, %s34, %s50
	ldl.zx %s50, (,%s50)
	ldl.zx %s51, 12(,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v4,%v0,%s46,%v17
	pvfmad %v4,%v4,%s47,%v18
	pvfmad %v4,%v4,%s48,%v19
	pvfmad %v4,%v4,%s49,%v20
	ldl.zx %s46, 16(,%s34)
	ldl.zx %s47, 16(,%s52)
	pvfmad %v4,%v4,%s50,%v21
	pvfmad %v4,%v4,%s51,%v22
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	ldl.zx %s48, 16(,%s53)
	ldl.zx %s49, 16(,%s54)
	ldl.zx %s50, 16(,%s55)
	ldl.zx %s51, 16(,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v5,%v0,%s46,%v17
	pvfmad %v5,%v5,%s47,%v18
	pvfmad %v5,%v5,%s48,%v19
	pvfmad %v5,%v5,%s49,%v20
	ldl.zx %s46, 20(,%s34)
	ldl.zx %s47, 20(,%s52)
	pvfmad %v5,%v5,%s50,%v21
	pvfmad %v5,%v5,%s51,%v22
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	ldl.zx %s48, 20(,%s53)
	ldl.zx %s49, 20(,%s54)
	ldl.zx %s50, 20(,%s55)
	ldl.zx %s51, 20(,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v6,%v0,%s46,%v17
	pvfmad %v6,%v6,%s47,%v18
	pvfmad %v6,%v6,%s48,%v19
	pvfmad %v6,%v6,%s49,%v20
	ldl.zx %s46, 24(,%s34)
	ldl.zx %s47, 24(,%s52)
	pvfmad %v6,%v6,%s50,%v21
	pvfmad %v6,%v6,%s51,%v22
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	ldl.zx %s48, 24(,%s53)
	ldl.zx %s49, 24(,%s54)
	ldl.zx %s50, 24(,%s55)
	ldl.zx %s51, 24(,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v23,%v0,%s46,%v17
	pvfmad %v23,%v23,%s47,%v18
	pvfmad %v23,%v23,%s48,%v19
	pvfmad %v23,%v23,%s49,%v20
	ldl.zx %s46, 28(,%s34)
	ldl.zx %s47, 28(,%s52)
	pvfmad %v23,%v23,%s50,%v21
	pvfmad %v23,%v23,%s51,%v22
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	ldl.zx %s48, 28(,%s53)
	ldl.zx %s49, 28(,%s54)
	ldl.zx %s50, 28(,%s55)
	ldl.zx %s51, 28(,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v24,%v0,%s46,%v17
	pvfmad %v24,%v24,%s47,%v18
	pvfmad %v24,%v24,%s48,%v19
	pvfmad %v24,%v24,%s49,%v20
	ldl.zx %s46, 32(,%s34)
	ldl.zx %s47, 32(,%s52)
	pvfmad %v24,%v24,%s50,%v21
	pvfmad %v24,%v24,%s51,%v22
	muls.l %s46, %s37, %s46
	muls.l %s47, %s37, %s47
	ldl.zx %s48, 32(,%s53)
	ldl.zx %s49, 32(,%s54)
	ldl.zx %s50, 32(,%s55)
	ldl.zx %s51, 32(,%s56)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	pvfmad %v25,%v0,%s46,%v17
	pvfmad %v25,%v25,%s47,%v18
	pvfmad %v25,%v25,%s48,%v19
	pvfmad %v25,%v25,%s49,%v20
	ldl.zx %s34, 36(,%s34)
	ldl.zx %s46, 36(,%s52)
	pvfmad %v25,%v25,%s50,%v21
	pvfmad %v25,%v25,%s51,%v22
	muls.l %s34, %s37, %s34
	muls.l %s46, %s37, %s46
	ldl.zx %s47, 36(,%s53)
	ldl.zx %s48, 36(,%s54)
	ldl.zx %s49, 36(,%s55)
	ldl.zx %s50, 36(,%s56)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s37, %s37, %s50
	pvfmad %v0,%v0,%s34,%v17
	pvfmad %v0,%v0,%s46,%v18
	pvfmad %v0,%v0,%s47,%v19
	pvfmad %v0,%v0,%s48,%v20
	pvfmad %v0,%v0,%s49,%v21
	pvfmad %v0,%v0,%s37,%v22
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v23
	pvfadd %v8,%v14,%v24
	pvfadd %v9,%v15,%v25
	pvfadd %v0,%v16,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s38
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v8,8,%s43
	vst %v9,8,%s44
	vst %v0,8,%s45
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end9:
	.size	w2v_kernel_N6_HU512_2X_W10, .Lfunc_end9-w2v_kernel_N6_HU512_2X_W10

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI10_0:
	.4byte	1065353216
.LCPI10_1:
	.4byte	0
.LCPI10_2:
	.4byte	3212836864
.LCPI10_3:
	.4byte	1086324736
.LCPI10_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI10_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W11
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W11,@function
w2v_kernel_N6_HU512_2X_W11:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB10_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB10_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s37, -8(,%s9)
	adds.w.sx %s53, %s34, (0)1
	lvl %s53
	ldl.sx %s34, (,%s4)
	ld %s46, 240(,%s9)
	muls.l %s34, %s34, %s1
	ldl.sx %s36, 4(,%s4)
	sll %s34, %s34, 2
	adds.l %s35, %s46, %s34
	ldl.sx %s34, 8(,%s4)
	muls.l %s36, %s36, %s1
	sll %s36, %s36, 2
	adds.l %s36, %s46, %s36
	muls.l %s34, %s34, %s1
	ldl.sx %s39, 12(,%s4)
	sll %s34, %s34, 2
	adds.l %s38, %s46, %s34
	ldl.sx %s34, 16(,%s4)
	muls.l %s39, %s39, %s1
	sll %s39, %s39, 2
	adds.l %s39, %s46, %s39
	muls.l %s34, %s34, %s1
	ldl.sx %s41, 20(,%s4)
	sll %s34, %s34, 2
	adds.l %s40, %s46, %s34
	ldl.sx %s34, 24(,%s4)
	muls.l %s41, %s41, %s1
	sll %s41, %s41, 2
	adds.l %s41, %s46, %s41
	muls.l %s34, %s34, %s1
	ldl.sx %s43, 28(,%s4)
	sll %s34, %s34, 2
	adds.l %s42, %s46, %s34
	ldl.sx %s34, 32(,%s4)
	muls.l %s43, %s43, %s1
	sll %s43, %s43, 2
	adds.l %s43, %s46, %s43
	muls.l %s34, %s34, %s1
	ldl.sx %s45, 36(,%s4)
	sll %s34, %s34, 2
	adds.l %s44, %s46, %s34
	ldl.sx %s34, 40(,%s4)
	muls.l %s45, %s45, %s1
	sll %s45, %s45, 2
	adds.l %s45, %s46, %s45
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s46, %s46, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	ldl.sx %s34, (,%s5)
	ldl.sx %s48, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s47, %s7, %s34
	muls.l %s34, %s48, %s1
	ldl.sx %s49, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s48, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s49, %s49, %s1
	sll %s49, %s49, 2
	adds.l %s49, %s7, %s49
	muls.l %s34, %s34, %s1
	ldl.sx %s51, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s50, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s51, %s51, %s1
	sll %s51, %s51, 2
	adds.l %s51, %s7, %s51
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s52, %s7, %s34
	vld %v18,8,%s47
	vld %v19,8,%s48
	vld %v20,8,%s49
	vld %v21,8,%s50
	vld %v22,8,%s51
	vld %v23,8,%s52
	lvl %s53
	pvfmul %v0,%v18,%v7
	pvfmul %v1,%v18,%v8
	pvfmul %v2,%v18,%v9
	pvfmul %v3,%v18,%v10
	pvfmul %v4,%v18,%v11
	pvfmul %v5,%v18,%v12
	pvfmul %v6,%v18,%v13
	pvfmul %v24,%v18,%v14
	pvfmul %v25,%v18,%v15
	pvfmul %v26,%v18,%v16
	pvfmul %v27,%v18,%v17
	vsll %v28,%v0,32
	vsll %v29,%v1,32
	vsll %v30,%v2,32
	vsll %v31,%v3,32
	vsll %v32,%v4,32
	vsll %v33,%v5,32
	vsll %v34,%v6,32
	vsll %v35,%v24,32
	vsll %v36,%v25,32
	vsll %v37,%v26,32
	vsll %v38,%v27,32
	vfadd.s %v0,%v0,%v28
	vfadd.s %v1,%v1,%v29
	vfadd.s %v2,%v2,%v30
	vfadd.s %v3,%v3,%v31
	vfadd.s %v4,%v4,%v32
	vfadd.s %v5,%v5,%v33
	vfadd.s %v6,%v6,%v34
	vfadd.s %v24,%v24,%v35
	vfadd.s %v25,%v25,%v36
	vfadd.s %v26,%v26,%v37
	vfadd.s %v27,%v27,%v38
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	or %s54, 1, (0)1
	lvl %s54
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s55, 4(%s6)
	vstu %v1,4,%s55
	lea %s55, 8(%s6)
	vstu %v2,4,%s55
	lea %s55, 12(%s6)
	vstu %v3,4,%s55
	lea %s55, 16(%s6)
	vstu %v4,4,%s55
	lea %s55, 20(%s6)
	vstu %v5,4,%s55
	lea %s55, 24(%s6)
	vstu %v6,4,%s55
	lea %s55, 28(%s6)
	vstu %v24,4,%s55
	lea %s55, 32(%s6)
	vstu %v25,4,%s55
	lea %s55, 36(%s6)
	vstu %v26,4,%s55
	lea %s55, 40(%s6)
	vstu %v27,4,%s55
	lvl %s53
	pvfmul %v0,%v19,%v7
	pvfmul %v1,%v19,%v8
	pvfmul %v2,%v19,%v9
	pvfmul %v3,%v19,%v10
	pvfmul %v4,%v19,%v11
	pvfmul %v5,%v19,%v12
	pvfmul %v6,%v19,%v13
	pvfmul %v24,%v19,%v14
	pvfmul %v25,%v19,%v15
	pvfmul %v26,%v19,%v16
	pvfmul %v27,%v19,%v17
	vsll %v28,%v0,32
	vsll %v29,%v1,32
	vsll %v30,%v2,32
	vsll %v31,%v3,32
	vsll %v32,%v4,32
	vsll %v33,%v5,32
	vsll %v34,%v6,32
	vsll %v35,%v24,32
	vsll %v36,%v25,32
	vsll %v37,%v26,32
	vsll %v38,%v27,32
	vfadd.s %v0,%v0,%v28
	vfadd.s %v1,%v1,%v29
	vfadd.s %v2,%v2,%v30
	vfadd.s %v3,%v3,%v31
	vfadd.s %v4,%v4,%v32
	vfadd.s %v5,%v5,%v33
	vfadd.s %v6,%v6,%v34
	vfadd.s %v24,%v24,%v35
	vfadd.s %v25,%v25,%v36
	vfadd.s %v26,%v26,%v37
	vfadd.s %v27,%v27,%v38
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	lvl %s54
	lea %s55, 44(%s6)
	vstu %v0,4,%s55
	lea %s55, 48(%s6)
	vstu %v1,4,%s55
	lea %s55, 52(%s6)
	vstu %v2,4,%s55
	lea %s55, 56(%s6)
	vstu %v3,4,%s55
	lea %s55, 60(%s6)
	vstu %v4,4,%s55
	lea %s55, 64(%s6)
	vstu %v5,4,%s55
	lea %s55, 68(%s6)
	vstu %v6,4,%s55
	lea %s55, 72(%s6)
	vstu %v24,4,%s55
	lea %s55, 76(%s6)
	vstu %v25,4,%s55
	lea %s55, 80(%s6)
	vstu %v26,4,%s55
	lea %s55, 84(%s6)
	vstu %v27,4,%s55
	lvl %s53
	pvfmul %v0,%v20,%v7
	pvfmul %v1,%v20,%v8
	pvfmul %v2,%v20,%v9
	pvfmul %v3,%v20,%v10
	pvfmul %v4,%v20,%v11
	pvfmul %v5,%v20,%v12
	pvfmul %v6,%v20,%v13
	pvfmul %v24,%v20,%v14
	pvfmul %v25,%v20,%v15
	pvfmul %v26,%v20,%v16
	pvfmul %v27,%v20,%v17
	vsll %v28,%v0,32
	vsll %v29,%v1,32
	vsll %v30,%v2,32
	vsll %v31,%v3,32
	vsll %v32,%v4,32
	vsll %v33,%v5,32
	vsll %v34,%v6,32
	vsll %v35,%v24,32
	vsll %v36,%v25,32
	vsll %v37,%v26,32
	vsll %v38,%v27,32
	vfadd.s %v0,%v0,%v28
	vfadd.s %v1,%v1,%v29
	vfadd.s %v2,%v2,%v30
	vfadd.s %v3,%v3,%v31
	vfadd.s %v4,%v4,%v32
	vfadd.s %v5,%v5,%v33
	vfadd.s %v6,%v6,%v34
	vfadd.s %v24,%v24,%v35
	vfadd.s %v25,%v25,%v36
	vfadd.s %v26,%v26,%v37
	vfadd.s %v27,%v27,%v38
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	lvl %s54
	lea %s55, 88(%s6)
	vstu %v0,4,%s55
	lea %s55, 92(%s6)
	vstu %v1,4,%s55
	lea %s55, 96(%s6)
	vstu %v2,4,%s55
	lea %s55, 100(%s6)
	vstu %v3,4,%s55
	lea %s55, 104(%s6)
	vstu %v4,4,%s55
	lea %s55, 108(%s6)
	vstu %v5,4,%s55
	lea %s55, 112(%s6)
	vstu %v6,4,%s55
	lea %s55, 116(%s6)
	vstu %v24,4,%s55
	lea %s55, 120(%s6)
	vstu %v25,4,%s55
	lea %s55, 124(%s6)
	vstu %v26,4,%s55
	lea %s55, 128(%s6)
	vstu %v27,4,%s55
	lvl %s53
	pvfmul %v0,%v21,%v7
	pvfmul %v1,%v21,%v8
	pvfmul %v2,%v21,%v9
	pvfmul %v3,%v21,%v10
	pvfmul %v4,%v21,%v11
	pvfmul %v5,%v21,%v12
	pvfmul %v6,%v21,%v13
	pvfmul %v24,%v21,%v14
	pvfmul %v25,%v21,%v15
	pvfmul %v26,%v21,%v16
	pvfmul %v27,%v21,%v17
	vsll %v28,%v0,32
	vsll %v29,%v1,32
	vsll %v30,%v2,32
	vsll %v31,%v3,32
	vsll %v32,%v4,32
	vsll %v33,%v5,32
	vsll %v34,%v6,32
	vsll %v35,%v24,32
	vsll %v36,%v25,32
	vsll %v37,%v26,32
	vsll %v38,%v27,32
	vfadd.s %v0,%v0,%v28
	vfadd.s %v1,%v1,%v29
	vfadd.s %v2,%v2,%v30
	vfadd.s %v3,%v3,%v31
	vfadd.s %v4,%v4,%v32
	vfadd.s %v5,%v5,%v33
	vfadd.s %v6,%v6,%v34
	vfadd.s %v24,%v24,%v35
	vfadd.s %v25,%v25,%v36
	vfadd.s %v26,%v26,%v37
	vfadd.s %v27,%v27,%v38
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	lvl %s54
	lea %s55, 132(%s6)
	vstu %v0,4,%s55
	lea %s55, 136(%s6)
	vstu %v1,4,%s55
	lea %s55, 140(%s6)
	vstu %v2,4,%s55
	lea %s55, 144(%s6)
	vstu %v3,4,%s55
	lea %s55, 148(%s6)
	vstu %v4,4,%s55
	lea %s55, 152(%s6)
	vstu %v5,4,%s55
	lea %s55, 156(%s6)
	vstu %v6,4,%s55
	lea %s55, 160(%s6)
	vstu %v24,4,%s55
	lea %s55, 164(%s6)
	vstu %v25,4,%s55
	lea %s55, 168(%s6)
	vstu %v26,4,%s55
	lea %s55, 172(%s6)
	vstu %v27,4,%s55
	lvl %s53
	pvfmul %v0,%v22,%v7
	pvfmul %v1,%v22,%v8
	pvfmul %v2,%v22,%v9
	pvfmul %v3,%v22,%v10
	pvfmul %v4,%v22,%v11
	pvfmul %v5,%v22,%v12
	pvfmul %v6,%v22,%v13
	pvfmul %v24,%v22,%v14
	pvfmul %v25,%v22,%v15
	pvfmul %v26,%v22,%v16
	pvfmul %v27,%v22,%v17
	vsll %v28,%v0,32
	vsll %v29,%v1,32
	vsll %v30,%v2,32
	vsll %v31,%v3,32
	vsll %v32,%v4,32
	vsll %v33,%v5,32
	vsll %v34,%v6,32
	vsll %v35,%v24,32
	vsll %v36,%v25,32
	vsll %v37,%v26,32
	vsll %v38,%v27,32
	vfadd.s %v0,%v0,%v28
	vfadd.s %v1,%v1,%v29
	vfadd.s %v2,%v2,%v30
	vfadd.s %v3,%v3,%v31
	vfadd.s %v4,%v4,%v32
	vfadd.s %v5,%v5,%v33
	vfadd.s %v6,%v6,%v34
	vfadd.s %v24,%v24,%v35
	vfadd.s %v25,%v25,%v36
	vfadd.s %v26,%v26,%v37
	vfadd.s %v27,%v27,%v38
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	lvl %s54
	lea %s55, 176(%s6)
	vstu %v0,4,%s55
	lea %s55, 180(%s6)
	vstu %v1,4,%s55
	lea %s55, 184(%s6)
	vstu %v2,4,%s55
	lea %s55, 188(%s6)
	vstu %v3,4,%s55
	lea %s55, 192(%s6)
	vstu %v4,4,%s55
	lea %s55, 196(%s6)
	vstu %v5,4,%s55
	lea %s55, 200(%s6)
	vstu %v6,4,%s55
	lea %s55, 204(%s6)
	vstu %v24,4,%s55
	lea %s55, 208(%s6)
	vstu %v25,4,%s55
	lea %s55, 212(%s6)
	vstu %v26,4,%s55
	lea %s55, 216(%s6)
	vstu %v27,4,%s55
	lvl %s53
	pvfmul %v0,%v23,%v7
	pvfmul %v1,%v23,%v8
	pvfmul %v2,%v23,%v9
	pvfmul %v3,%v23,%v10
	pvfmul %v4,%v23,%v11
	pvfmul %v5,%v23,%v12
	pvfmul %v6,%v23,%v13
	pvfmul %v24,%v23,%v14
	pvfmul %v25,%v23,%v15
	pvfmul %v26,%v23,%v16
	pvfmul %v27,%v23,%v17
	vsll %v28,%v0,32
	vsll %v29,%v1,32
	vsll %v30,%v2,32
	vsll %v31,%v3,32
	vsll %v32,%v4,32
	vsll %v33,%v5,32
	vsll %v34,%v6,32
	vsll %v35,%v24,32
	vsll %v36,%v25,32
	vsll %v37,%v26,32
	vsll %v38,%v27,32
	vfadd.s %v0,%v0,%v28
	vfadd.s %v1,%v1,%v29
	vfadd.s %v2,%v2,%v30
	vfadd.s %v3,%v3,%v31
	vfadd.s %v4,%v4,%v32
	vfadd.s %v5,%v5,%v33
	vfadd.s %v6,%v6,%v34
	vfadd.s %v24,%v24,%v35
	vfadd.s %v25,%v25,%v36
	vfadd.s %v26,%v26,%v37
	vfadd.s %v27,%v27,%v38
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v24,%v24
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	lvl %s54
	lea %s54, 220(%s6)
	vstu %v0,4,%s54
	lea %s54, 224(%s6)
	vstu %v1,4,%s54
	lea %s54, 228(%s6)
	vstu %v2,4,%s54
	lea %s54, 232(%s6)
	vstu %v3,4,%s54
	lea %s54, 236(%s6)
	vstu %v4,4,%s54
	lea %s54, 240(%s6)
	vstu %v5,4,%s54
	lea %s54, 244(%s6)
	vstu %v6,4,%s54
	lea %s54, 248(%s6)
	vstu %v24,4,%s54
	lea %s54, 252(%s6)
	vstu %v25,4,%s54
	lea %s54, 256(%s6)
	vstu %v26,4,%s54
	lea %s54, 260(%s6)
	vstu %v27,4,%s54
	muls.l %s54, %s2, %s0
	adds.w.sx %s54, %s54, (0)1
	lvl %s54
	lea.sl %s54, .LCPI10_0@hi
	ldu %s54, .LCPI10_0@lo(,%s54)
	vseq %v0
	vbrdu %v24,%s54
	adds.w.sx %s55, %s2, (0)1
	vsubs.w.sx %v0,%s55,%v0
	vldu %v25,4,%s6
	lea.sl %s55, .LCPI10_2@hi
	ldu %s55, .LCPI10_2@lo(,%s55)
	vfmk.w.gt %vm1,%v0
	lea.sl %s56, .LCPI10_1@hi
	ldu %s56, .LCPI10_1@lo(,%s56)
	vfmul.s %v0,%s55,%v25
	lea %s55, __vec_expf@lo
	and %s55, %s55, (32)0
	lea.sl %s55, __vec_expf@hi(%s55)
	vbrdu %v24,%s56,%vm1
	bsic %lr, (,%s55)
	lea.sl %s55, .LCPI10_3@hi
	ldu %s55, .LCPI10_3@lo(,%s55)
	vfadd.s %v0,%s54,%v0
	vfsub.s %v1,%s55,%v25
	lea.sl %s55, .LCPI10_4@hi
	ldu %s55, .LCPI10_4@lo(,%s55)
	vfdiv.s %v0,%s54,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s54,%vm1
	vfsub.s %v1,%s55,%v25
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s56,%vm1
	vfsub.s %v0,%v24,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s53
	lea.sl %s53, .LCPI10_5@hi
	ld %s54, .LCPI10_5@lo(,%s53)
	ldl.zx %s55, (,%s34)
	sll %s60, %s2, 2
	adds.l %s53, %s34, %s60
	ldl.zx %s56, (,%s53)
	vbrd %v0,%s54
	muls.l %s63, %s37, %s55
	muls.l %s0, %s37, %s56
	sll %s59, %s2, 3
	adds.l %s54, %s34, %s59
	ldl.zx %s1, (,%s54)
	muls.l %s61, 12, %s2
	adds.l %s55, %s34, %s61
	ldl.zx %s3, (,%s55)
	sll %s58, %s2, 4
	adds.l %s56, %s34, %s58
	ldl.zx %s4, (,%s56)
	muls.l %s62, 20, %s2
	adds.l %s57, %s34, %s62
	ldl.zx %s2, (,%s57)
	muls.l %s1, %s37, %s1
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v0,%s63,%v7
	pvfmad %v2,%v0,%s0,%v7
	pvfmad %v3,%v0,%s1,%v7
	pvfmad %v4,%v0,%s3,%v7
	ldl.zx %s63, 4(,%s34)
	lea %s0, 4(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v0,%s4,%v7
	pvfmad %v6,%v0,%s2,%v7
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 4(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 4(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 4(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 4(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v8
	pvfmad %v2,%v2,%s0,%v8
	pvfmad %v3,%v3,%s1,%v8
	pvfmad %v4,%v4,%s2,%v8
	ldl.zx %s63, 8(,%s34)
	lea %s0, 8(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s3,%v8
	pvfmad %v6,%v6,%s4,%v8
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 8(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 8(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 8(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 8(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v9
	pvfmad %v2,%v2,%s0,%v9
	pvfmad %v3,%v3,%s1,%v9
	pvfmad %v4,%v4,%s2,%v9
	ldl.zx %s63, 12(,%s34)
	lea %s0, 12(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s3,%v9
	pvfmad %v6,%v6,%s4,%v9
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 12(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 12(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 12(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 12(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v10
	pvfmad %v2,%v2,%s0,%v10
	pvfmad %v3,%v3,%s1,%v10
	pvfmad %v4,%v4,%s2,%v10
	ldl.zx %s63, 16(,%s34)
	lea %s0, 16(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s3,%v10
	pvfmad %v6,%v6,%s4,%v10
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 16(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 16(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 16(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 16(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v11
	pvfmad %v2,%v2,%s0,%v11
	pvfmad %v3,%v3,%s1,%v11
	pvfmad %v4,%v4,%s2,%v11
	ldl.zx %s63, 20(,%s34)
	lea %s0, 20(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s3,%v11
	pvfmad %v6,%v6,%s4,%v11
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 20(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 20(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 20(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 20(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v12
	pvfmad %v2,%v2,%s0,%v12
	pvfmad %v3,%v3,%s1,%v12
	pvfmad %v4,%v4,%s2,%v12
	ldl.zx %s63, 24(,%s34)
	lea %s0, 24(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s3,%v12
	pvfmad %v6,%v6,%s4,%v12
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 24(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 24(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 24(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 24(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v13
	pvfmad %v2,%v2,%s0,%v13
	pvfmad %v3,%v3,%s1,%v13
	pvfmad %v4,%v4,%s2,%v13
	ldl.zx %s63, 28(,%s34)
	lea %s0, 28(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s3,%v13
	pvfmad %v6,%v6,%s4,%v13
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 28(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 28(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 28(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 28(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v14
	pvfmad %v2,%v2,%s0,%v14
	pvfmad %v3,%v3,%s1,%v14
	pvfmad %v4,%v4,%s2,%v14
	ldl.zx %s63, 32(,%s34)
	lea %s0, 32(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s3,%v14
	pvfmad %v6,%v6,%s4,%v14
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 32(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 32(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 32(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 32(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v15
	pvfmad %v2,%v2,%s0,%v15
	pvfmad %v3,%v3,%s1,%v15
	pvfmad %v4,%v4,%s2,%v15
	ldl.zx %s63, 36(,%s34)
	lea %s0, 36(%s34, %s60)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s3,%v15
	pvfmad %v6,%v6,%s4,%v15
	muls.l %s63, %s37, %s63
	muls.l %s0, %s37, %s0
	lea %s1, 36(%s34, %s59)
	ldl.zx %s1, (,%s1)
	lea %s2, 36(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s3, 36(%s34, %s58)
	ldl.zx %s3, (,%s3)
	lea %s4, 36(%s34, %s62)
	ldl.zx %s4, (,%s4)
	muls.l %s1, %s37, %s1
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	pvfmad %v1,%v1,%s63,%v16
	pvfmad %v2,%v2,%s0,%v16
	pvfmad %v3,%v3,%s1,%v16
	pvfmad %v4,%v4,%s2,%v16
	ldl.zx %s63, 40(,%s34)
	lea %s60, 40(%s34, %s60)
	ldl.zx %s60, (,%s60)
	pvfmad %v5,%v5,%s3,%v16
	pvfmad %v6,%v6,%s4,%v16
	muls.l %s63, %s37, %s63
	muls.l %s60, %s37, %s60
	lea %s0, 40(%s34, %s59)
	ldl.zx %s0, (,%s0)
	lea %s61, 40(%s34, %s61)
	ldl.zx %s61, (,%s61)
	lea %s1, 40(%s34, %s58)
	ldl.zx %s1, (,%s1)
	lea %s62, 40(%s34, %s62)
	ldl.zx %s62, (,%s62)
	muls.l %s0, %s37, %s0
	muls.l %s61, %s37, %s61
	muls.l %s1, %s37, %s1
	muls.l %s62, %s37, %s62
	pvfmad %v1,%v1,%s63,%v17
	pvfmad %v2,%v2,%s60,%v17
	pvfmad %v3,%v3,%s0,%v17
	pvfmad %v4,%v4,%s61,%v17
	pvfmad %v5,%v5,%s1,%v17
	pvfmad %v6,%v6,%s62,%v17
	pvfadd %v1,%v18,%v1
	pvfadd %v2,%v19,%v2
	pvfadd %v3,%v20,%v3
	pvfadd %v4,%v21,%v4
	pvfadd %v5,%v22,%v5
	pvfadd %v6,%v23,%v6
	vst %v1,8,%s47
	vst %v2,8,%s48
	vst %v3,8,%s49
	vst %v4,8,%s50
	vst %v5,8,%s51
	vst %v6,8,%s52
	ldl.zx %s47, (,%s34)
	ldl.zx %s48, (,%s53)
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, (,%s54)
	ldl.zx %s50, (,%s55)
	ldl.zx %s51, (,%s56)
	ldl.zx %s52, (,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v1,%v0,%s47,%v18
	pvfmad %v1,%v1,%s48,%v19
	pvfmad %v1,%v1,%s49,%v20
	pvfmad %v1,%v1,%s50,%v21
	ldl.zx %s47, 4(,%s34)
	ldl.zx %s48, 4(,%s53)
	pvfmad %v1,%v1,%s51,%v22
	pvfmad %v1,%v1,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	or %s49, 4, %s59
	adds.l %s49, %s34, %s49
	ldl.zx %s49, (,%s49)
	ldl.zx %s50, 4(,%s55)
	or %s51, 4, %s58
	adds.l %s51, %s34, %s51
	ldl.zx %s51, (,%s51)
	ldl.zx %s52, 4(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v2,%v0,%s47,%v18
	pvfmad %v2,%v2,%s48,%v19
	pvfmad %v2,%v2,%s49,%v20
	pvfmad %v2,%v2,%s50,%v21
	ldl.zx %s47, 8(,%s34)
	ldl.zx %s48, 8(,%s53)
	pvfmad %v2,%v2,%s51,%v22
	pvfmad %v2,%v2,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 8(,%s54)
	ldl.zx %s50, 8(,%s55)
	or %s51, 8, %s58
	adds.l %s51, %s34, %s51
	ldl.zx %s51, (,%s51)
	ldl.zx %s52, 8(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v3,%v0,%s47,%v18
	pvfmad %v3,%v3,%s48,%v19
	pvfmad %v3,%v3,%s49,%v20
	pvfmad %v3,%v3,%s50,%v21
	ldl.zx %s47, 12(,%s34)
	ldl.zx %s48, 12(,%s53)
	pvfmad %v3,%v3,%s51,%v22
	pvfmad %v3,%v3,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 12(,%s54)
	ldl.zx %s50, 12(,%s55)
	or %s51, 12, %s58
	adds.l %s51, %s34, %s51
	ldl.zx %s51, (,%s51)
	ldl.zx %s52, 12(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v4,%v0,%s47,%v18
	pvfmad %v4,%v4,%s48,%v19
	pvfmad %v4,%v4,%s49,%v20
	pvfmad %v4,%v4,%s50,%v21
	ldl.zx %s47, 16(,%s34)
	ldl.zx %s48, 16(,%s53)
	pvfmad %v4,%v4,%s51,%v22
	pvfmad %v4,%v4,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 16(,%s54)
	ldl.zx %s50, 16(,%s55)
	ldl.zx %s51, 16(,%s56)
	ldl.zx %s52, 16(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v5,%v0,%s47,%v18
	pvfmad %v5,%v5,%s48,%v19
	pvfmad %v5,%v5,%s49,%v20
	pvfmad %v5,%v5,%s50,%v21
	ldl.zx %s47, 20(,%s34)
	ldl.zx %s48, 20(,%s53)
	pvfmad %v5,%v5,%s51,%v22
	pvfmad %v5,%v5,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 20(,%s54)
	ldl.zx %s50, 20(,%s55)
	ldl.zx %s51, 20(,%s56)
	ldl.zx %s52, 20(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v6,%v0,%s47,%v18
	pvfmad %v6,%v6,%s48,%v19
	pvfmad %v6,%v6,%s49,%v20
	pvfmad %v6,%v6,%s50,%v21
	ldl.zx %s47, 24(,%s34)
	ldl.zx %s48, 24(,%s53)
	pvfmad %v6,%v6,%s51,%v22
	pvfmad %v6,%v6,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 24(,%s54)
	ldl.zx %s50, 24(,%s55)
	ldl.zx %s51, 24(,%s56)
	ldl.zx %s52, 24(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v24,%v0,%s47,%v18
	pvfmad %v24,%v24,%s48,%v19
	pvfmad %v24,%v24,%s49,%v20
	pvfmad %v24,%v24,%s50,%v21
	ldl.zx %s47, 28(,%s34)
	ldl.zx %s48, 28(,%s53)
	pvfmad %v24,%v24,%s51,%v22
	pvfmad %v24,%v24,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 28(,%s54)
	ldl.zx %s50, 28(,%s55)
	ldl.zx %s51, 28(,%s56)
	ldl.zx %s52, 28(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v25,%v0,%s47,%v18
	pvfmad %v25,%v25,%s48,%v19
	pvfmad %v25,%v25,%s49,%v20
	pvfmad %v25,%v25,%s50,%v21
	ldl.zx %s47, 32(,%s34)
	ldl.zx %s48, 32(,%s53)
	pvfmad %v25,%v25,%s51,%v22
	pvfmad %v25,%v25,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 32(,%s54)
	ldl.zx %s50, 32(,%s55)
	ldl.zx %s51, 32(,%s56)
	ldl.zx %s52, 32(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v26,%v0,%s47,%v18
	pvfmad %v26,%v26,%s48,%v19
	pvfmad %v26,%v26,%s49,%v20
	pvfmad %v26,%v26,%s50,%v21
	ldl.zx %s47, 36(,%s34)
	ldl.zx %s48, 36(,%s53)
	pvfmad %v26,%v26,%s51,%v22
	pvfmad %v26,%v26,%s52,%v23
	muls.l %s47, %s37, %s47
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 36(,%s54)
	ldl.zx %s50, 36(,%s55)
	ldl.zx %s51, 36(,%s56)
	ldl.zx %s52, 36(,%s57)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	pvfmad %v27,%v0,%s47,%v18
	pvfmad %v27,%v27,%s48,%v19
	pvfmad %v27,%v27,%s49,%v20
	pvfmad %v27,%v27,%s50,%v21
	ldl.zx %s34, 40(,%s34)
	ldl.zx %s47, 40(,%s53)
	pvfmad %v27,%v27,%s51,%v22
	pvfmad %v27,%v27,%s52,%v23
	muls.l %s34, %s37, %s34
	muls.l %s47, %s37, %s47
	ldl.zx %s48, 40(,%s54)
	ldl.zx %s49, 40(,%s55)
	ldl.zx %s50, 40(,%s56)
	ldl.zx %s51, 40(,%s57)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s37, %s37, %s51
	pvfmad %v0,%v0,%s34,%v18
	pvfmad %v0,%v0,%s47,%v19
	pvfmad %v0,%v0,%s48,%v20
	pvfmad %v0,%v0,%s49,%v21
	pvfmad %v0,%v0,%s50,%v22
	pvfmad %v0,%v0,%s37,%v23
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v24
	pvfadd %v8,%v14,%v25
	pvfadd %v9,%v15,%v26
	pvfadd %v10,%v16,%v27
	pvfadd %v0,%v17,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s38
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v8,8,%s43
	vst %v9,8,%s44
	vst %v10,8,%s45
	vst %v0,8,%s46
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end10:
	.size	w2v_kernel_N6_HU512_2X_W11, .Lfunc_end10-w2v_kernel_N6_HU512_2X_W11

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI11_0:
	.4byte	1065353216
.LCPI11_1:
	.4byte	0
.LCPI11_2:
	.4byte	3212836864
.LCPI11_3:
	.4byte	1086324736
.LCPI11_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI11_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W12
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W12,@function
w2v_kernel_N6_HU512_2X_W12:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB11_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB11_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s37, -8(,%s9)
	adds.w.sx %s54, %s34, (0)1
	lvl %s54
	ldl.sx %s34, (,%s4)
	ld %s47, 240(,%s9)
	ldl.sx %s36, 4(,%s4)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s35, %s47, %s34
	muls.l %s34, %s36, %s1
	ldl.sx %s38, 8(,%s4)
	sll %s34, %s34, 2
	adds.l %s36, %s47, %s34
	ldl.sx %s34, 12(,%s4)
	muls.l %s38, %s38, %s1
	sll %s38, %s38, 2
	adds.l %s38, %s47, %s38
	muls.l %s34, %s34, %s1
	ldl.sx %s40, 16(,%s4)
	sll %s34, %s34, 2
	adds.l %s39, %s47, %s34
	ldl.sx %s34, 20(,%s4)
	muls.l %s40, %s40, %s1
	sll %s40, %s40, 2
	adds.l %s40, %s47, %s40
	muls.l %s34, %s34, %s1
	ldl.sx %s42, 24(,%s4)
	sll %s34, %s34, 2
	adds.l %s41, %s47, %s34
	ldl.sx %s34, 28(,%s4)
	muls.l %s42, %s42, %s1
	sll %s42, %s42, 2
	adds.l %s42, %s47, %s42
	muls.l %s34, %s34, %s1
	ldl.sx %s44, 32(,%s4)
	sll %s34, %s34, 2
	adds.l %s43, %s47, %s34
	ldl.sx %s34, 36(,%s4)
	muls.l %s44, %s44, %s1
	sll %s44, %s44, 2
	adds.l %s44, %s47, %s44
	muls.l %s34, %s34, %s1
	ldl.sx %s46, 40(,%s4)
	sll %s34, %s34, 2
	adds.l %s45, %s47, %s34
	ldl.sx %s34, 44(,%s4)
	muls.l %s46, %s46, %s1
	sll %s46, %s46, 2
	adds.l %s46, %s47, %s46
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s47, %s47, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s38
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	ldl.sx %s34, (,%s5)
	ldl.sx %s49, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s48, %s7, %s34
	muls.l %s34, %s49, %s1
	ldl.sx %s50, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s49, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s50, %s50, %s1
	sll %s50, %s50, 2
	adds.l %s50, %s7, %s50
	muls.l %s34, %s34, %s1
	ldl.sx %s52, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s51, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s52, %s52, %s1
	sll %s52, %s52, 2
	adds.l %s52, %s7, %s52
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s53, %s7, %s34
	vld %v19,8,%s48
	vld %v20,8,%s49
	vld %v21,8,%s50
	vld %v22,8,%s51
	vld %v23,8,%s52
	vld %v24,8,%s53
	lvl %s54
	pvfmul %v0,%v19,%v7
	pvfmul %v1,%v19,%v8
	pvfmul %v2,%v19,%v9
	pvfmul %v3,%v19,%v10
	pvfmul %v4,%v19,%v11
	pvfmul %v5,%v19,%v12
	pvfmul %v6,%v19,%v13
	pvfmul %v25,%v19,%v14
	pvfmul %v26,%v19,%v15
	pvfmul %v27,%v19,%v16
	pvfmul %v28,%v19,%v17
	pvfmul %v29,%v19,%v18
	vsll %v30,%v0,32
	vsll %v31,%v1,32
	vsll %v32,%v2,32
	vsll %v33,%v3,32
	vsll %v34,%v4,32
	vsll %v35,%v5,32
	vsll %v36,%v6,32
	vsll %v37,%v25,32
	vsll %v38,%v26,32
	vsll %v39,%v27,32
	vsll %v40,%v28,32
	vsll %v41,%v29,32
	vfadd.s %v0,%v0,%v30
	vfadd.s %v1,%v1,%v31
	vfadd.s %v2,%v2,%v32
	vfadd.s %v3,%v3,%v33
	vfadd.s %v4,%v4,%v34
	vfadd.s %v5,%v5,%v35
	vfadd.s %v6,%v6,%v36
	vfadd.s %v25,%v25,%v37
	vfadd.s %v26,%v26,%v38
	vfadd.s %v27,%v27,%v39
	vfadd.s %v28,%v28,%v40
	vfadd.s %v29,%v29,%v41
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	or %s55, 1, (0)1
	lvl %s55
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s56, 4(%s6)
	vstu %v1,4,%s56
	lea %s56, 8(%s6)
	vstu %v2,4,%s56
	lea %s56, 12(%s6)
	vstu %v3,4,%s56
	lea %s56, 16(%s6)
	vstu %v4,4,%s56
	lea %s56, 20(%s6)
	vstu %v5,4,%s56
	lea %s56, 24(%s6)
	vstu %v6,4,%s56
	lea %s56, 28(%s6)
	vstu %v25,4,%s56
	lea %s56, 32(%s6)
	vstu %v26,4,%s56
	lea %s56, 36(%s6)
	vstu %v27,4,%s56
	lea %s56, 40(%s6)
	vstu %v28,4,%s56
	lea %s56, 44(%s6)
	vstu %v29,4,%s56
	lvl %s54
	pvfmul %v0,%v20,%v7
	pvfmul %v1,%v20,%v8
	pvfmul %v2,%v20,%v9
	pvfmul %v3,%v20,%v10
	pvfmul %v4,%v20,%v11
	pvfmul %v5,%v20,%v12
	pvfmul %v6,%v20,%v13
	pvfmul %v25,%v20,%v14
	pvfmul %v26,%v20,%v15
	pvfmul %v27,%v20,%v16
	pvfmul %v28,%v20,%v17
	pvfmul %v29,%v20,%v18
	vsll %v30,%v0,32
	vsll %v31,%v1,32
	vsll %v32,%v2,32
	vsll %v33,%v3,32
	vsll %v34,%v4,32
	vsll %v35,%v5,32
	vsll %v36,%v6,32
	vsll %v37,%v25,32
	vsll %v38,%v26,32
	vsll %v39,%v27,32
	vsll %v40,%v28,32
	vsll %v41,%v29,32
	vfadd.s %v0,%v0,%v30
	vfadd.s %v1,%v1,%v31
	vfadd.s %v2,%v2,%v32
	vfadd.s %v3,%v3,%v33
	vfadd.s %v4,%v4,%v34
	vfadd.s %v5,%v5,%v35
	vfadd.s %v6,%v6,%v36
	vfadd.s %v25,%v25,%v37
	vfadd.s %v26,%v26,%v38
	vfadd.s %v27,%v27,%v39
	vfadd.s %v28,%v28,%v40
	vfadd.s %v29,%v29,%v41
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	lvl %s55
	lea %s56, 48(%s6)
	vstu %v0,4,%s56
	lea %s56, 52(%s6)
	vstu %v1,4,%s56
	lea %s56, 56(%s6)
	vstu %v2,4,%s56
	lea %s56, 60(%s6)
	vstu %v3,4,%s56
	lea %s56, 64(%s6)
	vstu %v4,4,%s56
	lea %s56, 68(%s6)
	vstu %v5,4,%s56
	lea %s56, 72(%s6)
	vstu %v6,4,%s56
	lea %s56, 76(%s6)
	vstu %v25,4,%s56
	lea %s56, 80(%s6)
	vstu %v26,4,%s56
	lea %s56, 84(%s6)
	vstu %v27,4,%s56
	lea %s56, 88(%s6)
	vstu %v28,4,%s56
	lea %s56, 92(%s6)
	vstu %v29,4,%s56
	lvl %s54
	pvfmul %v0,%v21,%v7
	pvfmul %v1,%v21,%v8
	pvfmul %v2,%v21,%v9
	pvfmul %v3,%v21,%v10
	pvfmul %v4,%v21,%v11
	pvfmul %v5,%v21,%v12
	pvfmul %v6,%v21,%v13
	pvfmul %v25,%v21,%v14
	pvfmul %v26,%v21,%v15
	pvfmul %v27,%v21,%v16
	pvfmul %v28,%v21,%v17
	pvfmul %v29,%v21,%v18
	vsll %v30,%v0,32
	vsll %v31,%v1,32
	vsll %v32,%v2,32
	vsll %v33,%v3,32
	vsll %v34,%v4,32
	vsll %v35,%v5,32
	vsll %v36,%v6,32
	vsll %v37,%v25,32
	vsll %v38,%v26,32
	vsll %v39,%v27,32
	vsll %v40,%v28,32
	vsll %v41,%v29,32
	vfadd.s %v0,%v0,%v30
	vfadd.s %v1,%v1,%v31
	vfadd.s %v2,%v2,%v32
	vfadd.s %v3,%v3,%v33
	vfadd.s %v4,%v4,%v34
	vfadd.s %v5,%v5,%v35
	vfadd.s %v6,%v6,%v36
	vfadd.s %v25,%v25,%v37
	vfadd.s %v26,%v26,%v38
	vfadd.s %v27,%v27,%v39
	vfadd.s %v28,%v28,%v40
	vfadd.s %v29,%v29,%v41
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	lvl %s55
	lea %s56, 96(%s6)
	vstu %v0,4,%s56
	lea %s56, 100(%s6)
	vstu %v1,4,%s56
	lea %s56, 104(%s6)
	vstu %v2,4,%s56
	lea %s56, 108(%s6)
	vstu %v3,4,%s56
	lea %s56, 112(%s6)
	vstu %v4,4,%s56
	lea %s56, 116(%s6)
	vstu %v5,4,%s56
	lea %s56, 120(%s6)
	vstu %v6,4,%s56
	lea %s56, 124(%s6)
	vstu %v25,4,%s56
	lea %s56, 128(%s6)
	vstu %v26,4,%s56
	lea %s56, 132(%s6)
	vstu %v27,4,%s56
	lea %s56, 136(%s6)
	vstu %v28,4,%s56
	lea %s56, 140(%s6)
	vstu %v29,4,%s56
	lvl %s54
	pvfmul %v0,%v22,%v7
	pvfmul %v1,%v22,%v8
	pvfmul %v2,%v22,%v9
	pvfmul %v3,%v22,%v10
	pvfmul %v4,%v22,%v11
	pvfmul %v5,%v22,%v12
	pvfmul %v6,%v22,%v13
	pvfmul %v25,%v22,%v14
	pvfmul %v26,%v22,%v15
	pvfmul %v27,%v22,%v16
	pvfmul %v28,%v22,%v17
	pvfmul %v29,%v22,%v18
	vsll %v30,%v0,32
	vsll %v31,%v1,32
	vsll %v32,%v2,32
	vsll %v33,%v3,32
	vsll %v34,%v4,32
	vsll %v35,%v5,32
	vsll %v36,%v6,32
	vsll %v37,%v25,32
	vsll %v38,%v26,32
	vsll %v39,%v27,32
	vsll %v40,%v28,32
	vsll %v41,%v29,32
	vfadd.s %v0,%v0,%v30
	vfadd.s %v1,%v1,%v31
	vfadd.s %v2,%v2,%v32
	vfadd.s %v3,%v3,%v33
	vfadd.s %v4,%v4,%v34
	vfadd.s %v5,%v5,%v35
	vfadd.s %v6,%v6,%v36
	vfadd.s %v25,%v25,%v37
	vfadd.s %v26,%v26,%v38
	vfadd.s %v27,%v27,%v39
	vfadd.s %v28,%v28,%v40
	vfadd.s %v29,%v29,%v41
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	lvl %s55
	lea %s56, 144(%s6)
	vstu %v0,4,%s56
	lea %s56, 148(%s6)
	vstu %v1,4,%s56
	lea %s56, 152(%s6)
	vstu %v2,4,%s56
	lea %s56, 156(%s6)
	vstu %v3,4,%s56
	lea %s56, 160(%s6)
	vstu %v4,4,%s56
	lea %s56, 164(%s6)
	vstu %v5,4,%s56
	lea %s56, 168(%s6)
	vstu %v6,4,%s56
	lea %s56, 172(%s6)
	vstu %v25,4,%s56
	lea %s56, 176(%s6)
	vstu %v26,4,%s56
	lea %s56, 180(%s6)
	vstu %v27,4,%s56
	lea %s56, 184(%s6)
	vstu %v28,4,%s56
	lea %s56, 188(%s6)
	vstu %v29,4,%s56
	lvl %s54
	pvfmul %v0,%v23,%v7
	pvfmul %v1,%v23,%v8
	pvfmul %v2,%v23,%v9
	pvfmul %v3,%v23,%v10
	pvfmul %v4,%v23,%v11
	pvfmul %v5,%v23,%v12
	pvfmul %v6,%v23,%v13
	pvfmul %v25,%v23,%v14
	pvfmul %v26,%v23,%v15
	pvfmul %v27,%v23,%v16
	pvfmul %v28,%v23,%v17
	pvfmul %v29,%v23,%v18
	vsll %v30,%v0,32
	vsll %v31,%v1,32
	vsll %v32,%v2,32
	vsll %v33,%v3,32
	vsll %v34,%v4,32
	vsll %v35,%v5,32
	vsll %v36,%v6,32
	vsll %v37,%v25,32
	vsll %v38,%v26,32
	vsll %v39,%v27,32
	vsll %v40,%v28,32
	vsll %v41,%v29,32
	vfadd.s %v0,%v0,%v30
	vfadd.s %v1,%v1,%v31
	vfadd.s %v2,%v2,%v32
	vfadd.s %v3,%v3,%v33
	vfadd.s %v4,%v4,%v34
	vfadd.s %v5,%v5,%v35
	vfadd.s %v6,%v6,%v36
	vfadd.s %v25,%v25,%v37
	vfadd.s %v26,%v26,%v38
	vfadd.s %v27,%v27,%v39
	vfadd.s %v28,%v28,%v40
	vfadd.s %v29,%v29,%v41
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	lvl %s55
	lea %s56, 192(%s6)
	vstu %v0,4,%s56
	lea %s56, 196(%s6)
	vstu %v1,4,%s56
	lea %s56, 200(%s6)
	vstu %v2,4,%s56
	lea %s56, 204(%s6)
	vstu %v3,4,%s56
	lea %s56, 208(%s6)
	vstu %v4,4,%s56
	lea %s56, 212(%s6)
	vstu %v5,4,%s56
	lea %s56, 216(%s6)
	vstu %v6,4,%s56
	lea %s56, 220(%s6)
	vstu %v25,4,%s56
	lea %s56, 224(%s6)
	vstu %v26,4,%s56
	lea %s56, 228(%s6)
	vstu %v27,4,%s56
	lea %s56, 232(%s6)
	vstu %v28,4,%s56
	lea %s56, 236(%s6)
	vstu %v29,4,%s56
	lvl %s54
	pvfmul %v0,%v24,%v7
	pvfmul %v1,%v24,%v8
	pvfmul %v2,%v24,%v9
	pvfmul %v3,%v24,%v10
	pvfmul %v4,%v24,%v11
	pvfmul %v5,%v24,%v12
	pvfmul %v6,%v24,%v13
	pvfmul %v25,%v24,%v14
	pvfmul %v26,%v24,%v15
	pvfmul %v27,%v24,%v16
	pvfmul %v28,%v24,%v17
	pvfmul %v29,%v24,%v18
	vsll %v30,%v0,32
	vsll %v31,%v1,32
	vsll %v32,%v2,32
	vsll %v33,%v3,32
	vsll %v34,%v4,32
	vsll %v35,%v5,32
	vsll %v36,%v6,32
	vsll %v37,%v25,32
	vsll %v38,%v26,32
	vsll %v39,%v27,32
	vsll %v40,%v28,32
	vsll %v41,%v29,32
	vfadd.s %v0,%v0,%v30
	vfadd.s %v1,%v1,%v31
	vfadd.s %v2,%v2,%v32
	vfadd.s %v3,%v3,%v33
	vfadd.s %v4,%v4,%v34
	vfadd.s %v5,%v5,%v35
	vfadd.s %v6,%v6,%v36
	vfadd.s %v25,%v25,%v37
	vfadd.s %v26,%v26,%v38
	vfadd.s %v27,%v27,%v39
	vfadd.s %v28,%v28,%v40
	vfadd.s %v29,%v29,%v41
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v25,%v25
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	lvl %s55
	lea %s55, 240(%s6)
	vstu %v0,4,%s55
	lea %s55, 244(%s6)
	vstu %v1,4,%s55
	lea %s55, 248(%s6)
	vstu %v2,4,%s55
	lea %s55, 252(%s6)
	vstu %v3,4,%s55
	lea %s55, 256(%s6)
	vstu %v4,4,%s55
	lea %s55, 260(%s6)
	vstu %v5,4,%s55
	lea %s55, 264(%s6)
	vstu %v6,4,%s55
	lea %s55, 268(%s6)
	vstu %v25,4,%s55
	lea %s55, 272(%s6)
	vstu %v26,4,%s55
	lea %s55, 276(%s6)
	vstu %v27,4,%s55
	lea %s55, 280(%s6)
	vstu %v28,4,%s55
	lea %s55, 284(%s6)
	vstu %v29,4,%s55
	muls.l %s55, %s2, %s0
	adds.w.sx %s55, %s55, (0)1
	lvl %s55
	lea.sl %s55, .LCPI11_0@hi
	ldu %s55, .LCPI11_0@lo(,%s55)
	vseq %v0
	vbrdu %v25,%s55
	adds.w.sx %s56, %s2, (0)1
	vsubs.w.sx %v0,%s56,%v0
	vldu %v26,4,%s6
	lea.sl %s56, .LCPI11_2@hi
	ldu %s56, .LCPI11_2@lo(,%s56)
	vfmk.w.gt %vm1,%v0
	lea.sl %s57, .LCPI11_1@hi
	ldu %s57, .LCPI11_1@lo(,%s57)
	vfmul.s %v0,%s56,%v26
	lea %s56, __vec_expf@lo
	and %s56, %s56, (32)0
	lea.sl %s56, __vec_expf@hi(%s56)
	vbrdu %v25,%s57,%vm1
	bsic %lr, (,%s56)
	lea.sl %s56, .LCPI11_3@hi
	ldu %s56, .LCPI11_3@lo(,%s56)
	vfadd.s %v0,%s55,%v0
	vfsub.s %v1,%s56,%v26
	lea.sl %s56, .LCPI11_4@hi
	ldu %s56, .LCPI11_4@lo(,%s56)
	vfdiv.s %v0,%s55,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s55,%vm1
	vfsub.s %v1,%s56,%v26
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s57,%vm1
	vfsub.s %v0,%v25,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s54
	lea.sl %s54, .LCPI11_5@hi
	ld %s55, .LCPI11_5@lo(,%s54)
	ldl.zx %s56, (,%s34)
	sll %s61, %s2, 2
	adds.l %s54, %s34, %s61
	ldl.zx %s57, (,%s54)
	vbrd %v0,%s55
	muls.l %s0, %s37, %s56
	muls.l %s1, %s37, %s57
	sll %s60, %s2, 3
	adds.l %s55, %s34, %s60
	ldl.zx %s3, (,%s55)
	muls.l %s62, 12, %s2
	adds.l %s56, %s34, %s62
	ldl.zx %s4, (,%s56)
	sll %s59, %s2, 4
	adds.l %s57, %s34, %s59
	ldl.zx %s5, (,%s57)
	muls.l %s63, 20, %s2
	adds.l %s58, %s34, %s63
	ldl.zx %s2, (,%s58)
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	muls.l %s2, %s37, %s2
	pvfmad %v1,%v0,%s0,%v7
	pvfmad %v2,%v0,%s1,%v7
	pvfmad %v3,%v0,%s3,%v7
	pvfmad %v4,%v0,%s4,%v7
	ldl.zx %s0, 4(,%s34)
	lea %s1, 4(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v0,%s5,%v7
	pvfmad %v6,%v0,%s2,%v7
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 4(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 4(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 4(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 4(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v8
	pvfmad %v2,%v2,%s1,%v8
	pvfmad %v3,%v3,%s2,%v8
	pvfmad %v4,%v4,%s3,%v8
	ldl.zx %s0, 8(,%s34)
	lea %s1, 8(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v8
	pvfmad %v6,%v6,%s5,%v8
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 8(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 8(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 8(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 8(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v9
	pvfmad %v2,%v2,%s1,%v9
	pvfmad %v3,%v3,%s2,%v9
	pvfmad %v4,%v4,%s3,%v9
	ldl.zx %s0, 12(,%s34)
	lea %s1, 12(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v9
	pvfmad %v6,%v6,%s5,%v9
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 12(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 12(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 12(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 12(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v10
	pvfmad %v2,%v2,%s1,%v10
	pvfmad %v3,%v3,%s2,%v10
	pvfmad %v4,%v4,%s3,%v10
	ldl.zx %s0, 16(,%s34)
	lea %s1, 16(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v10
	pvfmad %v6,%v6,%s5,%v10
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 16(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 16(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 16(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 16(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v11
	pvfmad %v2,%v2,%s1,%v11
	pvfmad %v3,%v3,%s2,%v11
	pvfmad %v4,%v4,%s3,%v11
	ldl.zx %s0, 20(,%s34)
	lea %s1, 20(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v11
	pvfmad %v6,%v6,%s5,%v11
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 20(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 20(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 20(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 20(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v12
	pvfmad %v2,%v2,%s1,%v12
	pvfmad %v3,%v3,%s2,%v12
	pvfmad %v4,%v4,%s3,%v12
	ldl.zx %s0, 24(,%s34)
	lea %s1, 24(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v12
	pvfmad %v6,%v6,%s5,%v12
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 24(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 24(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 24(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 24(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v13
	pvfmad %v2,%v2,%s1,%v13
	pvfmad %v3,%v3,%s2,%v13
	pvfmad %v4,%v4,%s3,%v13
	ldl.zx %s0, 28(,%s34)
	lea %s1, 28(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v13
	pvfmad %v6,%v6,%s5,%v13
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 28(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 28(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 28(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 28(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v14
	pvfmad %v2,%v2,%s1,%v14
	pvfmad %v3,%v3,%s2,%v14
	pvfmad %v4,%v4,%s3,%v14
	ldl.zx %s0, 32(,%s34)
	lea %s1, 32(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v14
	pvfmad %v6,%v6,%s5,%v14
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 32(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 32(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 32(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 32(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v15
	pvfmad %v2,%v2,%s1,%v15
	pvfmad %v3,%v3,%s2,%v15
	pvfmad %v4,%v4,%s3,%v15
	ldl.zx %s0, 36(,%s34)
	lea %s1, 36(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v15
	pvfmad %v6,%v6,%s5,%v15
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 36(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 36(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 36(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 36(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v16
	pvfmad %v2,%v2,%s1,%v16
	pvfmad %v3,%v3,%s2,%v16
	pvfmad %v4,%v4,%s3,%v16
	ldl.zx %s0, 40(,%s34)
	lea %s1, 40(%s34, %s61)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s4,%v16
	pvfmad %v6,%v6,%s5,%v16
	muls.l %s0, %s37, %s0
	muls.l %s1, %s37, %s1
	lea %s2, 40(%s34, %s60)
	ldl.zx %s2, (,%s2)
	lea %s3, 40(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s4, 40(%s34, %s59)
	ldl.zx %s4, (,%s4)
	lea %s5, 40(%s34, %s63)
	ldl.zx %s5, (,%s5)
	muls.l %s2, %s37, %s2
	muls.l %s3, %s37, %s3
	muls.l %s4, %s37, %s4
	muls.l %s5, %s37, %s5
	pvfmad %v1,%v1,%s0,%v17
	pvfmad %v2,%v2,%s1,%v17
	pvfmad %v3,%v3,%s2,%v17
	pvfmad %v4,%v4,%s3,%v17
	ldl.zx %s0, 44(,%s34)
	lea %s61, 44(%s34, %s61)
	ldl.zx %s61, (,%s61)
	pvfmad %v5,%v5,%s4,%v17
	pvfmad %v6,%v6,%s5,%v17
	muls.l %s0, %s37, %s0
	muls.l %s61, %s37, %s61
	lea %s1, 44(%s34, %s60)
	ldl.zx %s1, (,%s1)
	lea %s62, 44(%s34, %s62)
	ldl.zx %s62, (,%s62)
	lea %s2, 44(%s34, %s59)
	ldl.zx %s2, (,%s2)
	lea %s63, 44(%s34, %s63)
	ldl.zx %s63, (,%s63)
	muls.l %s1, %s37, %s1
	muls.l %s62, %s37, %s62
	muls.l %s2, %s37, %s2
	muls.l %s63, %s37, %s63
	pvfmad %v1,%v1,%s0,%v18
	pvfmad %v2,%v2,%s61,%v18
	pvfmad %v3,%v3,%s1,%v18
	pvfmad %v4,%v4,%s62,%v18
	pvfmad %v5,%v5,%s2,%v18
	pvfmad %v6,%v6,%s63,%v18
	pvfadd %v1,%v19,%v1
	pvfadd %v2,%v20,%v2
	pvfadd %v3,%v21,%v3
	pvfadd %v4,%v22,%v4
	pvfadd %v5,%v23,%v5
	pvfadd %v6,%v24,%v6
	vst %v1,8,%s48
	vst %v2,8,%s49
	vst %v3,8,%s50
	vst %v4,8,%s51
	vst %v5,8,%s52
	vst %v6,8,%s53
	ldl.zx %s48, (,%s34)
	ldl.zx %s49, (,%s54)
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, (,%s55)
	ldl.zx %s51, (,%s56)
	ldl.zx %s52, (,%s57)
	ldl.zx %s53, (,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v1,%v0,%s48,%v19
	pvfmad %v1,%v1,%s49,%v20
	pvfmad %v1,%v1,%s50,%v21
	pvfmad %v1,%v1,%s51,%v22
	ldl.zx %s48, 4(,%s34)
	ldl.zx %s49, 4(,%s54)
	pvfmad %v1,%v1,%s52,%v23
	pvfmad %v1,%v1,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	or %s50, 4, %s60
	adds.l %s50, %s34, %s50
	ldl.zx %s50, (,%s50)
	ldl.zx %s51, 4(,%s56)
	or %s52, 4, %s59
	adds.l %s52, %s34, %s52
	ldl.zx %s52, (,%s52)
	ldl.zx %s53, 4(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v2,%v0,%s48,%v19
	pvfmad %v2,%v2,%s49,%v20
	pvfmad %v2,%v2,%s50,%v21
	pvfmad %v2,%v2,%s51,%v22
	ldl.zx %s48, 8(,%s34)
	ldl.zx %s49, 8(,%s54)
	pvfmad %v2,%v2,%s52,%v23
	pvfmad %v2,%v2,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 8(,%s55)
	ldl.zx %s51, 8(,%s56)
	or %s52, 8, %s59
	adds.l %s52, %s34, %s52
	ldl.zx %s52, (,%s52)
	ldl.zx %s53, 8(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v3,%v0,%s48,%v19
	pvfmad %v3,%v3,%s49,%v20
	pvfmad %v3,%v3,%s50,%v21
	pvfmad %v3,%v3,%s51,%v22
	ldl.zx %s48, 12(,%s34)
	ldl.zx %s49, 12(,%s54)
	pvfmad %v3,%v3,%s52,%v23
	pvfmad %v3,%v3,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 12(,%s55)
	ldl.zx %s51, 12(,%s56)
	or %s52, 12, %s59
	adds.l %s52, %s34, %s52
	ldl.zx %s52, (,%s52)
	ldl.zx %s53, 12(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v4,%v0,%s48,%v19
	pvfmad %v4,%v4,%s49,%v20
	pvfmad %v4,%v4,%s50,%v21
	pvfmad %v4,%v4,%s51,%v22
	ldl.zx %s48, 16(,%s34)
	ldl.zx %s49, 16(,%s54)
	pvfmad %v4,%v4,%s52,%v23
	pvfmad %v4,%v4,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 16(,%s55)
	ldl.zx %s51, 16(,%s56)
	ldl.zx %s52, 16(,%s57)
	ldl.zx %s53, 16(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v5,%v0,%s48,%v19
	pvfmad %v5,%v5,%s49,%v20
	pvfmad %v5,%v5,%s50,%v21
	pvfmad %v5,%v5,%s51,%v22
	ldl.zx %s48, 20(,%s34)
	ldl.zx %s49, 20(,%s54)
	pvfmad %v5,%v5,%s52,%v23
	pvfmad %v5,%v5,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 20(,%s55)
	ldl.zx %s51, 20(,%s56)
	ldl.zx %s52, 20(,%s57)
	ldl.zx %s53, 20(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v6,%v0,%s48,%v19
	pvfmad %v6,%v6,%s49,%v20
	pvfmad %v6,%v6,%s50,%v21
	pvfmad %v6,%v6,%s51,%v22
	ldl.zx %s48, 24(,%s34)
	ldl.zx %s49, 24(,%s54)
	pvfmad %v6,%v6,%s52,%v23
	pvfmad %v6,%v6,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 24(,%s55)
	ldl.zx %s51, 24(,%s56)
	ldl.zx %s52, 24(,%s57)
	ldl.zx %s53, 24(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v25,%v0,%s48,%v19
	pvfmad %v25,%v25,%s49,%v20
	pvfmad %v25,%v25,%s50,%v21
	pvfmad %v25,%v25,%s51,%v22
	ldl.zx %s48, 28(,%s34)
	ldl.zx %s49, 28(,%s54)
	pvfmad %v25,%v25,%s52,%v23
	pvfmad %v25,%v25,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 28(,%s55)
	ldl.zx %s51, 28(,%s56)
	ldl.zx %s52, 28(,%s57)
	ldl.zx %s53, 28(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v26,%v0,%s48,%v19
	pvfmad %v26,%v26,%s49,%v20
	pvfmad %v26,%v26,%s50,%v21
	pvfmad %v26,%v26,%s51,%v22
	ldl.zx %s48, 32(,%s34)
	ldl.zx %s49, 32(,%s54)
	pvfmad %v26,%v26,%s52,%v23
	pvfmad %v26,%v26,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 32(,%s55)
	ldl.zx %s51, 32(,%s56)
	ldl.zx %s52, 32(,%s57)
	ldl.zx %s53, 32(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v27,%v0,%s48,%v19
	pvfmad %v27,%v27,%s49,%v20
	pvfmad %v27,%v27,%s50,%v21
	pvfmad %v27,%v27,%s51,%v22
	ldl.zx %s48, 36(,%s34)
	ldl.zx %s49, 36(,%s54)
	pvfmad %v27,%v27,%s52,%v23
	pvfmad %v27,%v27,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 36(,%s55)
	ldl.zx %s51, 36(,%s56)
	ldl.zx %s52, 36(,%s57)
	ldl.zx %s53, 36(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v28,%v0,%s48,%v19
	pvfmad %v28,%v28,%s49,%v20
	pvfmad %v28,%v28,%s50,%v21
	pvfmad %v28,%v28,%s51,%v22
	ldl.zx %s48, 40(,%s34)
	ldl.zx %s49, 40(,%s54)
	pvfmad %v28,%v28,%s52,%v23
	pvfmad %v28,%v28,%s53,%v24
	muls.l %s48, %s37, %s48
	muls.l %s49, %s37, %s49
	ldl.zx %s50, 40(,%s55)
	ldl.zx %s51, 40(,%s56)
	ldl.zx %s52, 40(,%s57)
	ldl.zx %s53, 40(,%s58)
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s52, %s37, %s52
	muls.l %s53, %s37, %s53
	pvfmad %v29,%v0,%s48,%v19
	pvfmad %v29,%v29,%s49,%v20
	pvfmad %v29,%v29,%s50,%v21
	pvfmad %v29,%v29,%s51,%v22
	ldl.zx %s34, 44(,%s34)
	ldl.zx %s48, 44(,%s54)
	pvfmad %v29,%v29,%s52,%v23
	pvfmad %v29,%v29,%s53,%v24
	muls.l %s34, %s37, %s34
	muls.l %s48, %s37, %s48
	ldl.zx %s49, 44(,%s55)
	ldl.zx %s50, 44(,%s56)
	ldl.zx %s51, 44(,%s57)
	ldl.zx %s52, 44(,%s58)
	muls.l %s49, %s37, %s49
	muls.l %s50, %s37, %s50
	muls.l %s51, %s37, %s51
	muls.l %s37, %s37, %s52
	pvfmad %v0,%v0,%s34,%v19
	pvfmad %v0,%v0,%s48,%v20
	pvfmad %v0,%v0,%s49,%v21
	pvfmad %v0,%v0,%s50,%v22
	pvfmad %v0,%v0,%s51,%v23
	pvfmad %v0,%v0,%s37,%v24
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v25
	pvfadd %v8,%v14,%v26
	pvfadd %v9,%v15,%v27
	pvfadd %v10,%v16,%v28
	pvfadd %v11,%v17,%v29
	pvfadd %v0,%v18,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s38
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v8,8,%s43
	vst %v9,8,%s44
	vst %v10,8,%s45
	vst %v11,8,%s46
	vst %v0,8,%s47
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end11:
	.size	w2v_kernel_N6_HU512_2X_W12, .Lfunc_end11-w2v_kernel_N6_HU512_2X_W12

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI12_0:
	.4byte	1065353216
.LCPI12_1:
	.4byte	0
.LCPI12_2:
	.4byte	3212836864
.LCPI12_3:
	.4byte	1086324736
.LCPI12_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI12_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W13
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W13,@function
w2v_kernel_N6_HU512_2X_W13:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB12_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB12_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s38, -8(,%s9)
	adds.w.sx %s55, %s34, (0)1
	lvl %s55
	ldl.sx %s34, (,%s4)
	ld %s48, 240(,%s9)
	muls.l %s34, %s34, %s1
	ldl.sx %s36, 4(,%s4)
	sll %s34, %s34, 2
	adds.l %s35, %s48, %s34
	ldl.sx %s34, 8(,%s4)
	muls.l %s36, %s36, %s1
	sll %s36, %s36, 2
	adds.l %s36, %s48, %s36
	muls.l %s34, %s34, %s1
	ldl.sx %s39, 12(,%s4)
	sll %s34, %s34, 2
	adds.l %s37, %s48, %s34
	ldl.sx %s34, 16(,%s4)
	muls.l %s39, %s39, %s1
	sll %s39, %s39, 2
	adds.l %s39, %s48, %s39
	muls.l %s34, %s34, %s1
	ldl.sx %s41, 20(,%s4)
	sll %s34, %s34, 2
	adds.l %s40, %s48, %s34
	ldl.sx %s34, 24(,%s4)
	muls.l %s41, %s41, %s1
	sll %s41, %s41, 2
	adds.l %s41, %s48, %s41
	muls.l %s34, %s34, %s1
	ldl.sx %s43, 28(,%s4)
	sll %s34, %s34, 2
	adds.l %s42, %s48, %s34
	ldl.sx %s34, 32(,%s4)
	muls.l %s43, %s43, %s1
	sll %s43, %s43, 2
	adds.l %s43, %s48, %s43
	muls.l %s34, %s34, %s1
	ldl.sx %s45, 36(,%s4)
	sll %s34, %s34, 2
	adds.l %s44, %s48, %s34
	ldl.sx %s34, 40(,%s4)
	muls.l %s45, %s45, %s1
	sll %s45, %s45, 2
	adds.l %s45, %s48, %s45
	muls.l %s34, %s34, %s1
	ldl.sx %s47, 44(,%s4)
	sll %s34, %s34, 2
	adds.l %s46, %s48, %s34
	ldl.sx %s34, 48(,%s4)
	muls.l %s47, %s47, %s1
	sll %s47, %s47, 2
	adds.l %s47, %s48, %s47
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s48, %s48, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s37
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	vld %v19,8,%s48
	ldl.sx %s34, (,%s5)
	ldl.sx %s50, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s49, %s7, %s34
	muls.l %s34, %s50, %s1
	ldl.sx %s51, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s50, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s51, %s51, %s1
	sll %s51, %s51, 2
	adds.l %s51, %s7, %s51
	muls.l %s34, %s34, %s1
	ldl.sx %s53, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s52, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s53, %s53, %s1
	sll %s53, %s53, 2
	adds.l %s53, %s7, %s53
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s54, %s7, %s34
	vld %v20,8,%s49
	vld %v21,8,%s50
	vld %v22,8,%s51
	vld %v23,8,%s52
	vld %v24,8,%s53
	vld %v25,8,%s54
	lvl %s55
	pvfmul %v0,%v20,%v7
	pvfmul %v1,%v20,%v8
	pvfmul %v2,%v20,%v9
	pvfmul %v3,%v20,%v10
	pvfmul %v4,%v20,%v11
	pvfmul %v5,%v20,%v12
	pvfmul %v6,%v20,%v13
	pvfmul %v26,%v20,%v14
	pvfmul %v27,%v20,%v15
	pvfmul %v28,%v20,%v16
	pvfmul %v29,%v20,%v17
	pvfmul %v30,%v20,%v18
	pvfmul %v31,%v20,%v19
	vsll %v32,%v0,32
	vsll %v33,%v1,32
	vsll %v34,%v2,32
	vsll %v35,%v3,32
	vsll %v36,%v4,32
	vsll %v37,%v5,32
	vsll %v38,%v6,32
	vsll %v39,%v26,32
	vsll %v40,%v27,32
	vsll %v41,%v28,32
	vsll %v42,%v29,32
	vsll %v43,%v30,32
	vsll %v44,%v31,32
	vfadd.s %v0,%v0,%v32
	vfadd.s %v1,%v1,%v33
	vfadd.s %v2,%v2,%v34
	vfadd.s %v3,%v3,%v35
	vfadd.s %v4,%v4,%v36
	vfadd.s %v5,%v5,%v37
	vfadd.s %v6,%v6,%v38
	vfadd.s %v26,%v26,%v39
	vfadd.s %v27,%v27,%v40
	vfadd.s %v28,%v28,%v41
	vfadd.s %v29,%v29,%v42
	vfadd.s %v30,%v30,%v43
	vfadd.s %v31,%v31,%v44
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	or %s56, 1, (0)1
	lvl %s56
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s57, 4(%s6)
	vstu %v1,4,%s57
	lea %s57, 8(%s6)
	vstu %v2,4,%s57
	lea %s57, 12(%s6)
	vstu %v3,4,%s57
	lea %s57, 16(%s6)
	vstu %v4,4,%s57
	lea %s57, 20(%s6)
	vstu %v5,4,%s57
	lea %s57, 24(%s6)
	vstu %v6,4,%s57
	lea %s57, 28(%s6)
	vstu %v26,4,%s57
	lea %s57, 32(%s6)
	vstu %v27,4,%s57
	lea %s57, 36(%s6)
	vstu %v28,4,%s57
	lea %s57, 40(%s6)
	vstu %v29,4,%s57
	lea %s57, 44(%s6)
	vstu %v30,4,%s57
	lea %s57, 48(%s6)
	vstu %v31,4,%s57
	lvl %s55
	pvfmul %v0,%v21,%v7
	pvfmul %v1,%v21,%v8
	pvfmul %v2,%v21,%v9
	pvfmul %v3,%v21,%v10
	pvfmul %v4,%v21,%v11
	pvfmul %v5,%v21,%v12
	pvfmul %v6,%v21,%v13
	pvfmul %v26,%v21,%v14
	pvfmul %v27,%v21,%v15
	pvfmul %v28,%v21,%v16
	pvfmul %v29,%v21,%v17
	pvfmul %v30,%v21,%v18
	pvfmul %v31,%v21,%v19
	vsll %v32,%v0,32
	vsll %v33,%v1,32
	vsll %v34,%v2,32
	vsll %v35,%v3,32
	vsll %v36,%v4,32
	vsll %v37,%v5,32
	vsll %v38,%v6,32
	vsll %v39,%v26,32
	vsll %v40,%v27,32
	vsll %v41,%v28,32
	vsll %v42,%v29,32
	vsll %v43,%v30,32
	vsll %v44,%v31,32
	vfadd.s %v0,%v0,%v32
	vfadd.s %v1,%v1,%v33
	vfadd.s %v2,%v2,%v34
	vfadd.s %v3,%v3,%v35
	vfadd.s %v4,%v4,%v36
	vfadd.s %v5,%v5,%v37
	vfadd.s %v6,%v6,%v38
	vfadd.s %v26,%v26,%v39
	vfadd.s %v27,%v27,%v40
	vfadd.s %v28,%v28,%v41
	vfadd.s %v29,%v29,%v42
	vfadd.s %v30,%v30,%v43
	vfadd.s %v31,%v31,%v44
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	lvl %s56
	lea %s57, 52(%s6)
	vstu %v0,4,%s57
	lea %s57, 56(%s6)
	vstu %v1,4,%s57
	lea %s57, 60(%s6)
	vstu %v2,4,%s57
	lea %s57, 64(%s6)
	vstu %v3,4,%s57
	lea %s57, 68(%s6)
	vstu %v4,4,%s57
	lea %s57, 72(%s6)
	vstu %v5,4,%s57
	lea %s57, 76(%s6)
	vstu %v6,4,%s57
	lea %s57, 80(%s6)
	vstu %v26,4,%s57
	lea %s57, 84(%s6)
	vstu %v27,4,%s57
	lea %s57, 88(%s6)
	vstu %v28,4,%s57
	lea %s57, 92(%s6)
	vstu %v29,4,%s57
	lea %s57, 96(%s6)
	vstu %v30,4,%s57
	lea %s57, 100(%s6)
	vstu %v31,4,%s57
	lvl %s55
	pvfmul %v0,%v22,%v7
	pvfmul %v1,%v22,%v8
	pvfmul %v2,%v22,%v9
	pvfmul %v3,%v22,%v10
	pvfmul %v4,%v22,%v11
	pvfmul %v5,%v22,%v12
	pvfmul %v6,%v22,%v13
	pvfmul %v26,%v22,%v14
	pvfmul %v27,%v22,%v15
	pvfmul %v28,%v22,%v16
	pvfmul %v29,%v22,%v17
	pvfmul %v30,%v22,%v18
	pvfmul %v31,%v22,%v19
	vsll %v32,%v0,32
	vsll %v33,%v1,32
	vsll %v34,%v2,32
	vsll %v35,%v3,32
	vsll %v36,%v4,32
	vsll %v37,%v5,32
	vsll %v38,%v6,32
	vsll %v39,%v26,32
	vsll %v40,%v27,32
	vsll %v41,%v28,32
	vsll %v42,%v29,32
	vsll %v43,%v30,32
	vsll %v44,%v31,32
	vfadd.s %v0,%v0,%v32
	vfadd.s %v1,%v1,%v33
	vfadd.s %v2,%v2,%v34
	vfadd.s %v3,%v3,%v35
	vfadd.s %v4,%v4,%v36
	vfadd.s %v5,%v5,%v37
	vfadd.s %v6,%v6,%v38
	vfadd.s %v26,%v26,%v39
	vfadd.s %v27,%v27,%v40
	vfadd.s %v28,%v28,%v41
	vfadd.s %v29,%v29,%v42
	vfadd.s %v30,%v30,%v43
	vfadd.s %v31,%v31,%v44
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	lvl %s56
	lea %s57, 104(%s6)
	vstu %v0,4,%s57
	lea %s57, 108(%s6)
	vstu %v1,4,%s57
	lea %s57, 112(%s6)
	vstu %v2,4,%s57
	lea %s57, 116(%s6)
	vstu %v3,4,%s57
	lea %s57, 120(%s6)
	vstu %v4,4,%s57
	lea %s57, 124(%s6)
	vstu %v5,4,%s57
	lea %s57, 128(%s6)
	vstu %v6,4,%s57
	lea %s57, 132(%s6)
	vstu %v26,4,%s57
	lea %s57, 136(%s6)
	vstu %v27,4,%s57
	lea %s57, 140(%s6)
	vstu %v28,4,%s57
	lea %s57, 144(%s6)
	vstu %v29,4,%s57
	lea %s57, 148(%s6)
	vstu %v30,4,%s57
	lea %s57, 152(%s6)
	vstu %v31,4,%s57
	lvl %s55
	pvfmul %v0,%v23,%v7
	pvfmul %v1,%v23,%v8
	pvfmul %v2,%v23,%v9
	pvfmul %v3,%v23,%v10
	pvfmul %v4,%v23,%v11
	pvfmul %v5,%v23,%v12
	pvfmul %v6,%v23,%v13
	pvfmul %v26,%v23,%v14
	pvfmul %v27,%v23,%v15
	pvfmul %v28,%v23,%v16
	pvfmul %v29,%v23,%v17
	pvfmul %v30,%v23,%v18
	pvfmul %v31,%v23,%v19
	vsll %v32,%v0,32
	vsll %v33,%v1,32
	vsll %v34,%v2,32
	vsll %v35,%v3,32
	vsll %v36,%v4,32
	vsll %v37,%v5,32
	vsll %v38,%v6,32
	vsll %v39,%v26,32
	vsll %v40,%v27,32
	vsll %v41,%v28,32
	vsll %v42,%v29,32
	vsll %v43,%v30,32
	vsll %v44,%v31,32
	vfadd.s %v0,%v0,%v32
	vfadd.s %v1,%v1,%v33
	vfadd.s %v2,%v2,%v34
	vfadd.s %v3,%v3,%v35
	vfadd.s %v4,%v4,%v36
	vfadd.s %v5,%v5,%v37
	vfadd.s %v6,%v6,%v38
	vfadd.s %v26,%v26,%v39
	vfadd.s %v27,%v27,%v40
	vfadd.s %v28,%v28,%v41
	vfadd.s %v29,%v29,%v42
	vfadd.s %v30,%v30,%v43
	vfadd.s %v31,%v31,%v44
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	lvl %s56
	lea %s57, 156(%s6)
	vstu %v0,4,%s57
	lea %s57, 160(%s6)
	vstu %v1,4,%s57
	lea %s57, 164(%s6)
	vstu %v2,4,%s57
	lea %s57, 168(%s6)
	vstu %v3,4,%s57
	lea %s57, 172(%s6)
	vstu %v4,4,%s57
	lea %s57, 176(%s6)
	vstu %v5,4,%s57
	lea %s57, 180(%s6)
	vstu %v6,4,%s57
	lea %s57, 184(%s6)
	vstu %v26,4,%s57
	lea %s57, 188(%s6)
	vstu %v27,4,%s57
	lea %s57, 192(%s6)
	vstu %v28,4,%s57
	lea %s57, 196(%s6)
	vstu %v29,4,%s57
	lea %s57, 200(%s6)
	vstu %v30,4,%s57
	lea %s57, 204(%s6)
	vstu %v31,4,%s57
	lvl %s55
	pvfmul %v0,%v24,%v7
	pvfmul %v1,%v24,%v8
	pvfmul %v2,%v24,%v9
	pvfmul %v3,%v24,%v10
	pvfmul %v4,%v24,%v11
	pvfmul %v5,%v24,%v12
	pvfmul %v6,%v24,%v13
	pvfmul %v26,%v24,%v14
	pvfmul %v27,%v24,%v15
	pvfmul %v28,%v24,%v16
	pvfmul %v29,%v24,%v17
	pvfmul %v30,%v24,%v18
	pvfmul %v31,%v24,%v19
	vsll %v32,%v0,32
	vsll %v33,%v1,32
	vsll %v34,%v2,32
	vsll %v35,%v3,32
	vsll %v36,%v4,32
	vsll %v37,%v5,32
	vsll %v38,%v6,32
	vsll %v39,%v26,32
	vsll %v40,%v27,32
	vsll %v41,%v28,32
	vsll %v42,%v29,32
	vsll %v43,%v30,32
	vsll %v44,%v31,32
	vfadd.s %v0,%v0,%v32
	vfadd.s %v1,%v1,%v33
	vfadd.s %v2,%v2,%v34
	vfadd.s %v3,%v3,%v35
	vfadd.s %v4,%v4,%v36
	vfadd.s %v5,%v5,%v37
	vfadd.s %v6,%v6,%v38
	vfadd.s %v26,%v26,%v39
	vfadd.s %v27,%v27,%v40
	vfadd.s %v28,%v28,%v41
	vfadd.s %v29,%v29,%v42
	vfadd.s %v30,%v30,%v43
	vfadd.s %v31,%v31,%v44
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	lvl %s56
	lea %s57, 208(%s6)
	vstu %v0,4,%s57
	lea %s57, 212(%s6)
	vstu %v1,4,%s57
	lea %s57, 216(%s6)
	vstu %v2,4,%s57
	lea %s57, 220(%s6)
	vstu %v3,4,%s57
	lea %s57, 224(%s6)
	vstu %v4,4,%s57
	lea %s57, 228(%s6)
	vstu %v5,4,%s57
	lea %s57, 232(%s6)
	vstu %v6,4,%s57
	lea %s57, 236(%s6)
	vstu %v26,4,%s57
	lea %s57, 240(%s6)
	vstu %v27,4,%s57
	lea %s57, 244(%s6)
	vstu %v28,4,%s57
	lea %s57, 248(%s6)
	vstu %v29,4,%s57
	lea %s57, 252(%s6)
	vstu %v30,4,%s57
	lea %s57, 256(%s6)
	vstu %v31,4,%s57
	lvl %s55
	pvfmul %v0,%v25,%v7
	pvfmul %v1,%v25,%v8
	pvfmul %v2,%v25,%v9
	pvfmul %v3,%v25,%v10
	pvfmul %v4,%v25,%v11
	pvfmul %v5,%v25,%v12
	pvfmul %v6,%v25,%v13
	pvfmul %v26,%v25,%v14
	pvfmul %v27,%v25,%v15
	pvfmul %v28,%v25,%v16
	pvfmul %v29,%v25,%v17
	pvfmul %v30,%v25,%v18
	pvfmul %v31,%v25,%v19
	vsll %v32,%v0,32
	vsll %v33,%v1,32
	vsll %v34,%v2,32
	vsll %v35,%v3,32
	vsll %v36,%v4,32
	vsll %v37,%v5,32
	vsll %v38,%v6,32
	vsll %v39,%v26,32
	vsll %v40,%v27,32
	vsll %v41,%v28,32
	vsll %v42,%v29,32
	vsll %v43,%v30,32
	vsll %v44,%v31,32
	vfadd.s %v0,%v0,%v32
	vfadd.s %v1,%v1,%v33
	vfadd.s %v2,%v2,%v34
	vfadd.s %v3,%v3,%v35
	vfadd.s %v4,%v4,%v36
	vfadd.s %v5,%v5,%v37
	vfadd.s %v6,%v6,%v38
	vfadd.s %v26,%v26,%v39
	vfadd.s %v27,%v27,%v40
	vfadd.s %v28,%v28,%v41
	vfadd.s %v29,%v29,%v42
	vfadd.s %v30,%v30,%v43
	vfadd.s %v31,%v31,%v44
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v26,%v26
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	lvl %s56
	lea %s56, 260(%s6)
	vstu %v0,4,%s56
	lea %s56, 264(%s6)
	vstu %v1,4,%s56
	lea %s56, 268(%s6)
	vstu %v2,4,%s56
	lea %s56, 272(%s6)
	vstu %v3,4,%s56
	lea %s56, 276(%s6)
	vstu %v4,4,%s56
	lea %s56, 280(%s6)
	vstu %v5,4,%s56
	lea %s56, 284(%s6)
	vstu %v6,4,%s56
	lea %s56, 288(%s6)
	vstu %v26,4,%s56
	lea %s56, 292(%s6)
	vstu %v27,4,%s56
	lea %s56, 296(%s6)
	vstu %v28,4,%s56
	lea %s56, 300(%s6)
	vstu %v29,4,%s56
	lea %s56, 304(%s6)
	vstu %v30,4,%s56
	lea %s56, 308(%s6)
	vstu %v31,4,%s56
	muls.l %s56, %s2, %s0
	adds.w.sx %s56, %s56, (0)1
	lvl %s56
	lea.sl %s56, .LCPI12_0@hi
	ldu %s56, .LCPI12_0@lo(,%s56)
	vseq %v0
	vbrdu %v26,%s56
	adds.w.sx %s57, %s2, (0)1
	vsubs.w.sx %v0,%s57,%v0
	vldu %v27,4,%s6
	lea.sl %s57, .LCPI12_2@hi
	ldu %s57, .LCPI12_2@lo(,%s57)
	vfmk.w.gt %vm1,%v0
	lea.sl %s58, .LCPI12_1@hi
	ldu %s58, .LCPI12_1@lo(,%s58)
	vfmul.s %v0,%s57,%v27
	lea %s57, __vec_expf@lo
	and %s57, %s57, (32)0
	lea.sl %s57, __vec_expf@hi(%s57)
	vbrdu %v26,%s58,%vm1
	bsic %lr, (,%s57)
	lea.sl %s57, .LCPI12_3@hi
	ldu %s57, .LCPI12_3@lo(,%s57)
	vfadd.s %v0,%s56,%v0
	vfsub.s %v1,%s57,%v27
	lea.sl %s57, .LCPI12_4@hi
	ldu %s57, .LCPI12_4@lo(,%s57)
	vfdiv.s %v0,%s56,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s56,%vm1
	vfsub.s %v1,%s57,%v27
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s58,%vm1
	vfsub.s %v0,%v26,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s55
	lea.sl %s55, .LCPI12_5@hi
	ld %s56, .LCPI12_5@lo(,%s55)
	ldl.zx %s57, (,%s34)
	sll %s62, %s2, 2
	adds.l %s55, %s34, %s62
	ldl.zx %s58, (,%s55)
	vbrd %v0,%s56
	muls.l %s1, %s38, %s57
	muls.l %s3, %s38, %s58
	sll %s61, %s2, 3
	adds.l %s56, %s34, %s61
	ldl.zx %s4, (,%s56)
	muls.l %s63, 12, %s2
	adds.l %s57, %s34, %s63
	ldl.zx %s5, (,%s57)
	sll %s60, %s2, 4
	adds.l %s58, %s34, %s60
	ldl.zx %s6, (,%s58)
	muls.l %s0, 20, %s2
	adds.l %s59, %s34, %s0
	ldl.zx %s2, (,%s59)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s2, %s38, %s2
	pvfmad %v1,%v0,%s1,%v7
	pvfmad %v2,%v0,%s3,%v7
	pvfmad %v3,%v0,%s4,%v7
	pvfmad %v4,%v0,%s5,%v7
	ldl.zx %s1, 4(,%s34)
	lea %s3, 4(%s34, %s62)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v0,%s6,%v7
	pvfmad %v6,%v0,%s2,%v7
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s3
	lea %s3, 4(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 4(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 4(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 4(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v8
	pvfmad %v2,%v2,%s2,%v8
	pvfmad %v3,%v3,%s3,%v8
	pvfmad %v4,%v4,%s4,%v8
	ldl.zx %s1, 8(,%s34)
	lea %s2, 8(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v8
	pvfmad %v6,%v6,%s6,%v8
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 8(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 8(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 8(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 8(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v9
	pvfmad %v2,%v2,%s2,%v9
	pvfmad %v3,%v3,%s3,%v9
	pvfmad %v4,%v4,%s4,%v9
	ldl.zx %s1, 12(,%s34)
	lea %s2, 12(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v9
	pvfmad %v6,%v6,%s6,%v9
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 12(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 12(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 12(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 12(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v10
	pvfmad %v2,%v2,%s2,%v10
	pvfmad %v3,%v3,%s3,%v10
	pvfmad %v4,%v4,%s4,%v10
	ldl.zx %s1, 16(,%s34)
	lea %s2, 16(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v10
	pvfmad %v6,%v6,%s6,%v10
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 16(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 16(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 16(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 16(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v11
	pvfmad %v2,%v2,%s2,%v11
	pvfmad %v3,%v3,%s3,%v11
	pvfmad %v4,%v4,%s4,%v11
	ldl.zx %s1, 20(,%s34)
	lea %s2, 20(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v11
	pvfmad %v6,%v6,%s6,%v11
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 20(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 20(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 20(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 20(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v12
	pvfmad %v2,%v2,%s2,%v12
	pvfmad %v3,%v3,%s3,%v12
	pvfmad %v4,%v4,%s4,%v12
	ldl.zx %s1, 24(,%s34)
	lea %s2, 24(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v12
	pvfmad %v6,%v6,%s6,%v12
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 24(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 24(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 24(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 24(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v13
	pvfmad %v2,%v2,%s2,%v13
	pvfmad %v3,%v3,%s3,%v13
	pvfmad %v4,%v4,%s4,%v13
	ldl.zx %s1, 28(,%s34)
	lea %s2, 28(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v13
	pvfmad %v6,%v6,%s6,%v13
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 28(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 28(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 28(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 28(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v14
	pvfmad %v2,%v2,%s2,%v14
	pvfmad %v3,%v3,%s3,%v14
	pvfmad %v4,%v4,%s4,%v14
	ldl.zx %s1, 32(,%s34)
	lea %s2, 32(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v14
	pvfmad %v6,%v6,%s6,%v14
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 32(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 32(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 32(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 32(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v15
	pvfmad %v2,%v2,%s2,%v15
	pvfmad %v3,%v3,%s3,%v15
	pvfmad %v4,%v4,%s4,%v15
	ldl.zx %s1, 36(,%s34)
	lea %s2, 36(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v15
	pvfmad %v6,%v6,%s6,%v15
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 36(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 36(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 36(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 36(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v16
	pvfmad %v2,%v2,%s2,%v16
	pvfmad %v3,%v3,%s3,%v16
	pvfmad %v4,%v4,%s4,%v16
	ldl.zx %s1, 40(,%s34)
	lea %s2, 40(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v16
	pvfmad %v6,%v6,%s6,%v16
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 40(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 40(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 40(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 40(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v17
	pvfmad %v2,%v2,%s2,%v17
	pvfmad %v3,%v3,%s3,%v17
	pvfmad %v4,%v4,%s4,%v17
	ldl.zx %s1, 44(,%s34)
	lea %s2, 44(%s34, %s62)
	ldl.zx %s2, (,%s2)
	pvfmad %v5,%v5,%s5,%v17
	pvfmad %v6,%v6,%s6,%v17
	muls.l %s1, %s38, %s1
	muls.l %s2, %s38, %s2
	lea %s3, 44(%s34, %s61)
	ldl.zx %s3, (,%s3)
	lea %s4, 44(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s5, 44(%s34, %s60)
	ldl.zx %s5, (,%s5)
	lea %s6, 44(%s34, %s0)
	ldl.zx %s6, (,%s6)
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	pvfmad %v1,%v1,%s1,%v18
	pvfmad %v2,%v2,%s2,%v18
	pvfmad %v3,%v3,%s3,%v18
	pvfmad %v4,%v4,%s4,%v18
	ldl.zx %s1, 48(,%s34)
	lea %s62, 48(%s34, %s62)
	ldl.zx %s62, (,%s62)
	pvfmad %v5,%v5,%s5,%v18
	pvfmad %v6,%v6,%s6,%v18
	muls.l %s1, %s38, %s1
	muls.l %s62, %s38, %s62
	lea %s2, 48(%s34, %s61)
	ldl.zx %s2, (,%s2)
	lea %s63, 48(%s34, %s63)
	ldl.zx %s63, (,%s63)
	lea %s3, 48(%s34, %s60)
	ldl.zx %s3, (,%s3)
	lea %s0, 48(%s34, %s0)
	ldl.zx %s0, (,%s0)
	muls.l %s2, %s38, %s2
	muls.l %s63, %s38, %s63
	muls.l %s3, %s38, %s3
	muls.l %s0, %s38, %s0
	pvfmad %v1,%v1,%s1,%v19
	pvfmad %v2,%v2,%s62,%v19
	pvfmad %v3,%v3,%s2,%v19
	pvfmad %v4,%v4,%s63,%v19
	pvfmad %v5,%v5,%s3,%v19
	pvfmad %v6,%v6,%s0,%v19
	pvfadd %v1,%v20,%v1
	pvfadd %v2,%v21,%v2
	pvfadd %v3,%v22,%v3
	pvfadd %v4,%v23,%v4
	pvfadd %v5,%v24,%v5
	pvfadd %v6,%v25,%v6
	vst %v1,8,%s49
	vst %v2,8,%s50
	vst %v3,8,%s51
	vst %v4,8,%s52
	vst %v5,8,%s53
	vst %v6,8,%s54
	ldl.zx %s49, (,%s34)
	ldl.zx %s50, (,%s55)
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, (,%s56)
	ldl.zx %s52, (,%s57)
	ldl.zx %s53, (,%s58)
	ldl.zx %s54, (,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v1,%v0,%s49,%v20
	pvfmad %v1,%v1,%s50,%v21
	pvfmad %v1,%v1,%s51,%v22
	pvfmad %v1,%v1,%s52,%v23
	ldl.zx %s49, 4(,%s34)
	ldl.zx %s50, 4(,%s55)
	pvfmad %v1,%v1,%s53,%v24
	pvfmad %v1,%v1,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	or %s51, 4, %s61
	adds.l %s51, %s34, %s51
	ldl.zx %s51, (,%s51)
	ldl.zx %s52, 4(,%s57)
	or %s53, 4, %s60
	adds.l %s53, %s34, %s53
	ldl.zx %s53, (,%s53)
	ldl.zx %s54, 4(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v2,%v0,%s49,%v20
	pvfmad %v2,%v2,%s50,%v21
	pvfmad %v2,%v2,%s51,%v22
	pvfmad %v2,%v2,%s52,%v23
	ldl.zx %s49, 8(,%s34)
	ldl.zx %s50, 8(,%s55)
	pvfmad %v2,%v2,%s53,%v24
	pvfmad %v2,%v2,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 8(,%s56)
	ldl.zx %s52, 8(,%s57)
	or %s53, 8, %s60
	adds.l %s53, %s34, %s53
	ldl.zx %s53, (,%s53)
	ldl.zx %s54, 8(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v3,%v0,%s49,%v20
	pvfmad %v3,%v3,%s50,%v21
	pvfmad %v3,%v3,%s51,%v22
	pvfmad %v3,%v3,%s52,%v23
	ldl.zx %s49, 12(,%s34)
	ldl.zx %s50, 12(,%s55)
	pvfmad %v3,%v3,%s53,%v24
	pvfmad %v3,%v3,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 12(,%s56)
	ldl.zx %s52, 12(,%s57)
	or %s53, 12, %s60
	adds.l %s53, %s34, %s53
	ldl.zx %s53, (,%s53)
	ldl.zx %s54, 12(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v4,%v0,%s49,%v20
	pvfmad %v4,%v4,%s50,%v21
	pvfmad %v4,%v4,%s51,%v22
	pvfmad %v4,%v4,%s52,%v23
	ldl.zx %s49, 16(,%s34)
	ldl.zx %s50, 16(,%s55)
	pvfmad %v4,%v4,%s53,%v24
	pvfmad %v4,%v4,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 16(,%s56)
	ldl.zx %s52, 16(,%s57)
	ldl.zx %s53, 16(,%s58)
	ldl.zx %s54, 16(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v5,%v0,%s49,%v20
	pvfmad %v5,%v5,%s50,%v21
	pvfmad %v5,%v5,%s51,%v22
	pvfmad %v5,%v5,%s52,%v23
	ldl.zx %s49, 20(,%s34)
	ldl.zx %s50, 20(,%s55)
	pvfmad %v5,%v5,%s53,%v24
	pvfmad %v5,%v5,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 20(,%s56)
	ldl.zx %s52, 20(,%s57)
	ldl.zx %s53, 20(,%s58)
	ldl.zx %s54, 20(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v6,%v0,%s49,%v20
	pvfmad %v6,%v6,%s50,%v21
	pvfmad %v6,%v6,%s51,%v22
	pvfmad %v6,%v6,%s52,%v23
	ldl.zx %s49, 24(,%s34)
	ldl.zx %s50, 24(,%s55)
	pvfmad %v6,%v6,%s53,%v24
	pvfmad %v6,%v6,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 24(,%s56)
	ldl.zx %s52, 24(,%s57)
	ldl.zx %s53, 24(,%s58)
	ldl.zx %s54, 24(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v26,%v0,%s49,%v20
	pvfmad %v26,%v26,%s50,%v21
	pvfmad %v26,%v26,%s51,%v22
	pvfmad %v26,%v26,%s52,%v23
	ldl.zx %s49, 28(,%s34)
	ldl.zx %s50, 28(,%s55)
	pvfmad %v26,%v26,%s53,%v24
	pvfmad %v26,%v26,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 28(,%s56)
	ldl.zx %s52, 28(,%s57)
	ldl.zx %s53, 28(,%s58)
	ldl.zx %s54, 28(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v27,%v0,%s49,%v20
	pvfmad %v27,%v27,%s50,%v21
	pvfmad %v27,%v27,%s51,%v22
	pvfmad %v27,%v27,%s52,%v23
	ldl.zx %s49, 32(,%s34)
	ldl.zx %s50, 32(,%s55)
	pvfmad %v27,%v27,%s53,%v24
	pvfmad %v27,%v27,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 32(,%s56)
	ldl.zx %s52, 32(,%s57)
	ldl.zx %s53, 32(,%s58)
	ldl.zx %s54, 32(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v28,%v0,%s49,%v20
	pvfmad %v28,%v28,%s50,%v21
	pvfmad %v28,%v28,%s51,%v22
	pvfmad %v28,%v28,%s52,%v23
	ldl.zx %s49, 36(,%s34)
	ldl.zx %s50, 36(,%s55)
	pvfmad %v28,%v28,%s53,%v24
	pvfmad %v28,%v28,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 36(,%s56)
	ldl.zx %s52, 36(,%s57)
	ldl.zx %s53, 36(,%s58)
	ldl.zx %s54, 36(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v29,%v0,%s49,%v20
	pvfmad %v29,%v29,%s50,%v21
	pvfmad %v29,%v29,%s51,%v22
	pvfmad %v29,%v29,%s52,%v23
	ldl.zx %s49, 40(,%s34)
	ldl.zx %s50, 40(,%s55)
	pvfmad %v29,%v29,%s53,%v24
	pvfmad %v29,%v29,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 40(,%s56)
	ldl.zx %s52, 40(,%s57)
	ldl.zx %s53, 40(,%s58)
	ldl.zx %s54, 40(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v30,%v0,%s49,%v20
	pvfmad %v30,%v30,%s50,%v21
	pvfmad %v30,%v30,%s51,%v22
	pvfmad %v30,%v30,%s52,%v23
	ldl.zx %s49, 44(,%s34)
	ldl.zx %s50, 44(,%s55)
	pvfmad %v30,%v30,%s53,%v24
	pvfmad %v30,%v30,%s54,%v25
	muls.l %s49, %s38, %s49
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 44(,%s56)
	ldl.zx %s52, 44(,%s57)
	ldl.zx %s53, 44(,%s58)
	ldl.zx %s54, 44(,%s59)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	pvfmad %v31,%v0,%s49,%v20
	pvfmad %v31,%v31,%s50,%v21
	pvfmad %v31,%v31,%s51,%v22
	pvfmad %v31,%v31,%s52,%v23
	ldl.zx %s34, 48(,%s34)
	ldl.zx %s49, 48(,%s55)
	pvfmad %v31,%v31,%s53,%v24
	pvfmad %v31,%v31,%s54,%v25
	muls.l %s34, %s38, %s34
	muls.l %s49, %s38, %s49
	ldl.zx %s50, 48(,%s56)
	ldl.zx %s51, 48(,%s57)
	ldl.zx %s52, 48(,%s58)
	ldl.zx %s53, 48(,%s59)
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s38, %s38, %s53
	pvfmad %v0,%v0,%s34,%v20
	pvfmad %v0,%v0,%s49,%v21
	pvfmad %v0,%v0,%s50,%v22
	pvfmad %v0,%v0,%s51,%v23
	pvfmad %v0,%v0,%s52,%v24
	pvfmad %v0,%v0,%s38,%v25
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v26
	pvfadd %v8,%v14,%v27
	pvfadd %v9,%v15,%v28
	pvfadd %v10,%v16,%v29
	pvfadd %v11,%v17,%v30
	pvfadd %v12,%v18,%v31
	pvfadd %v0,%v19,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s37
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v8,8,%s43
	vst %v9,8,%s44
	vst %v10,8,%s45
	vst %v11,8,%s46
	vst %v12,8,%s47
	vst %v0,8,%s48
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end12:
	.size	w2v_kernel_N6_HU512_2X_W13, .Lfunc_end12-w2v_kernel_N6_HU512_2X_W13

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI13_0:
	.4byte	1065353216
.LCPI13_1:
	.4byte	0
.LCPI13_2:
	.4byte	3212836864
.LCPI13_3:
	.4byte	1086324736
.LCPI13_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI13_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W14
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W14,@function
w2v_kernel_N6_HU512_2X_W14:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB13_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB13_2:
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s38, -8(,%s9)
	adds.w.sx %s56, %s34, (0)1
	lvl %s56
	ldl.sx %s34, (,%s4)
	ld %s49, 240(,%s9)
	ldl.sx %s36, 4(,%s4)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s35, %s49, %s34
	muls.l %s34, %s36, %s1
	ldl.sx %s37, 8(,%s4)
	sll %s34, %s34, 2
	adds.l %s36, %s49, %s34
	ldl.sx %s34, 12(,%s4)
	muls.l %s37, %s37, %s1
	sll %s37, %s37, 2
	adds.l %s37, %s49, %s37
	muls.l %s34, %s34, %s1
	ldl.sx %s40, 16(,%s4)
	sll %s34, %s34, 2
	adds.l %s39, %s49, %s34
	ldl.sx %s34, 20(,%s4)
	muls.l %s40, %s40, %s1
	sll %s40, %s40, 2
	adds.l %s40, %s49, %s40
	muls.l %s34, %s34, %s1
	ldl.sx %s42, 24(,%s4)
	sll %s34, %s34, 2
	adds.l %s41, %s49, %s34
	ldl.sx %s34, 28(,%s4)
	muls.l %s42, %s42, %s1
	sll %s42, %s42, 2
	adds.l %s42, %s49, %s42
	muls.l %s34, %s34, %s1
	ldl.sx %s44, 32(,%s4)
	sll %s34, %s34, 2
	adds.l %s43, %s49, %s34
	ldl.sx %s34, 36(,%s4)
	muls.l %s44, %s44, %s1
	sll %s44, %s44, 2
	adds.l %s44, %s49, %s44
	muls.l %s34, %s34, %s1
	ldl.sx %s46, 40(,%s4)
	sll %s34, %s34, 2
	adds.l %s45, %s49, %s34
	ldl.sx %s34, 44(,%s4)
	muls.l %s46, %s46, %s1
	sll %s46, %s46, 2
	adds.l %s46, %s49, %s46
	muls.l %s34, %s34, %s1
	ldl.sx %s48, 48(,%s4)
	sll %s34, %s34, 2
	adds.l %s47, %s49, %s34
	ldl.sx %s34, 52(,%s4)
	muls.l %s48, %s48, %s1
	sll %s48, %s48, 2
	adds.l %s48, %s49, %s48
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s49, %s49, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s37
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	vld %v19,8,%s48
	vld %v20,8,%s49
	ldl.sx %s34, (,%s5)
	ldl.sx %s51, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s50, %s7, %s34
	muls.l %s34, %s51, %s1
	ldl.sx %s52, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s51, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s52, %s52, %s1
	sll %s52, %s52, 2
	adds.l %s52, %s7, %s52
	muls.l %s34, %s34, %s1
	ldl.sx %s54, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s53, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s54, %s54, %s1
	sll %s54, %s54, 2
	adds.l %s54, %s7, %s54
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s55, %s7, %s34
	vld %v21,8,%s50
	vld %v22,8,%s51
	vld %v23,8,%s52
	vld %v24,8,%s53
	vld %v25,8,%s54
	vld %v26,8,%s55
	lvl %s56
	pvfmul %v0,%v21,%v7
	pvfmul %v1,%v21,%v8
	pvfmul %v2,%v21,%v9
	pvfmul %v3,%v21,%v10
	pvfmul %v4,%v21,%v11
	pvfmul %v5,%v21,%v12
	pvfmul %v6,%v21,%v13
	pvfmul %v27,%v21,%v14
	pvfmul %v28,%v21,%v15
	pvfmul %v29,%v21,%v16
	pvfmul %v30,%v21,%v17
	pvfmul %v31,%v21,%v18
	pvfmul %v32,%v21,%v19
	pvfmul %v33,%v21,%v20
	vsll %v34,%v0,32
	vsll %v35,%v1,32
	vsll %v36,%v2,32
	vsll %v37,%v3,32
	vsll %v38,%v4,32
	vsll %v39,%v5,32
	vsll %v40,%v6,32
	vsll %v41,%v27,32
	vsll %v42,%v28,32
	vsll %v43,%v29,32
	vsll %v44,%v30,32
	vsll %v45,%v31,32
	vsll %v46,%v32,32
	vsll %v47,%v33,32
	vfadd.s %v0,%v0,%v34
	vfadd.s %v1,%v1,%v35
	vfadd.s %v2,%v2,%v36
	vfadd.s %v3,%v3,%v37
	vfadd.s %v4,%v4,%v38
	vfadd.s %v5,%v5,%v39
	vfadd.s %v6,%v6,%v40
	vfadd.s %v27,%v27,%v41
	vfadd.s %v28,%v28,%v42
	vfadd.s %v29,%v29,%v43
	vfadd.s %v30,%v30,%v44
	vfadd.s %v31,%v31,%v45
	vfadd.s %v32,%v32,%v46
	vfadd.s %v33,%v33,%v47
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	or %s57, 1, (0)1
	lvl %s57
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s58, 4(%s6)
	vstu %v1,4,%s58
	lea %s58, 8(%s6)
	vstu %v2,4,%s58
	lea %s58, 12(%s6)
	vstu %v3,4,%s58
	lea %s58, 16(%s6)
	vstu %v4,4,%s58
	lea %s58, 20(%s6)
	vstu %v5,4,%s58
	lea %s58, 24(%s6)
	vstu %v6,4,%s58
	lea %s58, 28(%s6)
	vstu %v27,4,%s58
	lea %s58, 32(%s6)
	vstu %v28,4,%s58
	lea %s58, 36(%s6)
	vstu %v29,4,%s58
	lea %s58, 40(%s6)
	vstu %v30,4,%s58
	lea %s58, 44(%s6)
	vstu %v31,4,%s58
	lea %s58, 48(%s6)
	vstu %v32,4,%s58
	lea %s58, 52(%s6)
	vstu %v33,4,%s58
	lvl %s56
	pvfmul %v0,%v22,%v7
	pvfmul %v1,%v22,%v8
	pvfmul %v2,%v22,%v9
	pvfmul %v3,%v22,%v10
	pvfmul %v4,%v22,%v11
	pvfmul %v5,%v22,%v12
	pvfmul %v6,%v22,%v13
	pvfmul %v27,%v22,%v14
	pvfmul %v28,%v22,%v15
	pvfmul %v29,%v22,%v16
	pvfmul %v30,%v22,%v17
	pvfmul %v31,%v22,%v18
	pvfmul %v32,%v22,%v19
	pvfmul %v33,%v22,%v20
	vsll %v34,%v0,32
	vsll %v35,%v1,32
	vsll %v36,%v2,32
	vsll %v37,%v3,32
	vsll %v38,%v4,32
	vsll %v39,%v5,32
	vsll %v40,%v6,32
	vsll %v41,%v27,32
	vsll %v42,%v28,32
	vsll %v43,%v29,32
	vsll %v44,%v30,32
	vsll %v45,%v31,32
	vsll %v46,%v32,32
	vsll %v47,%v33,32
	vfadd.s %v0,%v0,%v34
	vfadd.s %v1,%v1,%v35
	vfadd.s %v2,%v2,%v36
	vfadd.s %v3,%v3,%v37
	vfadd.s %v4,%v4,%v38
	vfadd.s %v5,%v5,%v39
	vfadd.s %v6,%v6,%v40
	vfadd.s %v27,%v27,%v41
	vfadd.s %v28,%v28,%v42
	vfadd.s %v29,%v29,%v43
	vfadd.s %v30,%v30,%v44
	vfadd.s %v31,%v31,%v45
	vfadd.s %v32,%v32,%v46
	vfadd.s %v33,%v33,%v47
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	lvl %s57
	lea %s58, 56(%s6)
	vstu %v0,4,%s58
	lea %s58, 60(%s6)
	vstu %v1,4,%s58
	lea %s58, 64(%s6)
	vstu %v2,4,%s58
	lea %s58, 68(%s6)
	vstu %v3,4,%s58
	lea %s58, 72(%s6)
	vstu %v4,4,%s58
	lea %s58, 76(%s6)
	vstu %v5,4,%s58
	lea %s58, 80(%s6)
	vstu %v6,4,%s58
	lea %s58, 84(%s6)
	vstu %v27,4,%s58
	lea %s58, 88(%s6)
	vstu %v28,4,%s58
	lea %s58, 92(%s6)
	vstu %v29,4,%s58
	lea %s58, 96(%s6)
	vstu %v30,4,%s58
	lea %s58, 100(%s6)
	vstu %v31,4,%s58
	lea %s58, 104(%s6)
	vstu %v32,4,%s58
	lea %s58, 108(%s6)
	vstu %v33,4,%s58
	lvl %s56
	pvfmul %v0,%v23,%v7
	pvfmul %v1,%v23,%v8
	pvfmul %v2,%v23,%v9
	pvfmul %v3,%v23,%v10
	pvfmul %v4,%v23,%v11
	pvfmul %v5,%v23,%v12
	pvfmul %v6,%v23,%v13
	pvfmul %v27,%v23,%v14
	pvfmul %v28,%v23,%v15
	pvfmul %v29,%v23,%v16
	pvfmul %v30,%v23,%v17
	pvfmul %v31,%v23,%v18
	pvfmul %v32,%v23,%v19
	pvfmul %v33,%v23,%v20
	vsll %v34,%v0,32
	vsll %v35,%v1,32
	vsll %v36,%v2,32
	vsll %v37,%v3,32
	vsll %v38,%v4,32
	vsll %v39,%v5,32
	vsll %v40,%v6,32
	vsll %v41,%v27,32
	vsll %v42,%v28,32
	vsll %v43,%v29,32
	vsll %v44,%v30,32
	vsll %v45,%v31,32
	vsll %v46,%v32,32
	vsll %v47,%v33,32
	vfadd.s %v0,%v0,%v34
	vfadd.s %v1,%v1,%v35
	vfadd.s %v2,%v2,%v36
	vfadd.s %v3,%v3,%v37
	vfadd.s %v4,%v4,%v38
	vfadd.s %v5,%v5,%v39
	vfadd.s %v6,%v6,%v40
	vfadd.s %v27,%v27,%v41
	vfadd.s %v28,%v28,%v42
	vfadd.s %v29,%v29,%v43
	vfadd.s %v30,%v30,%v44
	vfadd.s %v31,%v31,%v45
	vfadd.s %v32,%v32,%v46
	vfadd.s %v33,%v33,%v47
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	lvl %s57
	lea %s58, 112(%s6)
	vstu %v0,4,%s58
	lea %s58, 116(%s6)
	vstu %v1,4,%s58
	lea %s58, 120(%s6)
	vstu %v2,4,%s58
	lea %s58, 124(%s6)
	vstu %v3,4,%s58
	lea %s58, 128(%s6)
	vstu %v4,4,%s58
	lea %s58, 132(%s6)
	vstu %v5,4,%s58
	lea %s58, 136(%s6)
	vstu %v6,4,%s58
	lea %s58, 140(%s6)
	vstu %v27,4,%s58
	lea %s58, 144(%s6)
	vstu %v28,4,%s58
	lea %s58, 148(%s6)
	vstu %v29,4,%s58
	lea %s58, 152(%s6)
	vstu %v30,4,%s58
	lea %s58, 156(%s6)
	vstu %v31,4,%s58
	lea %s58, 160(%s6)
	vstu %v32,4,%s58
	lea %s58, 164(%s6)
	vstu %v33,4,%s58
	lvl %s56
	pvfmul %v0,%v24,%v7
	pvfmul %v1,%v24,%v8
	pvfmul %v2,%v24,%v9
	pvfmul %v3,%v24,%v10
	pvfmul %v4,%v24,%v11
	pvfmul %v5,%v24,%v12
	pvfmul %v6,%v24,%v13
	pvfmul %v27,%v24,%v14
	pvfmul %v28,%v24,%v15
	pvfmul %v29,%v24,%v16
	pvfmul %v30,%v24,%v17
	pvfmul %v31,%v24,%v18
	pvfmul %v32,%v24,%v19
	pvfmul %v33,%v24,%v20
	vsll %v34,%v0,32
	vsll %v35,%v1,32
	vsll %v36,%v2,32
	vsll %v37,%v3,32
	vsll %v38,%v4,32
	vsll %v39,%v5,32
	vsll %v40,%v6,32
	vsll %v41,%v27,32
	vsll %v42,%v28,32
	vsll %v43,%v29,32
	vsll %v44,%v30,32
	vsll %v45,%v31,32
	vsll %v46,%v32,32
	vsll %v47,%v33,32
	vfadd.s %v0,%v0,%v34
	vfadd.s %v1,%v1,%v35
	vfadd.s %v2,%v2,%v36
	vfadd.s %v3,%v3,%v37
	vfadd.s %v4,%v4,%v38
	vfadd.s %v5,%v5,%v39
	vfadd.s %v6,%v6,%v40
	vfadd.s %v27,%v27,%v41
	vfadd.s %v28,%v28,%v42
	vfadd.s %v29,%v29,%v43
	vfadd.s %v30,%v30,%v44
	vfadd.s %v31,%v31,%v45
	vfadd.s %v32,%v32,%v46
	vfadd.s %v33,%v33,%v47
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	lvl %s57
	lea %s58, 168(%s6)
	vstu %v0,4,%s58
	lea %s58, 172(%s6)
	vstu %v1,4,%s58
	lea %s58, 176(%s6)
	vstu %v2,4,%s58
	lea %s58, 180(%s6)
	vstu %v3,4,%s58
	lea %s58, 184(%s6)
	vstu %v4,4,%s58
	lea %s58, 188(%s6)
	vstu %v5,4,%s58
	lea %s58, 192(%s6)
	vstu %v6,4,%s58
	lea %s58, 196(%s6)
	vstu %v27,4,%s58
	lea %s58, 200(%s6)
	vstu %v28,4,%s58
	lea %s58, 204(%s6)
	vstu %v29,4,%s58
	lea %s58, 208(%s6)
	vstu %v30,4,%s58
	lea %s58, 212(%s6)
	vstu %v31,4,%s58
	lea %s58, 216(%s6)
	vstu %v32,4,%s58
	lea %s58, 220(%s6)
	vstu %v33,4,%s58
	lvl %s56
	pvfmul %v0,%v25,%v7
	pvfmul %v1,%v25,%v8
	pvfmul %v2,%v25,%v9
	pvfmul %v3,%v25,%v10
	pvfmul %v4,%v25,%v11
	pvfmul %v5,%v25,%v12
	pvfmul %v6,%v25,%v13
	pvfmul %v27,%v25,%v14
	pvfmul %v28,%v25,%v15
	pvfmul %v29,%v25,%v16
	pvfmul %v30,%v25,%v17
	pvfmul %v31,%v25,%v18
	pvfmul %v32,%v25,%v19
	pvfmul %v33,%v25,%v20
	vsll %v34,%v0,32
	vsll %v35,%v1,32
	vsll %v36,%v2,32
	vsll %v37,%v3,32
	vsll %v38,%v4,32
	vsll %v39,%v5,32
	vsll %v40,%v6,32
	vsll %v41,%v27,32
	vsll %v42,%v28,32
	vsll %v43,%v29,32
	vsll %v44,%v30,32
	vsll %v45,%v31,32
	vsll %v46,%v32,32
	vsll %v47,%v33,32
	vfadd.s %v0,%v0,%v34
	vfadd.s %v1,%v1,%v35
	vfadd.s %v2,%v2,%v36
	vfadd.s %v3,%v3,%v37
	vfadd.s %v4,%v4,%v38
	vfadd.s %v5,%v5,%v39
	vfadd.s %v6,%v6,%v40
	vfadd.s %v27,%v27,%v41
	vfadd.s %v28,%v28,%v42
	vfadd.s %v29,%v29,%v43
	vfadd.s %v30,%v30,%v44
	vfadd.s %v31,%v31,%v45
	vfadd.s %v32,%v32,%v46
	vfadd.s %v33,%v33,%v47
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	lvl %s57
	lea %s58, 224(%s6)
	vstu %v0,4,%s58
	lea %s58, 228(%s6)
	vstu %v1,4,%s58
	lea %s58, 232(%s6)
	vstu %v2,4,%s58
	lea %s58, 236(%s6)
	vstu %v3,4,%s58
	lea %s58, 240(%s6)
	vstu %v4,4,%s58
	lea %s58, 244(%s6)
	vstu %v5,4,%s58
	lea %s58, 248(%s6)
	vstu %v6,4,%s58
	lea %s58, 252(%s6)
	vstu %v27,4,%s58
	lea %s58, 256(%s6)
	vstu %v28,4,%s58
	lea %s58, 260(%s6)
	vstu %v29,4,%s58
	lea %s58, 264(%s6)
	vstu %v30,4,%s58
	lea %s58, 268(%s6)
	vstu %v31,4,%s58
	lea %s58, 272(%s6)
	vstu %v32,4,%s58
	lea %s58, 276(%s6)
	vstu %v33,4,%s58
	lvl %s56
	pvfmul %v0,%v26,%v7
	pvfmul %v1,%v26,%v8
	pvfmul %v2,%v26,%v9
	pvfmul %v3,%v26,%v10
	pvfmul %v4,%v26,%v11
	pvfmul %v5,%v26,%v12
	pvfmul %v6,%v26,%v13
	pvfmul %v27,%v26,%v14
	pvfmul %v28,%v26,%v15
	pvfmul %v29,%v26,%v16
	pvfmul %v30,%v26,%v17
	pvfmul %v31,%v26,%v18
	pvfmul %v32,%v26,%v19
	pvfmul %v33,%v26,%v20
	vsll %v34,%v0,32
	vsll %v35,%v1,32
	vsll %v36,%v2,32
	vsll %v37,%v3,32
	vsll %v38,%v4,32
	vsll %v39,%v5,32
	vsll %v40,%v6,32
	vsll %v41,%v27,32
	vsll %v42,%v28,32
	vsll %v43,%v29,32
	vsll %v44,%v30,32
	vsll %v45,%v31,32
	vsll %v46,%v32,32
	vsll %v47,%v33,32
	vfadd.s %v0,%v0,%v34
	vfadd.s %v1,%v1,%v35
	vfadd.s %v2,%v2,%v36
	vfadd.s %v3,%v3,%v37
	vfadd.s %v4,%v4,%v38
	vfadd.s %v5,%v5,%v39
	vfadd.s %v6,%v6,%v40
	vfadd.s %v27,%v27,%v41
	vfadd.s %v28,%v28,%v42
	vfadd.s %v29,%v29,%v43
	vfadd.s %v30,%v30,%v44
	vfadd.s %v31,%v31,%v45
	vfadd.s %v32,%v32,%v46
	vfadd.s %v33,%v33,%v47
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v27,%v27
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	lvl %s57
	lea %s57, 280(%s6)
	vstu %v0,4,%s57
	lea %s57, 284(%s6)
	vstu %v1,4,%s57
	lea %s57, 288(%s6)
	vstu %v2,4,%s57
	lea %s57, 292(%s6)
	vstu %v3,4,%s57
	lea %s57, 296(%s6)
	vstu %v4,4,%s57
	lea %s57, 300(%s6)
	vstu %v5,4,%s57
	lea %s57, 304(%s6)
	vstu %v6,4,%s57
	lea %s57, 308(%s6)
	vstu %v27,4,%s57
	lea %s57, 312(%s6)
	vstu %v28,4,%s57
	lea %s57, 316(%s6)
	vstu %v29,4,%s57
	lea %s57, 320(%s6)
	vstu %v30,4,%s57
	lea %s57, 324(%s6)
	vstu %v31,4,%s57
	lea %s57, 328(%s6)
	vstu %v32,4,%s57
	lea %s57, 332(%s6)
	vstu %v33,4,%s57
	muls.l %s57, %s2, %s0
	adds.w.sx %s57, %s57, (0)1
	lvl %s57
	lea.sl %s57, .LCPI13_0@hi
	ldu %s57, .LCPI13_0@lo(,%s57)
	vseq %v0
	vbrdu %v27,%s57
	adds.w.sx %s58, %s2, (0)1
	vsubs.w.sx %v0,%s58,%v0
	vldu %v28,4,%s6
	lea.sl %s58, .LCPI13_2@hi
	ldu %s58, .LCPI13_2@lo(,%s58)
	vfmk.w.gt %vm1,%v0
	lea.sl %s59, .LCPI13_1@hi
	ldu %s59, .LCPI13_1@lo(,%s59)
	vfmul.s %v0,%s58,%v28
	lea %s58, __vec_expf@lo
	and %s58, %s58, (32)0
	lea.sl %s58, __vec_expf@hi(%s58)
	vbrdu %v27,%s59,%vm1
	bsic %lr, (,%s58)
	lea.sl %s58, .LCPI13_3@hi
	ldu %s58, .LCPI13_3@lo(,%s58)
	vfadd.s %v0,%s57,%v0
	vfsub.s %v1,%s58,%v28
	lea.sl %s58, .LCPI13_4@hi
	ldu %s58, .LCPI13_4@lo(,%s58)
	vfdiv.s %v0,%s57,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s57,%vm1
	vfsub.s %v1,%s58,%v28
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s59,%vm1
	vfsub.s %v0,%v27,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s56
	lea.sl %s56, .LCPI13_5@hi
	ld %s57, .LCPI13_5@lo(,%s56)
	ldl.zx %s58, (,%s34)
	sll %s63, %s2, 2
	adds.l %s56, %s34, %s63
	ldl.zx %s59, (,%s56)
	vbrd %v0,%s57
	muls.l %s3, %s38, %s58
	muls.l %s4, %s38, %s59
	sll %s62, %s2, 3
	adds.l %s57, %s34, %s62
	ldl.zx %s5, (,%s57)
	muls.l %s0, 12, %s2
	adds.l %s58, %s34, %s0
	ldl.zx %s6, (,%s58)
	sll %s61, %s2, 4
	adds.l %s59, %s34, %s61
	ldl.zx %s7, (,%s59)
	muls.l %s1, 20, %s2
	adds.l %s60, %s34, %s1
	ldl.zx %s2, (,%s60)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s2, %s38, %s2
	pvfmad %v1,%v0,%s3,%v7
	pvfmad %v2,%v0,%s4,%v7
	pvfmad %v3,%v0,%s5,%v7
	pvfmad %v4,%v0,%s6,%v7
	ldl.zx %s3, 4(,%s34)
	lea %s4, 4(%s34, %s63)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v0,%s7,%v7
	pvfmad %v6,%v0,%s2,%v7
	muls.l %s2, %s38, %s3
	muls.l %s3, %s38, %s4
	lea %s4, 4(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 4(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 4(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 4(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v8
	pvfmad %v2,%v2,%s3,%v8
	pvfmad %v3,%v3,%s4,%v8
	pvfmad %v4,%v4,%s5,%v8
	ldl.zx %s2, 8(,%s34)
	lea %s3, 8(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v8
	pvfmad %v6,%v6,%s7,%v8
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 8(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 8(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 8(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 8(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v9
	pvfmad %v2,%v2,%s3,%v9
	pvfmad %v3,%v3,%s4,%v9
	pvfmad %v4,%v4,%s5,%v9
	ldl.zx %s2, 12(,%s34)
	lea %s3, 12(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v9
	pvfmad %v6,%v6,%s7,%v9
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 12(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 12(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 12(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 12(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v10
	pvfmad %v2,%v2,%s3,%v10
	pvfmad %v3,%v3,%s4,%v10
	pvfmad %v4,%v4,%s5,%v10
	ldl.zx %s2, 16(,%s34)
	lea %s3, 16(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v10
	pvfmad %v6,%v6,%s7,%v10
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 16(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 16(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 16(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 16(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v11
	pvfmad %v2,%v2,%s3,%v11
	pvfmad %v3,%v3,%s4,%v11
	pvfmad %v4,%v4,%s5,%v11
	ldl.zx %s2, 20(,%s34)
	lea %s3, 20(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v11
	pvfmad %v6,%v6,%s7,%v11
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 20(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 20(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 20(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 20(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v12
	pvfmad %v2,%v2,%s3,%v12
	pvfmad %v3,%v3,%s4,%v12
	pvfmad %v4,%v4,%s5,%v12
	ldl.zx %s2, 24(,%s34)
	lea %s3, 24(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v12
	pvfmad %v6,%v6,%s7,%v12
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 24(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 24(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 24(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 24(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v13
	pvfmad %v2,%v2,%s3,%v13
	pvfmad %v3,%v3,%s4,%v13
	pvfmad %v4,%v4,%s5,%v13
	ldl.zx %s2, 28(,%s34)
	lea %s3, 28(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v13
	pvfmad %v6,%v6,%s7,%v13
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 28(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 28(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 28(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 28(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v14
	pvfmad %v2,%v2,%s3,%v14
	pvfmad %v3,%v3,%s4,%v14
	pvfmad %v4,%v4,%s5,%v14
	ldl.zx %s2, 32(,%s34)
	lea %s3, 32(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v14
	pvfmad %v6,%v6,%s7,%v14
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 32(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 32(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 32(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 32(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v15
	pvfmad %v2,%v2,%s3,%v15
	pvfmad %v3,%v3,%s4,%v15
	pvfmad %v4,%v4,%s5,%v15
	ldl.zx %s2, 36(,%s34)
	lea %s3, 36(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v15
	pvfmad %v6,%v6,%s7,%v15
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 36(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 36(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 36(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 36(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v16
	pvfmad %v2,%v2,%s3,%v16
	pvfmad %v3,%v3,%s4,%v16
	pvfmad %v4,%v4,%s5,%v16
	ldl.zx %s2, 40(,%s34)
	lea %s3, 40(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v16
	pvfmad %v6,%v6,%s7,%v16
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 40(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 40(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 40(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 40(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v17
	pvfmad %v2,%v2,%s3,%v17
	pvfmad %v3,%v3,%s4,%v17
	pvfmad %v4,%v4,%s5,%v17
	ldl.zx %s2, 44(,%s34)
	lea %s3, 44(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v17
	pvfmad %v6,%v6,%s7,%v17
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 44(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 44(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 44(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 44(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v18
	pvfmad %v2,%v2,%s3,%v18
	pvfmad %v3,%v3,%s4,%v18
	pvfmad %v4,%v4,%s5,%v18
	ldl.zx %s2, 48(,%s34)
	lea %s3, 48(%s34, %s63)
	ldl.zx %s3, (,%s3)
	pvfmad %v5,%v5,%s6,%v18
	pvfmad %v6,%v6,%s7,%v18
	muls.l %s2, %s38, %s2
	muls.l %s3, %s38, %s3
	lea %s4, 48(%s34, %s62)
	ldl.zx %s4, (,%s4)
	lea %s5, 48(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s6, 48(%s34, %s61)
	ldl.zx %s6, (,%s6)
	lea %s7, 48(%s34, %s1)
	ldl.zx %s7, (,%s7)
	muls.l %s4, %s38, %s4
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	pvfmad %v1,%v1,%s2,%v19
	pvfmad %v2,%v2,%s3,%v19
	pvfmad %v3,%v3,%s4,%v19
	pvfmad %v4,%v4,%s5,%v19
	ldl.zx %s2, 52(,%s34)
	lea %s63, 52(%s34, %s63)
	ldl.zx %s63, (,%s63)
	pvfmad %v5,%v5,%s6,%v19
	pvfmad %v6,%v6,%s7,%v19
	muls.l %s2, %s38, %s2
	muls.l %s63, %s38, %s63
	lea %s3, 52(%s34, %s62)
	ldl.zx %s3, (,%s3)
	lea %s0, 52(%s34, %s0)
	ldl.zx %s0, (,%s0)
	lea %s4, 52(%s34, %s61)
	ldl.zx %s4, (,%s4)
	lea %s1, 52(%s34, %s1)
	ldl.zx %s1, (,%s1)
	muls.l %s3, %s38, %s3
	muls.l %s0, %s38, %s0
	muls.l %s4, %s38, %s4
	muls.l %s1, %s38, %s1
	pvfmad %v1,%v1,%s2,%v20
	pvfmad %v2,%v2,%s63,%v20
	pvfmad %v3,%v3,%s3,%v20
	pvfmad %v4,%v4,%s0,%v20
	pvfmad %v5,%v5,%s4,%v20
	pvfmad %v6,%v6,%s1,%v20
	pvfadd %v1,%v21,%v1
	pvfadd %v2,%v22,%v2
	pvfadd %v3,%v23,%v3
	pvfadd %v4,%v24,%v4
	pvfadd %v5,%v25,%v5
	pvfadd %v6,%v26,%v6
	vst %v1,8,%s50
	vst %v2,8,%s51
	vst %v3,8,%s52
	vst %v4,8,%s53
	vst %v5,8,%s54
	vst %v6,8,%s55
	ldl.zx %s50, (,%s34)
	ldl.zx %s51, (,%s56)
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, (,%s57)
	ldl.zx %s53, (,%s58)
	ldl.zx %s54, (,%s59)
	ldl.zx %s55, (,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v1,%v0,%s50,%v21
	pvfmad %v1,%v1,%s51,%v22
	pvfmad %v1,%v1,%s52,%v23
	pvfmad %v1,%v1,%s53,%v24
	ldl.zx %s50, 4(,%s34)
	ldl.zx %s51, 4(,%s56)
	pvfmad %v1,%v1,%s54,%v25
	pvfmad %v1,%v1,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	or %s52, 4, %s62
	adds.l %s52, %s34, %s52
	ldl.zx %s52, (,%s52)
	ldl.zx %s53, 4(,%s58)
	or %s54, 4, %s61
	adds.l %s54, %s34, %s54
	ldl.zx %s54, (,%s54)
	ldl.zx %s55, 4(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v2,%v0,%s50,%v21
	pvfmad %v2,%v2,%s51,%v22
	pvfmad %v2,%v2,%s52,%v23
	pvfmad %v2,%v2,%s53,%v24
	ldl.zx %s50, 8(,%s34)
	ldl.zx %s51, 8(,%s56)
	pvfmad %v2,%v2,%s54,%v25
	pvfmad %v2,%v2,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 8(,%s57)
	ldl.zx %s53, 8(,%s58)
	or %s54, 8, %s61
	adds.l %s54, %s34, %s54
	ldl.zx %s54, (,%s54)
	ldl.zx %s55, 8(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v3,%v0,%s50,%v21
	pvfmad %v3,%v3,%s51,%v22
	pvfmad %v3,%v3,%s52,%v23
	pvfmad %v3,%v3,%s53,%v24
	ldl.zx %s50, 12(,%s34)
	ldl.zx %s51, 12(,%s56)
	pvfmad %v3,%v3,%s54,%v25
	pvfmad %v3,%v3,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 12(,%s57)
	ldl.zx %s53, 12(,%s58)
	or %s54, 12, %s61
	adds.l %s54, %s34, %s54
	ldl.zx %s54, (,%s54)
	ldl.zx %s55, 12(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v4,%v0,%s50,%v21
	pvfmad %v4,%v4,%s51,%v22
	pvfmad %v4,%v4,%s52,%v23
	pvfmad %v4,%v4,%s53,%v24
	ldl.zx %s50, 16(,%s34)
	ldl.zx %s51, 16(,%s56)
	pvfmad %v4,%v4,%s54,%v25
	pvfmad %v4,%v4,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 16(,%s57)
	ldl.zx %s53, 16(,%s58)
	ldl.zx %s54, 16(,%s59)
	ldl.zx %s55, 16(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v5,%v0,%s50,%v21
	pvfmad %v5,%v5,%s51,%v22
	pvfmad %v5,%v5,%s52,%v23
	pvfmad %v5,%v5,%s53,%v24
	ldl.zx %s50, 20(,%s34)
	ldl.zx %s51, 20(,%s56)
	pvfmad %v5,%v5,%s54,%v25
	pvfmad %v5,%v5,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 20(,%s57)
	ldl.zx %s53, 20(,%s58)
	ldl.zx %s54, 20(,%s59)
	ldl.zx %s55, 20(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v6,%v0,%s50,%v21
	pvfmad %v6,%v6,%s51,%v22
	pvfmad %v6,%v6,%s52,%v23
	pvfmad %v6,%v6,%s53,%v24
	ldl.zx %s50, 24(,%s34)
	ldl.zx %s51, 24(,%s56)
	pvfmad %v6,%v6,%s54,%v25
	pvfmad %v6,%v6,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 24(,%s57)
	ldl.zx %s53, 24(,%s58)
	ldl.zx %s54, 24(,%s59)
	ldl.zx %s55, 24(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v27,%v0,%s50,%v21
	pvfmad %v27,%v27,%s51,%v22
	pvfmad %v27,%v27,%s52,%v23
	pvfmad %v27,%v27,%s53,%v24
	ldl.zx %s50, 28(,%s34)
	ldl.zx %s51, 28(,%s56)
	pvfmad %v27,%v27,%s54,%v25
	pvfmad %v27,%v27,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 28(,%s57)
	ldl.zx %s53, 28(,%s58)
	ldl.zx %s54, 28(,%s59)
	ldl.zx %s55, 28(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v28,%v0,%s50,%v21
	pvfmad %v28,%v28,%s51,%v22
	pvfmad %v28,%v28,%s52,%v23
	pvfmad %v28,%v28,%s53,%v24
	ldl.zx %s50, 32(,%s34)
	ldl.zx %s51, 32(,%s56)
	pvfmad %v28,%v28,%s54,%v25
	pvfmad %v28,%v28,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 32(,%s57)
	ldl.zx %s53, 32(,%s58)
	ldl.zx %s54, 32(,%s59)
	ldl.zx %s55, 32(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v29,%v0,%s50,%v21
	pvfmad %v29,%v29,%s51,%v22
	pvfmad %v29,%v29,%s52,%v23
	pvfmad %v29,%v29,%s53,%v24
	ldl.zx %s50, 36(,%s34)
	ldl.zx %s51, 36(,%s56)
	pvfmad %v29,%v29,%s54,%v25
	pvfmad %v29,%v29,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 36(,%s57)
	ldl.zx %s53, 36(,%s58)
	ldl.zx %s54, 36(,%s59)
	ldl.zx %s55, 36(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v30,%v0,%s50,%v21
	pvfmad %v30,%v30,%s51,%v22
	pvfmad %v30,%v30,%s52,%v23
	pvfmad %v30,%v30,%s53,%v24
	ldl.zx %s50, 40(,%s34)
	ldl.zx %s51, 40(,%s56)
	pvfmad %v30,%v30,%s54,%v25
	pvfmad %v30,%v30,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 40(,%s57)
	ldl.zx %s53, 40(,%s58)
	ldl.zx %s54, 40(,%s59)
	ldl.zx %s55, 40(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v31,%v0,%s50,%v21
	pvfmad %v31,%v31,%s51,%v22
	pvfmad %v31,%v31,%s52,%v23
	pvfmad %v31,%v31,%s53,%v24
	ldl.zx %s50, 44(,%s34)
	ldl.zx %s51, 44(,%s56)
	pvfmad %v31,%v31,%s54,%v25
	pvfmad %v31,%v31,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 44(,%s57)
	ldl.zx %s53, 44(,%s58)
	ldl.zx %s54, 44(,%s59)
	ldl.zx %s55, 44(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v32,%v0,%s50,%v21
	pvfmad %v32,%v32,%s51,%v22
	pvfmad %v32,%v32,%s52,%v23
	pvfmad %v32,%v32,%s53,%v24
	ldl.zx %s50, 48(,%s34)
	ldl.zx %s51, 48(,%s56)
	pvfmad %v32,%v32,%s54,%v25
	pvfmad %v32,%v32,%s55,%v26
	muls.l %s50, %s38, %s50
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 48(,%s57)
	ldl.zx %s53, 48(,%s58)
	ldl.zx %s54, 48(,%s59)
	ldl.zx %s55, 48(,%s60)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	pvfmad %v33,%v0,%s50,%v21
	pvfmad %v33,%v33,%s51,%v22
	pvfmad %v33,%v33,%s52,%v23
	pvfmad %v33,%v33,%s53,%v24
	ldl.zx %s34, 52(,%s34)
	ldl.zx %s50, 52(,%s56)
	pvfmad %v33,%v33,%s54,%v25
	pvfmad %v33,%v33,%s55,%v26
	muls.l %s34, %s38, %s34
	muls.l %s50, %s38, %s50
	ldl.zx %s51, 52(,%s57)
	ldl.zx %s52, 52(,%s58)
	ldl.zx %s53, 52(,%s59)
	ldl.zx %s54, 52(,%s60)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s38, %s38, %s54
	pvfmad %v0,%v0,%s34,%v21
	pvfmad %v0,%v0,%s50,%v22
	pvfmad %v0,%v0,%s51,%v23
	pvfmad %v0,%v0,%s52,%v24
	pvfmad %v0,%v0,%s53,%v25
	pvfmad %v0,%v0,%s38,%v26
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v27
	pvfadd %v8,%v14,%v28
	pvfadd %v9,%v15,%v29
	pvfadd %v10,%v16,%v30
	pvfadd %v11,%v17,%v31
	pvfadd %v12,%v18,%v32
	pvfadd %v13,%v19,%v33
	pvfadd %v0,%v20,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s37
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v8,8,%s43
	vst %v9,8,%s44
	vst %v10,8,%s45
	vst %v11,8,%s46
	vst %v12,8,%s47
	vst %v13,8,%s48
	vst %v0,8,%s49
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end13:
	.size	w2v_kernel_N6_HU512_2X_W14, .Lfunc_end13-w2v_kernel_N6_HU512_2X_W14

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI14_0:
	.4byte	1065353216
.LCPI14_1:
	.4byte	0
.LCPI14_2:
	.4byte	3212836864
.LCPI14_3:
	.4byte	1086324736
.LCPI14_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI14_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W15
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W15,@function
w2v_kernel_N6_HU512_2X_W15:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB14_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB14_2:
	st %s18, 48(,%s9)
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s38, -8(,%s9)
	adds.w.sx %s57, %s34, (0)1
	lvl %s57
	ldl.sx %s34, (,%s4)
	ld %s50, 240(,%s9)
	muls.l %s34, %s34, %s1
	ldl.sx %s36, 4(,%s4)
	sll %s34, %s34, 2
	adds.l %s35, %s50, %s34
	ldl.sx %s34, 8(,%s4)
	muls.l %s36, %s36, %s1
	sll %s36, %s36, 2
	adds.l %s36, %s50, %s36
	muls.l %s34, %s34, %s1
	ldl.sx %s39, 12(,%s4)
	sll %s34, %s34, 2
	adds.l %s37, %s50, %s34
	ldl.sx %s34, 16(,%s4)
	muls.l %s39, %s39, %s1
	sll %s39, %s39, 2
	adds.l %s39, %s50, %s39
	muls.l %s34, %s34, %s1
	ldl.sx %s41, 20(,%s4)
	sll %s34, %s34, 2
	adds.l %s40, %s50, %s34
	ldl.sx %s34, 24(,%s4)
	muls.l %s41, %s41, %s1
	sll %s41, %s41, 2
	adds.l %s41, %s50, %s41
	muls.l %s34, %s34, %s1
	ldl.sx %s43, 28(,%s4)
	sll %s34, %s34, 2
	adds.l %s42, %s50, %s34
	ldl.sx %s34, 32(,%s4)
	muls.l %s43, %s43, %s1
	sll %s43, %s43, 2
	adds.l %s43, %s50, %s43
	muls.l %s34, %s34, %s1
	ldl.sx %s45, 36(,%s4)
	sll %s34, %s34, 2
	adds.l %s44, %s50, %s34
	ldl.sx %s34, 40(,%s4)
	muls.l %s45, %s45, %s1
	sll %s45, %s45, 2
	adds.l %s45, %s50, %s45
	muls.l %s34, %s34, %s1
	ldl.sx %s47, 44(,%s4)
	sll %s34, %s34, 2
	adds.l %s46, %s50, %s34
	ldl.sx %s34, 48(,%s4)
	muls.l %s47, %s47, %s1
	sll %s47, %s47, 2
	adds.l %s47, %s50, %s47
	muls.l %s34, %s34, %s1
	ldl.sx %s49, 52(,%s4)
	sll %s34, %s34, 2
	adds.l %s48, %s50, %s34
	ldl.sx %s34, 56(,%s4)
	muls.l %s49, %s49, %s1
	sll %s49, %s49, 2
	adds.l %s49, %s50, %s49
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s50, %s50, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s37
	vld %v10,8,%s39
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	vld %v19,8,%s48
	vld %v20,8,%s49
	vld %v21,8,%s50
	ldl.sx %s34, (,%s5)
	ldl.sx %s52, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s51, %s7, %s34
	muls.l %s34, %s52, %s1
	ldl.sx %s53, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s52, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s53, %s53, %s1
	sll %s53, %s53, 2
	adds.l %s53, %s7, %s53
	muls.l %s34, %s34, %s1
	ldl.sx %s55, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s54, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s55, %s55, %s1
	sll %s55, %s55, 2
	adds.l %s55, %s7, %s55
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s56, %s7, %s34
	vld %v22,8,%s51
	vld %v23,8,%s52
	vld %v24,8,%s53
	vld %v25,8,%s54
	vld %v26,8,%s55
	vld %v27,8,%s56
	lvl %s57
	pvfmul %v0,%v22,%v7
	pvfmul %v1,%v22,%v8
	pvfmul %v2,%v22,%v9
	pvfmul %v3,%v22,%v10
	pvfmul %v4,%v22,%v11
	pvfmul %v5,%v22,%v12
	pvfmul %v6,%v22,%v13
	pvfmul %v28,%v22,%v14
	pvfmul %v29,%v22,%v15
	pvfmul %v30,%v22,%v16
	pvfmul %v31,%v22,%v17
	pvfmul %v32,%v22,%v18
	pvfmul %v33,%v22,%v19
	pvfmul %v34,%v22,%v20
	pvfmul %v35,%v22,%v21
	vsll %v36,%v0,32
	vsll %v37,%v1,32
	vsll %v38,%v2,32
	vsll %v39,%v3,32
	vsll %v40,%v4,32
	vsll %v41,%v5,32
	vsll %v42,%v6,32
	vsll %v43,%v28,32
	vsll %v44,%v29,32
	vsll %v45,%v30,32
	vsll %v46,%v31,32
	vsll %v47,%v32,32
	vsll %v48,%v33,32
	vsll %v49,%v34,32
	vsll %v50,%v35,32
	vfadd.s %v0,%v0,%v36
	vfadd.s %v1,%v1,%v37
	vfadd.s %v2,%v2,%v38
	vfadd.s %v3,%v3,%v39
	vfadd.s %v4,%v4,%v40
	vfadd.s %v5,%v5,%v41
	vfadd.s %v6,%v6,%v42
	vfadd.s %v28,%v28,%v43
	vfadd.s %v29,%v29,%v44
	vfadd.s %v30,%v30,%v45
	vfadd.s %v31,%v31,%v46
	vfadd.s %v32,%v32,%v47
	vfadd.s %v33,%v33,%v48
	vfadd.s %v34,%v34,%v49
	vfadd.s %v35,%v35,%v50
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	or %s58, 1, (0)1
	lvl %s58
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s59, 4(%s6)
	vstu %v1,4,%s59
	lea %s59, 8(%s6)
	vstu %v2,4,%s59
	lea %s59, 12(%s6)
	vstu %v3,4,%s59
	lea %s59, 16(%s6)
	vstu %v4,4,%s59
	lea %s59, 20(%s6)
	vstu %v5,4,%s59
	lea %s59, 24(%s6)
	vstu %v6,4,%s59
	lea %s59, 28(%s6)
	vstu %v28,4,%s59
	lea %s59, 32(%s6)
	vstu %v29,4,%s59
	lea %s59, 36(%s6)
	vstu %v30,4,%s59
	lea %s59, 40(%s6)
	vstu %v31,4,%s59
	lea %s59, 44(%s6)
	vstu %v32,4,%s59
	lea %s59, 48(%s6)
	vstu %v33,4,%s59
	lea %s59, 52(%s6)
	vstu %v34,4,%s59
	lea %s59, 56(%s6)
	vstu %v35,4,%s59
	lvl %s57
	pvfmul %v0,%v23,%v7
	pvfmul %v1,%v23,%v8
	pvfmul %v2,%v23,%v9
	pvfmul %v3,%v23,%v10
	pvfmul %v4,%v23,%v11
	pvfmul %v5,%v23,%v12
	pvfmul %v6,%v23,%v13
	pvfmul %v28,%v23,%v14
	pvfmul %v29,%v23,%v15
	pvfmul %v30,%v23,%v16
	pvfmul %v31,%v23,%v17
	pvfmul %v32,%v23,%v18
	pvfmul %v33,%v23,%v19
	pvfmul %v34,%v23,%v20
	pvfmul %v35,%v23,%v21
	vsll %v36,%v0,32
	vsll %v37,%v1,32
	vsll %v38,%v2,32
	vsll %v39,%v3,32
	vsll %v40,%v4,32
	vsll %v41,%v5,32
	vsll %v42,%v6,32
	vsll %v43,%v28,32
	vsll %v44,%v29,32
	vsll %v45,%v30,32
	vsll %v46,%v31,32
	vsll %v47,%v32,32
	vsll %v48,%v33,32
	vsll %v49,%v34,32
	vsll %v50,%v35,32
	vfadd.s %v0,%v0,%v36
	vfadd.s %v1,%v1,%v37
	vfadd.s %v2,%v2,%v38
	vfadd.s %v3,%v3,%v39
	vfadd.s %v4,%v4,%v40
	vfadd.s %v5,%v5,%v41
	vfadd.s %v6,%v6,%v42
	vfadd.s %v28,%v28,%v43
	vfadd.s %v29,%v29,%v44
	vfadd.s %v30,%v30,%v45
	vfadd.s %v31,%v31,%v46
	vfadd.s %v32,%v32,%v47
	vfadd.s %v33,%v33,%v48
	vfadd.s %v34,%v34,%v49
	vfadd.s %v35,%v35,%v50
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	lvl %s58
	lea %s59, 60(%s6)
	vstu %v0,4,%s59
	lea %s59, 64(%s6)
	vstu %v1,4,%s59
	lea %s59, 68(%s6)
	vstu %v2,4,%s59
	lea %s59, 72(%s6)
	vstu %v3,4,%s59
	lea %s59, 76(%s6)
	vstu %v4,4,%s59
	lea %s59, 80(%s6)
	vstu %v5,4,%s59
	lea %s59, 84(%s6)
	vstu %v6,4,%s59
	lea %s59, 88(%s6)
	vstu %v28,4,%s59
	lea %s59, 92(%s6)
	vstu %v29,4,%s59
	lea %s59, 96(%s6)
	vstu %v30,4,%s59
	lea %s59, 100(%s6)
	vstu %v31,4,%s59
	lea %s59, 104(%s6)
	vstu %v32,4,%s59
	lea %s59, 108(%s6)
	vstu %v33,4,%s59
	lea %s59, 112(%s6)
	vstu %v34,4,%s59
	lea %s59, 116(%s6)
	vstu %v35,4,%s59
	lvl %s57
	pvfmul %v0,%v24,%v7
	pvfmul %v1,%v24,%v8
	pvfmul %v2,%v24,%v9
	pvfmul %v3,%v24,%v10
	pvfmul %v4,%v24,%v11
	pvfmul %v5,%v24,%v12
	pvfmul %v6,%v24,%v13
	pvfmul %v28,%v24,%v14
	pvfmul %v29,%v24,%v15
	pvfmul %v30,%v24,%v16
	pvfmul %v31,%v24,%v17
	pvfmul %v32,%v24,%v18
	pvfmul %v33,%v24,%v19
	pvfmul %v34,%v24,%v20
	pvfmul %v35,%v24,%v21
	vsll %v36,%v0,32
	vsll %v37,%v1,32
	vsll %v38,%v2,32
	vsll %v39,%v3,32
	vsll %v40,%v4,32
	vsll %v41,%v5,32
	vsll %v42,%v6,32
	vsll %v43,%v28,32
	vsll %v44,%v29,32
	vsll %v45,%v30,32
	vsll %v46,%v31,32
	vsll %v47,%v32,32
	vsll %v48,%v33,32
	vsll %v49,%v34,32
	vsll %v50,%v35,32
	vfadd.s %v0,%v0,%v36
	vfadd.s %v1,%v1,%v37
	vfadd.s %v2,%v2,%v38
	vfadd.s %v3,%v3,%v39
	vfadd.s %v4,%v4,%v40
	vfadd.s %v5,%v5,%v41
	vfadd.s %v6,%v6,%v42
	vfadd.s %v28,%v28,%v43
	vfadd.s %v29,%v29,%v44
	vfadd.s %v30,%v30,%v45
	vfadd.s %v31,%v31,%v46
	vfadd.s %v32,%v32,%v47
	vfadd.s %v33,%v33,%v48
	vfadd.s %v34,%v34,%v49
	vfadd.s %v35,%v35,%v50
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	lvl %s58
	lea %s59, 120(%s6)
	vstu %v0,4,%s59
	lea %s59, 124(%s6)
	vstu %v1,4,%s59
	lea %s59, 128(%s6)
	vstu %v2,4,%s59
	lea %s59, 132(%s6)
	vstu %v3,4,%s59
	lea %s59, 136(%s6)
	vstu %v4,4,%s59
	lea %s59, 140(%s6)
	vstu %v5,4,%s59
	lea %s59, 144(%s6)
	vstu %v6,4,%s59
	lea %s59, 148(%s6)
	vstu %v28,4,%s59
	lea %s59, 152(%s6)
	vstu %v29,4,%s59
	lea %s59, 156(%s6)
	vstu %v30,4,%s59
	lea %s59, 160(%s6)
	vstu %v31,4,%s59
	lea %s59, 164(%s6)
	vstu %v32,4,%s59
	lea %s59, 168(%s6)
	vstu %v33,4,%s59
	lea %s59, 172(%s6)
	vstu %v34,4,%s59
	lea %s59, 176(%s6)
	vstu %v35,4,%s59
	lvl %s57
	pvfmul %v0,%v25,%v7
	pvfmul %v1,%v25,%v8
	pvfmul %v2,%v25,%v9
	pvfmul %v3,%v25,%v10
	pvfmul %v4,%v25,%v11
	pvfmul %v5,%v25,%v12
	pvfmul %v6,%v25,%v13
	pvfmul %v28,%v25,%v14
	pvfmul %v29,%v25,%v15
	pvfmul %v30,%v25,%v16
	pvfmul %v31,%v25,%v17
	pvfmul %v32,%v25,%v18
	pvfmul %v33,%v25,%v19
	pvfmul %v34,%v25,%v20
	pvfmul %v35,%v25,%v21
	vsll %v36,%v0,32
	vsll %v37,%v1,32
	vsll %v38,%v2,32
	vsll %v39,%v3,32
	vsll %v40,%v4,32
	vsll %v41,%v5,32
	vsll %v42,%v6,32
	vsll %v43,%v28,32
	vsll %v44,%v29,32
	vsll %v45,%v30,32
	vsll %v46,%v31,32
	vsll %v47,%v32,32
	vsll %v48,%v33,32
	vsll %v49,%v34,32
	vsll %v50,%v35,32
	vfadd.s %v0,%v0,%v36
	vfadd.s %v1,%v1,%v37
	vfadd.s %v2,%v2,%v38
	vfadd.s %v3,%v3,%v39
	vfadd.s %v4,%v4,%v40
	vfadd.s %v5,%v5,%v41
	vfadd.s %v6,%v6,%v42
	vfadd.s %v28,%v28,%v43
	vfadd.s %v29,%v29,%v44
	vfadd.s %v30,%v30,%v45
	vfadd.s %v31,%v31,%v46
	vfadd.s %v32,%v32,%v47
	vfadd.s %v33,%v33,%v48
	vfadd.s %v34,%v34,%v49
	vfadd.s %v35,%v35,%v50
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	lvl %s58
	lea %s59, 180(%s6)
	vstu %v0,4,%s59
	lea %s59, 184(%s6)
	vstu %v1,4,%s59
	lea %s59, 188(%s6)
	vstu %v2,4,%s59
	lea %s59, 192(%s6)
	vstu %v3,4,%s59
	lea %s59, 196(%s6)
	vstu %v4,4,%s59
	lea %s59, 200(%s6)
	vstu %v5,4,%s59
	lea %s59, 204(%s6)
	vstu %v6,4,%s59
	lea %s59, 208(%s6)
	vstu %v28,4,%s59
	lea %s59, 212(%s6)
	vstu %v29,4,%s59
	lea %s59, 216(%s6)
	vstu %v30,4,%s59
	lea %s59, 220(%s6)
	vstu %v31,4,%s59
	lea %s59, 224(%s6)
	vstu %v32,4,%s59
	lea %s59, 228(%s6)
	vstu %v33,4,%s59
	lea %s59, 232(%s6)
	vstu %v34,4,%s59
	lea %s59, 236(%s6)
	vstu %v35,4,%s59
	lvl %s57
	pvfmul %v0,%v26,%v7
	pvfmul %v1,%v26,%v8
	pvfmul %v2,%v26,%v9
	pvfmul %v3,%v26,%v10
	pvfmul %v4,%v26,%v11
	pvfmul %v5,%v26,%v12
	pvfmul %v6,%v26,%v13
	pvfmul %v28,%v26,%v14
	pvfmul %v29,%v26,%v15
	pvfmul %v30,%v26,%v16
	pvfmul %v31,%v26,%v17
	pvfmul %v32,%v26,%v18
	pvfmul %v33,%v26,%v19
	pvfmul %v34,%v26,%v20
	pvfmul %v35,%v26,%v21
	vsll %v36,%v0,32
	vsll %v37,%v1,32
	vsll %v38,%v2,32
	vsll %v39,%v3,32
	vsll %v40,%v4,32
	vsll %v41,%v5,32
	vsll %v42,%v6,32
	vsll %v43,%v28,32
	vsll %v44,%v29,32
	vsll %v45,%v30,32
	vsll %v46,%v31,32
	vsll %v47,%v32,32
	vsll %v48,%v33,32
	vsll %v49,%v34,32
	vsll %v50,%v35,32
	vfadd.s %v0,%v0,%v36
	vfadd.s %v1,%v1,%v37
	vfadd.s %v2,%v2,%v38
	vfadd.s %v3,%v3,%v39
	vfadd.s %v4,%v4,%v40
	vfadd.s %v5,%v5,%v41
	vfadd.s %v6,%v6,%v42
	vfadd.s %v28,%v28,%v43
	vfadd.s %v29,%v29,%v44
	vfadd.s %v30,%v30,%v45
	vfadd.s %v31,%v31,%v46
	vfadd.s %v32,%v32,%v47
	vfadd.s %v33,%v33,%v48
	vfadd.s %v34,%v34,%v49
	vfadd.s %v35,%v35,%v50
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	lvl %s58
	lea %s59, 240(%s6)
	vstu %v0,4,%s59
	lea %s59, 244(%s6)
	vstu %v1,4,%s59
	lea %s59, 248(%s6)
	vstu %v2,4,%s59
	lea %s59, 252(%s6)
	vstu %v3,4,%s59
	lea %s59, 256(%s6)
	vstu %v4,4,%s59
	lea %s59, 260(%s6)
	vstu %v5,4,%s59
	lea %s59, 264(%s6)
	vstu %v6,4,%s59
	lea %s59, 268(%s6)
	vstu %v28,4,%s59
	lea %s59, 272(%s6)
	vstu %v29,4,%s59
	lea %s59, 276(%s6)
	vstu %v30,4,%s59
	lea %s59, 280(%s6)
	vstu %v31,4,%s59
	lea %s59, 284(%s6)
	vstu %v32,4,%s59
	lea %s59, 288(%s6)
	vstu %v33,4,%s59
	lea %s59, 292(%s6)
	vstu %v34,4,%s59
	lea %s59, 296(%s6)
	vstu %v35,4,%s59
	lvl %s57
	pvfmul %v0,%v27,%v7
	pvfmul %v1,%v27,%v8
	pvfmul %v2,%v27,%v9
	pvfmul %v3,%v27,%v10
	pvfmul %v4,%v27,%v11
	pvfmul %v5,%v27,%v12
	pvfmul %v6,%v27,%v13
	pvfmul %v28,%v27,%v14
	pvfmul %v29,%v27,%v15
	pvfmul %v30,%v27,%v16
	pvfmul %v31,%v27,%v17
	pvfmul %v32,%v27,%v18
	pvfmul %v33,%v27,%v19
	pvfmul %v34,%v27,%v20
	pvfmul %v35,%v27,%v21
	vsll %v36,%v0,32
	vsll %v37,%v1,32
	vsll %v38,%v2,32
	vsll %v39,%v3,32
	vsll %v40,%v4,32
	vsll %v41,%v5,32
	vsll %v42,%v6,32
	vsll %v43,%v28,32
	vsll %v44,%v29,32
	vsll %v45,%v30,32
	vsll %v46,%v31,32
	vsll %v47,%v32,32
	vsll %v48,%v33,32
	vsll %v49,%v34,32
	vsll %v50,%v35,32
	vfadd.s %v0,%v0,%v36
	vfadd.s %v1,%v1,%v37
	vfadd.s %v2,%v2,%v38
	vfadd.s %v3,%v3,%v39
	vfadd.s %v4,%v4,%v40
	vfadd.s %v5,%v5,%v41
	vfadd.s %v6,%v6,%v42
	vfadd.s %v28,%v28,%v43
	vfadd.s %v29,%v29,%v44
	vfadd.s %v30,%v30,%v45
	vfadd.s %v31,%v31,%v46
	vfadd.s %v32,%v32,%v47
	vfadd.s %v33,%v33,%v48
	vfadd.s %v34,%v34,%v49
	vfadd.s %v35,%v35,%v50
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v28,%v28
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	lvl %s58
	lea %s58, 300(%s6)
	vstu %v0,4,%s58
	lea %s58, 304(%s6)
	vstu %v1,4,%s58
	lea %s58, 308(%s6)
	vstu %v2,4,%s58
	lea %s58, 312(%s6)
	vstu %v3,4,%s58
	lea %s58, 316(%s6)
	vstu %v4,4,%s58
	lea %s58, 320(%s6)
	vstu %v5,4,%s58
	lea %s58, 324(%s6)
	vstu %v6,4,%s58
	lea %s58, 328(%s6)
	vstu %v28,4,%s58
	lea %s58, 332(%s6)
	vstu %v29,4,%s58
	lea %s58, 336(%s6)
	vstu %v30,4,%s58
	lea %s58, 340(%s6)
	vstu %v31,4,%s58
	lea %s58, 344(%s6)
	vstu %v32,4,%s58
	lea %s58, 348(%s6)
	vstu %v33,4,%s58
	lea %s58, 352(%s6)
	vstu %v34,4,%s58
	lea %s58, 356(%s6)
	vstu %v35,4,%s58
	muls.l %s58, %s2, %s0
	adds.w.sx %s58, %s58, (0)1
	lvl %s58
	lea.sl %s58, .LCPI14_0@hi
	ldu %s58, .LCPI14_0@lo(,%s58)
	vseq %v0
	vbrdu %v28,%s58
	adds.w.sx %s59, %s2, (0)1
	vsubs.w.sx %v0,%s59,%v0
	vldu %v29,4,%s6
	lea.sl %s59, .LCPI14_2@hi
	ldu %s59, .LCPI14_2@lo(,%s59)
	vfmk.w.gt %vm1,%v0
	lea.sl %s60, .LCPI14_1@hi
	ldu %s60, .LCPI14_1@lo(,%s60)
	vfmul.s %v0,%s59,%v29
	lea %s59, __vec_expf@lo
	and %s59, %s59, (32)0
	lea.sl %s59, __vec_expf@hi(%s59)
	vbrdu %v28,%s60,%vm1
	bsic %lr, (,%s59)
	lea.sl %s59, .LCPI14_3@hi
	ldu %s59, .LCPI14_3@lo(,%s59)
	vfadd.s %v0,%s58,%v0
	vfsub.s %v1,%s59,%v29
	lea.sl %s59, .LCPI14_4@hi
	ldu %s59, .LCPI14_4@lo(,%s59)
	vfdiv.s %v0,%s58,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s58,%vm1
	vfsub.s %v1,%s59,%v29
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s60,%vm1
	vfsub.s %v0,%v28,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s57
	lea.sl %s57, .LCPI14_5@hi
	ld %s58, .LCPI14_5@lo(,%s57)
	ldl.zx %s59, (,%s34)
	sll %s0, %s2, 2
	adds.l %s57, %s34, %s0
	ldl.zx %s60, (,%s57)
	vbrd %v0,%s58
	muls.l %s3, %s38, %s59
	muls.l %s4, %s38, %s60
	sll %s63, %s2, 3
	adds.l %s58, %s34, %s63
	ldl.zx %s5, (,%s58)
	muls.l %s1, 12, %s2
	adds.l %s59, %s34, %s1
	ldl.zx %s6, (,%s59)
	sll %s62, %s2, 4
	adds.l %s60, %s34, %s62
	ldl.zx %s7, (,%s60)
	muls.l %s2, 20, %s2
	adds.l %s61, %s34, %s2
	ldl.zx %s18, (,%s61)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v0,%s3,%v7
	pvfmad %v2,%v0,%s4,%v7
	pvfmad %v3,%v0,%s5,%v7
	pvfmad %v4,%v0,%s6,%v7
	ldl.zx %s3, 4(,%s34)
	lea %s4, 4(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v0,%s7,%v7
	pvfmad %v6,%v0,%s18,%v7
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 4(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 4(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 4(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 4(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v8
	pvfmad %v2,%v2,%s4,%v8
	pvfmad %v3,%v3,%s5,%v8
	pvfmad %v4,%v4,%s6,%v8
	ldl.zx %s3, 8(,%s34)
	lea %s4, 8(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v8
	pvfmad %v6,%v6,%s18,%v8
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 8(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 8(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 8(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 8(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v9
	pvfmad %v2,%v2,%s4,%v9
	pvfmad %v3,%v3,%s5,%v9
	pvfmad %v4,%v4,%s6,%v9
	ldl.zx %s3, 12(,%s34)
	lea %s4, 12(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v9
	pvfmad %v6,%v6,%s18,%v9
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 12(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 12(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 12(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 12(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v10
	pvfmad %v2,%v2,%s4,%v10
	pvfmad %v3,%v3,%s5,%v10
	pvfmad %v4,%v4,%s6,%v10
	ldl.zx %s3, 16(,%s34)
	lea %s4, 16(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v10
	pvfmad %v6,%v6,%s18,%v10
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 16(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 16(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 16(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 16(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v11
	pvfmad %v2,%v2,%s4,%v11
	pvfmad %v3,%v3,%s5,%v11
	pvfmad %v4,%v4,%s6,%v11
	ldl.zx %s3, 20(,%s34)
	lea %s4, 20(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v11
	pvfmad %v6,%v6,%s18,%v11
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 20(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 20(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 20(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 20(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v12
	pvfmad %v2,%v2,%s4,%v12
	pvfmad %v3,%v3,%s5,%v12
	pvfmad %v4,%v4,%s6,%v12
	ldl.zx %s3, 24(,%s34)
	lea %s4, 24(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v12
	pvfmad %v6,%v6,%s18,%v12
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 24(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 24(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 24(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 24(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v13
	pvfmad %v2,%v2,%s4,%v13
	pvfmad %v3,%v3,%s5,%v13
	pvfmad %v4,%v4,%s6,%v13
	ldl.zx %s3, 28(,%s34)
	lea %s4, 28(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v13
	pvfmad %v6,%v6,%s18,%v13
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 28(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 28(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 28(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 28(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v14
	pvfmad %v2,%v2,%s4,%v14
	pvfmad %v3,%v3,%s5,%v14
	pvfmad %v4,%v4,%s6,%v14
	ldl.zx %s3, 32(,%s34)
	lea %s4, 32(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v14
	pvfmad %v6,%v6,%s18,%v14
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 32(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 32(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 32(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 32(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v15
	pvfmad %v2,%v2,%s4,%v15
	pvfmad %v3,%v3,%s5,%v15
	pvfmad %v4,%v4,%s6,%v15
	ldl.zx %s3, 36(,%s34)
	lea %s4, 36(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v15
	pvfmad %v6,%v6,%s18,%v15
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 36(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 36(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 36(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 36(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v16
	pvfmad %v2,%v2,%s4,%v16
	pvfmad %v3,%v3,%s5,%v16
	pvfmad %v4,%v4,%s6,%v16
	ldl.zx %s3, 40(,%s34)
	lea %s4, 40(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v16
	pvfmad %v6,%v6,%s18,%v16
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 40(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 40(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 40(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 40(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v17
	pvfmad %v2,%v2,%s4,%v17
	pvfmad %v3,%v3,%s5,%v17
	pvfmad %v4,%v4,%s6,%v17
	ldl.zx %s3, 44(,%s34)
	lea %s4, 44(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v17
	pvfmad %v6,%v6,%s18,%v17
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 44(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 44(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 44(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 44(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v18
	pvfmad %v2,%v2,%s4,%v18
	pvfmad %v3,%v3,%s5,%v18
	pvfmad %v4,%v4,%s6,%v18
	ldl.zx %s3, 48(,%s34)
	lea %s4, 48(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v18
	pvfmad %v6,%v6,%s18,%v18
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 48(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 48(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 48(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 48(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v19
	pvfmad %v2,%v2,%s4,%v19
	pvfmad %v3,%v3,%s5,%v19
	pvfmad %v4,%v4,%s6,%v19
	ldl.zx %s3, 52(,%s34)
	lea %s4, 52(%s34, %s0)
	ldl.zx %s4, (,%s4)
	pvfmad %v5,%v5,%s7,%v19
	pvfmad %v6,%v6,%s18,%v19
	muls.l %s3, %s38, %s3
	muls.l %s4, %s38, %s4
	lea %s5, 52(%s34, %s63)
	ldl.zx %s5, (,%s5)
	lea %s6, 52(%s34, %s1)
	ldl.zx %s6, (,%s6)
	lea %s7, 52(%s34, %s62)
	ldl.zx %s7, (,%s7)
	lea %s18, 52(%s34, %s2)
	ldl.zx %s18, (,%s18)
	muls.l %s5, %s38, %s5
	muls.l %s6, %s38, %s6
	muls.l %s7, %s38, %s7
	muls.l %s18, %s38, %s18
	pvfmad %v1,%v1,%s3,%v20
	pvfmad %v2,%v2,%s4,%v20
	pvfmad %v3,%v3,%s5,%v20
	pvfmad %v4,%v4,%s6,%v20
	ldl.zx %s3, 56(,%s34)
	lea %s0, 56(%s34, %s0)
	ldl.zx %s0, (,%s0)
	pvfmad %v5,%v5,%s7,%v20
	pvfmad %v6,%v6,%s18,%v20
	muls.l %s3, %s38, %s3
	muls.l %s0, %s38, %s0
	lea %s4, 56(%s34, %s63)
	ldl.zx %s4, (,%s4)
	lea %s1, 56(%s34, %s1)
	ldl.zx %s1, (,%s1)
	lea %s5, 56(%s34, %s62)
	ldl.zx %s5, (,%s5)
	lea %s2, 56(%s34, %s2)
	ldl.zx %s2, (,%s2)
	muls.l %s4, %s38, %s4
	muls.l %s1, %s38, %s1
	muls.l %s5, %s38, %s5
	muls.l %s2, %s38, %s2
	pvfmad %v1,%v1,%s3,%v21
	pvfmad %v2,%v2,%s0,%v21
	pvfmad %v3,%v3,%s4,%v21
	pvfmad %v4,%v4,%s1,%v21
	pvfmad %v5,%v5,%s5,%v21
	pvfmad %v6,%v6,%s2,%v21
	pvfadd %v1,%v22,%v1
	pvfadd %v2,%v23,%v2
	pvfadd %v3,%v24,%v3
	pvfadd %v4,%v25,%v4
	pvfadd %v5,%v26,%v5
	pvfadd %v6,%v27,%v6
	vst %v1,8,%s51
	vst %v2,8,%s52
	vst %v3,8,%s53
	vst %v4,8,%s54
	vst %v5,8,%s55
	vst %v6,8,%s56
	ldl.zx %s51, (,%s34)
	ldl.zx %s52, (,%s57)
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, (,%s58)
	ldl.zx %s54, (,%s59)
	ldl.zx %s55, (,%s60)
	ldl.zx %s56, (,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v1,%v0,%s51,%v22
	pvfmad %v1,%v1,%s52,%v23
	pvfmad %v1,%v1,%s53,%v24
	pvfmad %v1,%v1,%s54,%v25
	ldl.zx %s51, 4(,%s34)
	ldl.zx %s52, 4(,%s57)
	pvfmad %v1,%v1,%s55,%v26
	pvfmad %v1,%v1,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	or %s53, 4, %s63
	adds.l %s53, %s34, %s53
	ldl.zx %s53, (,%s53)
	ldl.zx %s54, 4(,%s59)
	or %s55, 4, %s62
	adds.l %s55, %s34, %s55
	ldl.zx %s55, (,%s55)
	ldl.zx %s56, 4(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v2,%v0,%s51,%v22
	pvfmad %v2,%v2,%s52,%v23
	pvfmad %v2,%v2,%s53,%v24
	pvfmad %v2,%v2,%s54,%v25
	ldl.zx %s51, 8(,%s34)
	ldl.zx %s52, 8(,%s57)
	pvfmad %v2,%v2,%s55,%v26
	pvfmad %v2,%v2,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 8(,%s58)
	ldl.zx %s54, 8(,%s59)
	or %s55, 8, %s62
	adds.l %s55, %s34, %s55
	ldl.zx %s55, (,%s55)
	ldl.zx %s56, 8(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v3,%v0,%s51,%v22
	pvfmad %v3,%v3,%s52,%v23
	pvfmad %v3,%v3,%s53,%v24
	pvfmad %v3,%v3,%s54,%v25
	ldl.zx %s51, 12(,%s34)
	ldl.zx %s52, 12(,%s57)
	pvfmad %v3,%v3,%s55,%v26
	pvfmad %v3,%v3,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 12(,%s58)
	ldl.zx %s54, 12(,%s59)
	or %s55, 12, %s62
	adds.l %s55, %s34, %s55
	ldl.zx %s55, (,%s55)
	ldl.zx %s56, 12(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v4,%v0,%s51,%v22
	pvfmad %v4,%v4,%s52,%v23
	pvfmad %v4,%v4,%s53,%v24
	pvfmad %v4,%v4,%s54,%v25
	ldl.zx %s51, 16(,%s34)
	ldl.zx %s52, 16(,%s57)
	pvfmad %v4,%v4,%s55,%v26
	pvfmad %v4,%v4,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 16(,%s58)
	ldl.zx %s54, 16(,%s59)
	ldl.zx %s55, 16(,%s60)
	ldl.zx %s56, 16(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v5,%v0,%s51,%v22
	pvfmad %v5,%v5,%s52,%v23
	pvfmad %v5,%v5,%s53,%v24
	pvfmad %v5,%v5,%s54,%v25
	ldl.zx %s51, 20(,%s34)
	ldl.zx %s52, 20(,%s57)
	pvfmad %v5,%v5,%s55,%v26
	pvfmad %v5,%v5,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 20(,%s58)
	ldl.zx %s54, 20(,%s59)
	ldl.zx %s55, 20(,%s60)
	ldl.zx %s56, 20(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v6,%v0,%s51,%v22
	pvfmad %v6,%v6,%s52,%v23
	pvfmad %v6,%v6,%s53,%v24
	pvfmad %v6,%v6,%s54,%v25
	ldl.zx %s51, 24(,%s34)
	ldl.zx %s52, 24(,%s57)
	pvfmad %v6,%v6,%s55,%v26
	pvfmad %v6,%v6,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 24(,%s58)
	ldl.zx %s54, 24(,%s59)
	ldl.zx %s55, 24(,%s60)
	ldl.zx %s56, 24(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v28,%v0,%s51,%v22
	pvfmad %v28,%v28,%s52,%v23
	pvfmad %v28,%v28,%s53,%v24
	pvfmad %v28,%v28,%s54,%v25
	ldl.zx %s51, 28(,%s34)
	ldl.zx %s52, 28(,%s57)
	pvfmad %v28,%v28,%s55,%v26
	pvfmad %v28,%v28,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 28(,%s58)
	ldl.zx %s54, 28(,%s59)
	ldl.zx %s55, 28(,%s60)
	ldl.zx %s56, 28(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v29,%v0,%s51,%v22
	pvfmad %v29,%v29,%s52,%v23
	pvfmad %v29,%v29,%s53,%v24
	pvfmad %v29,%v29,%s54,%v25
	ldl.zx %s51, 32(,%s34)
	ldl.zx %s52, 32(,%s57)
	pvfmad %v29,%v29,%s55,%v26
	pvfmad %v29,%v29,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 32(,%s58)
	ldl.zx %s54, 32(,%s59)
	ldl.zx %s55, 32(,%s60)
	ldl.zx %s56, 32(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v30,%v0,%s51,%v22
	pvfmad %v30,%v30,%s52,%v23
	pvfmad %v30,%v30,%s53,%v24
	pvfmad %v30,%v30,%s54,%v25
	ldl.zx %s51, 36(,%s34)
	ldl.zx %s52, 36(,%s57)
	pvfmad %v30,%v30,%s55,%v26
	pvfmad %v30,%v30,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 36(,%s58)
	ldl.zx %s54, 36(,%s59)
	ldl.zx %s55, 36(,%s60)
	ldl.zx %s56, 36(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v31,%v0,%s51,%v22
	pvfmad %v31,%v31,%s52,%v23
	pvfmad %v31,%v31,%s53,%v24
	pvfmad %v31,%v31,%s54,%v25
	ldl.zx %s51, 40(,%s34)
	ldl.zx %s52, 40(,%s57)
	pvfmad %v31,%v31,%s55,%v26
	pvfmad %v31,%v31,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 40(,%s58)
	ldl.zx %s54, 40(,%s59)
	ldl.zx %s55, 40(,%s60)
	ldl.zx %s56, 40(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v32,%v0,%s51,%v22
	pvfmad %v32,%v32,%s52,%v23
	pvfmad %v32,%v32,%s53,%v24
	pvfmad %v32,%v32,%s54,%v25
	ldl.zx %s51, 44(,%s34)
	ldl.zx %s52, 44(,%s57)
	pvfmad %v32,%v32,%s55,%v26
	pvfmad %v32,%v32,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 44(,%s58)
	ldl.zx %s54, 44(,%s59)
	ldl.zx %s55, 44(,%s60)
	ldl.zx %s56, 44(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v33,%v0,%s51,%v22
	pvfmad %v33,%v33,%s52,%v23
	pvfmad %v33,%v33,%s53,%v24
	pvfmad %v33,%v33,%s54,%v25
	ldl.zx %s51, 48(,%s34)
	ldl.zx %s52, 48(,%s57)
	pvfmad %v33,%v33,%s55,%v26
	pvfmad %v33,%v33,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 48(,%s58)
	ldl.zx %s54, 48(,%s59)
	ldl.zx %s55, 48(,%s60)
	ldl.zx %s56, 48(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v34,%v0,%s51,%v22
	pvfmad %v34,%v34,%s52,%v23
	pvfmad %v34,%v34,%s53,%v24
	pvfmad %v34,%v34,%s54,%v25
	ldl.zx %s51, 52(,%s34)
	ldl.zx %s52, 52(,%s57)
	pvfmad %v34,%v34,%s55,%v26
	pvfmad %v34,%v34,%s56,%v27
	muls.l %s51, %s38, %s51
	muls.l %s52, %s38, %s52
	ldl.zx %s53, 52(,%s58)
	ldl.zx %s54, 52(,%s59)
	ldl.zx %s55, 52(,%s60)
	ldl.zx %s56, 52(,%s61)
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s55, %s38, %s55
	muls.l %s56, %s38, %s56
	pvfmad %v35,%v0,%s51,%v22
	pvfmad %v35,%v35,%s52,%v23
	pvfmad %v35,%v35,%s53,%v24
	pvfmad %v35,%v35,%s54,%v25
	ldl.zx %s34, 56(,%s34)
	ldl.zx %s51, 56(,%s57)
	pvfmad %v35,%v35,%s55,%v26
	pvfmad %v35,%v35,%s56,%v27
	muls.l %s34, %s38, %s34
	muls.l %s51, %s38, %s51
	ldl.zx %s52, 56(,%s58)
	ldl.zx %s53, 56(,%s59)
	ldl.zx %s54, 56(,%s60)
	ldl.zx %s55, 56(,%s61)
	muls.l %s52, %s38, %s52
	muls.l %s53, %s38, %s53
	muls.l %s54, %s38, %s54
	muls.l %s38, %s38, %s55
	pvfmad %v0,%v0,%s34,%v22
	pvfmad %v0,%v0,%s51,%v23
	pvfmad %v0,%v0,%s52,%v24
	pvfmad %v0,%v0,%s53,%v25
	pvfmad %v0,%v0,%s54,%v26
	pvfmad %v0,%v0,%s38,%v27
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v28
	pvfadd %v8,%v14,%v29
	pvfadd %v9,%v15,%v30
	pvfadd %v10,%v16,%v31
	pvfadd %v11,%v17,%v32
	pvfadd %v12,%v18,%v33
	pvfadd %v13,%v19,%v34
	pvfadd %v14,%v20,%v35
	pvfadd %v0,%v21,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s37
	vst %v4,8,%s39
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v8,8,%s43
	vst %v9,8,%s44
	vst %v10,8,%s45
	vst %v11,8,%s46
	vst %v12,8,%s47
	vst %v13,8,%s48
	vst %v14,8,%s49
	vst %v0,8,%s50
	ld %s18, 48(,%s9)
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end14:
	.size	w2v_kernel_N6_HU512_2X_W15, .Lfunc_end14-w2v_kernel_N6_HU512_2X_W15

	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2
.LCPI15_0:
	.4byte	1065353216
.LCPI15_1:
	.4byte	0
.LCPI15_2:
	.4byte	3212836864
.LCPI15_3:
	.4byte	1086324736
.LCPI15_4:
	.4byte	3233808384
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3
.LCPI15_5:
	.8byte	0
	.text
	.globl	w2v_kernel_N6_HU512_2X_W16
	.p2align	4
	.type	w2v_kernel_N6_HU512_2X_W16,@function
w2v_kernel_N6_HU512_2X_W16:
	st %s9, (,%s11)
	st %s10, 8(,%s11)
	st %s15, 24(,%s11)
	st %s16, 32(,%s11)
	or %s9, 0, %s11
	lea %s13, -192
	and %s13, %s13, (32)0
	lea.sl %s11, -1(%s11, %s13)
	brge.l %s11, %s8, .LBB15_2
	ld %s61, 24(,%s14)
	or %s62, 0, %s0
	lea %s63, 315
	shm.l %s63, (%s61)
	shm.l %s8, 8(%s61)
	shm.l %s11, 16(%s61)
	monc
	or %s0, 0, %s62
.LBB15_2:
	st %s18, 48(,%s9)
	st %s19, 56(,%s9)
	srl %s34, %s1, 1
	lea %s35, 1
	and %s35, %s35, (32)0
	lea.sl %s35, 1(%s35)
	st %s35, -8(,%s9)
	ld %s39, -8(,%s9)
	adds.w.sx %s58, %s34, (0)1
	lvl %s58
	ldl.sx %s34, (,%s4)
	ld %s51, 240(,%s9)
	ldl.sx %s36, 4(,%s4)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s35, %s51, %s34
	muls.l %s34, %s36, %s1
	ldl.sx %s37, 8(,%s4)
	sll %s34, %s34, 2
	adds.l %s36, %s51, %s34
	ldl.sx %s34, 12(,%s4)
	muls.l %s37, %s37, %s1
	sll %s37, %s37, 2
	adds.l %s37, %s51, %s37
	muls.l %s34, %s34, %s1
	ldl.sx %s40, 16(,%s4)
	sll %s34, %s34, 2
	adds.l %s38, %s51, %s34
	ldl.sx %s34, 20(,%s4)
	muls.l %s40, %s40, %s1
	sll %s40, %s40, 2
	adds.l %s40, %s51, %s40
	muls.l %s34, %s34, %s1
	ldl.sx %s42, 24(,%s4)
	sll %s34, %s34, 2
	adds.l %s41, %s51, %s34
	ldl.sx %s34, 28(,%s4)
	muls.l %s42, %s42, %s1
	sll %s42, %s42, 2
	adds.l %s42, %s51, %s42
	muls.l %s34, %s34, %s1
	ldl.sx %s44, 32(,%s4)
	sll %s34, %s34, 2
	adds.l %s43, %s51, %s34
	ldl.sx %s34, 36(,%s4)
	muls.l %s44, %s44, %s1
	sll %s44, %s44, 2
	adds.l %s44, %s51, %s44
	muls.l %s34, %s34, %s1
	ldl.sx %s46, 40(,%s4)
	sll %s34, %s34, 2
	adds.l %s45, %s51, %s34
	ldl.sx %s34, 44(,%s4)
	muls.l %s46, %s46, %s1
	sll %s46, %s46, 2
	adds.l %s46, %s51, %s46
	muls.l %s34, %s34, %s1
	ldl.sx %s48, 48(,%s4)
	sll %s34, %s34, 2
	adds.l %s47, %s51, %s34
	ldl.sx %s34, 52(,%s4)
	muls.l %s48, %s48, %s1
	sll %s48, %s48, 2
	adds.l %s48, %s51, %s48
	muls.l %s34, %s34, %s1
	ldl.sx %s50, 56(,%s4)
	sll %s34, %s34, 2
	adds.l %s49, %s51, %s34
	ldl.sx %s34, 60(,%s4)
	muls.l %s50, %s50, %s1
	sll %s50, %s50, 2
	adds.l %s50, %s51, %s50
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s51, %s51, %s34
	vld %v7,8,%s35
	vld %v8,8,%s36
	vld %v9,8,%s37
	vld %v10,8,%s38
	vld %v11,8,%s40
	vld %v12,8,%s41
	vld %v13,8,%s42
	vld %v14,8,%s43
	vld %v15,8,%s44
	vld %v16,8,%s45
	vld %v17,8,%s46
	vld %v18,8,%s47
	vld %v19,8,%s48
	vld %v20,8,%s49
	vld %v21,8,%s50
	vld %v22,8,%s51
	ldl.sx %s34, (,%s5)
	ldl.sx %s53, 4(,%s5)
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s52, %s7, %s34
	muls.l %s34, %s53, %s1
	ldl.sx %s54, 8(,%s5)
	sll %s34, %s34, 2
	adds.l %s53, %s7, %s34
	ldl.sx %s34, 12(,%s5)
	muls.l %s54, %s54, %s1
	sll %s54, %s54, 2
	adds.l %s54, %s7, %s54
	muls.l %s34, %s34, %s1
	ldl.sx %s56, 16(,%s5)
	sll %s34, %s34, 2
	adds.l %s55, %s7, %s34
	ldl.sx %s34, 20(,%s5)
	muls.l %s56, %s56, %s1
	sll %s56, %s56, 2
	adds.l %s56, %s7, %s56
	muls.l %s34, %s34, %s1
	sll %s34, %s34, 2
	adds.l %s57, %s7, %s34
	vld %v23,8,%s52
	vld %v24,8,%s53
	vld %v25,8,%s54
	vld %v26,8,%s55
	vld %v27,8,%s56
	vld %v28,8,%s57
	lvl %s58
	pvfmul %v0,%v23,%v7
	pvfmul %v1,%v23,%v8
	pvfmul %v2,%v23,%v9
	pvfmul %v3,%v23,%v10
	pvfmul %v4,%v23,%v11
	pvfmul %v5,%v23,%v12
	pvfmul %v6,%v23,%v13
	pvfmul %v29,%v23,%v14
	pvfmul %v30,%v23,%v15
	pvfmul %v31,%v23,%v16
	pvfmul %v32,%v23,%v17
	pvfmul %v33,%v23,%v18
	pvfmul %v34,%v23,%v19
	pvfmul %v35,%v23,%v20
	pvfmul %v36,%v23,%v21
	pvfmul %v37,%v23,%v22
	vsll %v38,%v0,32
	vsll %v39,%v1,32
	vsll %v40,%v2,32
	vsll %v41,%v3,32
	vsll %v42,%v4,32
	vsll %v43,%v5,32
	vsll %v44,%v6,32
	vsll %v45,%v29,32
	vsll %v46,%v30,32
	vsll %v47,%v31,32
	vsll %v48,%v32,32
	vsll %v49,%v33,32
	vsll %v50,%v34,32
	vsll %v51,%v35,32
	vsll %v52,%v36,32
	vsll %v53,%v37,32
	vfadd.s %v0,%v0,%v38
	vfadd.s %v1,%v1,%v39
	vfadd.s %v2,%v2,%v40
	vfadd.s %v3,%v3,%v41
	vfadd.s %v4,%v4,%v42
	vfadd.s %v5,%v5,%v43
	vfadd.s %v6,%v6,%v44
	vfadd.s %v29,%v29,%v45
	vfadd.s %v30,%v30,%v46
	vfadd.s %v31,%v31,%v47
	vfadd.s %v32,%v32,%v48
	vfadd.s %v33,%v33,%v49
	vfadd.s %v34,%v34,%v50
	vfadd.s %v35,%v35,%v51
	vfadd.s %v36,%v36,%v52
	vfadd.s %v37,%v37,%v53
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	vfsum.s %v36,%v36
	vfsum.s %v37,%v37
	or %s59, 1, (0)1
	lvl %s59
	or %s34, 0, %s6
	vstu %v0,4,%s6
	lea %s60, 4(%s6)
	vstu %v1,4,%s60
	lea %s60, 8(%s6)
	vstu %v2,4,%s60
	lea %s60, 12(%s6)
	vstu %v3,4,%s60
	lea %s60, 16(%s6)
	vstu %v4,4,%s60
	lea %s60, 20(%s6)
	vstu %v5,4,%s60
	lea %s60, 24(%s6)
	vstu %v6,4,%s60
	lea %s60, 28(%s6)
	vstu %v29,4,%s60
	lea %s60, 32(%s6)
	vstu %v30,4,%s60
	lea %s60, 36(%s6)
	vstu %v31,4,%s60
	lea %s60, 40(%s6)
	vstu %v32,4,%s60
	lea %s60, 44(%s6)
	vstu %v33,4,%s60
	lea %s60, 48(%s6)
	vstu %v34,4,%s60
	lea %s60, 52(%s6)
	vstu %v35,4,%s60
	lea %s60, 56(%s6)
	vstu %v36,4,%s60
	lea %s60, 60(%s6)
	vstu %v37,4,%s60
	lvl %s58
	pvfmul %v0,%v24,%v7
	pvfmul %v1,%v24,%v8
	pvfmul %v2,%v24,%v9
	pvfmul %v3,%v24,%v10
	pvfmul %v4,%v24,%v11
	pvfmul %v5,%v24,%v12
	pvfmul %v6,%v24,%v13
	pvfmul %v29,%v24,%v14
	pvfmul %v30,%v24,%v15
	pvfmul %v31,%v24,%v16
	pvfmul %v32,%v24,%v17
	pvfmul %v33,%v24,%v18
	pvfmul %v34,%v24,%v19
	pvfmul %v35,%v24,%v20
	pvfmul %v36,%v24,%v21
	pvfmul %v37,%v24,%v22
	vsll %v38,%v0,32
	vsll %v39,%v1,32
	vsll %v40,%v2,32
	vsll %v41,%v3,32
	vsll %v42,%v4,32
	vsll %v43,%v5,32
	vsll %v44,%v6,32
	vsll %v45,%v29,32
	vsll %v46,%v30,32
	vsll %v47,%v31,32
	vsll %v48,%v32,32
	vsll %v49,%v33,32
	vsll %v50,%v34,32
	vsll %v51,%v35,32
	vsll %v52,%v36,32
	vsll %v53,%v37,32
	vfadd.s %v0,%v0,%v38
	vfadd.s %v1,%v1,%v39
	vfadd.s %v2,%v2,%v40
	vfadd.s %v3,%v3,%v41
	vfadd.s %v4,%v4,%v42
	vfadd.s %v5,%v5,%v43
	vfadd.s %v6,%v6,%v44
	vfadd.s %v29,%v29,%v45
	vfadd.s %v30,%v30,%v46
	vfadd.s %v31,%v31,%v47
	vfadd.s %v32,%v32,%v48
	vfadd.s %v33,%v33,%v49
	vfadd.s %v34,%v34,%v50
	vfadd.s %v35,%v35,%v51
	vfadd.s %v36,%v36,%v52
	vfadd.s %v37,%v37,%v53
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	vfsum.s %v36,%v36
	vfsum.s %v37,%v37
	lvl %s59
	lea %s60, 64(%s6)
	vstu %v0,4,%s60
	lea %s60, 68(%s6)
	vstu %v1,4,%s60
	lea %s60, 72(%s6)
	vstu %v2,4,%s60
	lea %s60, 76(%s6)
	vstu %v3,4,%s60
	lea %s60, 80(%s6)
	vstu %v4,4,%s60
	lea %s60, 84(%s6)
	vstu %v5,4,%s60
	lea %s60, 88(%s6)
	vstu %v6,4,%s60
	lea %s60, 92(%s6)
	vstu %v29,4,%s60
	lea %s60, 96(%s6)
	vstu %v30,4,%s60
	lea %s60, 100(%s6)
	vstu %v31,4,%s60
	lea %s60, 104(%s6)
	vstu %v32,4,%s60
	lea %s60, 108(%s6)
	vstu %v33,4,%s60
	lea %s60, 112(%s6)
	vstu %v34,4,%s60
	lea %s60, 116(%s6)
	vstu %v35,4,%s60
	lea %s60, 120(%s6)
	vstu %v36,4,%s60
	lea %s60, 124(%s6)
	vstu %v37,4,%s60
	lvl %s58
	pvfmul %v0,%v25,%v7
	pvfmul %v1,%v25,%v8
	pvfmul %v2,%v25,%v9
	pvfmul %v3,%v25,%v10
	pvfmul %v4,%v25,%v11
	pvfmul %v5,%v25,%v12
	pvfmul %v6,%v25,%v13
	pvfmul %v29,%v25,%v14
	pvfmul %v30,%v25,%v15
	pvfmul %v31,%v25,%v16
	pvfmul %v32,%v25,%v17
	pvfmul %v33,%v25,%v18
	pvfmul %v34,%v25,%v19
	pvfmul %v35,%v25,%v20
	pvfmul %v36,%v25,%v21
	pvfmul %v37,%v25,%v22
	vsll %v38,%v0,32
	vsll %v39,%v1,32
	vsll %v40,%v2,32
	vsll %v41,%v3,32
	vsll %v42,%v4,32
	vsll %v43,%v5,32
	vsll %v44,%v6,32
	vsll %v45,%v29,32
	vsll %v46,%v30,32
	vsll %v47,%v31,32
	vsll %v48,%v32,32
	vsll %v49,%v33,32
	vsll %v50,%v34,32
	vsll %v51,%v35,32
	vsll %v52,%v36,32
	vsll %v53,%v37,32
	vfadd.s %v0,%v0,%v38
	vfadd.s %v1,%v1,%v39
	vfadd.s %v2,%v2,%v40
	vfadd.s %v3,%v3,%v41
	vfadd.s %v4,%v4,%v42
	vfadd.s %v5,%v5,%v43
	vfadd.s %v6,%v6,%v44
	vfadd.s %v29,%v29,%v45
	vfadd.s %v30,%v30,%v46
	vfadd.s %v31,%v31,%v47
	vfadd.s %v32,%v32,%v48
	vfadd.s %v33,%v33,%v49
	vfadd.s %v34,%v34,%v50
	vfadd.s %v35,%v35,%v51
	vfadd.s %v36,%v36,%v52
	vfadd.s %v37,%v37,%v53
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	vfsum.s %v36,%v36
	vfsum.s %v37,%v37
	lvl %s59
	lea %s60, 128(%s6)
	vstu %v0,4,%s60
	lea %s60, 132(%s6)
	vstu %v1,4,%s60
	lea %s60, 136(%s6)
	vstu %v2,4,%s60
	lea %s60, 140(%s6)
	vstu %v3,4,%s60
	lea %s60, 144(%s6)
	vstu %v4,4,%s60
	lea %s60, 148(%s6)
	vstu %v5,4,%s60
	lea %s60, 152(%s6)
	vstu %v6,4,%s60
	lea %s60, 156(%s6)
	vstu %v29,4,%s60
	lea %s60, 160(%s6)
	vstu %v30,4,%s60
	lea %s60, 164(%s6)
	vstu %v31,4,%s60
	lea %s60, 168(%s6)
	vstu %v32,4,%s60
	lea %s60, 172(%s6)
	vstu %v33,4,%s60
	lea %s60, 176(%s6)
	vstu %v34,4,%s60
	lea %s60, 180(%s6)
	vstu %v35,4,%s60
	lea %s60, 184(%s6)
	vstu %v36,4,%s60
	lea %s60, 188(%s6)
	vstu %v37,4,%s60
	lvl %s58
	pvfmul %v0,%v26,%v7
	pvfmul %v1,%v26,%v8
	pvfmul %v2,%v26,%v9
	pvfmul %v3,%v26,%v10
	pvfmul %v4,%v26,%v11
	pvfmul %v5,%v26,%v12
	pvfmul %v6,%v26,%v13
	pvfmul %v29,%v26,%v14
	pvfmul %v30,%v26,%v15
	pvfmul %v31,%v26,%v16
	pvfmul %v32,%v26,%v17
	pvfmul %v33,%v26,%v18
	pvfmul %v34,%v26,%v19
	pvfmul %v35,%v26,%v20
	pvfmul %v36,%v26,%v21
	pvfmul %v37,%v26,%v22
	vsll %v38,%v0,32
	vsll %v39,%v1,32
	vsll %v40,%v2,32
	vsll %v41,%v3,32
	vsll %v42,%v4,32
	vsll %v43,%v5,32
	vsll %v44,%v6,32
	vsll %v45,%v29,32
	vsll %v46,%v30,32
	vsll %v47,%v31,32
	vsll %v48,%v32,32
	vsll %v49,%v33,32
	vsll %v50,%v34,32
	vsll %v51,%v35,32
	vsll %v52,%v36,32
	vsll %v53,%v37,32
	vfadd.s %v0,%v0,%v38
	vfadd.s %v1,%v1,%v39
	vfadd.s %v2,%v2,%v40
	vfadd.s %v3,%v3,%v41
	vfadd.s %v4,%v4,%v42
	vfadd.s %v5,%v5,%v43
	vfadd.s %v6,%v6,%v44
	vfadd.s %v29,%v29,%v45
	vfadd.s %v30,%v30,%v46
	vfadd.s %v31,%v31,%v47
	vfadd.s %v32,%v32,%v48
	vfadd.s %v33,%v33,%v49
	vfadd.s %v34,%v34,%v50
	vfadd.s %v35,%v35,%v51
	vfadd.s %v36,%v36,%v52
	vfadd.s %v37,%v37,%v53
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	vfsum.s %v36,%v36
	vfsum.s %v37,%v37
	lvl %s59
	lea %s60, 192(%s6)
	vstu %v0,4,%s60
	lea %s60, 196(%s6)
	vstu %v1,4,%s60
	lea %s60, 200(%s6)
	vstu %v2,4,%s60
	lea %s60, 204(%s6)
	vstu %v3,4,%s60
	lea %s60, 208(%s6)
	vstu %v4,4,%s60
	lea %s60, 212(%s6)
	vstu %v5,4,%s60
	lea %s60, 216(%s6)
	vstu %v6,4,%s60
	lea %s60, 220(%s6)
	vstu %v29,4,%s60
	lea %s60, 224(%s6)
	vstu %v30,4,%s60
	lea %s60, 228(%s6)
	vstu %v31,4,%s60
	lea %s60, 232(%s6)
	vstu %v32,4,%s60
	lea %s60, 236(%s6)
	vstu %v33,4,%s60
	lea %s60, 240(%s6)
	vstu %v34,4,%s60
	lea %s60, 244(%s6)
	vstu %v35,4,%s60
	lea %s60, 248(%s6)
	vstu %v36,4,%s60
	lea %s60, 252(%s6)
	vstu %v37,4,%s60
	lvl %s58
	pvfmul %v0,%v27,%v7
	pvfmul %v1,%v27,%v8
	pvfmul %v2,%v27,%v9
	pvfmul %v3,%v27,%v10
	pvfmul %v4,%v27,%v11
	pvfmul %v5,%v27,%v12
	pvfmul %v6,%v27,%v13
	pvfmul %v29,%v27,%v14
	pvfmul %v30,%v27,%v15
	pvfmul %v31,%v27,%v16
	pvfmul %v32,%v27,%v17
	pvfmul %v33,%v27,%v18
	pvfmul %v34,%v27,%v19
	pvfmul %v35,%v27,%v20
	pvfmul %v36,%v27,%v21
	pvfmul %v37,%v27,%v22
	vsll %v38,%v0,32
	vsll %v39,%v1,32
	vsll %v40,%v2,32
	vsll %v41,%v3,32
	vsll %v42,%v4,32
	vsll %v43,%v5,32
	vsll %v44,%v6,32
	vsll %v45,%v29,32
	vsll %v46,%v30,32
	vsll %v47,%v31,32
	vsll %v48,%v32,32
	vsll %v49,%v33,32
	vsll %v50,%v34,32
	vsll %v51,%v35,32
	vsll %v52,%v36,32
	vsll %v53,%v37,32
	vfadd.s %v0,%v0,%v38
	vfadd.s %v1,%v1,%v39
	vfadd.s %v2,%v2,%v40
	vfadd.s %v3,%v3,%v41
	vfadd.s %v4,%v4,%v42
	vfadd.s %v5,%v5,%v43
	vfadd.s %v6,%v6,%v44
	vfadd.s %v29,%v29,%v45
	vfadd.s %v30,%v30,%v46
	vfadd.s %v31,%v31,%v47
	vfadd.s %v32,%v32,%v48
	vfadd.s %v33,%v33,%v49
	vfadd.s %v34,%v34,%v50
	vfadd.s %v35,%v35,%v51
	vfadd.s %v36,%v36,%v52
	vfadd.s %v37,%v37,%v53
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	vfsum.s %v36,%v36
	vfsum.s %v37,%v37
	lvl %s59
	lea %s60, 256(%s6)
	vstu %v0,4,%s60
	lea %s60, 260(%s6)
	vstu %v1,4,%s60
	lea %s60, 264(%s6)
	vstu %v2,4,%s60
	lea %s60, 268(%s6)
	vstu %v3,4,%s60
	lea %s60, 272(%s6)
	vstu %v4,4,%s60
	lea %s60, 276(%s6)
	vstu %v5,4,%s60
	lea %s60, 280(%s6)
	vstu %v6,4,%s60
	lea %s60, 284(%s6)
	vstu %v29,4,%s60
	lea %s60, 288(%s6)
	vstu %v30,4,%s60
	lea %s60, 292(%s6)
	vstu %v31,4,%s60
	lea %s60, 296(%s6)
	vstu %v32,4,%s60
	lea %s60, 300(%s6)
	vstu %v33,4,%s60
	lea %s60, 304(%s6)
	vstu %v34,4,%s60
	lea %s60, 308(%s6)
	vstu %v35,4,%s60
	lea %s60, 312(%s6)
	vstu %v36,4,%s60
	lea %s60, 316(%s6)
	vstu %v37,4,%s60
	lvl %s58
	pvfmul %v0,%v28,%v7
	pvfmul %v1,%v28,%v8
	pvfmul %v2,%v28,%v9
	pvfmul %v3,%v28,%v10
	pvfmul %v4,%v28,%v11
	pvfmul %v5,%v28,%v12
	pvfmul %v6,%v28,%v13
	pvfmul %v29,%v28,%v14
	pvfmul %v30,%v28,%v15
	pvfmul %v31,%v28,%v16
	pvfmul %v32,%v28,%v17
	pvfmul %v33,%v28,%v18
	pvfmul %v34,%v28,%v19
	pvfmul %v35,%v28,%v20
	pvfmul %v36,%v28,%v21
	pvfmul %v37,%v28,%v22
	vsll %v38,%v0,32
	vsll %v39,%v1,32
	vsll %v40,%v2,32
	vsll %v41,%v3,32
	vsll %v42,%v4,32
	vsll %v43,%v5,32
	vsll %v44,%v6,32
	vsll %v45,%v29,32
	vsll %v46,%v30,32
	vsll %v47,%v31,32
	vsll %v48,%v32,32
	vsll %v49,%v33,32
	vsll %v50,%v34,32
	vsll %v51,%v35,32
	vsll %v52,%v36,32
	vsll %v53,%v37,32
	vfadd.s %v0,%v0,%v38
	vfadd.s %v1,%v1,%v39
	vfadd.s %v2,%v2,%v40
	vfadd.s %v3,%v3,%v41
	vfadd.s %v4,%v4,%v42
	vfadd.s %v5,%v5,%v43
	vfadd.s %v6,%v6,%v44
	vfadd.s %v29,%v29,%v45
	vfadd.s %v30,%v30,%v46
	vfadd.s %v31,%v31,%v47
	vfadd.s %v32,%v32,%v48
	vfadd.s %v33,%v33,%v49
	vfadd.s %v34,%v34,%v50
	vfadd.s %v35,%v35,%v51
	vfadd.s %v36,%v36,%v52
	vfadd.s %v37,%v37,%v53
	vfsum.s %v0,%v0
	vfsum.s %v1,%v1
	vfsum.s %v2,%v2
	vfsum.s %v3,%v3
	vfsum.s %v4,%v4
	vfsum.s %v5,%v5
	vfsum.s %v6,%v6
	vfsum.s %v29,%v29
	vfsum.s %v30,%v30
	vfsum.s %v31,%v31
	vfsum.s %v32,%v32
	vfsum.s %v33,%v33
	vfsum.s %v34,%v34
	vfsum.s %v35,%v35
	vfsum.s %v36,%v36
	vfsum.s %v37,%v37
	lvl %s59
	lea %s59, 320(%s6)
	vstu %v0,4,%s59
	lea %s59, 324(%s6)
	vstu %v1,4,%s59
	lea %s59, 328(%s6)
	vstu %v2,4,%s59
	lea %s59, 332(%s6)
	vstu %v3,4,%s59
	lea %s59, 336(%s6)
	vstu %v4,4,%s59
	lea %s59, 340(%s6)
	vstu %v5,4,%s59
	lea %s59, 344(%s6)
	vstu %v6,4,%s59
	lea %s59, 348(%s6)
	vstu %v29,4,%s59
	lea %s59, 352(%s6)
	vstu %v30,4,%s59
	lea %s59, 356(%s6)
	vstu %v31,4,%s59
	lea %s59, 360(%s6)
	vstu %v32,4,%s59
	lea %s59, 364(%s6)
	vstu %v33,4,%s59
	lea %s59, 368(%s6)
	vstu %v34,4,%s59
	lea %s59, 372(%s6)
	vstu %v35,4,%s59
	lea %s59, 376(%s6)
	vstu %v36,4,%s59
	lea %s59, 380(%s6)
	vstu %v37,4,%s59
	muls.l %s59, %s2, %s0
	adds.w.sx %s59, %s59, (0)1
	lvl %s59
	lea.sl %s59, .LCPI15_0@hi
	ldu %s59, .LCPI15_0@lo(,%s59)
	vseq %v0
	vbrdu %v29,%s59
	adds.w.sx %s60, %s2, (0)1
	vsubs.w.sx %v0,%s60,%v0
	vldu %v30,4,%s6
	lea.sl %s60, .LCPI15_2@hi
	ldu %s60, .LCPI15_2@lo(,%s60)
	vfmk.w.gt %vm1,%v0
	lea.sl %s61, .LCPI15_1@hi
	ldu %s4, .LCPI15_1@lo(,%s61)
	vfmul.s %v0,%s60,%v30
	lea %s60, __vec_expf@lo
	and %s60, %s60, (32)0
	lea.sl %s60, __vec_expf@hi(%s60)
	vbrdu %v29,%s4,%vm1
	bsic %lr, (,%s60)
	lea.sl %s60, .LCPI15_3@hi
	ldu %s60, .LCPI15_3@lo(,%s60)
	vfadd.s %v0,%s59,%v0
	vfsub.s %v1,%s60,%v30
	lea.sl %s60, .LCPI15_4@hi
	ldu %s60, .LCPI15_4@lo(,%s60)
	vfdiv.s %v0,%s59,%v0
	vfmk.w.lt %vm1,%v1
	vbrdu %v0,%s59,%vm1
	vfsub.s %v1,%s60,%v30
	vfmk.w.gt %vm1,%v1
	vbrdu %v0,%s4,%vm1
	vfsub.s %v0,%v29,%v0
	vfmul.s %v0,%s3,%v0
	vstu %v0,4,%s34
	lvl %s58
	lea.sl %s58, .LCPI15_5@hi
	ld %s59, .LCPI15_5@lo(,%s58)
	ldl.zx %s60, (,%s34)
	sll %s1, %s2, 2
	adds.l %s58, %s34, %s1
	ldl.zx %s61, (,%s58)
	vbrd %v0,%s59
	muls.l %s4, %s39, %s60
	muls.l %s5, %s39, %s61
	sll %s0, %s2, 3
	adds.l %s59, %s34, %s0
	ldl.zx %s6, (,%s59)
	muls.l %s3, 12, %s2
	adds.l %s60, %s34, %s3
	ldl.zx %s7, (,%s60)
	sll %s63, %s2, 4
	adds.l %s61, %s34, %s63
	ldl.zx %s18, (,%s61)
	muls.l %s2, 20, %s2
	adds.l %s62, %s34, %s2
	ldl.zx %s19, (,%s62)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v0,%s4,%v7
	pvfmad %v2,%v0,%s5,%v7
	pvfmad %v3,%v0,%s6,%v7
	pvfmad %v4,%v0,%s7,%v7
	ldl.zx %s4, 4(,%s34)
	lea %s5, 4(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v0,%s18,%v7
	pvfmad %v6,%v0,%s19,%v7
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 4(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 4(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 4(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 4(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v8
	pvfmad %v2,%v2,%s5,%v8
	pvfmad %v3,%v3,%s6,%v8
	pvfmad %v4,%v4,%s7,%v8
	ldl.zx %s4, 8(,%s34)
	lea %s5, 8(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v8
	pvfmad %v6,%v6,%s19,%v8
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 8(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 8(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 8(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 8(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v9
	pvfmad %v2,%v2,%s5,%v9
	pvfmad %v3,%v3,%s6,%v9
	pvfmad %v4,%v4,%s7,%v9
	ldl.zx %s4, 12(,%s34)
	lea %s5, 12(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v9
	pvfmad %v6,%v6,%s19,%v9
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 12(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 12(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 12(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 12(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v10
	pvfmad %v2,%v2,%s5,%v10
	pvfmad %v3,%v3,%s6,%v10
	pvfmad %v4,%v4,%s7,%v10
	ldl.zx %s4, 16(,%s34)
	lea %s5, 16(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v10
	pvfmad %v6,%v6,%s19,%v10
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 16(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 16(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 16(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 16(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v11
	pvfmad %v2,%v2,%s5,%v11
	pvfmad %v3,%v3,%s6,%v11
	pvfmad %v4,%v4,%s7,%v11
	ldl.zx %s4, 20(,%s34)
	lea %s5, 20(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v11
	pvfmad %v6,%v6,%s19,%v11
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 20(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 20(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 20(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 20(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v12
	pvfmad %v2,%v2,%s5,%v12
	pvfmad %v3,%v3,%s6,%v12
	pvfmad %v4,%v4,%s7,%v12
	ldl.zx %s4, 24(,%s34)
	lea %s5, 24(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v12
	pvfmad %v6,%v6,%s19,%v12
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 24(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 24(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 24(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 24(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v13
	pvfmad %v2,%v2,%s5,%v13
	pvfmad %v3,%v3,%s6,%v13
	pvfmad %v4,%v4,%s7,%v13
	ldl.zx %s4, 28(,%s34)
	lea %s5, 28(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v13
	pvfmad %v6,%v6,%s19,%v13
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 28(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 28(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 28(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 28(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v14
	pvfmad %v2,%v2,%s5,%v14
	pvfmad %v3,%v3,%s6,%v14
	pvfmad %v4,%v4,%s7,%v14
	ldl.zx %s4, 32(,%s34)
	lea %s5, 32(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v14
	pvfmad %v6,%v6,%s19,%v14
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 32(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 32(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 32(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 32(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v15
	pvfmad %v2,%v2,%s5,%v15
	pvfmad %v3,%v3,%s6,%v15
	pvfmad %v4,%v4,%s7,%v15
	ldl.zx %s4, 36(,%s34)
	lea %s5, 36(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v15
	pvfmad %v6,%v6,%s19,%v15
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 36(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 36(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 36(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 36(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v16
	pvfmad %v2,%v2,%s5,%v16
	pvfmad %v3,%v3,%s6,%v16
	pvfmad %v4,%v4,%s7,%v16
	ldl.zx %s4, 40(,%s34)
	lea %s5, 40(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v16
	pvfmad %v6,%v6,%s19,%v16
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 40(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 40(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 40(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 40(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v17
	pvfmad %v2,%v2,%s5,%v17
	pvfmad %v3,%v3,%s6,%v17
	pvfmad %v4,%v4,%s7,%v17
	ldl.zx %s4, 44(,%s34)
	lea %s5, 44(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v17
	pvfmad %v6,%v6,%s19,%v17
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 44(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 44(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 44(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 44(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v18
	pvfmad %v2,%v2,%s5,%v18
	pvfmad %v3,%v3,%s6,%v18
	pvfmad %v4,%v4,%s7,%v18
	ldl.zx %s4, 48(,%s34)
	lea %s5, 48(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v18
	pvfmad %v6,%v6,%s19,%v18
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 48(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 48(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 48(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 48(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v19
	pvfmad %v2,%v2,%s5,%v19
	pvfmad %v3,%v3,%s6,%v19
	pvfmad %v4,%v4,%s7,%v19
	ldl.zx %s4, 52(,%s34)
	lea %s5, 52(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v19
	pvfmad %v6,%v6,%s19,%v19
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 52(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 52(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 52(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 52(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v20
	pvfmad %v2,%v2,%s5,%v20
	pvfmad %v3,%v3,%s6,%v20
	pvfmad %v4,%v4,%s7,%v20
	ldl.zx %s4, 56(,%s34)
	lea %s5, 56(%s34, %s1)
	ldl.zx %s5, (,%s5)
	pvfmad %v5,%v5,%s18,%v20
	pvfmad %v6,%v6,%s19,%v20
	muls.l %s4, %s39, %s4
	muls.l %s5, %s39, %s5
	lea %s6, 56(%s34, %s0)
	ldl.zx %s6, (,%s6)
	lea %s7, 56(%s34, %s3)
	ldl.zx %s7, (,%s7)
	lea %s18, 56(%s34, %s63)
	ldl.zx %s18, (,%s18)
	lea %s19, 56(%s34, %s2)
	ldl.zx %s19, (,%s19)
	muls.l %s6, %s39, %s6
	muls.l %s7, %s39, %s7
	muls.l %s18, %s39, %s18
	muls.l %s19, %s39, %s19
	pvfmad %v1,%v1,%s4,%v21
	pvfmad %v2,%v2,%s5,%v21
	pvfmad %v3,%v3,%s6,%v21
	pvfmad %v4,%v4,%s7,%v21
	ldl.zx %s4, 60(,%s34)
	lea %s1, 60(%s34, %s1)
	ldl.zx %s1, (,%s1)
	pvfmad %v5,%v5,%s18,%v21
	pvfmad %v6,%v6,%s19,%v21
	muls.l %s4, %s39, %s4
	muls.l %s1, %s39, %s1
	lea %s5, 60(%s34, %s0)
	ldl.zx %s5, (,%s5)
	lea %s3, 60(%s34, %s3)
	ldl.zx %s3, (,%s3)
	lea %s6, 60(%s34, %s63)
	ldl.zx %s6, (,%s6)
	lea %s2, 60(%s34, %s2)
	ldl.zx %s2, (,%s2)
	muls.l %s5, %s39, %s5
	muls.l %s3, %s39, %s3
	muls.l %s6, %s39, %s6
	muls.l %s2, %s39, %s2
	pvfmad %v1,%v1,%s4,%v22
	pvfmad %v2,%v2,%s1,%v22
	pvfmad %v3,%v3,%s5,%v22
	pvfmad %v4,%v4,%s3,%v22
	pvfmad %v5,%v5,%s6,%v22
	pvfmad %v6,%v6,%s2,%v22
	pvfadd %v1,%v23,%v1
	pvfadd %v2,%v24,%v2
	pvfadd %v3,%v25,%v3
	pvfadd %v4,%v26,%v4
	pvfadd %v5,%v27,%v5
	pvfadd %v6,%v28,%v6
	vst %v1,8,%s52
	vst %v2,8,%s53
	vst %v3,8,%s54
	vst %v4,8,%s55
	vst %v5,8,%s56
	vst %v6,8,%s57
	ldl.zx %s52, (,%s34)
	ldl.zx %s53, (,%s58)
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, (,%s59)
	ldl.zx %s55, (,%s60)
	ldl.zx %s56, (,%s61)
	ldl.zx %s57, (,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v1,%v0,%s52,%v23
	pvfmad %v1,%v1,%s53,%v24
	pvfmad %v1,%v1,%s54,%v25
	pvfmad %v1,%v1,%s55,%v26
	ldl.zx %s52, 4(,%s34)
	ldl.zx %s53, 4(,%s58)
	pvfmad %v1,%v1,%s56,%v27
	pvfmad %v1,%v1,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	or %s54, 4, %s0
	adds.l %s54, %s34, %s54
	ldl.zx %s54, (,%s54)
	ldl.zx %s55, 4(,%s60)
	or %s56, 4, %s63
	adds.l %s56, %s34, %s56
	ldl.zx %s56, (,%s56)
	ldl.zx %s57, 4(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v2,%v0,%s52,%v23
	pvfmad %v2,%v2,%s53,%v24
	pvfmad %v2,%v2,%s54,%v25
	pvfmad %v2,%v2,%s55,%v26
	ldl.zx %s52, 8(,%s34)
	ldl.zx %s53, 8(,%s58)
	pvfmad %v2,%v2,%s56,%v27
	pvfmad %v2,%v2,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 8(,%s59)
	ldl.zx %s55, 8(,%s60)
	or %s56, 8, %s63
	adds.l %s56, %s34, %s56
	ldl.zx %s56, (,%s56)
	ldl.zx %s57, 8(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v3,%v0,%s52,%v23
	pvfmad %v3,%v3,%s53,%v24
	pvfmad %v3,%v3,%s54,%v25
	pvfmad %v3,%v3,%s55,%v26
	ldl.zx %s52, 12(,%s34)
	ldl.zx %s53, 12(,%s58)
	pvfmad %v3,%v3,%s56,%v27
	pvfmad %v3,%v3,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 12(,%s59)
	ldl.zx %s55, 12(,%s60)
	or %s56, 12, %s63
	adds.l %s56, %s34, %s56
	ldl.zx %s56, (,%s56)
	ldl.zx %s57, 12(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v4,%v0,%s52,%v23
	pvfmad %v4,%v4,%s53,%v24
	pvfmad %v4,%v4,%s54,%v25
	pvfmad %v4,%v4,%s55,%v26
	ldl.zx %s52, 16(,%s34)
	ldl.zx %s53, 16(,%s58)
	pvfmad %v4,%v4,%s56,%v27
	pvfmad %v4,%v4,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 16(,%s59)
	ldl.zx %s55, 16(,%s60)
	ldl.zx %s56, 16(,%s61)
	ldl.zx %s57, 16(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v5,%v0,%s52,%v23
	pvfmad %v5,%v5,%s53,%v24
	pvfmad %v5,%v5,%s54,%v25
	pvfmad %v5,%v5,%s55,%v26
	ldl.zx %s52, 20(,%s34)
	ldl.zx %s53, 20(,%s58)
	pvfmad %v5,%v5,%s56,%v27
	pvfmad %v5,%v5,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 20(,%s59)
	ldl.zx %s55, 20(,%s60)
	ldl.zx %s56, 20(,%s61)
	ldl.zx %s57, 20(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v6,%v0,%s52,%v23
	pvfmad %v6,%v6,%s53,%v24
	pvfmad %v6,%v6,%s54,%v25
	pvfmad %v6,%v6,%s55,%v26
	ldl.zx %s52, 24(,%s34)
	ldl.zx %s53, 24(,%s58)
	pvfmad %v6,%v6,%s56,%v27
	pvfmad %v6,%v6,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 24(,%s59)
	ldl.zx %s55, 24(,%s60)
	ldl.zx %s56, 24(,%s61)
	ldl.zx %s57, 24(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v29,%v0,%s52,%v23
	pvfmad %v29,%v29,%s53,%v24
	pvfmad %v29,%v29,%s54,%v25
	pvfmad %v29,%v29,%s55,%v26
	ldl.zx %s52, 28(,%s34)
	ldl.zx %s53, 28(,%s58)
	pvfmad %v29,%v29,%s56,%v27
	pvfmad %v29,%v29,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 28(,%s59)
	ldl.zx %s55, 28(,%s60)
	ldl.zx %s56, 28(,%s61)
	ldl.zx %s57, 28(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v30,%v0,%s52,%v23
	pvfmad %v30,%v30,%s53,%v24
	pvfmad %v30,%v30,%s54,%v25
	pvfmad %v30,%v30,%s55,%v26
	ldl.zx %s52, 32(,%s34)
	ldl.zx %s53, 32(,%s58)
	pvfmad %v30,%v30,%s56,%v27
	pvfmad %v30,%v30,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 32(,%s59)
	ldl.zx %s55, 32(,%s60)
	ldl.zx %s56, 32(,%s61)
	ldl.zx %s57, 32(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v31,%v0,%s52,%v23
	pvfmad %v31,%v31,%s53,%v24
	pvfmad %v31,%v31,%s54,%v25
	pvfmad %v31,%v31,%s55,%v26
	ldl.zx %s52, 36(,%s34)
	ldl.zx %s53, 36(,%s58)
	pvfmad %v31,%v31,%s56,%v27
	pvfmad %v31,%v31,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 36(,%s59)
	ldl.zx %s55, 36(,%s60)
	ldl.zx %s56, 36(,%s61)
	ldl.zx %s57, 36(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v32,%v0,%s52,%v23
	pvfmad %v32,%v32,%s53,%v24
	pvfmad %v32,%v32,%s54,%v25
	pvfmad %v32,%v32,%s55,%v26
	ldl.zx %s52, 40(,%s34)
	ldl.zx %s53, 40(,%s58)
	pvfmad %v32,%v32,%s56,%v27
	pvfmad %v32,%v32,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 40(,%s59)
	ldl.zx %s55, 40(,%s60)
	ldl.zx %s56, 40(,%s61)
	ldl.zx %s57, 40(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v33,%v0,%s52,%v23
	pvfmad %v33,%v33,%s53,%v24
	pvfmad %v33,%v33,%s54,%v25
	pvfmad %v33,%v33,%s55,%v26
	ldl.zx %s52, 44(,%s34)
	ldl.zx %s53, 44(,%s58)
	pvfmad %v33,%v33,%s56,%v27
	pvfmad %v33,%v33,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 44(,%s59)
	ldl.zx %s55, 44(,%s60)
	ldl.zx %s56, 44(,%s61)
	ldl.zx %s57, 44(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v34,%v0,%s52,%v23
	pvfmad %v34,%v34,%s53,%v24
	pvfmad %v34,%v34,%s54,%v25
	pvfmad %v34,%v34,%s55,%v26
	ldl.zx %s52, 48(,%s34)
	ldl.zx %s53, 48(,%s58)
	pvfmad %v34,%v34,%s56,%v27
	pvfmad %v34,%v34,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 48(,%s59)
	ldl.zx %s55, 48(,%s60)
	ldl.zx %s56, 48(,%s61)
	ldl.zx %s57, 48(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v35,%v0,%s52,%v23
	pvfmad %v35,%v35,%s53,%v24
	pvfmad %v35,%v35,%s54,%v25
	pvfmad %v35,%v35,%s55,%v26
	ldl.zx %s52, 52(,%s34)
	ldl.zx %s53, 52(,%s58)
	pvfmad %v35,%v35,%s56,%v27
	pvfmad %v35,%v35,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 52(,%s59)
	ldl.zx %s55, 52(,%s60)
	ldl.zx %s56, 52(,%s61)
	ldl.zx %s57, 52(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v36,%v0,%s52,%v23
	pvfmad %v36,%v36,%s53,%v24
	pvfmad %v36,%v36,%s54,%v25
	pvfmad %v36,%v36,%s55,%v26
	ldl.zx %s52, 56(,%s34)
	ldl.zx %s53, 56(,%s58)
	pvfmad %v36,%v36,%s56,%v27
	pvfmad %v36,%v36,%s57,%v28
	muls.l %s52, %s39, %s52
	muls.l %s53, %s39, %s53
	ldl.zx %s54, 56(,%s59)
	ldl.zx %s55, 56(,%s60)
	ldl.zx %s56, 56(,%s61)
	ldl.zx %s57, 56(,%s62)
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s56, %s39, %s56
	muls.l %s57, %s39, %s57
	pvfmad %v37,%v0,%s52,%v23
	pvfmad %v37,%v37,%s53,%v24
	pvfmad %v37,%v37,%s54,%v25
	pvfmad %v37,%v37,%s55,%v26
	ldl.zx %s34, 60(,%s34)
	ldl.zx %s52, 60(,%s58)
	pvfmad %v37,%v37,%s56,%v27
	pvfmad %v37,%v37,%s57,%v28
	muls.l %s34, %s39, %s34
	muls.l %s52, %s39, %s52
	ldl.zx %s53, 60(,%s59)
	ldl.zx %s54, 60(,%s60)
	ldl.zx %s55, 60(,%s61)
	ldl.zx %s56, 60(,%s62)
	muls.l %s53, %s39, %s53
	muls.l %s54, %s39, %s54
	muls.l %s55, %s39, %s55
	muls.l %s39, %s39, %s56
	pvfmad %v0,%v0,%s34,%v23
	pvfmad %v0,%v0,%s52,%v24
	pvfmad %v0,%v0,%s53,%v25
	pvfmad %v0,%v0,%s54,%v26
	pvfmad %v0,%v0,%s55,%v27
	pvfmad %v0,%v0,%s39,%v28
	pvfadd %v1,%v7,%v1
	pvfadd %v2,%v8,%v2
	pvfadd %v3,%v9,%v3
	pvfadd %v4,%v10,%v4
	pvfadd %v5,%v11,%v5
	pvfadd %v6,%v12,%v6
	pvfadd %v7,%v13,%v29
	pvfadd %v8,%v14,%v30
	pvfadd %v9,%v15,%v31
	pvfadd %v10,%v16,%v32
	pvfadd %v11,%v17,%v33
	pvfadd %v12,%v18,%v34
	pvfadd %v13,%v19,%v35
	pvfadd %v14,%v20,%v36
	pvfadd %v15,%v21,%v37
	pvfadd %v0,%v22,%v0
	vst %v1,8,%s35
	vst %v2,8,%s36
	vst %v3,8,%s37
	vst %v4,8,%s38
	vst %v5,8,%s40
	vst %v6,8,%s41
	vst %v7,8,%s42
	vst %v8,8,%s43
	vst %v9,8,%s44
	vst %v10,8,%s45
	vst %v11,8,%s46
	vst %v12,8,%s47
	vst %v13,8,%s48
	vst %v14,8,%s49
	vst %v15,8,%s50
	vst %v0,8,%s51
	ld %s19, 56(,%s9)
	ld %s18, 48(,%s9)
	or %s11, 0, %s9
	ld %s16, 32(,%s11)
	ld %s15, 24(,%s11)
	ld %s10, 8(,%s11)
	ld %s9, (,%s11)
	b.l (,%lr)
.Lfunc_end15:
	.size	w2v_kernel_N6_HU512_2X_W16, .Lfunc_end15-w2v_kernel_N6_HU512_2X_W16


	.ident	"clang version 8.0.0 (git@socsv218.svp.cl.nec.co.jp:ve-llvm/clang.git 8bcd022b5fc40aba482ae91a0ffdcd03c6c4c653) (llvm/llvm.git 8f7ff827ab7f0d4cfdea92faa88818af23e2995e)"
	.section	".note.GNU-stack","",@progbits
