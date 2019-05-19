/*
 arm-none-eabi-as putchar_mem.s -o putchar_mem.o
 arm-none-eabi-ld putchar_mem.o -Ttext 0x00010000 -o putchar_mem.elf
 arm-none-eabi-objcopy putchar_mem.elf -O binary putchar_mem.bin
 qemu-system-arm -M versatilepb -m 128M -nographic -kernel putchar_mem.bin -serial mon:stdio
*/
.globl _start
_start:
  ldr r13,=0x08000000
  ldr r0,=msg1
  bl print
  ldr r0,=msg2
  bl print
end:
  b end

putchar:
  stmdb r13!, {r1}
  ldr r1,=0x101f1000
  str r3, [r1]
  ldmia r13!, {r1}
  mov r15, r14


print:
  stmdb r13!, {r3, r14}
  ldr r1,=0x101f1000
  ldrb r3,[r0]
_loop:  


  bl putchar
  add r0, r0, #1
  ldrb r3,[r0]
  cmp r3,#0
  bne _loop
  ldmia r13!, {r3, r14}
  mov r15, r14

msg1: .asciz "First text.\n"
msg2: .asciz "Second text!\n"