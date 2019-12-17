==========================================================================
Pragma Note: Hacks and Patches for Custom Pragma DSLs in Clang
==========================================================================

A framework for providing rich, custom compiler hints to C programmers.

In technical terms, Pragma Note is a collection of hacks and patches that
adds a pragma DSL authoring framework in the spirit of a simplified,
generic version of OpenMP to Clang.

One might generously call it "a pretty nice kludge."

==========================================================================
Getting Started
==========================================================================

There's a patched copy of the LLVM + Clang source tree installed as a git
subtree at `llvm-tree/llvm`. In order to write a custom DSL, we need to
compile a patched Clang, and load our DSL as a plugin. It's recommended to
build with `ninja` instead of `make` for noticeably faster compilation.

--------------------------------------------------------------------------
Setting up and Compiling a Patched Clang + Note DSL Plugin
--------------------------------------------------------------------------

1. Compile the LLVM framework (this includes Clang, the Note DSL plugin, and the rest of the LLVM framework)
```
$ cd llvm
$ make 
```

2. Add what has been just compiled to your environment
```
$ source llvm/7.0.0/enable
```

3. In the root of the repository, check whether the plugin works:
```
$ automp -S -emit-llvm hello-note.c
```

You should see some debug output from the plugin in your terminal, and
there should be a `hello-note.ll` file created that contains `note.noelle`
metadata containing an anonymous struct of a single boolean `true` (`1`)
value.
