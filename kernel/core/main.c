#include "catin.h"
#include "codec.h"
extern u32 g_virtio_irq;
extern void codec_run_tests(void);
static void banner(void){
    uart_puts("\n");
    uart_puts("  Catin Kernel " CATIN_VERSION " (" CATIN_CODENAME ")\n");
    uart_puts("  arch: " ARCH_NAME "\n");
    uart_puts("  codecs: WAV MP3 MP4 PNG JPG GIF\n\n");
}
void kernel_main(u64 dtb){
    arch_early_init();
    arch_uart_init();
    banner();
#if ARCH_ID == 1 || ARCH_ID == 2
    if (dtb) fdt_parse(dtb);
#endif
    mm_init();
    arch_irq_init();
    arch_timer_init(1000);
#if ARCH_ID == 1 || ARCH_ID == 2
    arch_irq_enable(30);
    arch_irq_enable((int)g_virtio_irq);
#endif
    arch_irq_enable_all();
    vfs_init();
    proc_init();
    syscall_init();
    if (fb_init() != 0) log_err("fb failed");
    virtio_init();
    if (touch_init() != 0) log_err("touch failed");
    codec_run_tests();
    proc_t *init = proc_alloc();
    if (init){
        init->user_base = 0x200000;
        init->user_size = 0x100000;
        init->stack_top = 0x300000;
        init->heap_top  = 0x280000;
        kmemset(&init->regs, 0, sizeof(regs_t));
        init->regs.pc      = init->user_base;
        init->regs.sp_user = init->stack_top;
        kstrncpy(init->name, "init", PROC_NAME_MAX);
        log_info("starting init pid=%d", init->pid);
        proc_start_first(init);
    }
    for(;;) arch_wfi();
}
