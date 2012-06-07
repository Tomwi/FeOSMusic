.global binLog
.hidden binLog
.type binLog, %function

.arm
.align 2

@ r0 : int 
@ return in r0
binLog:

cmp 	r0, #0
mvnlt	r0, #0
bxlt 	lr
clz 	r0,r0 	
rsb		r0,r0,#31	@ if r0 was 0, it will return -1:D
bx lr



