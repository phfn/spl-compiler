#!/bin/python
import os
from subprocess import run
from argparse import ArgumentParser
def get_files(folder: str) -> list:
    files = []
    for el in os.listdir(folder):
        path = os.path.join(folder, el)
        if os.path.isfile(path):
            files.append(path)
        else:
            files = files + get_files(path)
    return files

def parse_known_args():
    parser = ArgumentParser()
    parser.add_argument("--progs", help="Path of the programs to test", default="progs")
    parser.add_argument("--refspl", help="path to the reference implementation", default="./eco32tools/bin/refspl")
    parser.add_argument("--spl", help="Path to your implementation", default="./build/spl")
    parser.add_argument("--exit_on_error", help="Stop on first mismatch", default=False, action="store_true")
    parser.add_argument("--save_output", help="Store the last output in ref.txt and our.txt", default=False, action="store_true")
    parser.add_argument("--quiet", help="Dont output output", default=False, action="store_true")
    return parser.parse_known_args()



def main():
    args, handover_args = parse_known_args()
    files = [file for file in get_files(args.progs) if ".spl" in str(file)]
    files.sort()

    for file in files:
        our = run([args.spl] + handover_args + [file], capture_output=True, check=False)
        ref = run([args.refspl] + handover_args + [file], capture_output=True, check=False)
        if our.returncode == ref.returncode and our.stdout == ref.stdout:
            print(f"{file} ✓")
        else:
            our_str = f"OUR exit({our.returncode}):\n{our.stdout.decode()}{our.stderr.decode()}"
            ref_str = f"REF exit({ref.returncode}):\n{ref.stdout.decode()}{ref.stderr.decode()}"
            print(f"{file} ✕")
            if not args.quiet:
                print(our_str)
                print(ref_str)
            if args.save_output:
                with open("ref.txt", "w") as f:
                    f.write(ref_str)
                with open("our.txt", "w") as f:
                    f.write(our_str)


            if args.exit_on_error:
                break
                



if __name__ == "__main__":
    main()
