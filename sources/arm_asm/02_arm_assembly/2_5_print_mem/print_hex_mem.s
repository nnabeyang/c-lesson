/*
 arm-none-eabi-as print_hex_mem.s -o print_hex_mem.o
 arm-none-eabi-ld print_hex_mem.o -Ttext 0x00010000 -o print_hex_mem.elf
 arm-none-eabi-objcopy print_hex_mem.elf -O binary print_hex_mem.bin
 qemu-system-arm -M versatilepb -m 128M -nographic -kernel print_hex_mem.bin -serial mon:stdio
*/
.globl _start
_start:
  ldr r13,=0x08000000
  mov r0, r15
  bl print_hex

  mov r0, #0x68
  bl print_hex

end:
  b end
/*
r2: character printed
r1: UART
*/
putchar:
  stmdb r13!, {r1}
  ldr r1,=0x101f1000
  str r2, [r1]
  ldmia r13!, {r1}
  mov r15, r14

/*
r0 : Input of print_hex
r1 : bit rotation number
r2 : char(0-F) printing via UART
*/
print_hex:
    stmdb r13!, {r1, r2, r14}
    mov r1, #32
_loop:
    sub r1, r1, #4
    lsr r2, r0, r1
    and r2, r2, #0xF
    cmp r2, #10
    bge _print_alpha
_print_number:
    add r2, r2, #0x30
    b _print
_print_alpha:
    add r2, r2, #55
_print:
    bl putchar
    cmp r1, #0
    bne _loop
    mov r2,#0x0D
    bl putchar
    mov r2,#0x0A
    bl putchar
    ldmia r13!, {r1, r2, r14}
    mov r15, r14
