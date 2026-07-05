#!/usr/bin/env python3
"""
给 flat Verilog 网表的 cell 类型加 _top / _bottom 后缀。

规则（与 HeteroSTA3D 的 partition_3d.py 一致）：
  - 时序 cell（DFF*/SDF*）  → 后缀 _bottom
  - 组合逻辑 cell（其他）    → 后缀 _top

用法：
  python add_die_suffix.py input.v -o output.v
  python add_die_suffix.py input.v             # 输出 input_suffixed.v
"""

import re
import sys
import os

# ── 正则 ──────────────────────────────────────────────
# 匹配 cell 例化行：  CELL_TYPE instance_name (
# 原始 flat 网表中 cell 类型不含 _top/_bottom
RE_INST = re.compile(r'^(\s*)(\w+)(\s+\S+\s*\(\s*)$')

# 时序 cell 识别（DFF / SDF 开头）
RE_SEQ = re.compile(r'^(DFF|SDF)\w*')


def is_sequential(cell_type: str) -> bool:
    """DFF*/SDF* 开头的是时序 cell → 放 bottom die"""
    return bool(RE_SEQ.match(cell_type))


def add_suffix(line: str) -> str:
    """如果当前行是 cell 例化行，给 cell 类型加 _top 或 _bottom"""
    m = RE_INST.match(line)
    if not m:
        return line
    indent, cell_type, rest = m.group(1), m.group(2), m.group(3)
    # 已有后缀就跳过，避免 _top_top / _bottom_bottom
    if cell_type.endswith("_top") or cell_type.endswith("_bottom"):
        return line
    suffix = "_bottom" if is_sequential(cell_type) else "_top"
    return f"{indent}{cell_type}{suffix}{rest}"


def process(input_path: str, output_path: str):
    with open(input_path, encoding="utf-8") as f:
        lines = f.readlines()

    modified = 0
    out_lines = []
    for line in lines:
        new_line = add_suffix(line)
        if new_line != line:
            modified += 1
        out_lines.append(new_line)

    with open(output_path, "w", encoding="utf-8") as f:
        f.writelines(out_lines)

    print(f"  输入: {input_path}")
    print(f"  输出: {output_path}")
    print(f"  修改 cell 数: {modified}")


# ── main ─────────────────────────────────────────────
if __name__ == "__main__":
    args = sys.argv[1:]
    if not args or "-h" in args or "--help" in args:
        print(__doc__)
        sys.exit(0)

    input_path = args[0]
    output_path = None
    for i, a in enumerate(args):
        if a in ("-o", "--output") and i + 1 < len(args):
            output_path = args[i + 1]

    if output_path is None:
        base, ext = os.path.splitext(input_path)
        output_path = f"{base}_suffixed{ext}"

    process(input_path, output_path)
