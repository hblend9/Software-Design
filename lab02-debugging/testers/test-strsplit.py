#!/usr/bin/env python3

import os, os.path
import re
from sys import platform
import subprocess

if platform != 'win32':
    compile_result = subprocess.run(['clang', '-g', '-fsanitize=address', '-Wno-nullability-completeness', '-o', 'mystringtest', 'mystringtest.c', 'mystring.c'])
    binary = 'mystringtest'
else:
    compile_result = subprocess.run(['cl.exe', '-Zi', '-W3', '-fsanitize=address', '-D_CRT_SECURE_NO_WARNINGS', 'mystringtest.c', 'mystring.c'])
    binary = 'mystringtest.exe'

def stringtest(f):
    try:
        with open(f, 'r') as ff:
            result = ""
            for i, x in enumerate([x for x in ff.read()[:-1].split(" ") if x != " " and x != ""]):
                result += str(i) + ": \"" + x + "\"\n"
            return result
    except:
        return "stringtest: " + f + ": No such file or directory\n"

def test(f):
    run_result = subprocess.run(['./' + binary, f], capture_output=True, text=True)
    result = (stringtest(f).strip(), run_result.stdout.strip(), run_result.stderr.strip())
    if result[0] == result[1]:
        print("PASSED " + f)
    else:
        print("FAILED " + f)
        print("    Correct is '" + result[0] + "'")
        print("    Output  is '" + result[1] + "'")
        if len(result[2]) > 0:
            print("    Standard Error is:\n")
            print(result[2])

if compile_result.returncode != 0:
    print("Compilation Failed.")
    exit(1)

for f in os.listdir("data"): 
    test(os.path.join("data", f))
