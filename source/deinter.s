.equ BUF_BYTES, 8192*2
.equ BUF_HW, 8192
.global _deInterleave

.arm
.align 2

_deInterleave:
cmp r2, #4
bxlt lr

push {r4-r9}

@ Rbuf
add r3, r1, #BUF_BYTES

.split:
@ vier samples in een keer
ldmia r0!,{r4-r7}

@ split right
lsr r8, r4, #16
lsr r9, r5, #16
orr r8, r8, r9, lsl #16
str r8,[r3], #4

@split left
lsl r9, r5, #16
lsl r8, r4, #16
orr r8, r9, r8, lsr #16
str r8,[r1], #4

@ split right
lsr r8, r6, #16
lsr r9, r7, #16
orr r8, r8, r9, lsl #16
str r8,[r3], #4

@split left
lsl r9, r6, #16
lsl r8, r7, #16
orr r8, r9, r8, lsr #16
str r8,[r1], #4

subs r2,#4
bne .split

pop {r4-r9}

bx lr

