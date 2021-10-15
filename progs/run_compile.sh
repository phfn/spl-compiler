#!/bin/bash
set -e
../eco32tools/compile.sh "$1.spl"
../eco32tools/run.sh "$1.bin"
rm -r *.s
rm -r *.o
rm -r *.x
