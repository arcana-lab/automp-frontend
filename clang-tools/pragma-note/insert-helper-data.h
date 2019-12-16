#pragma once

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

#include "./annotation.h"

namespace note {
  /* Class (for Clang to manage) that holds the metadata we want to insert
   * if our custom InsertHelper gets invoked. This way we can decouple
   * metadata generation from actual IR codegen.
   */
  class InsertHelperData {
    llvm::SmallVector<Annotation, 8> instruction_stack;
    llvm::SmallVector<Annotation, 8> function_stack;
    bool enabled = false;
    void pushNoteOn (
      llvm::SmallVectorImpl<Annotation> & stack,
      Annotation & note
    ) {
      if (stack.size() == 0) {
        stack.push_back(note);
        return;
      }
      Annotation parent = stack.back();
      Annotation merged = note;
      // NOTE(Simone): insert preserves existing keys on conflict
      merged.insert(parent.begin(), parent.end());
      stack.push_back(merged);
    }
    Annotation popNoteFrom (llvm::SmallVectorImpl<Annotation> & stack) {
      Annotation last = stack.back();
      stack.pop_back();
      return last;
    }
    public:
      InsertHelperData  () = default;
      ~InsertHelperData () = default;
      // NOTE(Simone): delete copy constructors
      InsertHelperData (InsertHelperData const &) = delete;
      InsertHelperData& operator= (InsertHelperData const &) = delete;
      // NOTE(Simone): annotation state
      bool pushNote (Annotation & note) {
        if (note.scope == Annotation::Scope::Function) {
          pushNoteOn(instruction_stack, note);
          pushNoteOn(function_stack, note);
          return true;
        } else if (note.scope == Annotation::Scope::Instruction) {
          pushNoteOn(instruction_stack, note);
          return true;
        } else {
          return false;
        }
      }
      bool popNote () {
        if (instruction_stack.size() == 0) {
          return false;
        }
        Annotation last = getInstructionNote();
        if (last.scope == Annotation::Scope::Instruction) {
          popNoteFrom(instruction_stack);
          return true;
        } else if (last.scope == Annotation::Scope::Function) {
          assert(function_stack.size() != 0);
          // FIXME(Simone): this check won't work currently due to copies.
          /* assert(getFunctionNote() == last); */
          popNoteFrom(instruction_stack);
          popNoteFrom(function_stack);
          return true;
        } else {
          return false;
        }
      }
      Annotation getFunctionNote () {
        return function_stack.back();
      }
      Annotation getInstructionNote () {
        return instruction_stack.back();
      }
      // NOTE(Simone): enabled state
      bool getEnabled () { return enabled; }
      void enable     () { enabled =  true; }
      void disable    () { enabled = false; }
      void disableIfEmpty () {
        if (instruction_stack.size() == 0)
          enabled = false;
      }
      bool getEnabledFor (Annotation::Scope scope) {
        if (scope == Annotation::Scope::Instruction) {
          return getEnabled() && (instruction_stack.size() != 0);
        } else if (scope == Annotation::Scope::Function) {
          return getEnabled() && (function_stack.size() != 0);
        } else {
          llvm::errs()
            << "note::InsertHelperData::getEnabledFor(scope):"
            << " unrecognized Annotation::Scope value for scope!\n";
          return false;
        }
      }
  };
}
