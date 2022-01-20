#!/usr/bin/python

import sys

for i, x in enumerate([x for x in sys.argv[1].split(" ") if x != " " and x != ""]):
    print(str(i) + ": \"" + x + "\"")
