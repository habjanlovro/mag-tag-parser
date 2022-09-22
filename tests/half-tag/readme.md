# HALF-TAG Test Case

The *half-tag* test case demonstrates different semantics of tag
propagation in relation to memory. There are three versions of the
test:

* 'spike': The test case uses `spike-tag` simulator without any
  additional features.
* 'spike-loose': The test case uses `spike-tag` with
  `--enable-tag-mem-loose` feature.
* 'spike-strict': Uses `spike-tag` with `--enable-tag-mem-strict`
  feature.

The program modifies a 'public' string with some 'public', 'private'
and 'unknown' data. Then it prints the string on standard output that
is guarded by perimeter guard permitting tag 'private'.

The state of the system is recorded at the beginning of main, before
the `write` system call and before the `exit` system call.

## Results

### Spike

The program writes out the string on standard output. We see that
stores of characters into the buffer change the tag values of specific
bytes. We also see that every time the old tag value is written over
by the new tag value.

### Spike loose

With the loose tag memory feature enabled, we can see that changes to
the string buffer are different, as the 'unknown' tag is not seen.
This shows that the tag propagation takes into acount the existing tag
data and uses it in the tag propagation.

### Spike strict

With the strict tag memory feature enabled, we see that the simulator
throws a tag propagation exception. This is because it disallows the
lower 'unknown' tag to be stored in place of a 'public' tag. The
simulator catches this and throws an exception as expected.
