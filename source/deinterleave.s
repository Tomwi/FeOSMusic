.equ BUF_BYTES, 8192*2
.equ BUF_HW, 8192
.global _deInterleave
.hidden _deInterleave

.arm
.align 2

_deInterleave:
	cmp r2, #4
	bxlt lr		@ nothing to split

	push {r4-r9}

	add r3, r1, #BUF_BYTES
	mvn r8, #0
	lsl r8,#16

	.split:

	ldmia r0!,{r4-r7}

	@ split right
	and r9, r5, r8
	orr r9, r9, r4, lsr #16
	str r9,[r3],#4

	@ split left
	bic ip, r4, r8
	orr ip, ip, r5, lsl #16
	str ip,[r1],#4

	@ split right
	and r9, r7, r8
	orr r9, r9, r6, lsr #16
	str r9,[r3],#4

	@ split left
	bic ip, r6, r8
	orr ip, ip, r7, lsl #16
	str ip,[r1],#4

	subs r2,#4
	bne .split

	pop {r4-r9}

	bx lr

