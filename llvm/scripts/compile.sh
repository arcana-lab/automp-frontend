#!/bin/bash -e

# Fetch the input
LLVM_VERSION=$1 ;
patchFile="`realpath $2`" ;
gitDir="`git rev-parse --show-toplevel`" ;

# Define the LLVM directory
LLVM_DIR=`realpath $1` ;

# Create the vanilla LLVM
git clone https://github.com/scampanoni/LLVM_installer.git $LLVM_VERSION ;
cd ${LLVM_DIR} ;
make src 

# Apply the patch
cd ${LLVM_DIR}/src ; 
tar xf $patchFile ;

# Link the clang plugin
cd ${LLVM_DIR}/src/tools/clang/tools ; 
ln -s ${gitDir}/clang-tools hobbit-tricks ;

# Compile
cd ${LLVM_DIR}; 
make EXTRAS="noextra" TESTS="notest" ;
