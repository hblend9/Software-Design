#!/usr/bin/env python3

import os, os.path
from sys import platform
import subprocess

if platform != 'win32':
    compile_result = subprocess.run(['clang', '-g', '-Wno-nullability-completeness', '-o', 'mywc', 'wc.c'])
    binary = 'mywc'
else:
    compile_result = subprocess.run(['cl.exe', '-Zi', '-W3', 'wc.c'])
    binary = 'wc.exe'


def wc(f):
    try:
        with open(f, 'r') as ff:
            return str(ff.read().count("\n")) + " " + f 
    except:
        return "wc: " + f + ": No such file or directory\n"

def test(f):
    run_result = subprocess.run(['./' + binary, f], capture_output=True, text=True)
    result = (wc(f).strip(), run_result.stdout.strip())
    if result[0] == result[1]:
        print("PASSED " + f)
    else:
        print("FAILED " + f)
        print("    Correct is '" + result[0] + "'")
        print("    Output    is '" + result[1] + "'")

if compile_result.returncode != 0:
    print("Compilation Failed.")
    exit(1)

for f in os.listdir("data"): 
    test(os.path.join("data", f))
test("file_does_not_exist")
