# SEPARATE-POLICIES Test Case

The *separate-policies* test case shows how tag propagation works with different
topologies at the same time. The program contains three scenarios:

* In *case-one*, tags from different topologies mix during the execution of the
  program.
* In *case-two*, tags from different topologies coexist, but do not
  mix during the execution of the program.
* In *case-three*, tags are from the same topology that is composed of
  two different topologies.

## Explanations of the debug commands

The debug command run until the main function. Then the addition of
the variables is closely inspected. Then we run to the first system
call and show the registers. Then we show the registers before the
`exit` system call.

## Results

* In *case-one*, we see that the tag propagation throws an exception
  at the time of additions, which is expected.
* In *case-two*, we see that the tag propagation throws an exception
  after the first `printf` function is executed. This is unexpected,
  but it can be explained by the PC register being tainted and later
  all other registers get tainted with a specific topology as well.
  This causes a crash when the data with second topology tags is
  loaded.
* In *case-three*, the program executes. This is expected, but we can
  also observe that all the registers got tainted in the process.
