HomebrewCalculator
==================

Homebrew commandline calculator program, basically an alternate to bc. Can handle arbitrary length arithmetic expressions, assignable variables with arbitrary names, and standard math functions.

To use, ensure the TABWIDTH macro has the appropriate value for your terminal, compile, and execute on the command line. To use with a script, simply redirect a file containing appropriate input to stdin.

This program has only been compiled with gcc and executed on a bash shell running under a linux-based operating system, so there are no guarantee that it will work on anything else.
