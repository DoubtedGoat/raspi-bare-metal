# Description
This repository contains code I am using to brush up on embedded-systems skills using a Raspberry Pi 2. It doesn't _do_ anything, there is no end goal, and there is no particular reason to want to read it except curiosity or boredom.

# Versioning + Releases
This repo will be tagged and "released" regularly as I work on it, with the timeline of releases serving as a dev log as I try accomplish increasing interesting things. Version numbers will follow a variation of semver - a major version increment will constitute a substantial change to the nature of the project, a minor version will be refactorings and improvements to the prior major version, and a patch will be minor corrections and bugfixes.

I will try to update the README with a description of what I've done and why upon each release.


# Version 0 - Project Inception and Blinky Lights

## Goals
The initial goal for the project was to run a bare-metal program on some kind of single board computer or microcontroller. The primary skills I was hoping to exercise were finding and reading documentation, toolchain configuration, and terminology. It's been 8 years since I've done any kind of work on embedded systems or microcontrollers, so we're starting just by shaking the dust off (and catching up on what I might have missed).

Deliverable #1 is the simplest of all projects: Blink an LED.

## Design decisions
### Why Raspbery Pi 2?
It's what I could find in my basement.

### Why LLVM+Clang?
Mostly because it's cool. LLVM+Clang also installs very easily on Windows, and supports cross-compilation as a first-class feature (sort of), making it feel like a strong choice. Being able to run entirely using LLVM tooling would prevent me from having to install GCC toolchains on Windows.

### Why Windows?
Mostly "why not". I've been an obstinate Linux user for my entire career (including my time at an all-Windows C#+ASP.NET shop), and I'm extremely comfortable working in Linux (and WSL). My time doing Windows Stuff showed me that Windows tooling is Largely OK and I just haven't taken the time to get as familiar with it as I have with Linux. Since my home computer runs Windows, and working with WSL can be a bit of a headache, it seems like now is the time to learn some new tools.

### Why no build system (CMake)?
I'm going to start out building with just a Powershell script because it puts me directly in touch with the Clang and LLD interfaces, which is helpful for learning what types of things I need them to do. CMake is also another layer of difficulty and complexity that I feel is best reserved for another time.

This project may grow Makefiles or CMake scripts in the future, but to start with it's not complex enough for the gain to be worth the pain.

## The Process
### Hardware Checkout
The first step here was verifying that the Raspberry Pi _still worked_. It was salvaged from my partners interactive-design project from college and had been living in our basement for several years. I would hate to spend hours puzzling over why my lights don't blink when the real problem is the Pi itself.

To do this, I followed the Officially Sanctioned Raspberry Pi guide to boot Raspbian. If the Pi could successfully boot Linux (and send debug output over the UART, a hardware feature I knew I wanted to use) it was probably in good enough shape to do anything I'd ask it to do.

This step resulted in having to dig up a micro-USB power supply capable of powering the Pi without Raspbian issuing low-voltage warnings, or failing when a USB device was plugged in.

I also had to modify the `config.txt` file on the boot SD to include `uart_2ndstage=1` to get debug output on the UART, to prove it was working. This particular bit of configuration was discovered through a blog post - it's still unclear where in the RPi documentation I would be able to find it.

### Blinky Lights
With the hardware proven to be Good Enough, it was time to actually blink some LEDs. I was able to find a blog post that did exactly what I was trying to do - run a bare-metal program that blinked an LED. Credit to William Durand for putting up a fairly comprehensive guide here: https://williamdurand.fr/2021/01/23/bare-metal-raspberry-pi-2-programming/ .

The guide is useful, and provided me with assumed-good code to run, but there were two issues:
1) The guide builds with GCC (and we'd settled on Clang for mostly arbitrary reasons)
2) Copy + pasting code isn't the most educational thing you can do

### Building with Clang
#### First Steps
The first challenge to tackle was to see if I could get Mr. Durand's code built+linked with Clang/LLVM, rather than GCC. Finding comprehensive references for cross-compiling with Clang was a bit of a challenge, but I was able to track down two useful bits of information:

The Clang arguments reference - https://clang.llvm.org/docs/ClangCommandLineReference.html

This StackOverflow post - https://stackoverflow.com/questions/14697614/clang-cross-compiling-for-arm

Between the two, I was able to figure out the most important command line arguments: `-target` for setting target architecture, `-mcpu` for setting specific target CPU (a Cortex-A7, for the RPi 2), `-mfloat-abi` for setting the floating point implementation, and `-nostdlib` to prevent automatically linking the C standard library.

The exact name for the `-mcpu` flag was found using the `-print-cupported-cpus` option.

The value of `-mfloat-abi` may be incorrect - soft float is a safe choice when you don't know what instructions / accelerators are available. I'm currently not aware of whether the RPi part has a NEON floating point accelerator or not (I don't _think_ so, but I don't know for sure. We'll tackle it later).

#### Dealing with Assembly
I spent far too long reading about dealing with assembly using Clang/LLVM and not nearly enough time actually trying it out. My initial assumptions were that I would have to specific an ARM assembler to be used and do some toolchain configuration. It turns out that's not true - Clang has an integrated ARM assembler that is mostly-compatible with GAS syntax. Passing the assembly file into the Clang command Just Works.

One note - Mr. Durands files use a `.S` extension for non-preprocessed assembly, which is a Unix convention. I had to swap the extension for `.asm`, the Windows convention. Preprocessed assembly uses the `.s` extension - which in the Windows non-case-sensitive world would cause a conflict with the `.S` extension for files in need of preprocessing.

#### Linking it all together
The one curveball I was thrown while setting up the build script was an issue with linking. One of the drivers for me wanting to use LLVM for building was to avoid having to install and manage a GCC toolchain. It turns out that by default, Clang will delegate linking to `gcc` (so GCC can pass it on to `ld` after doing some GCC-specific magic with the flags, I guess?).

This caused an issue in my build script, as `gcc` is not installed - resulting in a linker error. Turning on the `-v` verbose flag in Clang showed that the error was a "command not found" error when trying to run `gcc`. This _should_ be solvable by using a linker directly, rather than trying to proxy through `gcc`.

LLVM has a linker (`lld`), and forcing Clang compilation to use `lld` is (supposed to be) as simple as adding the `-fuse-ld=lld` argument - causing Clang to use `lld` directly for linking. However, setting this flag did nothing to resolve the error. Running `clang` with the `-c` option and manually linking the resulting `.o` files with `lld` seemed to work, but `-fuse-ld=lld` was resulting in `clang` continuing to attempt to use `gcc` to link.

The most information I could find about this issue is this thread from 2017:
https://groups.google.com/g/llvm-dev/c/TCCSzAICtDI/m/Tmi7I430AAAJ

Apparently using the `-target` flag conflicts with the `-fuse-ld` flag, and causes Clang to fall back onto it's default linking behavior. The thread doesn't seem to propose a fix (it focuses mostly on the other issues presented), so my assumption is that this situation persists. It's entirely unclear if it's a bug, or intentional due to some limitation of `lld` that's going to destroy me three days from now.

#### Missing link?
At this point, I was able to create a working image with `llvm-objcopy` to convert the `.elf` to a raw binary image. I confirmed that it worked by loading it onto the SD card and watching the blinky lights, then rebuilding it with different values for the loop "wait intervals" and confirming that the light blink frequency changed.

However, it turns out there is a piece missing . . . my linking command doesn't use the `linker.ld` script provided by Mr. Durand. It seems like the only critical function of the script is to ensure that our executable code starts at `0x8000` (which is where the Broadcom boot firmware jumps to after it finishes initialization).

## What's Next
I want to investigate _why_ I'm able to boot without specifying the linker script, which shouldn't be a particularly big endeavor.

After that, I'll spend some time poking around at Mr. Durands code to make sure I understand what it's doing in order to get things running.

Then I'll be done with Version 0, and move on to accessing peripherals - specifically the UART.