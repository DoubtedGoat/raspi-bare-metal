# We compile and link separately so we can avoid having to route through the
# GCC tools
# https://groups.google.com/g/llvm-dev/c/TCCSzAICtDI/m/Tmi7I430AAAJ

if (-Not (Test-Path "out/")) {
    New-Item "out" -ItemType Directory
}
Set-Location "out"

clang ../src/boot.asm   `
      ../src/kernel.c   `
      -v                `
      --target=arm      `
      -mcpu=cortex-a7   `
      -mfloat-abi=soft  `
      -Werror           `
      -nostdlib         `
      -c

ld.lld boot.o         `
       kernel.o       `
       -o kernel.elf

llvm-objcopy --output-target binary kernel.elf kernel7.img

Set-Location ".."
# Rpi 1: arm1176jzf-s
# Rpi 2: Cortext-A7