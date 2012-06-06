.equ PRECISION, (1<<16)
.global _visua
.hidden _visua
.type _visua, %function

.arm
.align 2

@ 	r0 = buf
@ 	r1 = len
@ 	r2 = arr
_visua:


subs r1, #1
bxmi lr

push {r4}

.loop:

ldrsh r3, [r0], #2
movs r3, r3
rsbmi r3, r3, #0
clzmi ip, r3
rsb ip, ip, #31
ldr r4, [r2, ip, lsl #2]
subs r1,#1
add r4,#PRECISION
str r4, [r2, ip, lsl #2]

bpl .loop

pop {r4}

bx lr