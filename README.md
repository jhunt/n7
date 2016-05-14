n7 - A LISP dialect for Systems Programming
===========================================

I love LISP, its elegance and simplicity, its promise of a fast,
safe, multi-paradigm language with true macros and little in the
way of syntax.  However, most implementations of LISP (or Scheme,
for that matter) fail to meet my criteria for a day-to-day
programming language, namely:

  1. Good OS integration (native system call interface, robust
     filesystem primitives, etc.)
  2. Compiles to a static, standalone binary with little to no
     requirements of the host execution environment.  (Nowadays,
     this means "can be the only thing in a Docker image")
  3. Well-wrought standard library of common programming tasks,
     including string manipulation, regular expressions, hash
     tables, etc.
  4. Memory safe without losing touch with the machine (i.e. bit-
     twiddling and endian-conversion are still doable)

I've looked at hundreds of languages over the years in the quest
to find a really good general purpose language that gives me just
what I want, and doesn't hold me back in the name of "safety".

Failing to find one, I've started this hobby side-project as a way
to experiment with building the language I want.  I don't expect
this to go anywhere.  Hell, I don't expect anyone beside me to
even read this far, let alone dive into the code or actually run
the interpreter or compiler.

