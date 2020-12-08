#!/bin/bash -e

function patchInstallDir {
  local fileToPatch="$1" ;

  awk -v installDirectory="${installDir}" '{
    if ($1 == "installDir"){
      printf("%s=\"%s\"\n", $1, installDirectory);
    } else {
      print ;
    }
  }' scripts/$fileToPatch > ${installDir}/bin/$fileToPatch ;

  return 
}

# Fetch the input
LLVM_VERSION=$1 ;
patchFile="`realpath $2`" ;
gitDir="`git rev-parse --show-toplevel`" ;

# Define the LLVM directories
LLVM_DIR=`realpath $1` ;
installDir="${LLVM_DIR}/release" ;

# Create the vanilla LLVM
if ! test -d $LLVM_VERSION ; then

  # Fetch the LLVM installer
  git clone https://github.com/scampanoni/LLVM_installer.git $LLVM_VERSION ;
  cd ${LLVM_VERSION} ;
  git checkout 0d876be2f90ee7ddfb16c2b131ab2c0e1f94708e ;
  cd ../ ;

  # Fetch LLVM sources
  cd ${LLVM_DIR} ;
  make src ;
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

# Copy the automp script
cd ${LLVM_DIR} ;
cd ../ ;

patchInstallDir "automp" ;
chmod 744 ${installDir}/bin/automp ;
