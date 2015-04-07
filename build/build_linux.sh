#!/bin/sh

mkdir -p unix_makefiles
cd unix_makefiles
cmake -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=icpc ../..
make
cd ..
