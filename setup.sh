#!/usr/bin/env bash
mkdir build
cd build
LLVM_DIR=/usr/local/opt/llvm/lib/cmake/llvm cmake ..  # Generate the Makefile.
make  # Actually build the pass.
