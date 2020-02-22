VSL Vibrational Symbolic Language

intro
=====

Symbolic language for compact sound synthesis and graphical representation, uses unicode symbols to represent wave modifiers, and multithreading for efficient generation.

examples (copy/paste each example at a time):

// example 1

~440; // generate a 440hz tone in chan 0

// example 2

~3 ~440 // a 440 hz tone modulated by a 3 hz one, note the implicit multiplication used -&gt; chan 1

// example 3

const l0=1/Ø, a1=Ø, f0=622, fd=10, fam=3, l0f0=l0\*f0, a1f0=a1\*f0, seconds=6;

{fam}sin(fd/(t+1)) \* (l0{f0} + {l0f0} + a1{a1f0}); // chan 0 expression definition

special constants
-----------------

Ø=1.618, π=3.141592

sample time *t*
---------------

Special variable 't' or ' τ' is the time frame variable, it's values sequence from 0 to '2π\*seconds' in '2π/sample\_rate' increments, sin function can be used to generate a 440hz tone by sin(t\*440)

channels
--------

each expression represent ended with ';' a different channel so a two channel definition is just:

~440;

~442;

sections
========

Notation
--------

Two different notation are supported: algebraic, RPN, algebraic is the default notation and does not need to be explicitly identified,

examples:

algebraic; // specify default algebraic notation

~2 ~440;

rpn; // reverse polish notation

2~ 440~ \*;

const
-----

defines constant symbols used in wave definition, special symbols include

-   seconds=expression, defines total wave duration
-   sample\_rate=expression, defines sample rate in hz
-   bits\_sample=expression, defines bits per sample, 8, 16, 32 and float

examples:

const seconds=10, sample\_rate=22050, bits\_sample=32;

const seconds=2+4, sample\_rate=44100/3, bits\_sample=float;

let
---

Defines variables evaluated in each sample, syntax is variable=expression, where expression may contain consts or previous defined variables.

examples:

const seconds=9, f0=440, f1=Ø·f0, f14=f1/4, s3=\[3\], s2=\[2\], k0=0.6, k1=0.2;

let ts3=τ+s3; // ts3 is sample time 'τ' or 't' plus const s3

functions
---------

syntax: func name(parameter list) ➡ return expression

example:

// balance phi

const f0=330, rc=3, k=-.3;

let f0rc=f0+rc;

func bal\_phi(x) ➡ ‹k›(∿(~x t)+Ø · ∿(~x t)); // exp(k\*t)\*(sin(sin(x\*t)+phi \* sin(sin(x\*t)))

bal\_phi(f0rc);

bal\_phi(f0);

// bell

const f0=440, offset=1;

func bp(f,x,y,z) ➡ z · ~(f·x) · exp(1-t·y);

func bell(f) ➡

 bp(f, 0.5, 0.075, 1) +

 bp(f, 1, 0.65, 1.35) +

 bp(f, 1.7, 0.35, 1.67) +

 bp(f, 1.19, 0.325, 2.67) +

 bp(f, 2, 0.25, 1.46) +

 bp(f, 2.74, 0.2, 1.33) +

 bp(f, 3, 0.15, 1.33) +

 bp(f, 3.75, 0.1, 0.75) +

 bp(f, 4.07, 0.075, 1.33);

bell(f0);

bell(f0+offset);

expressions
===========

Each sound channel is defined by an expression, we've already seen some examples, so lets present the function and operators set

implicit multiplication, default operator is multiply so two terms with no operator are multiplied, i.e. ~3 ~440 is ~3 \* ~440

||
||
||
||
||

Functions supported include:

||
||
||
||
||
||

 

-   wave(amp, freq, phase) function implements a sinusoidal wave ‘y=amp\*sin(w\*t+phase)’, parameters are wave(amp, freq, phase). Example: wave(1,440,0) plays a 440Hz sin tone wave. 
-   note(note, oct) function implements a note/octave sound, i.e. note(note\#, octave), notes are coded as:

C=0, C\#=1, D=2, D\#=3, E=4, F=5, F\#=6, G=7, G\#=8, A=9, A\#=10, B=11 

-   tone(note, oct) function returns the frequency associated with a note/octave, i.e. tone(note\#, octave), notes are coded as in the note functions. sin(tone(0,0)\*t) is equivalent to note(0,0). osc(freq) (oscillator) is equivalent to sin(freq\*t), i.e. osc(440) = sin(t\*440).   sec(seconds) function converts parameter in seconds to ‘x’ scale i.e. sec(val) = val \* 2\*PI. 

<!-- -->

-   saw(freq,angle) generates a saw tooth with attack angle, omitting angle generates a 45º saw. 
-   lap(time1, time2). Returns 1 if time variable ‘t’ is between time1..time2 and 0 in other case.
