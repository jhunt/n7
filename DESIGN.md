n7 Design Notes
===============

This document describes the overall design of the n7 language, the
n7i interpreter (REPL), and the n7c/n7l compiler/linker.

Compiler Pipeline
-----------------

The full pipeline of the n7 parser/compiler is as follows:

     < source >
         |
     [ reader ]
         |
     [ lexer ]
         |
     [ parser ]
         |
     [ annotater ]
         |              frontend
     ----|----------------------
         |               backend
     [ generalizer ]
         |
     [ optimizer ]
         |
     [ coder ]
         |                          lang-specific
     ----|---------------------------------------
         |                        target-specific
     [ targeter ]
         |
     [ assembler ]         (other jobs)
         |                 /  /  /  /
     [ linker ] ----------'--'--'--'
         |
      < executable >

The `reader` is responsible for converting the input source
(wither a file or interactive input) and converting it into a
stream of characters.  It is responsible for maintaining line and
column offsets for the `lexer`.

The `lexer` takes the output of the `reader` (a stream of
characters), and recognizes longest-match tokens, each consisting
of a type and a representation.  For example, the numeric constant
3.14159 has a type of `floating-point number` and a representation
of "3.14159".  All representations are strings.  Each token
recognized by the lexer can be linked back to the program source
by a (file, line, column) address tuple, for debugging.

The `parser` takes the output of the `lexer` (a stream of tokens),
and assembles them into a rudimentary abstract syntax tree, or
`AST`.  This first draft AST is called the `ast-0`, and describes
most verbosely the structure of the tokens found in the program
source.

The `annotater` takes the `ast-0` abstract syntax tree emitted by
the `parser` and performs whole-program annotation.  Actions
performed at this stage include type checking, label writing, and
simplification.  This results in an annotated abstract syntax
tree, called `ast-1`, which is both more concise and more
complete than the input AST.

The `generalizer` traverses the `ast-1` tree and rewrites it to
use more general (and hopefully simpler) AST nodes, with an eye
towards writing more efficient and portable machine code in later
stages.  An example of work done at this phase is the replacement
of higher-level control structures like while loops with jumps and
tests.  This results in another abstract syntax tree, `ast-2`.

The `optimizer` traverses the `ast-2` tree and attempts to
simplify the resulting AST even further by (for example) performing
constant-folding operations and replacing static computation with
the resultant values.  This results in yet another AST, `ast-3`.

The `coder` traverses the `ast-3` tree and generates symbolic
machine instructions and performs register allocation for an ideal
Von Neumann machine.  This intermediate representation (IR)
linearizes the structure of the abstract syntax tree into a flat
stream of instructions.

The `targeter` analyzes the instructions created by the `coder`,
replacing IR instructions with target machine instructions.
During this stage, entire sequences of instructions may be
compressed into fewer target machine instructions, depending on
the targeted machine, its instruction set, and the optimization
flags in effect.

The `assembler` encodes the instructions emitted by the `targeter`
down to binary object code for the target machine.  This is
combined with headers, relocation tables, symbol table data and
more to produce object code.

The `linker` takes the individual units of compilation produced by
the `assembler` stage(s) and stitches them together into a single
executable image, performing any necessary back-patching.  Note
that there may be multiple inputs to the linker, from different
runs of the compiler pipeline up through the `assembler`.

Interpreter Pipeline
--------------------

The full pipeline of the n7 interpreter (REPL) is as follows:

     < source >
         |
     [ reader ]
         |
     [ lexer ]
         |
     [ parser ]
         |
     [ annotater ]
         |              frontend
     ----|----------------------
         |               backend
     [ runner ]
         |
         *

The `reader` through `annotater` stages are exactly like those
for the Compiler Pipeline.  Indeed, both the compiler and the
interpreter share the exact same modules for them.

The `runner` traverses the `ast-1` tree from the `annotater`, and
performs the computations specified therein.  It maintains a
singular state of the world (the memory) against which expressions
are evaluated.

In interactive mode, the interpreter treats each complete form at
the top-level as a complete program that operates in the shared
context of the REPL session.  Batch mode behaves more like
compilation, in that the entire source file is read in and parsed
before being handed off to the `runner`.

Bootstrapping The Language
--------------------------

The implementation of n7 comes in two flavors: `C` and `self`.
The `C` flavor of n7 is a small, compiler-only implementation
written in portable, POSIX-compliant C89.  The `self` flavor is
the obligatory n7-in-n7 implementation, which uses the compiler
produced by the `C` flavor to compile itself into a hosted
executable.

The initial bootstrap (done only by the language implementers and
interested enthusiasts) consists of these steps:

  1. Compile the `C` flavor using your compiler of choice,
     targeting the hosting machine architecture.
  2. Use the resulting compiler binary to compile the `self`
     flavor, targeting the hosting machine architecture.
  3. Profit.

(It is for this reason that the `C` flavor only needs to implement
a limited subset of target architectures -- likely only amd64).

The normal flow for bootstrapping the language on a new
machine is:

  1. Build new targeter, assembler and linker modules for the
     chosen machine, in the `self` implementation.
  2. On a supported machine, cross-compile the `self` flavor (with
     the new targeter code) for the new machine.
  3. Profit.

(Note that the `C` flavor is not required for bootstrapping a new
machine architecture if one has access to a supported architecture
for which a `self` flavor compiler binary exists.)

Codebase Organization
---------------------

The codebase directory is organized thusly:

    *
    ├── c                `C` flavor implementation (for bootstrap)
    │   ├── backend            (5)-(7) of the compiler pipeline
    │   ├── frontend           (1)-(4) of the compiler pipeline
    │   └── linux-amd64       (8)-(10) of the compiler pipeline
    │
    └── n7            `self` flavor implementation (for bootstrap)
        ├── arch
        │   ├── darwin-amd64
        │   │   ├── assembler      (9) of the compiler pipeline
        │   │   ├── linker        (10) of the compiler pipeline
        │   │   └── targeter       (8) of the compiler pipeline
        │   └── linux-amd64
        │       ├── assembler         same, but for a different
        │       ├── linker             machine architecture and
        │       └── targeter               operating system pair
        │
        ├── backend
        │   ├── coder              (7) of the compiler pipeline
        │   ├── generalizer        (5) of the compiler pipeline
        │   ├── optimizer          (6) of the compiler pipeline
        │   └── runner             (5) of the interpreter pipeline
        │
        └── frontend
            ├── annotater          (4) of the (shared) pipeline
            ├── lexer              (2) of the (shared) pipeline
            ├── parser             (3) of the (shared) pipeline
            └── reader             (1) of the (shared) pipeline
