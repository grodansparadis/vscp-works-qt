# canal_mdf_to_code.py
#
# This file is part of the VSCP (https://www.vscp.org)
#
# The MIT License (MIT)
#
# Copyright © 2000-2022 Ake Hedman, Grodans Paradis AB
# <info@grodansparadis.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Tool to encode a CANAL configuratio MDF si it can be embedded in C/C++

import sys
import os
import getopt
import glob

def usage():
    print("usage: canal_mdf_to_code.py file-to-encode -h ")
    print("---------------------------------------------")
    print("-f/--file    - File to encode.")
    print("-h/--help    - This text.")

args = sys.argv[1:]
nargs = len(args)

try:
	opts, args = getopt.getopt(args,"hf:",["help","file="])
except getopt.GetoptError:
	print("unrecognized format!")
	usage()
	sys.exit(2)
for opt, arg in opts:
	if opt in ("-h", "--help"):
		print("HELP")
		usage()
		sys.exit()
	elif opt in ("-f", "--file"):
		infile = arg

# Using readlines()
file1 = open(infile, 'r')
Lines = file1.readlines()

count = 0
# Strips the newline character
for line in Lines:
	line = line.replace("\"","\\\"");
	print("{}".format(line.strip()))


