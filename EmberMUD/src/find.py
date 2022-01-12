#!/usr/bin/python3
import codecs
import os
from sys import argv

if len(argv) < 2:
    print("An argument is required")
else:
    st = " ".join(argv[1:])
    files = [f for f in os.listdir('.') if os.path.isfile(f)]
    for f in files:
        found = False
        if f[-2:] != ".c" and f[-2:] != ".h":
            continue
        line_num = 0
        with codecs.open(f,encoding='utf-8',mode="r") as a:
            for L in a:
                line_num += 1
                if st in L:
                    if found == False:
                        found = True
                        print("In "+f+":")
                    print("  %i:\t"%line_num+L,end="")

