#include "catin.h"
#include "xcui.h"
#include "codec.h"

static int wifi_on=1,bt_on=0,dnd_on=0,charging=0,battery=78,brightness=70;
static xc_input_t input;

extern const unsigned char wallpaper_png[];
extern const unsigned int  wallpaper_png_len;
static u32 *g_wallpaper_fb = 0;

static void get_time(int*hh,int*mm){
    u64 secs=arch_timer_ticks()/1000;
    *hh=(9+(int)(secs/3600))%24;
    *mm=(int)((secs/60)%60);
}

static void load_wallpaper(void){
    if (wallpaper_png_len == 0) return;

    image_t wp;
    if (png_decode(wallpaper_png, wallpaper_png_len, &wp) != 0){
        log_err("wallpaper: png_decode failed");
        return;
    }

    int W = fb_w(), H = fb_h();
    int src_w = wp.w, src_h = wp.h;

    g_wallpaper_fb = (u32*)kmalloc((usize)W * H * 4);
    if (!g_wallpaper_fb){ image_free(&wp); return; }

    u32 s_num, s_den;
    if ((u64)W * src_h > (u64)H * src_w){ s_num = W; s_den = src_w; }
    else                                 { s_num = H; s_den = src_h; }

    u32 sw = (u32)((u64)src_w * s_num / s_den);
    u32 sh = (u32)((u64)src_h * s_num / s_den);
    int ox = (W - (int)sw) / 2;
    int oy = (H - (int)sh) / 2;

    for (int y = 0; y < H; y++){
        u32 *dst = g_wallpaper_fb + (usize)y * W;
        int sy = y - oy;
        if (sy < 0 || sy >= (int)sh){
            for (int x = 0; x < W; x++) dst[x] = 0;
            continue;
        }
        int src_y = (int)((u64)sy * s_den / s_num);
        if (src_y < 0) src_y = 0;
        if (src_y >= src_h) src_y = src_h - 1;

        for (int x = 0; x < W; x++){
            int sx = x - ox;
            if (sx < 0 || sx >= (int)sw){ dst[x] = 0; continue; }
            int src_x = (int)((u64)sx * s_den / s_num);
            if (src_x < 0) src_x = 0;
            if (src_x >= src_w) src_x = src_w - 1;

            u32 px = wp.pixels[(usize)src_y * src_w + src_x];
            u8 r = (px >> 24) & 0xFF;
            u8 g = (px >> 16) & 0xFF;
            u8 b = (px >>  8) & 0xFF;
            dst[x] = ((u32)r << 16) | ((u32)g << 8) | b;
        }
    }

    image_free(&wp);
    log_info("wallpaper: %dx%d -> %dx%d cover", src_w, src_h, W, H);
}

static void cb_set(void*ud){ (void)ud; xc_toast(XC_TOAST_INFO,"设置"); }
static void cb_wifi(void*ud,int on){ (void)ud; xc_toast(on?XC_TOAST_OK:XC_TOAST_INFO,"WiFi %s",on?"开":"关"); }
static void cb_bt(void*ud,int on){ (void)ud; xc_toast(on?XC_TOAST_OK:XC_TOAST_INFO,"蓝牙 %s",on?"开":"关"); }
static void cb_dnd(void*ud,int on){ (void)ud; xc_toast(XC_TOAST_INFO,"勿扰 %s",on?"开":"关"); }

void demo_init(void){
    xc_toast(XC_TOAST_OK,"Catin " CATIN_VERSION);
    load_wallpaper();
}

void demo_frame(void){
    xc_input_begin(&input);
    touch_ev_t ev;
    while(touch_poll(&ev)) xc_input_feed(&input,ev);

    if (g_wallpaper_fb){
        int W = fb_w(), H = fb_h();
        u32 *fb = fb_backbuffer();
        kmemcpy(fb, g_wallpaper_fb, (usize)W * H * 4);
    } else {
        xc_clear(xc_theme.bg);
        xc_vgradient(XC_RECT(0,0,fb_w(),360),(xc_color_t){44,88,164,255},xc_theme.bg);
    }

    xc_status_t st;
    get_time(&st.hour,&st.minute);
    st.signal_level=4; st.wifi_on=wifi_on; st.wifi_strength=wifi_on?3:0;
    st.battery=battery; st.charging=charging; st.carrier="CATIN 5G";
    xc_status_bar(&st);

    xc_text(48,100,CATIN_NAME,xc_theme.text,6);
    xc_text(48,230,"XCUI " CATIN_VERSION,xc_theme.text_dim,2);
    xc_text(48,266,CATIN_CODENAME,xc_theme.primary,2);

    xc_rect_t c1={40,390,fb_w()-80,260};
    xc_round(c1,24,(xc_color_t){16,20,30,220});
    xc_text(c1.x+28,c1.y+20,"快捷开关",xc_theme.text,2);
    xc_switch_t s1={.r={c1.x+12,c1.y+72,c1.w-24,60},.title="WiFi",.subtitle="CATIN-5G",.icon=XC_ICON_WIFI,.on=wifi_on,.anim=wifi_on?100:0,.on_change=cb_wifi};
    xc_switch(&s1,&input); wifi_on=s1.on;
    xc_switch_t s2={.r={c1.x+12,c1.y+136,c1.w-24,60},.title="蓝牙",.subtitle="未连接",.icon=XC_ICON_BT,.on=bt_on,.anim=bt_on?100:0,.on_change=cb_bt};
    xc_switch(&s2,&input); bt_on=s2.on;
    xc_switch_t s3={.r={c1.x+12,c1.y+196,c1.w-24,60},.title="勿扰",.subtitle="静音",.icon=XC_ICON_MOON,.on=dnd_on,.anim=dnd_on?100:0,.on_change=cb_dnd};
    xc_switch(&s3,&input); dnd_on=s3.on;

    xc_rect_t c2={40,670,fb_w()-80,140};
    xc_round(c2,24,(xc_color_t){16,20,30,220});
    xc_text(c2.x+28,c2.y+20,"亮度",xc_theme.text,2);
    xc_slider_t sl={.r={c2.x+16,c2.y+70,c2.w-32,56},.icon=XC_ICON_MOON,.value=brightness};
    xc_slider(&sl,&input); brightness=sl.value;

    xc_rect_t c3={40,830,fb_w()-80,220};
    xc_round(c3,24,(xc_color_t){16,20,30,220});
    xc_text(c3.x+28,c3.y+20,"系统",xc_theme.text,2);
    xc_list_item(XC_RECT(c3.x+8,c3.y+68,c3.w-16,56),XC_ICON_INFO,"版本",CATIN_VERSION,&input);
    xc_list_item(XC_RECT(c3.x+8,c3.y+124,c3.w-16,56),XC_ICON_SET,"进程","查看",&input);
    xc_list_item(XC_RECT(c3.x+8,c3.y+180,c3.w-16,56),XC_ICON_HOME,"文件","/",&input);

    static const char*nl[]={"主页","应用","通知","设置"};
    static const char*ni[]={XC_ICON_HOME,XC_ICON_APPS,XC_ICON_BELL,XC_ICON_SET};
    xc_nav_bar(0,nl,ni,4);

    if(input.holding||input.x||input.y){
        xc_color_t cur=input.holding?xc_theme.primary:(xc_color_t){200,220,255,180};
        xc_circle(input.x,input.y,12,cur);
        xc_circle(input.x,input.y,4,XC_RGB(255,255,255));
    }
}