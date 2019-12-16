#!/bin/bash

rm -f $1 ;
pushd ./ ;
cd patch ;
tar cf * $1 ;
popd ;
