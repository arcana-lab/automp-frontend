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

1. Link the plugin into the clang source tree:
```
$ cd llvm-tree/llvm/tools/clang/tools
$ ln -s $(git rev-parse --show-toplevel)/clang-tools hobbit-tricks
```
2. Make a "dist" directory to hold CMake artifacts
3. Enter the "dist" directory
4. Configure CMake and generate build files
```
$ cmake -G 'Ninja'             \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DLLVM_BUILD_LLVM_DYLIB=ON   \
  -DLLVM_LINK_LLVM_DYLIB=ON    \
  ../llvm
```
  - To build with make : replace 'Ninja' with 'Unix Makefiles' above
  - For cquery: `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../llvm`
5. Build everything: `{ninja|make} clang pragma-note`

In the root of the repository, check whether the plugin works:

```
$ llvm-tree/dist/bin/clang                   \
  -S -emit-llvm                              \
  -fplugin=llvm-tree/dist/lib/pragma-note.so \
  hello-note.c
```

You should see some debug output from the plugin in your terminal, and
there should be a `hello-note.ll` file created that contains `note.noelle`
metadata containing an anonymous struct of a single boolean `true` (`1`)
value.
