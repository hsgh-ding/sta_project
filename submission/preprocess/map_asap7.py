#!/usr/bin/env python3
"""
Replace Yosys generic $_ cells with ASAP7 RVT cells in a gate-level Verilog netlist.

Mappings:
  \\$_AND_  → AND2x2_ASAP7_75t_R
  \\$_NOT_  → INVx1_ASAP7_75t_R
  \\$_OR_   → OR2x2_ASAP7_75t_R
  \\$_XOR_  → XOR2x2_ASAP7_75t_R

Already-mapped cells (e.g. DFFHQNx1_ASAP7_75t_R) are left unchanged.

Usage:
  python map_asap7.py gcd_flat.v -o gcd_asap7.v
"""

import sys
import os

MAP = {
    "\\$_AND_": "AND2x2_ASAP7_75t_R",
    "\\$_NOT_": "INVx1_ASAP7_75t_R",
    "\\$_OR_":  "OR2x2_ASAP7_75t_R",
    "\\$_XOR_": "XOR2x2_ASAP7_75t_R",
}


def process(input_path: str, output_path: str):
    with open(input_path, encoding="utf-8") as f:
        content = f.read()

    replaced = 0
    for old, new in MAP.items():
        count = content.count(old)
        if count:
            content = content.replace(old, new)
            replaced += count
            print(f"  {old} → {new}  x{count}")

    with open(output_path, "w", encoding="utf-8") as f:
        f.write(content)

    print(f"\n  Total replacements: {replaced}")
    print(f"  Output: {output_path}")


if __name__ == "__main__":
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        sys.exit(0)

    input_path = args[0]
    output_path = None
    for i, a in enumerate(args):
        if a in ("-o", "--output") and i + 1 < len(args):
            output_path = args[i + 1]

    if output_path is None:
        base, ext = os.path.splitext(input_path)
        output_path = f"{base}_asap7{ext}"

    process(input_path, output_path)
