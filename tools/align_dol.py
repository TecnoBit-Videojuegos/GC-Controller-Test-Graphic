import struct
import sys

path = sys.argv[1]
d = bytearray(open(path, "rb").read())

# DOL header: 18 section offsets, 18 load addresses, 18 sizes (7 text + 11 data)
offs = struct.unpack(">18I", d[0x00:0x48])
addrs = struct.unpack(">18I", d[0x48:0x90])
sizes = list(struct.unpack(">18I", d[0x90:0xD8]))

for i in range(18):
    if sizes[i] == 0:
        continue
    if addrs[i] % 32:
        print(f"WARNING: section {i} address 0x{addrs[i]:08X} not 32-byte aligned")
    new = (sizes[i] + 31) & ~31
    if new != sizes[i]:
        print(f"Section {i}: size 0x{sizes[i]:X} -> 0x{new:X}")
        end = offs[i] + new
        if end > len(d):
            d.extend(b"\0" * (end - len(d)))
        sizes[i] = new

d[0x90:0xD8] = struct.pack(">18I", *sizes)
open(path, "wb").write(d)
