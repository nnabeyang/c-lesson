  mov r0, r15
  bl print_hex
  mov r0, #0x68
  bl print_hex
end:
  b end
print_hex:
    ldr r1,=0x101f1000
    mov r2, #0x20
loop:
    sub r2, r2, #0x4
    lsr r3, r0, r2
    and r3, r3, #0xF
    cmp r3, #0xA
    bge print_alpha
print_number:
    add r3, r3, #0x30
    b print
print_alpha:
    add r3, r3, #0x37
print:
    str r3, [r1]
    cmp r2, #0x0
    bne loop
    mov r3,#0x0D
    str r3,[r1]
    mov r3,#0x0A
    str r3,[r1]
    mov r15, r14