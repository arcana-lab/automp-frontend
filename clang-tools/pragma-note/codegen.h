#pragma once

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"

#include "clang/AST/ASTContext.h"

#include "./pragma-note.h"
#include "./annotation.h"
#include "./insert-helper-data.h"

namespace note {
namespace codegen {
  using ASTContext  = clang::ASTContext;
  using LLVMContext = llvm::LLVMContext;
  using Instruction = llvm::Instruction;
  using Metadata    = llvm::Metadata;
  using MDString    = llvm::MDString;
  using MDNode      = llvm::MDNode;

  /* NOTE(Simone): generic metadata generation.
   *
   * Given an Annotation, turn it into Metadata tuples of:
   *   (MDString, ConstantInt)
   * ... which we can emit together in a single wrapper MDNode.
   */
  static NameAndMDNode construct_metadata (
    Annotation note,
    LLVMContext & Ctx
  ) {
    llvm::SmallVector<Metadata *, 8> annotation_metadata = {};
    for (auto annotation_entry : note) {
      Metadata * KeyValue[] = {
        MDString::get(Ctx, annotation_entry.first),
        MDString::get(Ctx, annotation_entry.second),
      };
      annotation_metadata.push_back(MDNode::get(Ctx, KeyValue));
    }
    MDNode * md = MDNode::get(Ctx, annotation_metadata);
    return NameAndMDNode("note.noelle", md);
  }

  static Annotation parse_metadata (MDNode * md) {
    // NOTE(Simone): MDNode is a tuple of MDString, MDString pairs
    // NOTE(Simone): Use mdconst::dyn_extract API from Metadata.h#483
    Annotation result = {};
    for (auto const & pair_operand : md->operands()) {
      using namespace llvm;
      auto * pair = dyn_cast<MDNode>(pair_operand.get());
      auto * key = dyn_cast<MDString>(pair->getOperand(0));
      auto * val = dyn_cast<MDString>(pair->getOperand(1));
      result.emplace(key->getString(), val->getString());
    }
    return result;
  }

  static bool EnableInsertHelper (
    void * node,
    ASTContext & ASTContext,
    LLVMContext & LLVMContext
  ) {
    auto annotations = ASTContext.NoteDSLAnnotations;
    auto & Helper = ASTContext.NoteInsertHelperData;
    llvm::errs()
      << "note:EnableInsertHelper: try node @ " << node << "\n";
    if (annotations.find(node) != annotations.end()) {
      llvm::errs()
        << "note:EnableInsertHelper: found annotation;"
        << " enabling insert helper.\n";
      Annotation * dsl_note_value = annotations.at(node);
      if (Helper.pushNote(*dsl_note_value)) {
        Helper.enable();
        return true;
      } else {
        llvm::errs()
          << "note:EnableInsertHelper: annotation for node found, but"
          << " InsertHelperData::pushNote(...) failed!\n";
        return false;
      }
    } else {
      llvm::errs()
        << "note:EnableInsertHelper: no annotation for node;"
        << " failing to enable insert helper for this block.\n";
      return false;
    }
  }

  static void DisableInsertHelper (ASTContext & ASTContext) {
    auto & Helper = ASTContext.NoteInsertHelperData;
    if (Helper.getEnabled()) {
      Helper.popNote();
      Helper.disableIfEmpty();
      llvm::outs() << "note:DisableInsertHelper: disabling.\n";
    } else {
      llvm::outs() << "note:DisableInsertHelper: not enabled;"
        << " failing to disable insert helper.\n";
    }
  }

  static bool isInsertHelperEnabled (
    ASTContext & ASTContext,
    Annotation::Scope scope = Annotation::Scope::Instruction
  ) {
    return ASTContext.NoteInsertHelperData.getEnabledFor(scope);
  }

  static void InsertHelper (
    Instruction * I,
    ASTContext & Ctx,
    LLVMContext & LLVMCtx,
    Annotation::Scope scope = Annotation::Scope::Instruction
  ) {
    llvm::outs()
      << "note:InsertHelper: considering instruction @ " << I << "\n";
    llvm::outs()
      << "note:InsertHelper: I is " << I->getOpcodeName() << "\n";
    auto & Helper = Ctx.NoteInsertHelperData;
    Annotation note;
    if (scope == Annotation::Scope::Function) {
      note = Helper.getFunctionNote();
    } else if (scope == Annotation::Scope::Instruction) {
      note = Helper.getInstructionNote();
    }
    NameAndMDNode md = construct_metadata(note, LLVMCtx);
    Annotation reconstructed = parse_metadata(md.second);
    llvm::outs() << "Annotation {\n";
    for (auto annotation_entry : reconstructed) {
      llvm::outs()
        << "  "  << annotation_entry.first
        << " = " << annotation_entry.second
        << "\n";
    }
    llvm::outs() << "};\n";
    I->setMetadata(md.first, md.second);
  }
} // namespace codegen
} // namespace note
