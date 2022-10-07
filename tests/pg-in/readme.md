# PG-IN Test Case

The *pg-in* test case shows how a perimeter guard for incoming data
works. The program takes a string from standard input and tries to
write it on the standard output. The perimeter guard marks the
incoming data with tag "PP.private", while the perimeter guard for
outgoing data accepts only "PP.public".

## Explanation of debug commands

The debug command runs until the main function. We print the state of
the registers and of the memory where the strings reside. Next we
inspect the state of the simulator after the system call *read* is
finished. We again check the registers and the memory. We let the
program run until the contents are actually written to the string -
this takes some time to perform inside the *scanf* function. Then we
run to completion.

## Results

The string is not printed on the screen, which was expected.
