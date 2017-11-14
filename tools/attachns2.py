#!/usr/bin/python
import os
import sys
import hashlib
import struct

try:
    inputfile = sys.argv[1]
    outputfile = sys.argv[2]
except IndexError:
    print "Usage: attachns2.py <input> <output>"
    print "Example:"
    print "   attachns2.py ./u-boot.bin ./u-boot.head.bin"
    sys.exit(1)

fin = open(inputfile, 'rb')
fout = open(outputfile, 'wb+')

h = hashlib.sha256();

data = fin.read()
size = fin.tell()

# prepare output file
size = (size + 32 + 2048) & ~(512 - 1)
fout.truncate(size)

# write binary image
fout.seek(32, 0)
fout.write(data)

# calc sha256
fout.seek(32, 0)
data = fout.read(size - 32 - 272)
h = hashlib.sha256(data)

# put headers
fout.seek(0, 0)
fout.write(struct.pack('III', size / 512, 0x0, 0x656d6264))
fout.seek(4, 0)
digest = h.digest()[:4]
fout.write(digest)

fin.close()
fout.close()
