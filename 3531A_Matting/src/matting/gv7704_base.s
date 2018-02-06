	.arch armv5te
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"gv7704_base.c"
	.data
	.align	2
	.type	gv7704_fd, %object
	.size	gv7704_fd, 4
gv7704_fd:
	.word	-1
	.local	gv7704_data
	.comm	gv7704_data,36,4
	.local	gv7704_last_read
	.comm	gv7704_last_read,4,4
	.section	.rodata
	.align	2
.LC0:
	.ascii	"/dev/gv7704\000"
	.text
	.align	2
	.global	gv7704_read
	.type	gv7704_read, %function
gv7704_read:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #16
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	mov	r3, #0
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-20]
	cmp	r3, #0
	bne	.L2
	mvn	r3, #0
	b	.L3
.L2:
	ldr	r3, .L27
	ldr	r3, [r3]
	cmp	r3, #0
	bge	.L4
	ldr	r0, .L27+4
	mov	r1, #2
	bl	open
	mov	r2, r0
	ldr	r3, .L27
	str	r2, [r3]
	ldr	r3, .L27
	ldr	r3, [r3]
	cmp	r3, #0
	bge	.L4
	mvn	r3, #0
	b	.L3
.L4:
	mov	r0, #0
	bl	time
	str	r0, [fp, #-12]
	ldr	r3, .L27+8
	ldr	r3, [r3]
	ldr	r2, [fp, #-12]
	rsb	r3, r3, r2
	cmp	r3, #1
	ble	.L5
	ldr	r0, .L27+12
	mov	r1, #0
	mov	r2, #36
	bl	memset
	ldr	r3, .L27
	ldr	r3, [r3]
	mov	r0, r3
	ldr	r1, .L27+16
	ldr	r2, .L27+12
	bl	ioctl
	str	r0, [fp, #-8]
	ldr	r3, .L27+8
	ldr	r2, [fp, #-12]
	str	r2, [r3]
	ldr	r3, [fp, #-8]
	cmp	r3, #0
	beq	.L5
	ldr	r3, [fp, #-8]
	b	.L3
.L5:
	ldr	r0, [fp, #-20]
	mov	r1, #0
	mov	r2, #16
	bl	memset
	ldr	r3, [fp, #-20]
	mov	r2, #1
	str	r2, [r3]
	ldr	r1, .L27+12
	ldr	r2, [fp, #-16]
	mov	r3, #4
	mov	r2, r2, asl #2
	add	r2, r1, r2
	add	r3, r2, r3
	ldr	r3, [r3]
	cmp	r3, #18
	ldrls	pc, [pc, r3, asl #2]
	b	.L6
.L8:
	.word	.L7
	.word	.L9
	.word	.L10
	.word	.L11
	.word	.L12
	.word	.L13
	.word	.L14
	.word	.L15
	.word	.L16
	.word	.L17
	.word	.L18
	.word	.L19
	.word	.L20
	.word	.L21
	.word	.L22
	.word	.L23
	.word	.L24
	.word	.L25
	.word	.L26
.L7:
	ldr	r3, [fp, #-20]
	mov	r2, #1280
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	mov	r2, #720
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #24
	str	r2, [r3, #12]
	b	.L6
.L9:
	ldr	r3, [fp, #-20]
	mov	r2, #1280
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	mov	r2, #720
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #25
	str	r2, [r3, #12]
	b	.L6
.L10:
	ldr	r3, [fp, #-20]
	mov	r2, #1280
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	mov	r2, #720
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #25
	str	r2, [r3, #12]
	b	.L6
.L11:
	ldr	r3, [fp, #-20]
	mov	r2, #1280
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	mov	r2, #720
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #30
	str	r2, [r3, #12]
	b	.L6
.L12:
	ldr	r3, [fp, #-20]
	mov	r2, #1280
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	mov	r2, #720
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #30
	str	r2, [r3, #12]
	b	.L6
.L13:
	ldr	r3, [fp, #-20]
	mov	r2, #1280
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	mov	r2, #720
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #50
	str	r2, [r3, #12]
	b	.L6
.L14:
	ldr	r3, [fp, #-20]
	mov	r2, #1280
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	mov	r2, #720
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #60
	str	r2, [r3, #12]
	b	.L6
.L15:
	ldr	r3, [fp, #-20]
	mov	r2, #1280
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	mov	r2, #720
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #60
	str	r2, [r3, #12]
	b	.L6
.L16:
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #24
	str	r2, [r3, #12]
	b	.L6
.L17:
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #25
	str	r2, [r3, #12]
	b	.L6
.L18:
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #25
	str	r2, [r3, #12]
	b	.L6
.L19:
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #30
	str	r2, [r3, #12]
	b	.L6
.L20:
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #30
	str	r2, [r3, #12]
	b	.L6
.L21:
	ldr	r3, [fp, #-20]
	mov	r2, #0
	str	r2, [r3]
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #25
	str	r2, [r3, #12]
	b	.L6
.L22:
	ldr	r3, [fp, #-20]
	mov	r2, #0
	str	r2, [r3]
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #30
	str	r2, [r3, #12]
	b	.L6
.L23:
	ldr	r3, [fp, #-20]
	mov	r2, #0
	str	r2, [r3]
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #30
	str	r2, [r3, #12]
	b	.L6
.L24:
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #50
	str	r2, [r3, #12]
	b	.L6
.L25:
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #60
	str	r2, [r3, #12]
	b	.L6
.L26:
	ldr	r3, [fp, #-20]
	mov	r2, #1920
	str	r2, [r3, #4]
	ldr	r3, [fp, #-20]
	ldr	r2, .L27+20
	str	r2, [r3, #8]
	ldr	r3, [fp, #-20]
	mov	r2, #60
	str	r2, [r3, #12]
	mov	r0, r0	@ nop
.L6:
	mov	r3, #0
.L3:
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L28:
	.align	2
.L27:
	.word	gv7704_fd
	.word	.LC0
	.word	gv7704_last_read
	.word	gv7704_data
	.word	-1073453565
	.word	1080
	.size	gv7704_read, .-gv7704_read
	.ident	"GCC: (Hisilicon_v400) 4.8.3 20131202 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
