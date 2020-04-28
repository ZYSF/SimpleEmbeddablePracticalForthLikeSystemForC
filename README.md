# SimpleEmbeddablePracticalForthLikeSystemForC
Like it says on the box. No frills.

## Why FORTH?

I was experimenting with C and Java style systems for embedded development but they are just not practical enough.

Using conventional tools for embedded systems is fine if you want to do a very specific simple program on a device, but not if you want to shoehorn complex
dynamic functionality into low-cost chips. You will almost invariably need FORTH for that, because any other useful solution
is bound to be either too slow (Smalltalk, LISP, JavaScript, etc.), too large (Java, .NET), too limited (Oberon, p-code) or too static (C, C++,
D). Of course, you could always change one of those solutions to fit, but you'd just end up with a really complicated FORTH.

## Why a new FORTH?

Primarily, licensing and portability. There are a number of "portable", "simple", "practical", "embeddable" and "public domain" 
"FORTH" systems already, but those terms can be extremely subjective.

This implementation focuses on getting the essential parts of FORTH right; It works entirely within a 32-bit word array (whether that
array is allocated statically or dynamically is up to downstream users) and, aside from the assembler and debugging functions, it operates
entirely in small, bounded-complexity steps, so it's suitable for implementing complex multitasking and, particularly, multimedia-capable systems. The program counter, stack pointers and other such information are all stored in normal program memory so (aside from I/O) no outside state management is required, making it extremely easy and efficient to pause/resume/load/save programs.

Other implementations often focus on standards compliance and self-hosting, despite the available FORTH standards being horrible and most real-world programs not benefiting from an iterative programming system.

So, this version isn't standards-compliant at all; You probably couldn't even cleanly implement a lot of standard FORTH words on top of it because the function names are restricted to sensible ones. But, it should be simpler, more embeddable, more practical and perhaps even more FORTH-like than the rest :).

It's also completely public domain. Fuck copyright.

## If it doesn't comply with FORTH standards, why not call it something new?

Because it would still be FORTH. I'm not a fan of marketing jargon.
