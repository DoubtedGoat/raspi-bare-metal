# We compile and link separately so we can avoid having to route through the
# GCC tools
# https://groups.google.com/g/llvm-dev/c/TCCSzAICtDI/m/Tmi7I430AAAJ

if (-Not (Test-Path "out/")) {
    New-Item "out" -ItemType Directory
}
Set-Location "out"
$cFiles = Get-ChildItem -Path ../src -Recurse -Filter "*.c"
clang ../src/boot.asm    `
      $cFiles.FullName   `
      -v                 `
      --target=arm       `
      -mcpu=cortex-a7    `
      -mfloat-abi=soft   `
      -Werror            `
      -nostdlib          `
      -c

$oFiles = Get-ChildItem -Path . -Filter "*.o"
ld.lld $oFiles.FullName  `
       -T ../linker.ld   `
       -o kernel.elf

llvm-objcopy --output-target binary kernel.elf kernel7.img

Set-Location ".."
# Rpi 1: arm1176jzf-s
# Rpi 2: Cortext-A7