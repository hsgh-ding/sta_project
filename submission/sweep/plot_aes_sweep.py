#!/usr/bin/env python3
"""Plot HBT capacitance sweep results — AES (dual Y-axis)."""
import csv, sys, os

def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "aes_hbt_sweep.csv"
    if not os.path.exists(csv_path):
        print(f"Error: {csv_path} not found.")
        sys.exit(1)

    c_vals, setup_wns, hold_wns = [], [], []
    with open(csv_path) as f:
        for row in csv.DictReader(f):
            c_vals.append(float(row["C_hbt_fF"]))
            setup_wns.append(float(row["Setup_WNS_ps"]))
            hold_wns.append(float(row["Hold_WNS_ps"]))

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print(f"{'C_hbt':>8}  {'Setup WNS':>12}  {'Hold WNS':>12}")
        for c, s, h in zip(c_vals, setup_wns, hold_wns):
            print(f"{c:>8.1f}  {s:>12.4f}  {h:>12.4f}")
        return

    fig, ax1 = plt.subplots(figsize=(10, 5))

    color_s = "#d62728"
    color_h = "#1f77b4"

    ax1.plot(c_vals, setup_wns, "o-", color=color_s, linewidth=2.5, markersize=8, label="Setup WNS")
    ax1.set_xlabel("HBT Capacitance C_hbt (fF)", fontsize=12)
    ax1.set_ylabel("Setup WNS (ps)", color=color_s, fontsize=12)
    ax1.tick_params(axis="y", labelcolor=color_s)

    s_min, s_max = min(setup_wns), max(setup_wns)
    pad_s = (s_max - s_min) * 0.15 + 2
    ax1.set_ylim(s_max + pad_s, s_min - pad_s)

    ax2 = ax1.twinx()
    ax2.plot(c_vals, hold_wns, "s--", color=color_h, linewidth=2, markersize=6, label="Hold WNS")
    ax2.set_ylabel("Hold WNS (ps)", color=color_h, fontsize=12)
    ax2.tick_params(axis="y", labelcolor=color_h)

    h_min, h_max = min(hold_wns), max(hold_wns)
    pad_h = (h_max - h_min) * 0.5 + 0.1
    ax2.set_ylim(h_min - pad_h, h_max + pad_h)

    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, fontsize=11)

    ax1.set_title("AES — HBT Capacitance vs. WNS (ss_ff corner)", fontsize=14)
    ax1.grid(True, alpha=0.3)

    plt.tight_layout()
    out = "aes_hbt_sweep.png"
    plt.savefig(out, dpi=150)
    print(f"Saved {out}")

if __name__ == "__main__":
    main()
