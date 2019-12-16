#pragma once

#include <map>
#include <string>

namespace note {
  /* NOTE(Simone): Annotations are allowed to be arbitrary strings,
   * which will improve extensibility: there's no code modification
   * required to add new keys and values to annotations.
   */
  struct Annotation : std::map<std::string, std::string> {
    enum class Scope { Function, Instruction };
    Scope scope = Scope::Instruction;
  };
  using AnnotationMap = std::map<void *, Annotation *>;
}
