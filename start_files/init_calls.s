.section .text.do_init_calls
.thumb
.global __sinit_func
.global __einit_func
.global do_init_calls

do_init_calls:
    push {lr}
    ldr r5, =__sinit_func
    ldr r6, =__einit_func
    sub r7, r6, r5
    asr r7, #2

__do_calls:
    ldr r3, [r5]
    blx r3
    add r5, #4
    sub r7, #1
    cmp r7, #0
    bne __do_calls
    pop {pc}
