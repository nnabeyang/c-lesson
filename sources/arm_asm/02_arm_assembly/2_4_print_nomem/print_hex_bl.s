/*
 arm-none-eabi-as print_hex_bl.s -o print_hex_bl.o
 arm-none-eabi-ld print_hex_bl.o -Ttext 0x00010000 -o print_hex_bl.elf
 arm-none-eabi-objcopy print_hex_bl.elf -O binary print_hex_bl.bin
 qemu-system-arm -M versatilepb -m 128M -nographic -kernel print_hex_bl.bin -serial mon:stdio
*/
.globl _start
_start:

  mov r0, r15
  bl print_hex

  mov r0, #0x68
  bl print_hex

end:
  b end

/*
r0 : Input of print_hex
r1 : UART
r2 : bit rotation number
r3 : char(0-F) printing via UART
*/
print_hex:
    ldr r1,=0x101f1000
    mov r2, #32
loop:
    sub r2, r2, #4
    lsr r3, r0, r2
    and r3, r3, #0xF
    cmp r3, #10
    bge print_alpha
print_number:
    add r3, r3, #0x30
    b print
print_alpha:
    add r3, r3, #55
print:
    str r3, [r1]
    cmp r2, #0
    bne loop
    mov r3,#0x0D
    str r3,[r1]
    mov r3,#0x0A
    str r3,[r1]
    mov r15, r14
