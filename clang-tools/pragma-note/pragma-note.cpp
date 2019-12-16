/* NOTE(Simone):
 * None of the definitions in this file can be included or used from
 * within Clang. This is because we do not want to alter Clang's link
 * configuration to depend explicitly on our code.
 *
 * While not ideal for compilation times, all definitions that Clang must
 * be able to use internally must be defined in header files in order to
 * avoid such an invasive change.
 *
 * So, if you're trying to add self-contained plugin functionality: put it
 * here. If you need to provide a new API surface to Clang, however,
 * you'll need to separate out that functionality into a shared header.
 */

#include "llvm/ADT/StringRef.h"
#include "clang/Lex/LexDiagnostic.h"

#include "pragma-note.h"
#include "annotation.h"

namespace note {
  namespace tok = clang::tok;
  using Token = clang::Token;
  /* NOTE(jodan): annotation debug printer. This cannot be implemented
   * statically in annotation.h because it generates far too many
   * warnings.
   */
  void print_annotation (Annotation value, llvm::raw_ostream & os) {
    os << "Annotation {\n";
    for (auto annotation_entry : value) {
      os
        << "  "  << annotation_entry.first
        << " = " << annotation_entry.second
        << "\n";
    }
    os << "};\n";
  }

  /* NOTE(Simone): token-kind-agnostic string content calculator.
   */
  std::string get_token_content (Token token) {
    if (token.isAnyIdentifier())
      return token.getIdentifierInfo()->getName();
    if (token.isLiteral()) {
      char const * token_with_overshoot = token.getLiteralData();
      unsigned length = token.getLength();
      return std::string(token_with_overshoot, length);
    }
    if (token.is(tok::equal)) // FIXME(Simone): generalize??
      return "=";
    llvm::errs()
      << "note::get_token_content: Token @ " << &token
      << " of kind " << token.getKind()
      << " has no content (that we know how to obtain)!";
    assert(false && "note::get_token_content: unknown token");
  }


  /* NOTE(Simone): namespace parsing/serialization.
   */
  std::string dsl_namespace_to_string (DSLNamespace dns) {
    switch (dns) {
      case DSLNamespace::noelle       : return "noelle";
      case DSLNamespace::unrecognized : return "unrecognized";
    }
    assert(false && "dsl_namespace_to_string: impossibly fell through!");
  }
  DSLNamespace parse_dsl_namespace (std::string string) {
    if (string == "noelle") return DSLNamespace::noelle;
    return DSLNamespace::unrecognized;
  }

  /* NOTE(Simone): generic handling of annotations of the form:
   *
   *   #pragma note <namespace> (<key> = <value>)+
   */
  bool handle_note (llvm::SmallVectorImpl<Token> const & tokens) {
    /*
     * We should have:
     * [ note_start, subnamespace ("noelle"), key, "=", value, ... ]
     */
    if (!((tokens.size() % 3) == 2)) {
      llvm::errs()
        << "note:handle_note: number of tokens in annotation"
        << " does not match '<note> <namespace> (<key> = <value>)+'"
        << " - that is, (num_tokens % 3 != 2)!";
      return false;
    }

    Token note_start_token = tokens[0]; // here's 'note'
    Token const dns_token  = tokens[1]; // here's our namespace
    int note_length  = (tokens.size() - 2);
    int note_pairs_count = note_length / 3;

    assert(note_start_token.is(tok::annot_ext_note));

    std::string dns_string = dns_token.getIdentifierInfo()->getName();
    DSLNamespace dns = parse_dsl_namespace(dns_string);
    if (dns == DSLNamespace::unrecognized) {
      llvm::errs()
        << "note:handle_note: unrecognized namespace"
        << " '" << dns_string << "'!";
      return false;
    }

    /* NOTE(Simone): cast the annotation token's `void *` data pointer.
     * This is the only 'extend the token' functionality in Clang afaict,
     * which makes it the mechanism for passing data from the Lexer to the
     * Parser.
     */
    Annotation * annotation = static_cast<Annotation *>(
      note_start_token.getAnnotationValue()
    );

    llvm::outs()
      << "note:handle_note: number of pairs: " << note_pairs_count
      << "\n";

    for (int pair_ix = 0; pair_ix < note_pairs_count; pair_ix++) {
      int token_start_ix = 2 + (3 * pair_ix);
      Token const note_key   = tokens[token_start_ix];
      Token const equals     = tokens[token_start_ix + 1];
      Token const note_value = tokens[token_start_ix + 2];
      llvm::outs()
        << "note:handle_note: handling:"
        << "\n\t" << get_token_content(note_key)
        << " " << get_token_content(equals)
        << " " << get_token_content(note_value)
        << "\n";
      assert(get_token_content(equals) == "=");
      std::string key_string   = get_token_content(note_key);
      std::string value_string = get_token_content(note_value);
      annotation->emplace(key_string, value_string);
    }

    note_start_token.setAnnotationValue((void *)(annotation));

    return true;
  }

  /*
   * NotePragmaHandler
   *
   * Lex a custom annotation of the form:
   *
   * `#pragma note <dsl namespace> <dsl tokens>...`
   */
  void NotePragmaHandler::HandlePragma (
    clang::Preprocessor & PP,
    clang::PragmaIntroducerKind, // NOTE(Simone): unused
    Token & note_source_token
  ) {
    Token dns_token;
    PP.LexUnexpandedToken(dns_token);
    if (dns_token.isNot(tok::identifier)) {
      PP.Diag(dns_token, clang::diag::err_pp_expected_after)
        << "'note'" << "an identifier (for its dsl namespace)";
      return;
    }

    llvm::StringRef dns_string = dns_token.getIdentifierInfo()->getName();
    DSLNamespace dsl_namespace = parse_dsl_namespace(dns_string);

    // TODO(Simone): debug- or verbosity-level-based printing
    llvm::outs() << "note: found dsl_namespace: "
      << dsl_namespace_to_string(dsl_namespace)
      << "\n";

    if (dsl_namespace == DSLNamespace::unrecognized) {
      PP.Diag(dns_token, clang::diag::err_pp_expected_after)
        << "'note'" << "a registered DSL namespace (e.g. 'noelle')";
      return;
    }

    // NOTE(Simone): create the dsl data our handler will modify
    Annotation * annotation = new Annotation {};

    // NOTE(Simone): build vector of annotation tokens
    llvm::SmallVector<Token, 32> tokens;

    Token note_token;
    note_token.startToken();
    note_token.setKind(tok::annot_ext_note);
    note_token.setLocation(note_source_token.getLocation());
    note_token.setAnnotationValue((void *) annotation);

    tokens.push_back(note_token);
    tokens.push_back(dns_token);

    Token current;
    PP.LexUnexpandedToken(current);
    while (current.isNot(tok::eod) && current.isNot(tok::eof)) {
      llvm::outs() << "note: pushing DSL token:"
          << " @ " << &current
          << " '" << get_token_content(current) << "'"
          << "\n";
      tokens.push_back(current);
      PP.LexUnexpandedToken(current);
    }

    if (!note::handle_note(tokens)) {
      llvm::errs()
        << "Note Pragma Handler: unable to handle a note annotation for"
        << " namespace '"
        << dsl_namespace_to_string(dsl_namespace)
        << "'\n";
      return;
    }

    print_annotation(*annotation, llvm::outs());

    llvm::outs() << "DSL handler appears to have processed tokens!\n";
    PP.EnterToken(note_token);
    return;
  }
} // namespace

static clang::PragmaHandlerRegistry::Add<note::NotePragmaHandler>
NotePragmaHandlerThrowawayInstance("note", "make a note of it");
