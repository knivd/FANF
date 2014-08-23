FANF
====

FANF is a new programming language for small systems. It descends from the popular Forth and adds some new concepts and a simpler approach to enable easier understanding by newcomers.

Some "typical" entry-level FANF programs look like this:

```
'! the world's best known "Hello World!" programming example in FANF
Hello:
"Hello World!_:" cout ;


'! print the sum of two numbers
Sum:
"Enter a number: " cout cin asnum
"Enter another number: " cout cin asnum
"The sum is: " cout + cout
;


'! leap year check
LeapYear:
year: 1 data ;
"Enter a year: " cout cin asnum @year =
@year cout
@year 400 // 0 == 
@year 100 // 0 <> @year 4 // 0 == and
or if
  " is a leap year_:" cout
else
  " is not a leap year_:" cout
endif
;
```

The core words of FANF are called "Atomic Words" and they are completely hardware-independent. They form the basis on which every next word is defined. All words defined by the user form a "Library" which grows with every next word to more and more complex code.

A user "program" is in fact the last word in the library. This word consists of previously defined lower level words, which themselves consist of even lower level ones, defined earlier, etc.

FANF supports multitasking at its very core level and without the need of an underlying operating system.

Other features of FANF include a simple two-type data model (numbers and text/binary), dynamically changeble array size, mixed use data containers, data accessed as code and vice versa, and others.

By its nature FANF is a "compiling interpreter" - a programming shell oriented towards terminal environment and allowing the user to enter and modify code. The FANF source text is compiled to pseudo-code, which is executed by a lightweight FANF Virtual Machine (FVM). If needed the code can be de-compiled to its initial text form together with all commentaries, and then edited by the user.
The speed of FANF ultimately depends on the hardware used to run it and the number of simultaneously running processes. It also depends on some hardware aspects such as supported functionality for the I/O ports, memry size, etc. In general the execution speed is very high due to the simplicity of the atomic words.
