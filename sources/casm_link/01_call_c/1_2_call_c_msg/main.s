.globl _start
_start:
    ldr r13,=0x07FFFFFF
    ldr r0,=msg
    bl print_msg
loop:
    b loop
msg:
    .asciz "Hello World\n"
