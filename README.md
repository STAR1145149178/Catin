# Catin
Catin/
│
├── README.md                              项目说明
│
├── arch/                                  ═══ 架构相关 (4 架构) ═══
│   │
│   ├── arm64/                             AArch64 (arm64-v8a)
│   │   ├── linker.ld                      链接脚本, 基址 0x40080000
│   │   ├── boot.S                         EL2→EL1, 清 BSS, 设 VBAR, 起 MMU
│   │   ├── mmu.S                          页表 (16×1GB 块, MAIR/TCR)
│   │   ├── vectors.S                      16 项异常向量 + SVC 系统调用入口
│   │   └── context.S                      上下文切换 + 首次进用户态
│   │
│   ├── arm32/                             AArch32 (armeabi-v7a)
│   │   ├── linker.ld                      基址 0x40008000
│   │   ├── boot.S                         引导 + VBAR 设置
│   │   ├── mmu.S                          MMU 占位
│   │   ├── vectors.S                      SVC 入口 (r7 = syscall 号)
│   │   └── context.S                      上下文切换
│   │
│   ├── x86_64/                            x86-64
│   │   ├── linker.ld                      基址 0x100000, multiboot2
│   │   ├── boot.S                         PML4/PDPT/PD 建立, 长模式切换
│   │   ├── vectors.S                      syscall / IRQ 入口
│   │   └── context.S                      上下文切换 + iretq 到 ring3
│   │
│   └── x86/                               i386 (v86 兼容)
│       ├── linker.ld                      基址 0x100000, multiboot1
│       ├── boot.S                         4MB 大页, 保护模式
│       ├── vectors.S                      int 0x80 系统调用入口
│       └── context.S                      上下文切换
│
├── kernel/                                ═══ 内核主体 ═══
│   │
│   ├── include/                           头文件
│   │   ├── types.h                        基础类型 u8/u16/u32/u64/usize
│   │   ├── arch.h                         架构抽象层 (统一 regs_t + arch_* API)
│   │   ├── catin.h                        内核全局接口汇总
│   │   ├── proc.h                         进程结构 + 接口
│   │   ├── vfs.h                          文件系统接口
│   │   ├── syscall.h                      19 个系统调用号
│   │   ├── xcui.h                         XCUI UI 组件库
│   │   ├── gesture.h                      手势识别
│   │   ├── virtio.h                       virtio-mmio 抽象
│   │   └── codec.h                        解码器统一接口
│   │
│   ├── lib/                               基础库
│   │   ├── string.c                       kstrlen / kmemcpy / kstrcmp / kmemcmp
│   │   ├── printf.c                       ksnprintf / kvsnprintf (支持 %llu/%p)
│   │   └── font8x16.c                     8x16 位图字体 + 13 个图标
│   │
│   ├── mm/                                内存管理
│   │   └── mm.c                           first-fit 内核堆 (kmalloc/kfree/merge)
│   │
│   ├── core/                              核心
│   │   ├── main.c                         kernel_main, 启动序列, init 进程
│   │   ├── arch_common.c                  架构无关公共 (panic/uart/log)
│   │   ├── arm_common.c                   ARM UART + GICv2 + Generic Timer
│   │   ├── x86_common.c                   x86 UART(16550) + PIC + PIT
│   │   └── fdt.c                          FDT 解析 (virtio 基址/中断号)
│   │
│   ├── proc/                              进程
│   │   ├── proc.c                         PCB + fork/exec/wait/exit/yield
│   │   └── syscall.c                      系统调用分发
│   │
│   ├── fs/                                文件系统
│   │   └── vfs.c                          内存文件系统 (inode/dentry/file)
│   │
│   ├── drivers/                           驱动
│   │   ├── fb.c                           ramfb (fw_cfg DMA) / 软件 framebuffer
│   │   ├── virtio_mmio.c                  virtio 总线 + 队列 + 静态池
│   │   ├── virtio_input.c                 virtio-tablet 触摸驱动
│   │   └── input.c                        触摸事件环形队列
│   │
│   ├── ui/                                ═══ XCUI 组件库 ═══
│   │   ├── gesture.c                      手势状态机 (下拉/上拉/点击)
│   │   ├── xcui_core.c                    主题 / 帧 / 像素 / 裁剪栈
│   │   ├── xcui_draw.c                    矩形/圆角/阴影/渐变/圆形 (行扫描)
│   │   ├── xcui_text.c                    位图文本渲染
│   │   ├── xcui_input.c                   输入抽象 (down/move/up/click)
│   │   ├── xcui_widget.c                  按钮/开关/滑块/卡片/状态栏/导航栏
│   │   └── xcui_toast.c                   Toast 通知 (4 种类型)
│   │
│   ├── apps/                              应用
│   │   ├── demo.c                         桌面 (快捷开关 / 亮度 / 系统信息)
│   │   ├── panels.c                       通知中心 + 控制中心
│   │   ├── shell.c                        串口控制台
│   │   └── codec_test.c                   解码器自检入口
│   │
│   └── codec/                             ═══ 解码器 (全部自研) ═══
│       ├── codec_common.c                 公共分配器 + 大小端读
│       ├── wav.c                          WAV (RIFF PCM/float, 8/16/24/32 位)
│       ├── mp3.c                          MPEG-1/2 Layer III (Huffman + IMDCT)
│       ├── mp4.c                          ISO BMFF (box 树 + stbl 解析)
│       ├── png.c                          PNG (Deflate + 5 种滤波)
│       ├── jpg.c                          Baseline JPEG (Huffman + IDCT)
│       └── gif.c                          GIF 87a/89a (LZW + 多帧动画)
│
├── user/                                  ═══ 用户态 API ═══
│   ├── include/
│   │   ├── catin.h                        应用 API (catin_*)
│   │   └── syscall.h                      4 架构 syscall 内联汇编包装
│   ├── lib/
│   │   ├── syscall.c                      catin_* 实现
│   │   └── crt0.S                         启动代码 (_start → main → exit)
│   ├── apps/
│   │   └── init.c                         首个用户进程
│   │    wallpaper.png                       壁纸
│   └── Makefile                           4 架构交叉编译
│
└── build/
    └── Makefile                           4 架构构建 + catin.img 打包
    

如需更换壁纸，请在更换后执行
python3 << 'PYEOF'
data = open('user/wallpaper.png', 'rb').read()
with open('kernel/apps/wallpaper.c', 'w') as f:
    f.write('#include "types.h"\n')
    f.write('const unsigned char wallpaper_png[] = {\n')
    for i, b in enumerate(data):
        if i % 12 == 0:
            f.write('  ')
        f.write('0x%02x,' % b)
        if i % 12 == 11:
            f.write('\n')
    f.write('\n};\n')
    f.write('const unsigned int wallpaper_png_len = %d;\n' % len(data))
print("生成成功，字节数:", len(data))
PYEOF

如果没有 root 的安卓环境下必须使用Termux然后进入proot容器
第一步输入
pkg update && pkg upgrade -y
pkg install -y proot-distro
proot-distro install ubuntu
第二步输入
proot-distro login ubuntu --bind /storage/emulated/0:/host
此时你已经进入这个proot容器了
第三步输入安装更多依赖
apt update
apt install -y \
    build-essential \
    gcc-aarch64-linux-gnu \
    gcc-arm-linux-gnueabihf \
    gcc-x86-64-linux-gnu \
    gcc-i686-linux-gnu \
    dosfstools \
    mtools \
    python3 \
    file
第四步输入，用来验证环境
for c in aarch64-linux-gnu-gcc arm-linux-gnueabihf-gcc x86_64-linux-gnu-gcc i686-linux-gnu-gcc mkfs.vfat mcopy python3; do
    command -v $c >/dev/null && echo "✓ $c" || echo "✗ $c"
done
编泽
cd /host/Catin
bash build_all.sh
换壁纸的时候
cd /host/Catin
cp /host/sdcard/新图.png user/wallpaper.png
python3 << 'PYEOF'
data = open('user/wallpaper.png', 'rb').read()
with open('kernel/apps/wallpaper.c', 'w') as f:
    f.write('#include "types.h"\n')
    f.write('const unsigned char wallpaper_png[] = {\n')
    for i, b in enumerate(data):
        if i % 12 == 0: f.write('  ')
        f.write('0x%02x,' % b)
        if i % 12 == 11: f.write('\n')
    f.write('\n};\n')
    f.write('const unsigned int wallpaper_png_len = %d;\n' % len(data))
print("生成成功，字节数:", len(data))
PYEOF
bash build_all.sh
退出当前环境，返回到特尔麦克斯
exit
重新进入无需重装
proot-distro login ubuntu --bind /storage/emulated/0:/host
cd /host/Catin
作者依旧是那个2014出生的这也是那个系统的前身。希望有人来改
外国人自己来翻译。
Foreigners translate it themselves.
