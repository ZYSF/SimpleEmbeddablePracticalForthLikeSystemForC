# SimpleEmbeddablePracticalForthLikeSystemForC
Like it says on the box. No frills.

**Still under development. Not thoroughly tested!**

## Features

* Supports simple reverse-polish stack-based operations, e.g. `1 1 +` would result in `2` being pushed to the stack
* Supports defining named words for complex behaviour (but requires some support to add new words to the dictionary from within the system, i.e. there's no built-in function for that exposed within the VM but it's easily accessible from within native calls or elsewhere in the host application)
* Suitable for use in real-time applications (each interpreter step is of bounded complexity and calls back into host code can be delayed/repeated later by returning an error code, there is no instruction that e.g. copies a whole array or searches a whole list, so a host program can easily run embedded programs in short bursts while still checking sensors and such regularly without ever skipping a beat - and importantly without relying on harder-to-debug interrupt setups)
* Implemented entirely in a C header for easy embedding in portable/native applications (a very simple example is given in main.c)
* Only defines static/inline functions (so a host application can include multiple versions or configurations of the VM without them conflicting, provided they're used in different modules of the host application)
* Programs within the VM are entirely encapsulated within a single array, the VM doesn't require any dynamic memory allocations or other complex interactions with the host environment
* Easy to pause/resume/load/save programs (as easy as copying or reading/writing an array of integers)
* Has a built-in assembler accessible to the host program, can compile interactively for a read-eval-print loop or compile in batches to run later (the assembler function just assembles one word at a time in any case)
* Doesn't have any built-in I/O operations, allowing the whole I/O system to be controlled by the host program

## Why FORTH?

I was experimenting with C and Java style systems for embedded development but they are just not practical enough.

Using conventional tools for embedded systems is fine if you want to do a very specific simple program on a device, but not if you want to shoehorn complex
dynamic functionality into low-cost chips. You will almost invariably need FORTH for that, because any other useful solution
is bound to be either too slow (Smalltalk, LISP, JavaScript, etc.), too large (Java, .NET), too limited (Oberon, p-code) or too static (C, C++,
D). Of course, you could always change one of those solutions to fit, but you'd just end up with a really complicated FORTH.

(I actually spent... Weeks? Working on a system that matched Java 1.1 feature-to-feature. By the time I got it half working, I realised "fuck this is much less efficient than FORTH").

## Why a new FORTH?

Primarily, licensing and portability. There are a number of "portable", "simple", "practical", "embeddable" and "public domain" 
"FORTH" systems already, but those terms can be extremely subjective.

This implementation focuses on getting the essential parts of FORTH right; It works entirely within a 32-bit word array (whether that
array is allocated statically or dynamically is up to downstream users) and, aside from the assembler and debugging functions, it operates
entirely in small, bounded-complexity steps (and host program callbacks can be designed the same way), so it's suitable for implementing complex multitasking and, particularly, multimedia-capable systems. The program counter, stack pointers and other such information are all stored in normal program memory so (aside from I/O) no outside state management is required, making it extremely easy and efficient to pause/resume/load/save programs.

Other implementations often focus on standards compliance and self-hosting, despite the available FORTH standards being horrible and most real-world programs not benefiting from an iterative programming system.

So, this version isn't standards-compliant at all; You probably couldn't even cleanly implement a lot of standard FORTH words on top of it because the function names are restricted to sensible ones. But, it should be simpler, more embeddable, more practical and perhaps even more FORTH-like than the rest :).

Some major differences between this and standard FORTH (most other FORTH-like systems) are:

1. Strings are handled "more intuitively" and the assembler only supports ASCII but they fit UTF-32 and programs should assume either full Unicode or unknown character encoding.
2. Extremely simple operations, such as integer maths, comparisons and primitive loops, are all inlined (not implemented as indexed high-level words, they're just instructions like for a CPU instead).
3. There is no built-in I/O. Input and output are considered operating system or application features, not language or VM features.
4. Named words are restricted to `_Simple.englishStyle99` (i.e. the same characters typically used to identify names in C/Pascal/Java/C#/Python/etc. names, including dots and underscores to enable application-specific namespaces or other conventions), which is a bit more readable than the usual `$#!7` style. I understand this won't be enough for some use cases, but customising the assembler to handle different cases (or just translating the names into readable ones first) should be easy enough.
5. The memory layout is significant and implementations may limit memory access of specific operations only to the relevant memory sections or kill programs that attempt to use memory outside conventional (safe) bounds.
6. Consideration has been given to potential modern use-cases including 1) garbage-collection is made easier by having a consistent and well-defined memory layout, 2) virtualisation of the system within itself is enabled by making the entire system embeddable to the extreme (a FORTH program could control other FORTH programs just by the host application exposing some of the VM functions to the program), 3) advanced multitasking is really just a special case of virtualisation, and 4) storing everything in 32-bit words makes it easy to exchange (even running-but-paused) programs between machines or networks with different endianness conventions.

It's also ridiculously small and completely public domain. Fuck copyright.

## If it doesn't comply with FORTH standards, why not call it something new?

Because it would still be FORTH. I'm not a fan of marketing jargon.
