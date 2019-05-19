/*
 arm-none-eabi-as putchar_bad.s -o putchar_bad.o
 arm-none-eabi-ld putchar_bad.o -Ttext 0x00010000 -o putchar_bad.elf
 arm-none-eabi-objcopy putchar_bad.elf -O binary putchar_bad.bin
 qemu-system-arm -M versatilepb -m 128M -nographic -kernel putchar_bad.bin -serial mon:stdio
*/
.globl _start
_start:

  ldr r0,=msg1
  bl print
  ldr r0,=msg2
  bl print
end:
  b end

/*
  putchar:
    arg r0: target character which is printed.
    used internal register: r1.
*/
putchar:
  ldr r1,=0x101f1000
  str r0, [r1]
  mov r15, r14


/*
  print:
    arg r0: Address of target string, must end by \0.
    used internal register: r0, r1, r3.
*/
print:
  ldrb r3,[r0]
  ldr r1,=0x101f1000
  mov r2, r0
  mov r4, r14
  ldrb r3,[r2]
_loop:
  // TODO: use putchar here someway.
  //

  mov r0, r3
  bl putchar
  add r2, r2, #1
  ldrb r3,[r2]
  cmp r3,#0
  bne _loop
  mov r15, r4

msg1: .asciz "First text.\n"
msg2: .asciz "Second text!\n"