/*
 arm-none-eabi-as print_hex.s -o print_hex.o
 arm-none-eabi-ld print_hex.o -Ttext 0x00010000 -o print_hex.elf
 arm-none-eabi-objcopy print_hex.elf -O binary print_hex.bin
 qemu-system-arm -M versatilepb -m 128M -nographic -kernel print_hex.bin -serial mon:stdio
*/
.globl _start
_start:
    ldr r0,=0x101f1000
    ldr r1,=0xdeadbeaf
print_hex:
    mov r2, #8
loop:
    mov r3, r1
    mov r4, r2
skip:
    and r5, r3, #0xF
    lsr r3, r3, #4
    sub r4, r4, #1
    cmp r4, #0
    bne skip
    cmp r5, #10
    bge print_alpha
print_number:
    add r5, r5, #0x30
    b print
print_alpha:
    add r5, r5, #55
print:
    str r5, [r0]
    sub r2, r2, #1
    cmp r2, #0
    bne loop
    b finish
    str r5, [r0]
    sub r2, r2, #1
    cmp r2, #0
    bne loop
finish:
    mov r5,#0x0D
    str r5,[r0]
    mov r5,#0x0A
    str r5,[r0]
end:
    b end
