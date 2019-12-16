# Derived from the CMakeLists.txt for examples/PrintFunctionNames

# Make this code loadable as a clang plugin
add_llvm_loadable_module(parallel-pal parallel-pal.cpp PLUGIN_TOOL clang)

# Declare link dependencies
# APPARENTLY this is only required on Windows platforms
if(LLVM_ENABLE_PLUGINS AND (WIN32 OR CYGWIN))
  target_link_libraries(parallel-pal
    PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangLex
    LLVMSupport
    )
endif()
