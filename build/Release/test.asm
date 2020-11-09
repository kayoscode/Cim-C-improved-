.text 
    jmp start

.data
    .const NULL = 0

.text
    label start:
    mov r0, 1
    mov r1, 2
    push [r0, r1]
    mov r0, 12
    mov r1, 12
    pop [r1, r0]
    int 0x0