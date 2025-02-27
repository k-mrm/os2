// %rdi: Context *prev 
// %rsi: Context *next
// %rdx: Proc *pprev
.text
.global cswitch
cswitch: 
  pushq   %rbp
  pushq   %rbx
  pushq   %r12
  pushq   %r13
  pushq   %r14
  pushq   %r15

  movq    %rsp, (%rdi)
  movq    (%rsi), %rsp

  popq    %r15
  popq    %r14
  popq    %r13
  popq    %r12
  popq    %rbx
  popq    %rbp

  jmp     __cswitch

.global forkret
forkret:
  call    schedtail
  jmp     trapret
