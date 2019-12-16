#!/bin/bash

LLVM_VERSION="$1" ;

if ! test -d ${LLVM_VERSION} ; then
  exit 0;
fi

cd ${LLVM_VERSION} ; 
make clean ;
