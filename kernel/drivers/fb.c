#include "catin.h"
#if ARCH_ID == 1 || ARCH_ID == 2
#define FW_DATA 0x09020000ULL
#define FW_SEL  0x09020008ULL
#define FW_DMA  0x09020010ULL
static u32 *g_fb; static int g_w, g_h;
static inline u8 fw_rd(u32 off){ return *(volatile u8*)(FW_DATA + off); }
static u16 fw_find(const char *name){
    mmio_w32(FW_SEL,0x19);
    u8 raw[64];
    for(int i=0;i<4;i++) raw[i]=fw_rd(i);
    u32 cnt=((u32)raw[0]<<24)|((u32)raw[1]<<16)|((u32)raw[2]<<8)|raw[3];
    for(u32 i=0;i<cnt;i++){
        for(int j=0;j<64;j++) raw[j]=fw_rd(j);
        u16 s=((u16)raw[4]<<8)|raw[5];
        if(!kstrcmp((const char*)(raw+8),name)) return s;
    }
    return 0;
}
struct fw_dma { u32 ctrl,len; u64 addr; } __attribute__((packed,aligned(8)));
struct ramfb_cfg { u64 addr; u32 fourcc,flags,width,height,stride; } __attribute__((packed,aligned(8)));
static int fw_write(u16 sel,const void*d,u32 len){
    static struct fw_dma dma __attribute__((aligned(8)));
    dma.ctrl=__builtin_bswap32(0x10U|((u32)sel<<16));
    dma.len=__builtin_bswap32(len);
    dma.addr=__builtin_bswap64((u64)(uintptr_t)d);
    mmio_w64(FW_DMA,__builtin_bswap64((u64)(uintptr_t)&dma));
    for(int i=0;i<10000000;i++) if(!mmio_r64(FW_DMA)) return 0;
    return -1;
}
int fb_init(void){
    g_w=SCREEN_W; g_h=SCREEN_H;
    g_fb=(u32*)kmalloc((usize)g_w*g_h*4);
    if(!g_fb) return -1;
    kmemset(g_fb,0,(usize)g_w*g_h*4);
    u16 sel=fw_find("etc/ramfb");
    if(!sel){ log_err("fb: no ramfb"); return -1; }
    static struct ramfb_cfg cfg __attribute__((aligned(8)));
    cfg.addr=__builtin_bswap64((u64)(uintptr_t)g_fb);
    cfg.fourcc=__builtin_bswap32(0x34325258U);
    cfg.flags=0;
    cfg.width=__builtin_bswap32((u32)g_w);
    cfg.height=__builtin_bswap32((u32)g_h);
    cfg.stride=__builtin_bswap32((u32)(g_w*4));
    if(fw_write(sel,&cfg,sizeof(cfg))!=0){ log_err("fb: timeout"); return -1; }
    log_info("fb: %dx%d",g_w,g_h);
    return 0;
}
#else
static u32 *g_fb; static int g_w, g_h;
int fb_init(void){
    g_w=SCREEN_W; g_h=SCREEN_H;
    g_fb=(u32*)kzalloc((usize)g_w*g_h*4);
    if(!g_fb) return -1;
    log_info("fb: sw %dx%d",g_w,g_h);
    return 0;
}
#endif
int fb_w(void){ return g_w; }
int fb_h(void){ return g_h; }
u32 *fb_backbuffer(void){ return g_fb; }
void fb_present(void){}
