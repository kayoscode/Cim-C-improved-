.text 
    jmp start

.data
    .const NULL = 0

    label string:
    .asciiz "string to reverse"

.text
    label start:
    mov sp, 0xFFFF * 10

    la r0, string
    push r0
    call reverseString

    la r0, string
    lb r1, r0[1]
    lb r2, r0[2]
    lb r0, r0[0]
    int 0x0

    label reverseString:
    pop r0
    mov r3, r0

    label pushStr:
    lb r1, r0[0]
    push r1
    add r0, r0, 1
    cmp r1, NULL
    jne pushStr

    mov r0, r3
    pop r1

    label popStr:
    pop r1
    sb r1, r0[0]
    lb r1, r0[1]
    add r0, r0, 1
    cmp r1, NULL
    jne popStr

    jmp ra