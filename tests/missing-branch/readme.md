# MISSING BRANCH Test Case

The *missing-branch* test case shows how tag propagation handles
private data together with branching. A simple program shows two
scenarios when private data can be leaked with an if statement missing
an else statement. The test has two versions:

* In the *branch-false* version the conditional expression is false
  and a public variable is only printed on standard output.
* In the *branch-true* version the conditional expression is true, so
  we increment the public variable before printing it.

## Explanations of the debug commands


### branch-false

```
# Run until the main function
until pc 0 000000000001014c
pc 0
reg 0
# Reach the load instruction for private variable `a`
r 4
pc 0
reg 0
# Execute the load instruction for `a`
r 1
pc 0
reg 0
# Reach the branch instruction
r 3
pc 0
reg 0
# Execute the branch instruction
r 1
pc 0
reg 0
# Execute the load instruction for `b`
r 1
pc 0
reg 0
# Go to `ecall` for printf for printing:
until pc 0 0x000000000001ac34
pc 0
reg 0
# Go to exit:
until pc 0 0x000000000001ac06
pc 0
reg 0
rs
```

## branch-true

```
# Run until main function:
untiln pc 0 000000000001014c
pc 0
reg 0
# Reach load instruction for `a`
r 4
pc 0
reg 0
# Execute load instruction for `a`
r 1
pc 0
reg 0
# Reach branch instruction
r 3
pc 0
reg 0
# Execute branch instruction
r 1
pc 0
reg 0
# Load instruction for `b`
r 1
pc 0
reg 0
# Increment `b`, do not store yet
r 3
pc 0
reg 0
mem 0 0x1f2dc
# Store instruction for `b`
r 1
pc 0
reg 0
mem 0 0x1f2dc
# Run until the end
  r
```

## Results

We see that in both cases when we load `a` the registers start to get
tainted. When the branch instruction is executed, the `pc` also gets
tainted. Because all tag propagation take into account the tag of
`pc`, the register into which the variable `b` is loaded will get a
higher tag.

In the second case where we modify variable `b`, we see that the tag
changes in the memory region.
