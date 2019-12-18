# Derived from the CMakeLists.txt for examples/PrintFunctionNames

# Make this code loadable as a clang plugin
add_llvm_library(
  pragma-note

  MODULE

  pragma-note.cpp

  PLUGIN_TOOL

  clang
)

# Declare link dependencies
# APPARENTLY this is only required on Windows platforms
if(LLVM_ENABLE_PLUGINS AND (WIN32 OR CYGWIN))
  target_link_libraries(pragma-note
    PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangLex
    LLVMSupport
    )
endif()
