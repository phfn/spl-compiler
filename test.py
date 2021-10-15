#!/bin/python
import os
import subprocess
from difflib import Differ
from posix import listdir
import filecmp
files = [file for file in os.listdir("progs") if ".spl" in str(file)]
spl_ref = "./eco32tools/bin/refspl"
spl_our = "./build/spl"
for file in files:
    os.system(f"{spl_our} --tokens ./progs/{file} > our.txt")
    os.system(f"{spl_ref} --tokens ./progs/{file} > ref.txt")
    e = filecmp.cmp("our.txt", "ref.txt")
    print(file, e)
    if not e:
        exit(1)
    os.system("rm our.txt ref.txt")
