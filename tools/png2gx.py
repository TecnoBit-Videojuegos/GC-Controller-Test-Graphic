#!/usr/bin/env python3
"""Convierte PNG a texturas GX embebidas como C. Uso: png2gx.py <carpeta_png> <salida_sin_ext>"""
import sys, glob, os
from PIL import Image

def to_rgba8(im):
    w, h = im.size
    tw, th = (w + 3) & ~3, (h + 3) & ~3
    if (tw, th) != (w, h):
        c = Image.new("RGBA", (tw, th), (0, 0, 0, 0))
        c.paste(im, (0, 0))
        im = c
    px = im.load()
    out = bytearray()
    for ty in range(0, th, 4):
        for tx in range(0, tw, 4):
            ar = bytearray()
            gb = bytearray()
            for y in range(4):
                for x in range(4):
                    r, g, b, a = px[tx + x, ty + y]
                    ar += bytes((a, r))
                    gb += bytes((g, b))
            out += ar + gb
    return tw, th, out

def main(src, base):
    files = sorted(glob.glob(os.path.join(src, "*.png")))
    if not files:
        sys.exit("No hay PNG en " + src)
    c = ['#include "textures_gen.h"', ""]
    h = ["#ifndef TEXTURES_GEN_H", "#define TEXTURES_GEN_H",
         "typedef struct { const unsigned char *data; unsigned short w, h, tw, th; } gx_texdesc;"]
    for f in files:
        name = os.path.splitext(os.path.basename(f))[0].lower()
        im = Image.open(f).convert("RGBA")
        tw, th, data = to_rgba8(im)
        c.append("static const unsigned char d_%s[] __attribute__((aligned(32))) = {" % name)
        for i in range(0, len(data), 24):
            c.append(",".join(str(b) for b in data[i:i+24]) + ",")
        c.append("};")
        c.append("const gx_texdesc tex_%s = { d_%s, %d, %d, %d, %d };" % (name, name, im.width, im.height, tw, th))
        h.append("extern const gx_texdesc tex_%s;" % name)
        print("%s: %dx%d -> %dx%d" % (name, im.width, im.height, tw, th))
    h.append("#endif")
    open(base + ".c", "w").write("\n".join(c) + "\n")
    open(base + ".h", "w").write("\n".join(h) + "\n")

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
