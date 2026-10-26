# -*- coding: utf-8 -*-
"""
make_picture_bin.py -- merge RGB565 picture files (.bin / .ebm) and generate
the LCD picture resource table.

Two modes:

1) SCAN mode (no arguments):
       python make_picture_bin.py
   scans the current folder for *.bin / *.ebm files, sorts them by name
   (natural sort, same style as Windows Explorer: pic2 < pic10), and writes
   a list template `pictures.txt`. Then YOU fill in the width and height of
   every picture (replace the ? ? with real values).

2) PACK mode (with the list file):
       python make_picture_bin.py pictures.txt
   merges the pictures into merged.bin (tightly packed, starting at 0) and
   writes picture_table.txt, ready to paste into Inf_lcd_picture.h.

List file format (one picture per line, order = picture id):
       filename  width  height
   lines starting with # are comments; UTF-8 (with or without BOM) and ANSI/
   GBK list files are both accepted.

Notes:
    - pictures are packed tightly (no gaps), one after another, so the
      flash usage is minimal; the whole merged.bin is always re-flashed
      as one piece, starting at address 0
    - the picture file must be exactly width*height*2 bytes
      (RGB565, big endian, row-major), the script verifies this
"""

import os
import sys
import re
import struct
import locale

EXTENSIONS = (".bin", ".ebm")           # picture data file types
SOURCE_TYPES = (".png", ".bmp", ".jpg", ".jpeg")

def read_list(path):
    """read the list file, trying UTF-8 (with BOM) first, then ANSI/GBK"""
    for enc in ("utf-8-sig", "gbk"):
        try:
            with open(path, "r", encoding=enc) as f:
                return f.readlines()
        except UnicodeDecodeError:
            continue
    # last resort: permissive read, replace undecodable characters
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.readlines()

def natural_key(name):
    """natural sort key: 'pic2' < 'pic10', chinese sorted by system locale"""
    key = [int(t) if t.isdigit() else t.lower()
           for t in re.split(r"(\d+)", name)]
    try:
        locale.setlocale(locale.LC_ALL, "")
        return ([locale.strxfrm(t) if isinstance(t, str) else t for t in key])
    except locale.Error:
        return key

def png_size(path):
    """PNG: width/height live in the IHDR chunk at a fixed offset"""
    with open(path, "rb") as f:
        head = f.read(24)
    if head[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    return struct.unpack(">II", head[16:24])

def bmp_size(path):
    """BMP: width/height at offset 18 (two int32, height may be negative)"""
    with open(path, "rb") as f:
        head = f.read(26)
    if head[:2] != b"BM":
        return None
    w, h = struct.unpack("<ii", head[18:26])
    return (w, abs(h))

def jpg_size(path):
    """JPEG: walk the marker segments, dimensions are in the SOF marker"""
    with open(path, "rb") as f:
        data = f.read()
    if data[:2] != b"\xFF\xD8":
        return None
    i = 2
    while i + 9 < len(data):
        if data[i] != 0xFF:
            i += 1
            continue
        marker = data[i + 1]
        if marker in (0xC0, 0xC1, 0xC2):            # SOF0/1/2
            h, w = struct.unpack(">HH", data[i + 5:i + 9])
            return (w, h)
        if marker == 0xD8 or 0xD0 <= marker <= 0xD9:
            i += 2
            continue
        seg_len = struct.unpack(">H", data[i + 2:i + 4])[0]
        i += 2 + seg_len
    return None

def image_size(path):
    """detect dimensions of a source image file, None if unsupported"""
    ext = os.path.splitext(path)[1].lower()
    if ext == ".png":
        return png_size(path)
    if ext == ".bmp":
        return bmp_size(path)
    if ext in (".jpg", ".jpeg"):
        return jpg_size(path)
    return None

def detect_size(name, base):
    """work out (w, h) of a data file automatically:
       1. source image with the same base name
       2. "WxH" pattern in the file name
       3. perfect square guess
       returns ((w, h), note) or (None, '')"""
    for ext in SOURCE_TYPES:
        src = base + ext
        if os.path.isfile(src):
            s = image_size(src)
            if s:
                return s, "from %s" % src
    m = re.search(r"(\d+)\s*[xX\u00d7]\s*(\d+)", name)
    if m:
        return (int(m.group(1)), int(m.group(2))), "from file name"
    pixels = os.path.getsize(name) // 2
    root = int(round(pixels ** 0.5))
    if root > 0 and root * root == pixels:
        return (root, root), "guessed square, PLEASE VERIFY"
    return None, ""

def pack(entries):
    """merge the pictures and write the C table; entries = [(name, w, h)]"""
    out_bin = "merged.bin"
    out_txt = "picture_table.txt"

    addr = 0
    rows = []
    with open(out_bin, "wb") as out:
        for name, w, h in entries:
            with open(name, "rb") as f:
                data = f.read()
            out.write(data)
            rows.append((addr, w, h, name, len(data)))
            print("id %-3d addr 0x%08X  %3dx%-3d  %6d bytes  %s"
                  % (len(rows) - 1, addr, w, h, len(data), name))
            addr += len(data)   # tightly packed: next picture starts here

    with open(out_txt, "w") as f:
        f.write("/* paste into Inf_lcd_picture.h, lcd_picture_table[] */\n")
        for i, (a, w, h, name, n) in enumerate(rows):
            f.write("{ 0x%08X, %d, %d },      /* %d: %s */\n" % (a, w, h, i, name))

    print("\nmerged.bin  : %s (%d bytes total)" % (out_bin, addr))
    print("table       : %s" % out_txt)
    print("flash the whole merged.bin to address 0 (spiflash_write erases sectors automatically)")

def do_auto():
    """auto mode: scan, detect sizes, pack everything if possible"""
    files = [n for n in os.listdir(".")
             if os.path.isfile(n) and n.lower().endswith(EXTENSIONS)
             and not n.startswith("_")]
    files.sort(key=natural_key)

    if not files:
        print("no .bin/.ebm files found in this folder")
        sys.exit(1)

    # dedupe: keep only one data file per base name (.ebm wins over .bin)
    by_base = {}
    for name in files:
        base = os.path.splitext(name)[0].lower()
        if base not in by_base:
            by_base[base] = name
            continue
        cur = by_base[base]
        # .ebm is the preferred format
        if name.lower().endswith(".ebm") and not cur.lower().endswith(".ebm"):
            by_base[base] = name
            print("skip      %-26s duplicate, %s is used instead"
                  % (cur, name))
        else:
            print("skip      %-26s duplicate, %s is used instead"
                  % (name, cur))
    files = sorted(by_base.values(), key=natural_key)

    entries = []
    missing = []
    for name in files:
        base = os.path.splitext(name)[0]
        size, note = detect_size(name, base)
        if size:
            w, h = size
            expect = w * h * 2
            actual = os.path.getsize(name)
            if actual != expect:
                print("MISMATCH  %-26s is %d bytes, but %dx%d needs %d  (%s)"
                      % (name, actual, w, h, expect, note))
                missing.append((name, os.path.getsize(name)))
                continue
            entries.append((name, w, h))
            print("ok        %-26s %3dx%-3d  (%s)" % (name, w, h, note))
        else:
            print("unknown   %-26s %d bytes -> fill in pictures.txt"
                  % (name, os.path.getsize(name)))
            missing.append((name, os.path.getsize(name)))

    if missing:
        # write a list template: detected sizes filled in, unknown as "? ?"
        with open("pictures.txt", "w", encoding="utf-8") as f:
            f.write("# some sizes could not be detected automatically\n")
            f.write("# replace every '? ?' with the real width and height,\n")
            f.write("# then run: python make_picture_bin.py pictures.txt\n")
            for name, w, h in entries:
                f.write("%s  %d  %d\n" % (name, w, h))
            for name, nbytes in missing:
                f.write("%s  ?  ?      # %d bytes = W*H*2\n" % (name, nbytes))
        print("\n%d picture(s) need manual input, wrote pictures.txt" % len(missing))
        print("fill it in and run: python make_picture_bin.py pictures.txt")
        sys.exit(1)

    pack(entries)

def do_pack(list_file):
    """pack mode: merge the listed pictures and generate the C table"""
    entries = []
    for line_no, raw in enumerate(read_list(list_file)):
        line = raw.split("#")[0].strip()    # strip trailing comments
        if not line:
            continue
        parts = line.split()
        if len(parts) != 3:
            print("line %d: expected 'file width height', got: %s"
                  % (line_no + 1, line))
            sys.exit(1)
        name, w, h = parts[0], parts[1], parts[2]
        if not (w.isdigit() and h.isdigit()):
            print("line %d: width/height not filled in yet (%s %s), "
                  "replace the ? ? with real numbers" % (line_no + 1, w, h))
            sys.exit(1)
        w, h = int(w), int(h)
        path = os.path.join(os.path.dirname(os.path.abspath(list_file)), name)
        size = w * h * 2
        if not os.path.isfile(path):
            print("missing file: %s" % path)
            sys.exit(1)
        actual = os.path.getsize(path)
        if actual != size:
            print("size mismatch: %s is %d bytes, expected %d (%dx%d RGB565)"
                  % (name, actual, size, w, h))
            sys.exit(1)
        entries.append((name, path, w, h))

    if not entries:
        print("no pictures in the list")
        sys.exit(1)

    out_bin = os.path.join(os.path.dirname(os.path.abspath(list_file)), "merged.bin")
    out_txt = os.path.join(os.path.dirname(os.path.abspath(list_file)), "picture_table.txt")

    addr = 0
    rows = []
    with open(out_bin, "wb") as out:
        for name, path, w, h in entries:
            with open(path, "rb") as f:
                data = f.read()
            out.write(data)
            rows.append((addr, w, h, name, len(data)))
            print("id %-3d addr 0x%08X  %3dx%-3d  %6d bytes  %s"
                  % (len(rows) - 1, addr, w, h, len(data), name))
            addr += len(data)   # tightly packed: next picture starts here

    with open(out_txt, "w") as f:
        f.write("/* paste into Inf_lcd_picture.h, lcd_picture_table[] */\n")
        for i, (a, w, h, name, n) in enumerate(rows):
            f.write("{ 0x%08X, %d, %d },      /* %d: %s */\n" % (a, w, h, i, name))

    print("\nmerged.bin  : %s (%d bytes total)" % (out_bin, addr))
    print("table       : %s" % out_txt)
    print("flash the whole merged.bin to address 0 (spiflash_write erases sectors automatically)")

def main():
    # make the console tolerate chinese file names on any code page
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(errors="replace")

    if len(sys.argv) < 2:
        do_auto()
    else:
        do_pack(sys.argv[1])

if __name__ == "__main__":
    main()

