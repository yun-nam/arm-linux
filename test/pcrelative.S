@ Some functions to do some PC relative type stuff

.section mypcsection, "awx"
.align 12
dummy_label:
.word 10


@ Put the PC relative variable in the page above
@ our extensions don't like executing and modifying the same page
.equ pc_relative_var, pc_relative_store-4


.align 12
.arm
.global pc_relative_store
.global pc_relative_load

pc_relative_store:
str  r0, [pc, #(pc_relative_var-pc_relative_store -8)]
bx   lr

pc_relative_load:
ldr  r0, [pc, #(pc_relative_var-pc_relative_load-8)]
bx   lr
