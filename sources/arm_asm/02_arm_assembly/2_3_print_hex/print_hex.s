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
    mov r2, #32
loop:
    sub r2, r2, #4
    lsr r3, r1, r2
    and r3, r3, #0xF
    cmp r3, #10
    bge print_alpha
print_number:
    add r3, r3, #0x30
    b print
print_alpha:
    add r3, r3, #55
print:
    str r3, [r0]
    cmp r2, #0
    bne loop
    mov r3,#0x0D
    str r3,[r0]
    mov r3,#0x0A
    str r3,[r0]
end:
    b end
