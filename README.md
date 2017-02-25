# RFC 3797 - Verifiable Random Selection

## Overview

This project provides an **modified** implementation, in strict ANSI X3.159-1989
(i.e. C89) compliant C, of RFC 3797.

The modifications made are the replacement of the MD5 hash function with the
WHIRLPOOL hash function.

## Building and Running the Demonstration Application

### Windows (Visual Studio)

To build/compile the demonstration application put the files in the same directory and using Microsoft Visual Studio run the following command:

    cl demo.c rfc3797.c whirlpool.c /Ox /Fedemo.exe

### UNIX/Linux

Compile using the following command line:

    gcc demo.c rfc3797.c whirlpool.c -O3 -o demo

The above command will also work on Windows systems running cygwin, or mingw
plus msys.

If using a system with LLVM instead of GCC, replace `gcc` with `clang`.

## Copyrights and Acknowledgements

The implementation of the RFC 3797 algorithm is heavily cribbed from the
reference implementation in the actual RFC 3797 publication. The implementation
of WHIRLPOOL is a nearly direct transcription of the reference implementation by
Barreto and Rijmen, with modifications only to the function names and replacing
the multitude of `#define`'d types into the standardized types in `stdint.h`.

Reformatting the code into BSD Kernel Normal Form, the complete demonstration
application, and the single call WHIRLPOOL hash were implemented by
C. Gauger-Cosgrove.
