#!/bin/python
import os
import filecmp
from subprocess import call, run
def get_files(folder: str) -> list:
    files = []
    for el in os.listdir(folder):
        path = os.path.join(folder, el)
        if os.path.isfile(path):
            files.append(path)
        else:
            files = files + get_files(path)
    return files


files = [file for file in get_files("progs") if ".spl" in str(file)]
files.sort()
spl_ref = "./eco32tools/bin/refspl"
spl_our = "./build/spl"

for file in files:
    our = run([spl_our, "--tables", file], capture_output=True, check=False)
    ref = run([spl_ref, "--tables", file], capture_output=True, check=False)
    if our.stdout == ref.stdout:
        print(f"{file} ✓")
    else:
        print(f"{file} ✕")
        print(f"OUR: {our.stdout.decode()}{our.stderr.decode()}")
        print(f"REF: {ref.stdout.decode()}{ref.stderr.decode()}")

