# We compile and link separately so we can avoid having to route through the
# GCC tools
# https://groups.google.com/g/llvm-dev/c/TCCSzAICtDI/m/Tmi7I430AAAJ

Set-Location "test"
if (-Not (Test-Path "out/")) {
    New-Item "out" -ItemType Directory
}
Set-Location "out"
$cFiles = Get-ChildItem -Path ../../src -Recurse -Filter "*.c"
$testFiles = Get-ChildItem -Path ../ -Filter "*.c"
clang $cFiles.FullName    `
      $testFiles.FullName `
      -v                  `
      -Werror             `
      -o tests.exe

Set-Location "../.."

.\test\out\tests.exe
# Rpi 1: arm1176jzf-s
# Rpi 2: Cortext-A7