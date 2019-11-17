#!/usr/bin/env bash
set -e

if [[ $# -eq 0 ]]; then
  echo "Incorrect number of arguments."
  echo "Usage: $0 <input file> [-O2]"
  exit 1;
fi

# Get name of file
filename=$(basename -- "$1")
extension="${filename##*.}"
filename="${filename%.*}"

input_file="$1"
unoptimized_file="$filename.ll"
ssa_file="$filename.ssa.ll"
alias_file="$filename.alias.ll"
mpass_file="$filename.mpass.ll"
replace_null_file="$filename.replace-null.ll"
optimized_file="$filename.opt.ll"

/usr/local/opt/llvm/bin/clang++ -std=c++17 -fno-discard-value-names -S -emit-llvm -Xclang -disable-O0-optnone "$1" -o "$unoptimized_file"
/usr/local/opt/llvm/bin/opt -S -load ../build/skeleton/libSkeletonPass.so -add-nullcheck-func "$unoptimized_file" -o "$mpass_file"
#/usr/local/opt/llvm/bin/opt -S -load ../build/skeleton/libSkeletonPass.so -replace-null "$mpass_file" -o "$replace_null_file"
/usr/local/opt/llvm/bin/opt -S -load ../build/skeleton/libSkeletonPass.so "$mpass_file" -o "$replace_null_file"
/usr/local/opt/llvm/bin/opt -S -mem2reg "$replace_null_file" -o "$ssa_file"
/usr/local/opt/llvm/bin/opt -S -basicaa -print-alias-sets "$ssa_file" -o "$alias_file"
/usr/local/opt/llvm/bin/opt -S -load ../build/skeleton/libSkeletonPass.so -add-nullcheck "$alias_file" -o "$optimized_file"
if [[ $# -eq 2 ]]; then
  /usr/local/opt/llvm/bin/clang++ -O2 -std=c++17 -fno-discard-value-names "$optimized_file"
else
  /usr/local/opt/llvm/bin/clang++ -std=c++17 -fno-discard-value-names "$optimized_file"
fi
