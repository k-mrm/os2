.macro SYSCALL name, id
.global \name
\name:
        movl    $\id, %eax
        int     $0x80
        ret
.endm

SYSCALL exec, 1
SYSCALL read, 2
SYSCALL write, 3
