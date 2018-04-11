#!/usr/bin/env python

import sys
import lzma


def read_bytes(filename):
  with open(filename, "rb") as f:
    return f.read()


bytes = read_bytes(sys.argv[1])

data_out = lzma.compress(bytes, lzma.FORMAT_ALONE)

sys.stdout.buffer.write(data_out)
