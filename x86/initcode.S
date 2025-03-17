 .text
 main:
        movl    $0, %eax
        int     $0x80
        jmp     main
