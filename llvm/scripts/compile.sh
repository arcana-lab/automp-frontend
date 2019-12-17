#!/bin/bash -e

# Fetch the input
LLVM_VERSION=$1 ;
patchFile="`realpath $2`" ;
gitDir="`git rev-parse --show-toplevel`" ;

# Define the LLVM directory
LLVM_DIR=`realpath $1` ;

# Create the vanilla LLVM
if ! test -d $LLVM_VERSION ; then
  git clone https://github.com/scampanoni/LLVM_installer.git $LLVM_VERSION ;
  cd ${LLVM_DIR} ;
  make src 
fi

# Apply the patch
cd ${LLVM_DIR}/src ; 
tar xf $patchFile ;

# Link the clang plugin
cd ${LLVM_DIR}/src/tools/clang/tools ; 
if ! test -e hobbit-tricks ; then
  ln -s ${gitDir}/clang-tools hobbit-tricks ;
fi

# Compile
cd ${LLVM_DIR}; 
make EXTRAS="noextra" TESTS="notest" EXTRA_CMAKE_OPTIONS="-DCMAKE_CXX_COMPILER=g++ -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_LINK_LLVM_DYLIB=ON";
