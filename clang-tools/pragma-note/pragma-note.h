/*========================================================================
 * Your Pragmatic Notebook
 *========================================================================
 *
 * General-purpose pragma-based annotation system.
 *
 * Parses annotations of the form:
 *   <name> = <integer>
 *
 * And handles scoped redefinitions. See, for example, the omp-critical.c
 * example file, which gives the OpenMP pragmas and equivalent Noelle Note
 * annotations commented out before them.
 *
 * Then, Clang is extended to emit MDNodes at appropriate places in the IR
 * for an annotatable subset of Stmt and Decl AST nodes.
 */

#pragma once

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"

#include "clang/Lex/Preprocessor.h"

#include "./annotation.h"

namespace note {
  // Useful interface alises
  using NameAndMDNode = std::pair<llvm::StringRef, llvm::MDNode *>;

  /* NOTE(Simone): extensible enumeration of namespaces. This way
   * different namespaces can be used to interpret the same expression in
   * different ways; ie. these two are not equivalent:
   *  #pragma note noelle independent = 1
   *  #pragma note marc   independent = 1
   */
  enum struct DSLNamespace {
    noelle,
    unrecognized,
  };
  std::string dsl_namespace_to_string (DSLNamespace dns);
  DSLNamespace parse_dsl_namespace (std::string string);

  /* NOTE(Simone): generic handling of annotations of the form:
   *
   *   #pragma note <namespace> (<key> = <value>)+
   *
   * Where <namespace> must correspond to one of the enumerated namespaces
   * defined in note::DSLNamespace.
   *
   * Returns whether the handler succeeded in parsing an annotation and
   * attaching it the the 1st token (the 'note' annotation token) as its
   * annotation value.
   */
  bool handle_note (llvm::SmallVectorImpl<clang::Token> const & tokens);

  /* NOTE(Simone): iterate over and construct metadata nodes for all of
   * the key-value pairs from our annotation. You can create an MDNode out
   * of an ArrayRef<MDNode *>, which results in an MDTuple. So, at the
   * end, we will have a Metadata tuple of (MDString, MDNode) Metadata
   * tuples.
   *
   * NOTE THAT, In order to get back out the ConstantInt value, we must
   * use an API marked as "provisional" in llvm Metadata.h. Example:
   *
   *    MDNode * N = ...;
   *    auto * CI = mdconst::dyn_extract<ConstantInt>(N->getOperand(2));
   *
   * cf. LLVM 7.0.0 Metadata.h line 481.
   *
   * Due to this API being "provisional," it is likely we will need to
   * update this code in an upcoming release of LLVM - perhaps even as
   * soon as LLVM 8 / LLVM 9.
   */
  NameAndMDNode construct_metadata (void * note, llvm::LLVMContext &);

  /*
   * NotePragmaHandler
   *
   * Lex a custom annotation of the form:
   *
   * `#pragma note <dsl namespace> <dsl tokens>...`
   *
   * On success, this will insert a placeholder `annot_ext_note`
   * annotation Token (patched into Clang's Token defs) that contains as
   * its AnnotationValue the note::Annotation constructed by parsing the
   * <dsl tokens> with the namespace's DSLHandler.
   *
   * During parse, the `annot_ext_note` token can check that the AST node
   * immediately following it is an annotatable node. If so, then the
   * note::Annotation is converted into an llvm::MDNode and added to a
   * Map<void *, MDNode>.
   *
   * Finally, during CodeGen, the above Map can be referenced to access
   * the metadata that we want to attach to instructions output from the
   * AST nodes.
   */
  class NotePragmaHandler : public clang::PragmaHandler {
  public:
    NotePragmaHandler() : PragmaHandler("note") { }
    void HandlePragma (
      clang::Preprocessor & PP,
      clang::PragmaIntroducerKind,
      clang::Token & note_source_token
    ) override;
  };
} // namespace note
