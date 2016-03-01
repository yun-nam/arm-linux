@ This creates some test routines to test the ability of prefetch
@ It consists of routines that have only a few instructions each

.text
.align 12       @ align on a page
.arm
.global prefetch_1
.type   prefetch_1 STT_FUNC
prefetch_1:
bx lr


.align 12       @ align on a page
.arm
.global prefetch_2
.type   prefetch_2 STT_FUNC

prefetch_2:
add r0, r0, r1
bx lr


.align 12       @ align on a page
.arm
.global prefetch_3
.type   prefetch_3 STT_FUNC
prefetch_3:
cmp r0,r1
moveq r0, #1
bx lr


.align 12       @ align on a page
.arm
.global prefetch_4
.type   prefetch_4 STT_FUNC

prefetch_4:
ldr r1, [r0]
and r1, r1, #0xff0
mov r0, r1
bx lr


.align 12       @ align on a page
.arm
.global prefetch_5
.type   prefetch_5 STT_FUNC

prefetch_5:
cmp r0,r1
moveq r0, #1
nop
nop
bx lr


.align 12       @ align on a page
.arm
.global prefetch_6
.type   prefetch_6 STT_FUNC

prefetch_6:
nop
nop
nop
nop
nop
bx lr


.align 12       @ align on a page
.arm
.global prefetch_7
.type   prefetch_7 STT_FUNC

prefetch_7:
nop
nop
nop
nop
nop
nop
bx lr


.align 12       @ align on a page
.arm
.global prefetch_8
.type   prefetch_8 STT_FUNC

prefetch_8:
nop
nop
nop
nop
nop
nop
nop
bx lr



.align 12       @ align on a page
.arm
.global prefetch_9
.type   prefetch_9 STT_FUNC
prefetch_9:
nop
nop
nop
nop
nop
nop
nop
nop
bx lr


.align 12       @ align on a page
.arm
.global prefetch_10
.type   prefetch_10 STT_FUNC
prefetch_10:
nop
nop
nop
nop
nop
nop
nop
nop
nop
bx lr


.align 12       @ align on a page
.arm
.global prefetch_11
.type   prefetch_11 STT_FUNC
prefetch_11:
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
bx lr
