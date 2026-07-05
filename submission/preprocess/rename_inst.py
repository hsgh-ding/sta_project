#!/usr/bin/env python3
"""Rename all cell instances in a Verilog netlist to simple names (u1, u2, ...)."""
import re
import sys

RE_INST = re.compile(r'^(\s+)(\w+_ASAP7_75t_R_(?:top|bottom))\s+(\S+)\s*(\()')

def process(input_path, output_path):
    with open(input_path, encoding='utf-8') as f:
        lines = f.readlines()

    count = 0
    out = []
    for line in lines:
        m = RE_INST.match(line)
        if m:
            count += 1
            line = f"{m.group(1)}{m.group(2)} u{count} {m.group(4)}\n"
        out.append(line)

    with open(output_path, 'w', encoding='utf-8') as f:
        f.writelines(out)

    print(f"Renamed {count} instances → {output_path}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} input.v output.v")
        sys.exit(1)
    process(sys.argv[1], sys.argv[2])
