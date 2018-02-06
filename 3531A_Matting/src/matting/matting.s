	.arch armv5te
	.eabi_attribute 27, 3
	.fpu neon
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"matting.c"
	.global	y_thresh
	.data
	.align	2
	.type	y_thresh, %object
	.size	y_thresh, 4
y_thresh:
	.word	168
	.global	u_thresh
	.align	2
	.type	u_thresh, %object
	.size	u_thresh, 4
u_thresh:
	.word	120
	.global	v_thresh
	.align	2
	.type	v_thresh, %object
	.size	v_thresh, 4
v_thresh:
	.word	60
	.global	i_thresh
	.align	2
	.type	i_thresh, %object
	.size	i_thresh, 4
i_thresh:
	.word	68
	.global	o_thresh
	.align	2
	.type	o_thresh, %object
	.size	o_thresh, 4
o_thresh:
	.word	6
	.text
	.align	2
	.type	fastMatting, %function
fastMatting:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	add	fp, sp, #32
	sub	sp, sp, #20
	str	r0, [fp, #-40]
	str	r1, [fp, #-44]
	str	r2, [fp, #-48]
	ldr	ip, [fp, #-48]
	ldr	lr, [fp, #-44]
	ldr	r10, [fp, #-40]
#APP
@ 20 "matting.c" 1
	ldr	r4, =i_thresh
	ldr	r5, [r4]
	vdup.16	q15, r5
	ldr	r4, =o_thresh
	ldr	r5, [r4]
	vdup.16	q14, r5
	vmov.U16 q13, #0x0100
	ldr	r5, =211
	vmov.U16	d0[0], r5
	ldr	r5, =179
	vmov.U16	d0[1], r5
	ldr	r5, =21
	vmov.U16	d0[2], r5
	ldr	r5, =232
	vmov.U16	d0[3], r5
	ldr	r5, =4091
	vmov.U16	d1[0], r5
	ldr	r5, =32498
	vmov.U16	d1[1], r5
	ldr	r5, =4
	vmov.U16	d1[2], r5
	mov	r4, r2
	ldr	r5, =2073600
	add	r2, r0, r5
	add	r3, r1, r5
	sub	r4, #1
	mov	r5, r2
	mov	r6, #540
	.hLoop:
	mov	r7, #1920
	.wLoop:
	vld1.8 {d2-d3}, [r4]!
	vld1.8 {d4-d5}, [r4]!
	vld1.8 {d6-d7}, [r1]!
	vld1.8 {d8-d9}, [r0]
	vshr.U16	q1, #8
	vshr.U16	q2, #8
	vqsub.U16	q1, q1, q15
	vqsub.U16	q2, q2, q15
	vmul.I16	q1, q1, q14
	vmul.I16	q2, q2, q14
	vcge.U16	q1, q1, q13
	vcge.U16	q2, q2, q13
	vshl.U16	q1, q1, #8
	vshl.U16	q2, q2, #8
	vshr.U16	q1, q1, #8
	vshr.U16	q2, q2, #8
	vsub.U16 q11, q13, q1
	vsub.U16 q12, q13, q2
	vmovl.U8	q6, d6
	vmovl.U8	q8, d8
	vmul.U16	q6, q6, q1
	vmla.U16	q6, q8, q11
	vshr.U16	q6, #8
	vmovn.U16	d8, q6
	vmovl.U8	q7, d7
	vmovl.U8	q9, d9
	vmul.U16	q7, q7, q2
	vmla.U16	q7, q9, q12
	vshr.U16	q7, #8
	vmovn.U16	d9, q7
	vst1.8 {d8-d9}, [r0]!
	vld1.8 {d2-d3}, [r4]!
	vld1.8 {d4-d5}, [r4]!
	vld1.8 {d6-d7}, [r1]!
	vld1.8 {d8-d9}, [r0]
	vshr.U16	q1, #8
	vshr.U16	q2, #8
	vqsub.U16	q1, q1, q15
	vqsub.U16	q2, q2, q15
	vmul.I16	q1, q1, q14
	vmul.I16	q2, q2, q14
	vcge.U16	q1, q1, q13
	vcge.U16	q2, q2, q13
	vshl.U16	q1, q1, #8
	vshl.U16	q2, q2, #8
	vshr.U16	q1, q1, #8
	vshr.U16	q2, q2, #8
	vsub.U16 q11, q13, q1
	vsub.U16 q12, q13, q2
	vmovl.U8	q6, d6
	vmovl.U8	q8, d8
	vmul.U16	q6, q6, q1
	vmla.U16	q6, q8, q11
	vshr.U16	q6, #8
	vmovn.U16	d8, q6
	vmovl.U8	q7, d7
	vmovl.U8	q9, d9
	vmul.U16	q7, q7, q2
	vmla.U16	q7, q9, q12
	vshr.U16	q7, #8
	vmovn.U16	d9, q7
	vst1.8 {d8-d9}, [r0]!
	vld1.8 {d6-d7}, [r3]!
	vld1.8 {d8-d9}, [r2]
	vshl.U16	q1, q3, #8
	vshr.U16	q1, q1, #8
	vqsub.U16	q1, q1, q15
	vmul.I16	q1, q1, q14
	vcge.U16	q1, q1, q13
	vshl.U16	q1, q1, #8
	vshr.U16	q1, q1, #8
	vshl.U16	q2, q1, #8
	vorr.U16	q7, q2, q1
	vmovl.U8	q1, d14
	vmovl.U8	q2, d15
	vsub.U16 q11, q13, q1
	vsub.U16 q12, q13, q2
	vmovl.U8	q6, d6
	vmovl.U8	q8, d8
	vmul.U16	q6, q6, q1
	vmla.U16	q6, q8, q11
	vshr.U16	q6, #8
	vmovn.U16	d8, q6
	vmovl.U8	q7, d7
	vmovl.U8	q9, d9
	vmul.U16	q7, q7, q2
	vmla.U16	q7, q9, q12
	vshr.U16	q7, #8
	vmovn.U16	d9, q7
	vst1.8 {d8-d9}, [r2]!
	sub	r7, #16
	cmp	r7, #0
	bne	.wLoop
	sub	r6, #1
	cmp	r6, #0
	bne	.hLoop
	.theEnd:
	
@ 0 "" 2
	sub	sp, fp, #32
	@ sp needed
	ldmfd	sp!, {r4, r5, r6, r7, r8, r9, r10, fp, pc}
	.size	fastMatting, .-fastMatting
	.section	.rodata
	.align	2
.LC1:
	.ascii	"[%s:%s:%d]---HI_MPI_IVE_Thresh failed with 0x%x!\012"
	.ascii	"\000"
	.align	2
.LC2:
	.ascii	"matting.c\000"
	.text
	.align	2
	.type	imgSub, %function
imgSub:
	@ args = 0, pretend = 0, frame = 144
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #152
	str	r0, [fp, #-136]
	str	r1, [fp, #-140]
	str	r2, [fp, #-144]
	str	r3, [fp, #-148]
	mov	r3, #0
	str	r3, [fp, #-8]
	mov	r3, #0
	str	r3, [fp, #-12]
	mov	r3, #0
	str	r3, [fp, #-52]
	mov	r3, #1920
	strh	r3, [fp, #-18]	@ movhi
	ldr	r3, .L5
	strh	r3, [fp, #-16]	@ movhi
	mov	r3, #1920
	strh	r3, [fp, #-24]	@ movhi
	ldr	r3, [fp, #-140]
	str	r3, [fp, #-48]
	mov	r3, #0
	str	r3, [fp, #-92]
	mov	r3, #1920
	strh	r3, [fp, #-58]	@ movhi
	ldr	r3, .L5
	strh	r3, [fp, #-56]	@ movhi
	mov	r3, #1920
	strh	r3, [fp, #-64]	@ movhi
	ldr	r3, [fp, #-144]
	str	r3, [fp, #-88]
	mov	r3, #0
	str	r3, [fp, #-132]
	mov	r3, #1920
	strh	r3, [fp, #-98]	@ movhi
	ldr	r3, .L5
	strh	r3, [fp, #-96]	@ movhi
	mov	r3, #1920
	strh	r3, [fp, #-104]	@ movhi
	ldr	r3, [fp, #-148]
	str	r3, [fp, #-128]
	sub	r1, fp, #52
	sub	r2, fp, #92
	sub	r3, fp, #132
	sub	r0, fp, #12
	str	r0, [sp]
	mov	r0, #0
	str	r0, [sp, #4]
	ldr	r0, [fp, #-136]
	bl	HI_MPI_IVE_Sub
	str	r0, [fp, #-8]
	ldr	r3, [fp, #-8]
	cmp	r3, #0
	beq	.L3
	mov	r3, #242
	str	r3, [sp]
	ldr	r3, [fp, #-8]
	str	r3, [sp, #4]
	mov	r0, #1
	ldr	r1, .L5+4
	ldr	r2, .L5+8
	ldr	r3, .L5+12
	bl	write_log_content
.L3:
	ldr	r3, [fp, #-8]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L6:
	.align	2
.L5:
	.word	1080
	.word	.LC1
	.word	.LC2
	.word	__FUNCTION__.21553
	.size	imgSub, .-imgSub
	.align	2
	.type	imgThr, %function
imgThr:
	@ args = 0, pretend = 0, frame = 112
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #120
	str	r0, [fp, #-104]
	str	r1, [fp, #-108]
	str	r2, [fp, #-112]
	str	r3, [fp, #-116]
	mov	r3, #0
	str	r3, [fp, #-8]
	mov	r3, #0
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-116]
	and	r3, r3, #255
	strb	r3, [fp, #-16]
	mov	r3, #0
	strb	r3, [fp, #-14]
	mvn	r3, #0
	strb	r3, [fp, #-12]
	mov	r3, #0
	str	r3, [fp, #-60]
	mov	r3, #1920
	strh	r3, [fp, #-26]	@ movhi
	ldr	r3, .L10
	strh	r3, [fp, #-24]	@ movhi
	mov	r3, #1920
	strh	r3, [fp, #-32]	@ movhi
	ldr	r3, [fp, #-108]
	str	r3, [fp, #-56]
	mov	r3, #0
	str	r3, [fp, #-100]
	mov	r3, #1920
	strh	r3, [fp, #-66]	@ movhi
	ldr	r3, .L10
	strh	r3, [fp, #-64]	@ movhi
	mov	r3, #1920
	strh	r3, [fp, #-72]	@ movhi
	ldr	r3, [fp, #-112]
	str	r3, [fp, #-96]
	sub	r1, fp, #60
	sub	r2, fp, #100
	sub	r3, fp, #20
	mov	r0, #0
	str	r0, [sp]
	ldr	r0, [fp, #-104]
	bl	HI_MPI_IVE_Thresh
	str	r0, [fp, #-8]
	ldr	r3, [fp, #-8]
	cmp	r3, #0
	beq	.L8
	mov	r3, #288
	str	r3, [sp]
	ldr	r3, [fp, #-8]
	str	r3, [sp, #4]
	mov	r0, #1
	ldr	r1, .L10+4
	ldr	r2, .L10+8
	ldr	r3, .L10+12
	bl	write_log_content
.L8:
	ldr	r3, [fp, #-8]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L11:
	.align	2
.L10:
	.word	1080
	.word	.LC1
	.word	.LC2
	.word	__FUNCTION__.21564
	.size	imgThr, .-imgThr
	.section	.rodata
	.align	2
.LC3:
	.ascii	"[%s:%s:%d]---HI_MPI_IVE_And failed with 0x%x!\012\000"
	.align	2
.LC0:
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	1
	.byte	2
	.byte	1
	.byte	0
	.byte	0
	.byte	2
	.byte	4
	.byte	2
	.byte	0
	.byte	0
	.byte	1
	.byte	2
	.byte	1
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.text
	.align	2
	.type	imgFilter, %function
imgFilter:
	@ args = 0, pretend = 0, frame = 232
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #240
	str	r0, [fp, #-232]
	str	r1, [fp, #-236]
	mov	r3, #0
	str	r3, [fp, #-8]
	mov	r3, #4
	strb	r3, [fp, #-11]
	ldr	r3, .L15
	sub	r1, fp, #64
	mov	r2, r3
	mov	r3, #25
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	bl	memcpy
	sub	ip, fp, #36
	sub	lr, fp, #64
	ldmia	lr!, {r0, r1, r2, r3}
	stmia	ip!, {r0, r1, r2, r3}
	ldmia	lr, {r0, r1, r2}
	stmia	ip!, {r0, r1}
	strb	r2, [ip]
	mov	r3, #0
	str	r3, [fp, #-104]
	mov	r3, #1920
	strh	r3, [fp, #-70]	@ movhi
	mov	r3, #540
	strh	r3, [fp, #-68]	@ movhi
	mov	r3, #1920
	strh	r3, [fp, #-76]	@ movhi
	ldr	r3, [fp, #-236]
	str	r3, [fp, #-100]
	mov	r3, #0
	str	r3, [fp, #-144]
	mov	r3, #1920
	strh	r3, [fp, #-110]	@ movhi
	mov	r3, #540
	strh	r3, [fp, #-108]	@ movhi
	mov	r3, #1920
	strh	r3, [fp, #-116]	@ movhi
	ldr	r3, [fp, #-236]
	str	r3, [fp, #-140]
	sub	r1, fp, #104
	sub	r2, fp, #144
	sub	r3, fp, #36
	mov	r0, #0
	str	r0, [sp]
	ldr	r0, [fp, #-232]
	bl	HI_MPI_IVE_Filter
	str	r0, [fp, #-8]
	ldr	r3, [fp, #-8]
	cmp	r3, #0
	beq	.L13
	ldr	r3, .L15+4
	str	r3, [sp]
	ldr	r3, [fp, #-8]
	str	r3, [sp, #4]
	mov	r0, #1
	ldr	r1, .L15+8
	ldr	r2, .L15+12
	ldr	r3, .L15+16
	bl	write_log_content
.L13:
	ldr	r3, [fp, #-8]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L16:
	.align	2
.L15:
	.word	.LC0
	.word	343
	.word	.LC3
	.word	.LC2
	.word	__FUNCTION__.21576
	.size	imgFilter, .-imgFilter
	.align	2
	.type	matting, %function
matting:
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #32
	str	r0, [fp, #-24]
	str	r1, [fp, #-28]
	str	r2, [fp, #-32]
	mov	r3, #1
	str	r3, [fp, #-20]
	mov	r3, #0
	str	r3, [fp, #-8]
	mov	r3, #0
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-24]
	ldr	r3, [r3, #24]
	mov	r0, r3
	ldr	r1, .L19
	bl	HI_MPI_SYS_MmapCache
	str	r0, [fp, #-12]
	ldr	r0, [fp, #-12]
	mov	r1, #0
	ldr	r2, .L19
	bl	memset
	ldr	r3, [fp, #-24]
	ldr	r3, [r3, #24]
	mov	r0, r3
	ldr	r1, [fp, #-12]
	ldr	r2, .L19
	bl	HI_MPI_SYS_MflushCache
	ldr	r0, [fp, #-12]
	ldr	r1, .L19
	bl	HI_MPI_SYS_Munmap
	ldr	r3, [fp, #-28]
	ldr	r1, [r3, #24]
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #24]
	ldr	r3, [fp, #-24]
	ldr	r3, [r3, #24]
	sub	r0, fp, #16
	bl	imgSub
	ldr	r3, [fp, #-24]
	ldr	r3, [r3, #24]
	sub	r2, fp, #16
	mov	r0, r2
	mov	r1, r3
	bl	imgFilter
	ldr	r2, [fp, #-16]
	sub	r3, fp, #20
	mov	r0, r2
	mov	r1, r3
	mov	r2, #1
	bl	HI_MPI_IVE_Query
	ldr	r3, [fp, #-24]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L20:
	.align	2
.L19:
	.word	3110400
	.size	matting, .-matting
	.section	.rodata
	.align	2
.LC4:
	.ascii	"//mnt//disk//1.nv21\000"
	.align	2
.LC5:
	.ascii	"r\000"
	.align	2
.LC6:
	.ascii	"fopen errno = %d\012\000"
	.align	2
.LC7:
	.ascii	"fseek errno = %d\012\000"
	.align	2
.LC8:
	.ascii	"%s:%d\012\000"
	.align	2
.LC9:
	.ascii	"%ld\012\000"
	.text
	.align	2
	.global	mattingProcessThread
	.type	mattingProcessThread, %function
mattingProcessThread:
	@ args = 0, pretend = 0, frame = 488
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {r4, fp, lr}
	add	fp, sp, #8
	sub	sp, sp, #492
	str	r0, [fp, #-496]
	mov	r3, #0
	str	r3, [fp, #-16]
	mov	r3, #0
	str	r3, [fp, #-20]
	sub	r3, fp, #492
	mov	r0, r3
	mvn	r1, #0
	bl	framePoolInit
	sub	r3, fp, #348
	mov	r0, r3
	mvn	r1, #0
	bl	framePoolInit
	mov	r3, #0
	str	r3, [fp, #-24]
	mov	r3, #0
	str	r3, [fp, #-28]
	ldr	r0, .L28
	ldr	r1, .L28+4
	bl	fopen
	str	r0, [fp, #-24]
	ldr	r3, [fp, #-24]
	cmp	r3, #0
	bne	.L22
	ldr	r3, .L28+8
	ldr	r4, [r3]
	bl	__errno_location
	mov	r3, r0
	ldr	r3, [r3]
	mov	r0, r4
	ldr	r1, .L28+12
	mov	r2, r3
	bl	fprintf
	mov	r0, #0
	bl	exit
.L22:
	ldr	r0, [fp, #-24]
	ldr	r1, .L28+16
	mov	r2, #0
	bl	fseek
	str	r0, [fp, #-16]
	ldr	r3, [fp, #-16]
	cmp	r3, #0
	beq	.L23
	ldr	r3, .L28+8
	ldr	r4, [r3]
	bl	__errno_location
	mov	r3, r0
	ldr	r3, [r3]
	mov	r0, r4
	ldr	r1, .L28+20
	mov	r2, r3
	bl	fprintf
	mov	r0, #0
	bl	exit
.L23:
	ldr	r3, [fp, #-468]
	mov	r0, r3
	ldr	r1, .L28+24
	bl	HI_MPI_SYS_MmapCache
	str	r0, [fp, #-28]
	ldr	r0, [fp, #-28]
	ldr	r1, .L28+24
	mov	r2, #1
	ldr	r3, [fp, #-24]
	bl	fread
	ldr	r3, [fp, #-468]
	mov	r0, r3
	ldr	r1, [fp, #-28]
	ldr	r2, .L28+24
	bl	HI_MPI_SYS_MflushCache
	ldr	r0, [fp, #-28]
	ldr	r1, .L28+24
	bl	HI_MPI_SYS_Munmap
	ldr	r0, [fp, #-24]
	ldr	r1, .L28+28
	mov	r2, #0
	bl	fseek
	str	r0, [fp, #-16]
	mov	r3, #0
	str	r3, [fp, #-32]
	ldr	r3, .L28+8
	ldr	r3, [r3]
	mov	r0, r3
	ldr	r1, .L28+32
	ldr	r2, .L28+36
	ldr	r3, .L28+40
	bl	fprintf
	sub	r3, fp, #204
	mov	r0, #6
	mov	r1, #0
	mov	r2, r3
	mov	r3, #1000
	bl	HI_MPI_VPSS_GetChnFrame
	str	r0, [fp, #-16]
	ldr	r3, [fp, #-16]
	cmp	r3, #0
	bne	.L24
	ldr	r3, [fp, #-324]
	mov	r0, r3
	ldr	r1, .L28+24
	bl	HI_MPI_SYS_MmapCache
	str	r0, [fp, #-28]
	ldr	r0, [fp, #-28]
	ldr	r1, .L28+24
	mov	r2, #1
	ldr	r3, [fp, #-24]
	bl	fread
	mov	r3, r0
	str	r3, [fp, #-16]
	ldr	r3, [fp, #-32]
	add	r3, r3, #1
	str	r3, [fp, #-32]
	ldr	r3, [fp, #-324]
	mov	r0, r3
	ldr	r1, [fp, #-28]
	ldr	r2, .L28+24
	bl	HI_MPI_SYS_MflushCache
	ldr	r0, [fp, #-28]
	ldr	r1, .L28+24
	bl	HI_MPI_SYS_Munmap
	ldr	r2, [fp, #-32]
	ldr	r3, .L28+44
	cmp	r2, r3
	bne	.L25
	mov	r3, #0
	str	r3, [fp, #-32]
	ldr	r0, [fp, #-24]
	ldr	r1, .L28+48
	mov	r2, #0
	bl	fseek
.L25:
	ldrd	r2, [fp, #-100]
	strd	r2, [fp, #-244]
	ldr	r3, [fp, #-236]
	add	r3, r3, #2
	str	r3, [fp, #-236]
	sub	r2, fp, #48
	sub	r3, fp, #40
	mov	r0, r2
	mov	r1, r3
	bl	gettimeofday
	sub	r1, fp, #204
	sub	r2, fp, #348
	sub	r3, fp, #492
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	bl	matting
	str	r0, [fp, #-20]
	ldr	r3, [fp, #-20]
	cmp	r3, #0
	beq	.L26
	sub	r2, fp, #56
	sub	r3, fp, #40
	mov	r0, r2
	mov	r1, r3
	bl	gettimeofday
	ldr	r3, [fp, #-56]
	ldr	r2, .L28+52
	mul	r2, r3, r2
	ldr	r3, [fp, #-52]
	add	r2, r2, r3
	ldr	r3, [fp, #-48]
	ldr	r1, .L28+56
	mul	r3, r1, r3
	add	r2, r2, r3
	ldr	r3, [fp, #-44]
	rsb	r3, r3, r2
	mov	r0, r3
	bl	setInfo
	ldr	r3, .L28+8
	ldr	r2, [r3]
	ldr	r3, [fp, #-56]
	ldr	r1, .L28+52
	mul	r1, r3, r1
	ldr	r3, [fp, #-52]
	add	r1, r1, r3
	ldr	r3, [fp, #-48]
	ldr	r0, .L28+56
	mul	r3, r0, r3
	add	r1, r1, r3
	ldr	r3, [fp, #-44]
	rsb	r3, r3, r1
	mov	r0, r2
	ldr	r1, .L28+60
	mov	r2, r3
	bl	fprintf
	mov	r0, #12
	ldr	r1, [fp, #-20]
	mov	r2, #1000
	bl	HI_MPI_VPSS_SendFrame
.L26:
	sub	r3, fp, #204
	mov	r0, #0
	mov	r1, #0
	mov	r2, r3
	bl	HI_MPI_VPSS_ReleaseChnFrame
.L24:
	ldr	r0, [fp, #-24]
	bl	fclose
	mov	r3, #0
	mov	r0, r3
	sub	sp, fp, #8
	@ sp needed
	ldmfd	sp!, {r4, fp, pc}
.L29:
	.align	2
.L28:
	.word	.LC4
	.word	.LC5
	.word	stderr
	.word	.LC6
	.word	83980800
	.word	.LC7
	.word	3110400
	.word	311040000
	.word	.LC8
	.word	__FUNCTION__.21600
	.word	442
	.word	2600
	.word	93312000
	.word	1000000
	.word	-1000000
	.word	.LC9
	.size	mattingProcessThread, .-mattingProcessThread
	.section	.rodata
	.align	2
	.type	__FUNCTION__.21553, %object
	.size	__FUNCTION__.21553, 7
__FUNCTION__.21553:
	.ascii	"imgSub\000"
	.align	2
	.type	__FUNCTION__.21564, %object
	.size	__FUNCTION__.21564, 7
__FUNCTION__.21564:
	.ascii	"imgThr\000"
	.align	2
	.type	__FUNCTION__.21576, %object
	.size	__FUNCTION__.21576, 10
__FUNCTION__.21576:
	.ascii	"imgFilter\000"
	.align	2
	.type	__FUNCTION__.21600, %object
	.size	__FUNCTION__.21600, 21
__FUNCTION__.21600:
	.ascii	"mattingProcessThread\000"
	.ident	"GCC: (Hisilicon_v400) 4.8.3 20131202 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
