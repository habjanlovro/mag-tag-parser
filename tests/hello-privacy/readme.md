# HELLO PRIVACY Test Case

The *hello privacy* test case is a simple case that shows the
perimeter guard at work. There are two versions of the test.

* 1st version shows a string tagged with 'public' being printed on
  standrad output, that is guarded by a perimeter guard perimitting
  tag 'public' or lower.
* 2nd version shows a string tagged with 'private' being printed on
  standard output with the same perimeter guard in place.

Both versions contain the execution trace, state of the registers and
state of memory where the string is stored.

The state is recorded before the `write` system call and before the
`exit` system call, as in this time we can read the memory from the
simulator.

## Results

### 1st run

In the first run, the string is written out on the standard output. We
see that the string buffer remains tagged with 'public' and that the
perimeter guard sees a tagged string on the check. At the end we see
that the registers are tainted by the tag propagation.

### 2nd run

In the second run, the string is not written out on the standard
output. We can see that the perimeter guard has caught sensitive data.
We also see that the string buffer remains unaffected and that the
registers get tainted by tag propagation.
