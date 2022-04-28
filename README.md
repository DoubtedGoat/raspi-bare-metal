# Description
This repository contains code I am using to brush up on embedded-systems skills using a Raspberry Pi 2. It doesn't _do_ anything, there is no end goal, and there is no particular reason to want to read it except curiosity or boredom.

# Versioning + Releases
This repo will be tagged and "released" regularly as I work on it, with the timeline of releases serving as a dev log as I try accomplish increasing interesting things. Version numbers will follow a variation of semver - a major version increment will constitute a substantial change to the nature of the project, a minor version will be refactorings and improvements to the prior major version, and a patch will be minor corrections and bugfixes.

I will try to update the README with a description of what I've done and why upon each release.


# Version 1.1.0 - Clean Up
For this release, I've split up a lot of the functionality from our proof of concept into sensible groups of modules, and tried to clean up the usage of the UART slightly.
I've also added a rudimentary host-machine test script - I can build and run tests on an x86 box to check out logical functionality. This was something I had intended to do _eventually_, and regret not doing _sooner_ - I lost several hours of effort to a slow and stupid debug process while building out the register-access module. A similar bug that appeared later took under an hour, due in large part to the increased ease of testing from having put together this script.

For this write-up I'm going to do a quick walk-through of the major design decisions that got made. There's not too much of a story to this one - I had a plan, and I pretty much just Did That.

## Module Boundaries
My original plan was to build three modules - `register_access`, `gpio`, and `uart`. I ended up adding a `debug` module later in the process. The module lines were intended to provide a pyramid of abstraction, and cut down on the amount of repetitive code (ie, chances for human error). The foundation for everything would be `register_access`. Rather than bespoke, hand-crafted operations for every poke on every register, I wanted to have a tried-and-true set of functions I could use to just Flip Bits without any fuss.

The `gpio` and `uart` modules roughly correspond to distinct subsystems on the Pi - with `uart` depending on `gpio` in order to function. With both of these modules in hand, my goal was to have our `kernel.c` file not need to use any of the methods in `register_access` directly - all logical operations could be performed through the public interface of either `gpio` or `uart` with no manual bit-flipping.

The `debug` module came about as random test code started to clutter up the top of `kernel.c`. Support functions like hex-conversion and manual-waits needed a ~dumping ground~ proper home, so this module came into existence.

## Register Access
The key decision in `register_access` is the choice to `typedef` a `Register` type, rather than using `#define` for every address and putting `(volatile uint32_t *)` casts directly into the register access methods. My primary goal here was to create readable function signatures - the user should be able to trivially compare input type to argument type to see if they're Doing It Right. Unfortunately C doesn't offer type enforcement for `typedef`s so this won't give compile-time errors, but it will serve as a strong hint to the user (me). The tradeoff here is that we're requiring defined constants for all of our addresses, which will take up space. This doesn't particularly bother me since we have a ton of space on the Pi, and I'm prioritizing writing for readability rather than constraints (for now). It is worth noting that this issues gets compounded a decent amount later on, though I'll still choose to ignore it.

The other 'choice' of note in this module is offering a bitfield write that does a read-modify-write for you. There are some register interfaces that change on read - reading from a FIFO, reading the UART overrun bit, etc. A read-modify-write can perturb the state of any read-sensitive bits, and may be unexpected to the user if they're not familiar with the inner workings of `register_bitfield_write`. The other option here would be to only offer `register_write` and force the user to make their own choices about whether they want to use a read first, or just completely clobber the register. Ultimately I opted for the convenience of a bitfield-write function, knowing that I will be the only person writing code here and I will totally not ever get got by this. Another alternative would be to explicitly name the method `register_bitfield_read_modify_write` but it would clutter my absolutely delightful naming convention, so that's a no.

One note about this module - the `make_bitmask` method cause me all kinds of issues (as you can see in the commit history). Perhaps it's just too long away from bitwise operations, but I spent an undue amount of time determining the right way to get it to work and _still_ got bitten when my final answer ended up failing to handle the `X:0` case due to overlooking integer wrapping being A Thing. The first round of trying to get this to work right culminated in my writing the test scripts, which saved me a ton of pain when fixing the `X:0` edge case.

## GPIO
The `gpio` module was the first time I had to use my `Register` type from `register_access` as reusable constants. This causes a minor tradeoff. Defining the constants as `static` (with internal linkage) will cause them to be duplicated in memory for every file that includes the header (unless there is a clever optimization that I am not aware of). Had I instead chosen to define them as `extern` (for external linkage), there would only be one copy in program memory _but_ I would have had to move the initializations into `gpio.c` and only have the `extern` definitions in `gpio.h`. I chose to go with `static` because making edits in two places to add a constant is the type of thing that gets forgotten or messed up, and I'm not memory constrained enough (or pretending to be memory constrained enough) that I care about the duplication. If I _do_ end up caring, I'll likely inspect the `elf` first to make sure that Clang isn't already optimizing and I'm worried about nothing.

Another minor choice in `gpio` is using an `enum` for pin function. While (much like `typedefs`) `enum`s in C aren't typechecked or enforced in any way, it supports my desire to make informative function signatures, and groups valid argument values together. Using the `enum` value declarations as constants is moderately controversial, since I am now depending on the underlying values to Do Stuff and can fall victim to "rude people passing in random integers" in a way that a `switch` statement with `default` would not. Conversely, if I want to guard against nefarious usage I can just as easily validate my inputs, and I'm not sure the ideological purity of "enums only as named types" really gains me anything.

## UART
The `uart` module follows basically the same set of choices as the `gpio` module - `static` register addresses, and `enums` for constrained arguments. There are two pieces that are distinct and moderately interesting - configuration options (and validation), and the `Uart` struct.

### UartConfiguration
I chose to capture configuration options as a `struct` rather than an argument list for one primary reason: I can change the `struct` definition without changing every callsite. This is only _very debateably_ a good thing - after all, if you have new required arguments, shouldn't you fail builds if they're missing? My main reasoning here is twofold: 
1) I can (and should) implement Sane Default Values for all struct fields, which would obviate the need to set values anywhere unless I explicitly care about them
2) This code is _very_ likely to change, as it stands. In combination with point number 1, a struct-as-options is extremely change friendly.

These two points in combination will let me choose exactly how much I do or don't want to expose for configuration, without requiring (possibly extensive) code updates. The astute reader will note here that almost no updates to uart configuration will be extensive, because there is only one uart. Consider this implementation as a test-drive for a pattern I may like to adopt in other pieces of code moving forward.

### A Struct Named Uart
The other `struct` of interest and likely controversy is `Uart` itself. Currently it holds almost nothing - a singular flag that indicates whether the Uart has been configured. A flag that could easily have been a `static` value in the `uart.c` module.

The reasoning here is that it is an abstract representation of external side-effect-y state that needs to be respected - think of it as a shitty monad. Since the UART itself is stateful (cannot be used until initialized), the default implementation is a set of functions with no return values that have to be called in a specific order otherwise Bad Stuff Happens. This is Bad because there is nothing to telegraph to the consumer that they _should_ have done things in a particular order.

Even though the user could conceivably construct their own `Uart` struct and leave it unconfigured, the presence of a function that explicitly converts a `UartConfiguration` into a `Uart` (albeit with weird C reference-as-output and return-code shenanigans) suggests the proper usage. Likewise, requiring that a `Uart` struct be passed into all uart functions pushes the user towards proper usage. If your code doesn't have a `Uart` struct, your code is likely also unsure of whether the `Uart` is actually ready to be used.

The `.configured` field on the struct is there mostly because there has to be _something_ in the struct. 90% of the goal of having this struct is accomplished simply by having it exist - reporting errors when calling against an unconfigured `Uart` is largely just a pleasant side-effect.

### The Uart API
Currently the API surface area of uart is at a pretty low-level of abstraction - reads and writes are done as single bytes, and the consumer still has to poll for new bytes. This isn't all that great if you consider that we'd like to use the uart as a basis for debug output. A better API would likely let us read and write longer byte buffers, and have an abstracted notification for readability so we can grab a large block from a software buffer all at once, rather than waiting around for bytes to float in. We'll try and get this cleaned up in the next release.

## Error Handling
Error handling in C is a long chain of difficult decisions, none of which ever feel particularly correct. My preference to-date is to have almost-all functions return return codes, with negative numbers indicating errors. An error-handling method can be called, which will do something useful (in our current case, turn on an LED and halt the program). This isn't ideal for a number of reasons.
1) Return codes are easily ignored - just don't bother looking at them, and all error handling is gone. Unfortunately this is going to be an issue with every error-handling strategy in C, so it's kind of cost-of-doing-business.
2) "Functions return values" semantics are _amazing_ for code ergonomics. I cannot overstate how nice it is to have your code conform to a clear logical model of "inputs create outputs". While there are minor benefits to "outputs as reference arguments" (like not copying large structs during returns), the ergonomics of having your outputs be function inputs are kinda repulsive.
3) There are always going to be predicate functions that _desperately_ want their outputs to be return values (eg `uart_can_read()`). Even if these functions don't have failure modes, there is now still two types of usage modes present in the codebase, and no indicator as to which mode a particular function uses. You could make a reasonable guideline "predicates return `bool`, return codes are `int`" but it's just _begging_ for exceptions to crop up. Prefixes feel like ugly band-aids - no one wants to call `rc_uart_configure` or `p_uart_can_read()`. The same goes for `typedef`'ing return codes - it just feels incorrect.

Even writing this section has convinced me that I don't particularly like my own stated "preferred method". A possibly better option is to let outputs be outputs, and force return codes to be reference arguments. Unfortunately this leaves them just as ignorable, but would at least preserve those delicious "functions return values" semantics.

There are other options as well - if we're committed to our object/struct/monad structure as demonstrated by UART, it would be reasonable to attach an error field to it - this is functionally not that different from return-code-as-reference-argument, but could be a nice way of letting us give the error an `enum` type that's actually useful to the user and not just immediately cast to an int.

Clearly I've got some things to think about with error handling. I may try and settle on a "final" strategy for the next release so I can be consistent in future code before the codebase gets too large.

# What's Next
Next up I would like to bring the uart API up a level of abstraction, as discussed above. This likely means making read/write bytes and can-read/write local to the uart module, adding a receive buffer, and only exposing read/write of byte arrays. In addition, I would _love_ to start hooking into the interrupt and event systems so I can make uart interaction non-blocking.
