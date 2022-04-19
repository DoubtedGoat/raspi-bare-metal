# Description
This repository contains code I am using to brush up on embedded-systems skills using a Raspberry Pi 2. It doesn't _do_ anything, there is no end goal, and there is no particular reason to want to read it except curiosity or boredom.

# Versioning + Releases
This repo will be tagged and "released" regularly as I work on it, with the timeline of releases serving as a dev log as I try accomplish increasing interesting things. Version numbers will follow a variation of semver - a major version increment will constitute a substantial change to the nature of the project, a minor version will be refactorings and improvements to the prior major version, and a patch will be minor corrections and bugfixes.

I will try to update the README with a description of what I've done and why upon each release.


# Version 0.0.1 - Linker Questions: Answered
In version 0.0.0, we stole someone elses code but forgot to use the linker script they provided. The code still ran correctly, raising the question of .. . why.

The linker script in question comes from this gist by William Durand:
https://gist.github.com/willdurand/614ad3ad1cac0189691f67c0ac71b9e6

Most of what it seems to do is to start the code section at 0x8000 (I'm not particularly familiar with linker scripts, which might have to change).

## Starting The Investigation
To start with, I built two versions of the image - one using the linker script, and one without, to compare how they work.

Hex-dumping the two images with `xxd` shows some differences. They both start with the same line (the entrypoint any anything possibly added for stack unwinding?), and contain the same section of interesting data (presumably our code). The first major difference is that the non-script version has a ton of `0000`'s (no-ops) between the entrypoint and the actual code (the first word of code is at `0x00010010`). The script version does not have this long set of no-ops. 

The second major difference is that the linker script version has many lines of `d4d4` after the code section, with seemingly no purpose.

Somewhat unexpectedly, both files have their entrypoint at `0x00000000`, even though the linker script specifies a starting address of `0x8000`.

## Entrypoint location
The first thing I looked into was the entrypoint location, which I had expected to be at `0x8000` for the linker-script version of the image. Looking into the RPi boot process further, I learned that the bootloader puts our kernel file at `0x8000` and then jumps to it - so `0x8000` starting point is not expected to be built into the image file itself. This suggests that the purpose of this offset in the linker script isn't to _place_ the code at a specific location.

More likely, the offset is so that the _program itself_ knows where it is located in memory, in the absence of an OS and configured MMU. If the program is linked with an incorrect idea of where it is located in memory, any instructions that use absolute addresses will be pointed badly. I'd like to _test_ that this is the case, but I don't have a great plan for it currently.

To confirm that the linker instruction is working correctly and I'm not barking up the wrong tree, I used `readelf` to dump the sections of each of the `kernel.elf` files generated during my builds. The section tables look like this:

With Script
```
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .ARM.exidx        ARM_EXIDX       00008000 008000 000010 00  AL  2   0  4
  [ 2] .text             PROGBITS        00008010 008010 0000d4 00  AX  0   0  4
  [ 3] .rodata           PROGBITS        000080e4 0080e4 000f1c 00  AX  0   0  1
  [ 4] .ARM.attributes   ARM_ATTRIBUTES  00000000 009000 00002b 00      0   0  1
  [ 5] .symtab           SYMTAB          00000000 00902c 000080 10      7   5  4
  [ 6] .shstrtab         STRTAB          00000000 0090ac 000044 00      0   0  1
  [ 7] .strtab           STRTAB          00000000 0090f0 000037 00      0   0  1
```

Without Script
```
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .ARM.exidx        ARM_EXIDX       000100d4 0000d4 000010 00  AL  2   0  4
  [ 2] .text             PROGBITS        000200e4 0000e4 0000d4 00  AX  0   0  4
  [ 3] .ARM.attributes   ARM_ATTRIBUTES  00000000 0001b8 00002b 00      0   0  1
  [ 4] .comment          PROGBITS        00000000 0001e3 000029 01  MS  0   0  1
  [ 5] .symtab           SYMTAB          00000000 00020c 000080 10      7   5  4
  [ 6] .shstrtab         STRTAB          00000000 00028c 000045 00      0   0  1
  [ 7] .strtab           STRTAB          00000000 0002d1 000037 00      0   0  1
```

You can see that the ARM_EXIDX section (which seems to be an LLVM-added stack trace unwinding helper, and the real "start" of the program) sits at `0x8000` in the with-script version, and `0x100d4` in the non-script version. This suggests that our linker script is appropriately situating the start of our program in memory, but the `objcopy` to convert it to a machine-code image is taking only the machine-code parts of the ELF, so our entrypoint is going to be at `0x0000` _within the image file_ no matter what. Making sure that machine code gets loaded into the correct part of memory falls to us.

My conclusion here is that I should keep the linker script with offset `0x8000` because I strongly suspect it's important, but I'd like to _prove_ that it's important at a later date.

## D4D4 padding
The next issue to tackle is the mysterious `d4d4` padding. This one ended up being pretty easy. Our linkerscript requests the `.rodata` section be aligned to the next 4k increment. This will insert padding so that the first real word of `.rodata` can be at the next interval of `0x1000` in memory. We can see in our readelf output that the `.rodata` section starts at `0x000080e4` and has a size of `0x000f1c`. That would put the next word after this section at `0x9000` . . . which is 4k aligned. This looks fishy (this is the `.rodata` section? Isn't that the one we _wanted_ to be aligned, but you're saying the _next_ section would start there?) until you consider . . . our program doesn't have any `.rodata`. There's no constant data in our very simple example, so all the ends up existing in this section is padding, and that padding makes it's way into our machine-code image file because it's technically part of a code section.

## Many many no-ops
The no-op issue is still somewhat unresolved. The readelf output shows the `.text` (code) sections of both ELF's being the same, The `.ARM.exidx` sections are _also_ the same size. The only notable difference is that there is a large offset between the end of `.ARM.exidx` and start of `.text` in the no-script version. We know that LLVM is adding the `.ARM.exidx` section immediately in front of our `.text` section by default (see question below), so we wouldn't really expect there to be a huge offset.

_However_, the default placement strategy for LLD is kind of arcane - it's an actually programmed in (rather than a Default Linker Script) and makes Decisions on the fly about what to place there (ref: https://groups.google.com/g/llvm-dev/c/3y15MZRgVZ4 ). It's not entirely unreasonable that LLVM would _prefer_ to have this very spacious layout with room for activities, but is content to just slap the section directly onto the front of your `.text` sections if you're forcing a linker script.


## What is the `.ARM.exidx` section, anyway?
Some research suggests that LLVM adds this section to help with stack trace unwinding / exception handling, whether or not it's actually needed. It's dumpable with `readelf -u <.elf>`, and looks like this:

With Script:
```
Unwind table index '.ARM.exidx' at offset 0x8000 contains 2 entries:

0x8010: 0x1 [cantunwind]

0x80e4 <kernel_main+0xc0>: 0x1 [cantunwind]
```

Without Script:
```
Unwind table index '.ARM.exidx' at offset 0xd4 contains 2 entries:

0x200e4: 0x1 [cantunwind]

0x201b8 <kernel_main+0xc0>: 0x1 [cantunwind]
```

so . . . functionally the same. How / why this bit of Data gets to live at what I would have thought would be the first executable instruction is uncertain to me, but I'm not sure it's really interesting or relevant enough to look into right now.

## Conclusion
We're going to use the linker script pretty much as-is for now, on the assumption that it's actively preventing horrible memory-access issues from arising if the code gets more complex. I might remove the `.rodata` alignment requirement, since I wasn't able to see any real use for it - we can discover what it's saving us from for ourselves.

I would also like to _prove_ that the script is preventing bad addresses from being written, but I'm confident enough that that is the case to backburner it for now.

# What's Next
Next we're actually reading the _code_ part to make sure we know what's going on. It likely won't result in any outstanding revelations (or commits, even).

This is likely the last commit in version 0 - next we'll be uprevving to version 1 and trying to get the UART to work