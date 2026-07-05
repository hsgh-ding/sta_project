#!/usr/bin/env python3
"""
3D Partitioning Script for Gate-Level Verilog Netlists
Optimized for large netlists (100K+ lines)

Usage:
  python3 partition_3d.py <input.v> hbt_split --outdir <dir>
  python3 partition_3d.py <input.v> hierarchy_mark --outdir <dir>
"""

import re
import sys
import os

# Compiled regexes
RE_SEQ      = re.compile(r'^\s*(DFF|SDF)\w*_ASAP7')
RE_INST     = re.compile(r'^\s*(\w+_ASAP7_75t_R)\s+(\S+)\s*\(\s*$')
RE_PORT     = re.compile(r'^\s*\.(\w+)\s*\(\s*(.+?)\s*\)\s*,?\s*$')
RE_CLOSE    = re.compile(r'^\s*\)\s*;\s*$')
RE_MODULE   = re.compile(r'^\s*module\s+(\w+)\s*\((.*?)\)\s*;\s*$')
RE_ENDMOD   = re.compile(r'^\s*endmodule\s*$')
RE_BUS_DECL = re.compile(r'\[(\d+):(\d+)\]\s*(\w+)')


def parse(filepath):
    with open(filepath, encoding='utf-8') as f:
        lines = f.readlines()

    ports, inputs, outputs, wires = [], set(), set(), set()
    instances = []
    in_decl = True

    for line in lines:
        s = line.rstrip()
        if in_decl:
            m = RE_MODULE.match(s)
            if m:
                ports = [p.strip() for p in m.group(2).split(',') if p.strip()]
                continue
            if s.startswith('  input '):
                for n in _names(s[8:].rstrip(';')):
                    inputs.add(n)
                continue
            if s.startswith('  output '):
                for n in _names(s[9:].rstrip(';')):
                    outputs.add(n)
                continue
            if s.startswith('  wire '):
                for n in _names(s[7:].rstrip(';')):
                    if n and n not in ports:
                        wires.add(n)
                continue

        m = RE_INST.match(s)
        if m:
            in_decl = False
            instances.append([m.group(1), m.group(2), {}])
            continue

        if not in_decl:
            m = RE_PORT.match(s)
            if m and instances:
                net = m.group(2).strip()
                net = re.sub(r'\s*\[.*?\]', '', net).strip()
                instances[-1][2][m.group(1)] = net

    return {
        'ports': ports, 'inputs': inputs, 'outputs': outputs,
        'wires': wires, 'instances': instances, 'lines': lines
    }


def _names(decl):
    decl = re.sub(r'//.*$', '', decl)
    decl = re.sub(r'\[[^\]]*\]', '', decl)
    for tok in decl.split(','):
        tok = re.sub(r'\b(signed|wire|reg|supply[01]|tri)\b', '', tok).strip()
        if tok:
            yield tok


def classify(instances):
    seq, comb = [], []
    for t, n, p in instances:
        (seq if RE_SEQ.match(t) else comb).append((t, n, p))
    return seq, comb


def hbt_split(filepath, outdir):
    d = parse(filepath)
    seq, comb = classify(d['instances'])

    # Collect nets connected to CLK
    clk_nets = set()
    for _, _, ports in seq:
        if 'CLK' in ports:
            clk_nets.add(ports['CLK'])
    clk_port = next((p for p in d['ports'] if p in clk_nets), 'clk')

    # Build net fanout info: which nets are outputs of seq vs comb
    seq_q_nets = set()   # Q/QN outputs of FFs
    seq_d_nets = set()   # D inputs of FFs
    comb_out_nets = set()
    comb_in_nets = set()

    for _, _, ports in seq:
        for pin, net in ports.items():
            if pin in ('Q', 'QN'):
                seq_q_nets.add(net)
            elif pin in ('D'):
                seq_d_nets.add(net)
    for _, _, ports in comb:
        for pin, net in ports.items():
            if pin in ('Y', 'CON', 'SN'):
                comb_out_nets.add(net)
            elif pin not in ('Y', 'CON', 'SN'):
                comb_in_nets.add(net)

    to_top = sorted(seq_q_nets & comb_in_nets)    # FF→comb
    to_bottom = sorted(comb_out_nets & seq_d_nets)  # comb→FF
    all_hbt = set(to_top) | set(to_bottom)

    # Bus widths from declarations
    widths = {}
    for line in d['lines']:
        m = RE_BUS_DECL.search(line)
        if m:
            hi, lo, name = int(m.group(1)), int(m.group(2)), m.group(3)
            widths[name] = max(widths.get(name, 0), hi + 1)

    def w(sig):
        return widths.get(sig, 1)

    # ── Output ──
    mod_name = os.path.splitext(os.path.basename(filepath))[0]
    out = {'top': [], 'bottom': [], 'wrapper': []}

    # ── TOP ──
    t = out['top']
    t.append(f'// 3D Partition: TOP = combinational logic')
    t.append(f'// Cells: {len(comb)}, HBT in: {len(to_top)}, HBT out: {len(to_bottom)}')
    top_ports = list(d['ports'])
    for s in to_top:
        top_ports.append(f'hbt_from_ff_{s}')
    for s in to_bottom:
        top_ports.append(f'hbt_to_ff_{s}')
    t.append(f'module {mod_name}_top ({", ".join(top_ports)});')

    for p in d['ports']:
        if p in d['inputs']:
            t.append(f'  input {"[{0}:{0}] ".format(w(p)-1) if w(p)>1 else ""}{p};')
        elif p in d['outputs']:
            t.append(f'  output {"[{0}:{0}] ".format(w(p)-1) if w(p)>1 else ""}{p};')
        else:
            t.append(f'  inout {p};')
    for s in to_top:
        t.append(f'  input {"[{0}:{0}] ".format(w(s)-1) if w(s)>1 else ""}hbt_from_ff_{s};')
    for s in to_bottom:
        t.append(f'  output {"[{0}:{0}] ".format(w(s)-1) if w(s)>1 else ""}hbt_to_ff_{s};')
    for s in sorted(d['wires']):
        if s not in all_hbt:
            t.append(f'  wire {s};')
    for s in to_top:
        t.append(f'  wire {s};')
    for s in to_bottom:
        t.append(f'  wire {s};')
    t.append('')
    for s in to_top:
        t.append(f'  assign {s} = hbt_from_ff_{s};')
    for s in to_bottom:
        t.append(f'  assign hbt_to_ff_{s} = {s};')
    t.append('')
    for ct, cn, cp in comb:
        t.append(f'  {ct} {cn} (')
        t.append(',\n'.join(f'    .{k}({v})' for k, v in cp.items()))
        t.append('  );')
    t.append('endmodule\n')

    # ── BOTTOM ──
    b = out['bottom']
    b.append(f'// 3D Partition: BOTTOM = sequential logic')
    b.append(f'// Cells: {len(seq)}, HBT in: {len(to_bottom)}, HBT out: {len(to_top)}')
    btm_ports = [clk_port]
    for s in to_bottom:
        btm_ports.append(f'hbt_to_ff_{s}')
    for s in to_top:
        btm_ports.append(f'hbt_from_ff_{s}')
    b.append(f'module {mod_name}_bottom ({", ".join(btm_ports)});')
    b.append(f'  input {clk_port};')
    for s in to_bottom:
        b.append(f'  input {"[{0}:{0}] ".format(w(s)-1) if w(s)>1 else ""}hbt_to_ff_{s};')
    for s in to_top:
        b.append(f'  output {"[{0}:{0}] ".format(w(s)-1) if w(s)>1 else ""}hbt_from_ff_{s};')
    for s in to_bottom:
        b.append(f'  wire {s};')
    for s in to_top:
        b.append(f'  wire {s};')
    b.append('')
    for s in to_bottom:
        b.append(f'  assign {s} = hbt_to_ff_{s};')
    for s in to_top:
        b.append(f'  assign hbt_from_ff_{s} = {s};')
    b.append('')
    for ct, cn, cp in seq:
        b.append(f'  {ct} {cn} (')
        b.append(',\n'.join(f'    .{k}({v})' for k, v in cp.items()))
        b.append('  );')
    b.append('endmodule\n')

    # ── WRAPPER ──
    w_lines = out['wrapper']
    w_lines.append(f'// 3D Stacked Wrapper: {mod_name}')
    w_lines.append(f'module {mod_name}_3d ({", ".join(d["ports"])});')
    for p in d['ports']:
        if p in d['inputs']:
            w_lines.append(f'  input {"[{0}:{0}] ".format(w(p)-1) if w(p)>1 else ""}{p};')
        elif p in d['outputs']:
            w_lines.append(f'  output {"[{0}:{0}] ".format(w(p)-1) if w(p)>1 else ""}{p};')
        else:
            w_lines.append(f'  inout {p};')
    for s in to_top:
        n = f'hbt_ff2comb_{s}'
        w_lines.append(f'  wire {n};')
    for s in to_bottom:
        n = f'hbt_comb2ff_{s}'
        w_lines.append(f'  wire {n};')
    w_lines.append('')
    w_lines.append(f'  // TOP DIE')
    tc = [f'    .{p}({p})' for p in d['ports']]
    for s in to_top:
        tc.append(f'    .hbt_from_ff_{s}(hbt_ff2comb_{s})')
    for s in to_bottom:
        tc.append(f'    .hbt_to_ff_{s}(hbt_comb2ff_{s})')
    w_lines.append(f'  {mod_name}_top u_top (')
    w_lines.append(',\n'.join(tc))
    w_lines.append('  );')
    w_lines.append('')
    w_lines.append(f'  // BOTTOM DIE')
    bc = [f'    .{clk_port}({clk_port})']
    for s in to_bottom:
        bc.append(f'    .hbt_to_ff_{s}(hbt_comb2ff_{s})')
    for s in to_top:
        bc.append(f'    .hbt_from_ff_{s}(hbt_ff2comb_{s})')
    w_lines.append(f'  {mod_name}_bottom u_bottom (')
    w_lines.append(',\n'.join(bc))
    w_lines.append('  );')
    w_lines.append('endmodule\n')

    # Write
    os.makedirs(outdir, exist_ok=True)
    for suffix, lines in [('_top.v', out['top']), ('_bottom.v', out['bottom']),
                           ('_3d_wrapper.v', out['wrapper'])]:
        path = os.path.join(outdir, mod_name + suffix)
        with open(path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))

    print(f"  Module: {mod_name}")
    print(f"  Cells: {len(d['instances'])} ({len(seq)} seq, {len(comb)} comb)")
    print(f"  HBT signals: {len(all_hbt)} (FF->comb: {len(to_top)}, comb->FF: {len(to_bottom)})")
    print(f"  Output: {outdir}/")
    return len(all_hbt)


# ═══ Main ═══
if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python3 partition_3d.py <input.v> hbt_split --outdir <dir>")
        sys.exit(1)
    infile = sys.argv[1]
    mode = sys.argv[2]
    outdir = '.'
    for i, a in enumerate(sys.argv):
        if a == '--outdir' and i+1 < len(sys.argv):
            outdir = sys.argv[i+1]
    if mode == 'hbt_split':
        hbt_split(infile, outdir)
    else:
        print(f"Unknown mode: {mode}")
