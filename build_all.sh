#!/bin/bash
cd "$(dirname "$0")"

CROSS[1]="aarch64-linux-gnu-";   NAME[1]="a64";  DISP[1]="arm64-v8a"
CROSS[2]="arm-linux-gnueabihf-"; NAME[2]="a32";  DISP[2]="armeabi-v7a"
CROSS[3]="x86_64-linux-gnu-";    NAME[3]="x64";  DISP[3]="x86_64"
CROSS[4]="i686-linux-gnu-";      NAME[4]="x86";  DISP[4]="x86"

CFLAGS_COMMON="-ffreestanding -nostdlib -nostdinc -fno-builtin -fno-pic -fno-pie -Wall -Wextra -O2 -fno-common -fno-stack-protector -Ikernel/include"
CFLAGS_A64="$CFLAGS_COMMON"
CFLAGS_A32="$CFLAGS_COMMON -march=armv7-a -marm -mfpu=neon-vfpv4 -mfloat-abi=hard"
CFLAGS_X64="$CFLAGS_COMMON -mno-red-zone -mcmodel=large"
CFLAGS_X86="$CFLAGS_COMMON -m32"

COMMON_C=(
    kernel/core/main.c
    kernel/lib/string.c kernel/lib/printf.c kernel/lib/mem.c kernel/lib/font8x16.c
    kernel/mm/mm.c
    kernel/core/arch_common.c
    kernel/fs/vfs.c
    kernel/proc/proc.c kernel/proc/syscall.c
    kernel/ui/gesture.c kernel/ui/xcui_core.c kernel/ui/xcui_draw.c
    kernel/ui/xcui_text.c kernel/ui/xcui_input.c kernel/ui/xcui_widget.c
    kernel/ui/xcui_toast.c
    kernel/apps/demo.c kernel/apps/panels.c kernel/apps/shell.c
    kernel/apps/codec_test.c kernel/apps/wallpaper.c
    kernel/codec/codec_common.c
    kernel/codec/wav.c kernel/codec/mp3.c kernel/codec/mp4.c
    kernel/codec/png.c kernel/codec/jpg.c kernel/codec/gif.c
)
ARM_C=(
    kernel/core/arm_common.c kernel/core/fdt.c
    kernel/drivers/fb.c kernel/drivers/virtio_mmio.c
    kernel/drivers/virtio_input.c kernel/drivers/input.c
)
X86_C=(
    kernel/core/x86_common.c
    kernel/drivers/fb.c kernel/drivers/input.c \
    kernel/drivers/virtio_mmio.c kernel/drivers/virtio_input.c
)

A64_ASM=( arch/arm64/boot.S arch/arm64/mmu.S arch/arm64/vectors.S arch/arm64/context.S )
A32_ASM=( arch/arm32/boot.S arch/arm32/mmu.S arch/arm32/vectors.S arch/arm32/context.S )
X64_ASM=( arch/x86_64/boot.S arch/x86_64/vectors.S arch/x86_64/context.S )
X86_ASM=( arch/x86/boot.S arch/x86/vectors.S arch/x86/context.S )

line() { printf '%.0s─' $(seq 1 60); echo; }

build_one() {
    local i=$1
    local cross=${CROSS[$i]} name=${NAME[$i]} disp=${DISP[$i]}
    local cflags ldfile asm_list

    case $i in
      1) cflags="$CFLAGS_A64"; ldfile="arch/arm64/linker.ld"; asm_list=("${A64_ASM[@]}") ;;
      2) cflags="$CFLAGS_A32"; ldfile="arch/arm32/linker.ld"; asm_list=("${A32_ASM[@]}") ;;
      3) cflags="$CFLAGS_X64"; ldfile="arch/x86_64/linker.ld"; asm_list=("${X64_ASM[@]}") ;;
      4) cflags="$CFLAGS_X86"; ldfile="arch/x86/linker.ld"; asm_list=("${X86_ASM[@]}") ;;
    esac

    line
    echo "  [$disp] 编译中..."
    line

    if ! command -v "${cross}gcc" >/dev/null 2>&1; then
        echo "  ✗ 未找到 ${cross}gcc，跳过"
        return 1
    fi

    local outdir="out/$name"
    rm -rf "$outdir"; mkdir -p "$outdir"
    local objs=()

    for src in "${asm_list[@]}"; do
        local obj="$outdir/$(basename ${src%.S}).o"
        if ! msg=$("${cross}gcc" $cflags -c "$src" -o "$obj" 2>&1); then
            echo "  ✗ 汇编失败: $src"
            echo "$msg" | head -5
            return 1
        fi
        objs+=("$obj")
    done

    local c_list=("${COMMON_C[@]}")
    if [ $i -le 2 ]; then c_list+=("${ARM_C[@]}"); else c_list+=("${X86_C[@]}"); fi

    local n=0
    for src in "${c_list[@]}"; do
        local obj="$outdir/$(basename ${src%.c}).o"
        if ! msg=$("${cross}gcc" $cflags -c "$src" -o "$obj" 2>&1); then
            echo ""
            echo "  ✗ 编译失败: $src"
            echo "$msg" | head -15
            return 1
        fi
        objs+=("$obj")
        n=$((n+1)); printf "."
        [ $((n % 40)) -eq 0 ] && echo ""
    done
    echo ""
    echo "  ✓ 编译完成，开始链接..."

    local elf="$outdir/kernel.elf"
    if ! msg=$("${cross}gcc" -nostdlib -no-pie -static -T "$ldfile" -o "$elf" "${objs[@]}" -lgcc 2>&1); then
        echo "  ✗ 链接失败"
        echo "$msg" | head -15
        return 1
    fi

    local img="kernel_${name}.img"
    "${cross}objcopy" -O binary "$elf" "$img"
    echo "  ✓ $img  ($(stat -c%s $img) bytes)"
    return 0
}

echo ""
echo "════════════════════════════════════════════════════════════"
echo "  Catin 2.2  —  多架构构建"
echo "════════════════════════════════════════════════════════════"
echo ""

ok=0; fail=0; built=()
for i in 1 2 3 4; do
    if build_one $i; then
        ok=$((ok+1)); built+=("kernel_${NAME[$i]}.img")
    else
        fail=$((fail+1))
    fi
    echo ""
done

if [ $ok -gt 0 ]; then
    line
    echo "  打包 catin.img (含 $ok 个内核)"
    line
    rm -f catin.img
    dd if=/dev/zero of=catin.img bs=1M count=96 status=none
    mkfs.vfat -F 32 -n CATIN catin.img >/dev/null
    for img in "${built[@]}"; do
        mcopy -i catin.img "$img" "::/$img"
        echo "  + $img"
    done
    echo "  ✓ catin.img  ($(stat -c%s catin.img) bytes)"
fi

echo ""
line
echo "  汇总:  成功 $ok / 4   失败 $fail / 4"
line
if [ $ok -gt 0 ]; then
    echo "  产物:"
    for img in "${built[@]}"; do echo "    $img"; done
    [ -f catin.img ] && echo "    catin.img  ← 打包镜像"
fi
echo ""
