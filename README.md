# Description
This repository contains code I am using to brush up on embedded-systems skills using a Raspberry Pi 2. It doesn't _do_ anything, there is no end goal, and there is no particular reason to want to read it except curiosity or boredom.

# Versioning + Releases
This repo will be tagged and "released" regularly as I work on it, with the timeline of releases serving as a dev log as I try accomplish increasing interesting things. Version numbers will follow a variation of semver - a major version increment will constitute a substantial change to the nature of the project, a minor version will be refactorings and improvements to the prior major version, and a patch will be minor corrections and bugfixes.

I will try to update the README with a description of what I've done and why upon each release.


# Version 1.0.0 - Assuming Direct (UART) Control
Our first major version! I have gotten the UART to Do Stuff, successfully. This was _hard_ - much, much harder than it should have been, largely due to missing information and poor tooling. I was able to find pretty decent documentation to work from (as well as two Github projects attempting to do the same thing), but I don't have an oscilloscope or logic analyzer to actually test outputs, so my only feedback was "is it Completely Working or is it Not" via connecting a serial emulator (PuTTY). This also left me open to serial emulator mis-configuration issues as a possible source of failure.

All of these factors combined lead to a relatively slow implementation. There was a "code complete" implementation with all the broad strokes correct fairly quickly, but minor details trickled in as I did more research. Major thanks to Scott Teal and Mike St. Jean for giving the code a read and catching some nasty mistakes I had been overlooking.

## Design Goals
There are two UARTs on the Raspberry pi - UART0 is a fully featured 16550-esque UART, and UART1 is a "mini-UART" with some subset of 16550 features. The documentation states that the Mini-UART is intended to be used as a console. I've since lost the reference for where I found that information, and _other_ documentation ( https://www.raspberrypi.com/documentation/computers/configuration.html#raspberry-pi-zero-1-2-and-3 ) now leads me to believe that UART0 is intended as the primary UART . . . but I didn't encounter that until I'd already targeted the mini-UART, and I'm too stubborn to change course ( this is a mistake ).

Our goal with "controlling the UART" is to get a simple program running that echoes inputs. I'm not too concerned with code quality or organization at this point - I want to be able to get data off the chip to somewhere I can see it, so I've got a better outlet for debugging in the future.

## Sketchy Documents
My best reference for controlling the UART is this:

https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf

This is the peripheral documentation for the BCM2835 . . . the version of the chip from the RPi 1. The first major task here is figuring out how much this diverges from the actual situation on the RPi 2. Some forum posts suggest that the peripheral memory layout is identical between the BCM2835 and BCM2836, and only the base physical address has changed.

William Durands ( @willdurand ) code I was referencing during board bring-up puts the GPIO function selection 4 register at `0x3F200010`, which suggests the base physical address for the peripheral region is at `0x3F000000`. I was able to corroborate this with information from a forum post, but I've since lost the reference. Later in the process, I was also able to confirm that the base address I had chosen lined up with what other people had chosen when doing similar projects.


## First Steps
Knowing all of this, I was able sketch out all the basic configuration:
 - Enable mini-UART
 - Set it to 8bit mode
 - Turn off flow control
 - Set up TX/RX

At this point I didn't yet know that the 2ndstage bootloader was using UART0 - I was operating under the false assumption that UART0 was _probably_ basically already set up right (specifically, GPIO function and baud rate), and all my config was just Going Through The Motions. This, obviously, didn't work.

I took several passes at scrutinizing the code in-depth to see what I was missing, and fixed some minor errors. Perhaps the most important was moving "Enable the UART" from the final step (where I had assumed "it's configured, now turn it on") to the first step (because the docs clearly state you can't write any of the config registers while it's not enabled).

## Outside Help
Since I don't write the dev logs In Real Time while developing, the order of events here might be a little sketchy. After having my first pass fail with no obvious cause, I set out on the internet to see what other people had done, and what steps I might be missing. I came across this post, by Andre Leiradella ( @leiradel ):

https://leiradel.github.io/2019/02/10/The-Mini-UART.html

which ended up being supremely useful. This was my first hint that I _couldn't_ expect the mini-UART to be pre-configured - his code was setting GPIO function for the desired pins, setting the baudrate, _and_ disabling pullups on the UART GPIO pins (something I had not even considered).

At this point I updated my configuration to take the same steps as Mr. Leiradella's, comparing also against the relevant sections of documentation. One thing to note . . . I stole Mr. Leiradella's baud rate calculation, because I somehow failed to find the formula in the BCM2835 docs, despite Mr. Leiradella _explicitly stating_ that it was there.

After making sure I went through the same configuration steps as what I assumed was a working piece of code, the UART was _still_ failing to output anything that I could pick up with PuTTY.

## Baud Rate
I zeroed in on the baud-rate divisor calculation as something that I hadn't explicitly verified, and set out to see if I could find a concrete answer for how that register was supposed to be set (I still hadn't seen the formula in the docs). While looking for more information, I came across this piece of documentation from the Raspberry Pi website:

https://www.raspberrypi.com/documentation/computers/configuration.html#mini-uart-and-cpu-core-frequency

The claim here is that, since the mini-UART baud rate is divided out of the VPU core clock, the VPU core clock had to be fixed in order to get a constant baud rate from the UART. I added the specified lines to my `config.txt`, and ended up with . . . nothing, still.

## Phone A Friend
I'd gone about a day and a half without telling my Discord channel about what I'd been doing, which is apparently a big red flag that I'm stuck. After being asked if I'd had any success with the UART, I pushed my working branch and sent it out. I got Scott Teal ( @Cognoscan ) and Mike St. Jean to read it, and got some really helpful advice:
 - I had PuTTY configured with incorrect flow-control
 - My code for setting the GPIO function selection registers was blatantly incorrect

Both of those things were pretty easy fixes (though my first fix for the GPIO functions was _still_ incorrect, because it be like that sometimes), but the result was the same - no life on the UART.

## Sketchy Documents: Reprise
One other thing had come out of talking to some other people about the issues: a link to another project doing a bare-metal mini-UART implementation. This was done by David Welch ( @dwelch67 ):

https://github.com/dwelch67/raspberrypi/tree/master/uart01

The README in this folder claims that the documentation for several of the registers is _just plain wrong_ - including the 7/8bit config, and the Interrupt and FIFO Control registers. I updated my code to use Mr. Welch's configuration values and . . . it worked. After doing a couple of passes with variations of which configuration was included/excluded, I narrowed it down to specifically the 8bit configuration register. The changes to what was being written to the FIFO Control register had no discernible impact.

## Lessons Learned
It's pretty clear that there were three things that really hurt my ability to get the mini-UART running.

1) Bad assumptions. Ignoring GPIO setup and baud-rate configuration on the assumption that we'd be able to carry over configuration from the 2ndstage bootloader put some egg on my face when comparing against other implementations.

2) Sketchy documentation. By far the most _frustrating_ blocker here was the 7/8bit control register being incorrectly documented. There are other portions of the documentation that also undermine confidence - register names being inconsistent, and contradictory information between some bitfield descriptions and their long-form documentation counterparts lead me to second guess a lot of the information coming from the BCM2835 peripherals guide.

3) Poor tooling. The 7/8bit issue wouldn't have been nearly as irritating if I'd had an oscilloscope or logic analyzer available to check if I was at least getting _something_ coming out of the UART pins. Relying on hitting a "100% working" configuration to be able to get any feedback was a planning failure, born of hubris and the assumption that the UART was trivial enough to not need debug capability. This issue could have bit me _much_ harder if I had had any issues configuring baud rate. I'll be purchasing a cheapo logic analyzer for my desk to help with any other work I might end up doing with the GPIO pins.

# What's Next
One thing that you might have noticed while looking through the code for the UART is that it's _absolutely terrible_. It's currently still structured like a spike for configuration, which is pretty far from being a stable base for debug output in future projects. My next steps for now are going to be Cleaning Up - I want to get register access, GPIO access, and the mini-UART split into modules so the code can look a little nicer now that it's going to be used as a platform for Other Stuff.
