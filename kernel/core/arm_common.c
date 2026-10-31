#include "catin.h"
#if ARCH_ID == 1 || ARCH_ID == 2
static u64 U = 0;
void arch_uart_init(void){
    U = 0x09000000ULL;
    mmio_w32(U+0x30,0); mmio_w32(U+0x24,13); mmio_w32(U+0x28,1);
    mmio_w32(U+0x2C,(3U<<5)|(1U<<4));
    mmio_w32(U+0x30,(1U<<0)|(1U<<8)|(1U<<9));
}
void arch_uart_putc(char c){ if(!U) return; while(mmio_r32(U+0x18)&(1U<<5)); mmio_w32(U,(u32)(u8)c); }
int arch_uart_rx_ready(void){ return U?!(mmio_r32(U+0x18)&(1U<<4)):0; }
int arch_uart_getc(void){ while(!arch_uart_rx_ready()); return (int)(mmio_r32(U)&0xFF); }

static u64 GICD=0x08000000ULL, GICC=0x08010000ULL;
static u32 g_lines = 0;
void arch_irq_init(void){
    mmio_w32(GICD+0x000, 0);
    u32 typer = mmio_r32(GICD+0x004);
    u32 lines = ((typer & 0x1F) + 1) * 32;
    if (lines < 32) lines = 32;
    if (lines > 1024) lines = 1024;
    g_lines = lines;
    for (u32 i=0; i<lines/32; i++) mmio_w32(GICD + 0x180 + i*4, 0xFFFFFFFFU);
    for (u32 i=0; i<lines/4;  i++) mmio_w32(GICD + 0x400 + i*4, 0xA0A0A0A0U);
    for (u32 i=0; i<lines/4;  i++) mmio_w32(GICD + 0x800 + i*4, 0x01010101U);
    mmio_w32(GICD+0x000, 1);
    mmio_w32(GICC+0x004, 0xFF);
    mmio_w32(GICC+0x000, 1);
    log_info("gic: lines=%u", lines);
}
void arch_irq_enable(int irq){
    if (irq<0 || (u32)irq>=g_lines) return;
    mmio_w32(GICD+0x100+(irq/32)*4, 1U<<(irq%32));
}
void arch_irq_enable_all(void){
#if ARCH_ID == 1
    __asm__ volatile("msr daifclr, #2":::"memory");
#else
    __asm__ volatile("cpsie i":::"memory");
#endif
}
void arch_irq_disable_all(void){
#if ARCH_ID == 1
    __asm__ volatile("msr daifset, #2":::"memory");
#else
    __asm__ volatile("cpsid i":::"memory");
#endif
}

static volatile u64 g_ticks = 0;
static u32 g_hz = 0, g_period = 0;
void arch_timer_init(u32 hz){
    g_hz = hz ? hz : 1000;
#if ARCH_ID == 1
    u64 f; __asm__ volatile("mrs %0, cntfrq_el0":"=r"(f));
    g_period = (u32)(f/g_hz);
    __asm__ volatile("msr cntp_ctl_el0, %0"::"r"(0ULL));
    __asm__ volatile("msr cntp_tval_el0, %0"::"r"((u64)g_period));
    __asm__ volatile("msr cntp_ctl_el0, %0"::"r"(1ULL));
    __asm__ volatile("isb");
#else
    g_period = 24000000 / g_hz;
    __asm__ volatile("mcr p15, 0, %0, c14, c2, 0"::"r"(g_period));
    __asm__ volatile("mcr p15, 0, %0, c14, c2, 1"::"r"(1U));
#endif
    g_ticks = 0;
    log_info("timer: %u Hz", g_hz);
}
u64 arch_timer_ticks(void){ return g_ticks; }
u32 arch_timer_hz(void){ return g_hz; }
void arm_timer_tick_irq(void){
#if ARCH_ID == 1
    __asm__ volatile("msr cntp_tval_el0, %0"::"r"((u64)g_period));
#else
    __asm__ volatile("mcr p15, 0, %0, c14, c2, 0"::"r"(g_period));
#endif
    g_ticks++;
}
void arch_halt(void){ for(;;) __asm__ volatile("wfi"); }
void arch_wfi(void){ __asm__ volatile("wfi"); }
void arch_early_init(void){}

extern void touch_tick(void);
extern u32 g_virtio_irq;

void arch_irq_handler(regs_t *r){
    u32 iar = mmio_r32(GICC+0x0C);
    int irq = iar & 0x3FF;
    if (irq < 1020){
        if (irq == 30) arm_timer_tick_irq();
        if (g_virtio_irq && (u32)irq == g_virtio_irq) touch_tick();
        mmio_w32(GICC+0x10, iar);
    }
    proc_timer_tick();
    (void)r;
}
void arch_syscall_handler(regs_t *r){
    proc_save_regs(r);
    u64 ret = syscall_dispatch(r->syscall_nr, r->args[0], r->args[1],
                                r->args[2], r->args[3], r->args[4], r->args[5]);
    r->r[0] = ret;
}
#endif