    .section ".text._start","ax"
#if defined(__aarch64__)
    .global _start
_start:
    bl main
    mov x0, #0
    mov x8, #0
    svc 0
    b .
#elif defined(__arm__)
    .global _start
    .arm
_start:
    bl main
    mov r0, #0
    mov r7, #0
    svc 0
    b .
#elif defined(__x86_64__)
    .global _start
    .code64
_start:
    call main
    mov $0, %rax
    syscall
    hlt
#elif defined(__i386__)
    .global _start
    .code32
_start:
    call main
    mov $0, %eax
    int $0x80
    hlt
#endif
