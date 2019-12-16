Instead of creating a singular-purpose pragma with `#pragma pal`, we
should instead write a general-purpose annotation pragma framework. This
way we can introduce the `noelle` annotation DSL with a minimal number of
changes to the Clang source code, enhancing the portability and
maintainability of annotation pragma extensions in the face of no
backwards-compatability guarantees.

`#pragma note`: an extensible annotation framework.

`#pragma note <subnamespace> <dsl statements>...`

Annotations should be able to be attached to:

- Arbitrary statements (including blocks, i.e. `CompountStmt`s)
- Function definitions (not prototypes)
- Class definitions (attaching to the Class object, not instance)
- Class instances (at definition-site)
- Class instances (at instantiation-site)

This introduces ambiguity around annotations on class definitions. This is
resolved with:

`#pragma note/instances <subnamespace> <dsl statements>...`

for annotating all instances of a class definition, where `#pragma note`
defaults to annotating the class declaration itself.

A Pragma DSL is introduced using a subnamespace. The subnamespace
indicates to the Note framework what Parser to invoke for the DSL. The DSL
Parser should return Annotations, which Note will automatically attach to
the statement or declaration.

--------------------------------------------------------------------------
Changes to make to Clang
--------------------------------------------------------------------------

We want to model our enhancements to Clang after the `#clang loop`
loop-hint pragmas, which attach various attributes to whatever statement
follows them. In order to do this, we need to infect the codebase with a
bunch of small changes dispersed across a couple of files...

TokenKinds.def
  - add annot_note_start, annot_note_end

ParseStmt.cpp Parser::ParseStatementOrDeclarationAfterAttributes
  - add case for tok::annot_note_start
  - invoke registered "note" parser for the named dsl (from FETokenInfo)
  - model after ParseStmt.cpp Parser::ParsePragmaLoopHint
    - parse the next statement
    - attach attributes to the next statement from the `pragma` annotation
    - copy AnnotateFunctions example's use of AnnotateAttr::CreateImplicit
    - return the next statement's parsed StmtResult

Now, outside of Clang proper, we still need to create:

Token data struct
  - subnamespace
  - what parser to use
  - whether it was a "note/instances" declaration

Token Data
  - get/setFETokenInfo: hooks for associating arbitrary metadata w/ token
  - create a struct of the token info we care about

PragmaHandler
  - "note": bookend with start/end annotation tokens; re-enter into stream
  - "note/instances": as above; and set 'instances' of FETokenInfo to true
  - register using PragmaHandlerRegistry::Add

This approach is chosen in an effort to severely minimize the number of
lines of code introduced to the Clang codebase. With any luck, this will
make the solution more maintainable than an alternative approach that more
heavily leverages Clang's APIs, or touches more of the Clang compilation
process. We purposely avoid introducing any new Lex, Sema, or CodeGen
steps, but it is not possible to avoid performing *some kind* of Parse
step due to our desire to operate on the AST and attach metadata to nodes.

--------------------------------------------------------------------------
Notes on OpenMP
--------------------------------------------------------------------------

If we are to do a source-to-source translation of OpenMP Pragmas, it would
be instructive to carefully read and understand the OpenMP actions in
`lib/sema/SemaOpenMP.cpp`. In particular, the definition of an OpenMP
"Structured Block" is expressed in terms of Clang API calls as a part of
`Sema::ActOnOpenMPExecutableDirective`, in particular when it hits the
`Sema::ActOnOpenMPParallelDirective` case.

Good entrypoints for understanding OpenMP pragma handling:

- `PragmaOpenMPHandler` in ParsePragma.cpp
- `Parser::ParseOpenMPDeclarativeOrExecutableDirective` in ParseOpenMP.cpp
- `Sema::ActOnOpenMPExecutableDirective` in SemaOpenMP.cpp

Particularly interesting are the Parser and Sema phases. Lexing is more
complex, but not more instructive, than simpler examples of the same
basic process such as PragmaLoopHintHandler.
