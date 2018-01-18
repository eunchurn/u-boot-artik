#!/usr/bin/python
import sys
import hashlib
import struct
import binascii
from Crypto.PublicKey import RSA

try:
    inputfile = sys.argv[1]
    outputfile = sys.argv[2]
    keyfile = sys.argv[3]
except IndexError:
    print "Usage: attachns2-s.py <input> <output> <rsa_publickey>"
    print "Example:"
    print "   attachns2-s.py ./u-boot.bin ./u-boot.head.bin ./rsa_public.key"
    sys.exit(1)

fin = open(inputfile, 'rb')
fout = open(outputfile, 'wb+')
keyin = open(keyfile, 'rb')

h = hashlib.sha256()

data = fin.read()
size = fin.tell()

# prepare output file
size = (size + 32 + 2048) & ~(512 - 1)
fout.truncate(size)

# write binary image
fout.seek(32, 0)
fout.write(data)

# write RSA public key
pubkey = RSA.importKey(keyin.read())
n = binascii.unhexlify(hex(pubkey.n).rstrip("L").lstrip("0x"))
n = n[::-1]
e = int(hex(pubkey.e).rstrip("L").lstrip("0x"), 16)
fout.seek(-544, 2)
fout.write(struct.pack('I256sII', 256, n, 4, e))

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
keyin.close()
