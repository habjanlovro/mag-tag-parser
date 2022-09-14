# Tag parser

Tag parser for *spike-tag*, a RISCV simulator that supports information flow
tracking using tagging.

## Installation

Run the following commands to install the program:

```bash
mkdir build
cd build/
../configure --prefix=$RISCV
make
make install
```

where `$RISCV` points to the RISCV installation folder (optional).

## Running

The tag parser takes 3 arguments:

* ELF binary: The binary of the program that we want to tag.
* The tag file: Description of the symbols that we want to tag.
* The policy file: Description of the policy that comprises of tags
  and their relations.

We can run the program as:

```bash
tag-parser <ELF-file> <tag-file> <policy-file>
```

### Tag file

The tag file consists of lines of declarations. Each declaration
describes one symbol to be tagged. The symbol can represent an atomic
value or a pointer. Example of the syntax for atomic symbol:

```
atom <symbol-name> : "<tag>"
```

For symbols representing pointers, we have to add the 'size' of the
data object that the symbol points to (number of bytes). Example of
the syntax:

```
ptr <symbol-name> size = <integer> : "<tag>"
```

### Policy file

The policy file contains descriptions of topologies and perimiter
guards. Topologies describe the tags and their relations, while the
perimiter guards describe how the policy interacts with the data
coming in and going out of the system.

#### Topologies

Topologies can be of 3 types: basic, linear and expression.

The basic topology allows us to explicitly describe the edges of the
policy graph. Example of the topology definition:

```
topology <name> : basic {
	"<tag>" -> "<tag>",
	"<tag>" -> "<tag>"
}
```

The linear topology allows us to describe a graph that is a straight
line. Example:

```
topology <name> : linear
	"<tag>", "<tag>", "<tag>"
```

The expression topology allows us to combine the previous two into a
more sophisticated topology. It supports two operators:

* Disjoint union (**+**): The graphs exist together, but are still
  separated.
* Cartesian product (** * **): The graphs are combined into one bigger
  graph.

Example:

```
topology <name> : expr
	(<topology-a> * <topology-b>) + <topology-c>
```

The names of the combined tags will be of the following form:
`<name>.(<topology-a>.<tag-name>,<topology-b>.<tag-name>)` or
`<name>.<topology-c>.<tag-name>`.

#### Perimiter guards

The perimiter guards describe how the data coming in will be tagged
and which data going out will be passed.

The perimiter guard can be associated with a standard file descriptor
or it can be associated with a file. Example of definition for
standard file descriptor:

```
pg <name> {
	file : ["stdin"|"stdout"|"stderr"]
	tag = "<tag>"
}
```

Example for file:

```
pg <name> {
	file : "<file-name>"
	tag = "<tag>"
}
```

### Outputs

The program outputs two files:

* The policy file contains the policy description - the resulting
  graph and the perimiter guard definitions.
* The tag file is an ELF duplicate of the input ELF file. It contains
  the tag data of the variables.


## Limitations

The program contains several limitations:

* If we want a symbol to be tagged, it must be initialized in the ELF
  file. Uninitialized symbols (those in .bss section) cannot be
  tagged.
* The policy graph must be an acyclical directed graph.
* The final policy graph can only have maximum of 256 tags.
