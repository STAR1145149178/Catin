#include "catin.h"
void arch_panic_dump(regs_t *r) {
    arch_irq_disable_all();
    uart_puts("\n*** CATIN PANIC ***\n");
    char b[128];
    for (int i=0; i<16; i++){
        ksnprintf(b,sizeof(b),"r%d=%p  ", i, (void*)r->r[i]);
        uart_puts(b);
        if ((i&3)==3) arch_uart_putc('\n');
    }
    ksnprintf(b,sizeof(b),"pc=%p sp=%p pstate=%p\n",(void*)r->pc,(void*)r->sp_user,(void*)r->pstate);
    uart_puts(b);
    arch_halt();
}
void uart_init(void){ arch_uart_init(); }
void uart_puts(const char *s){ while(*s){ if(*s=='\n') arch_uart_putc('\r'); arch_uart_putc(*s++); } }
void log_info(const char *f,...){
    char b[256]; __builtin_va_list ap; __builtin_va_start(ap,f);
    extern int kvsnprintf(char*,usize,const char*,__builtin_va_list);
    kvsnprintf(b,sizeof(b),f,ap); __builtin_va_end(ap);
    uart_puts("[I] "); uart_puts(b); arch_uart_putc('\n');
}
void log_err(const char *f,...){
    char b[256]; __builtin_va_list ap; __builtin_va_start(ap,f);
    extern int kvsnprintf(char*,usize,const char*,__builtin_va_list);
    kvsnprintf(b,sizeof(b),f,ap); __builtin_va_end(ap);
    uart_puts("[E] "); uart_puts(b); arch_uart_putc('\n');
}
