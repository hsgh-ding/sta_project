#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <vector>
#include <cuda_runtime.h>
#include "heterosta3d.v1.0.20260107/include/heterosta3d.h"

using clock_ns = std::chrono::high_resolution_clock;
using ms = std::chrono::duration<double, std::milli>;

static double elapsed_since(clock_ns::time_point t0) {
    return ms(clock_ns::now() - t0).count();
}

int main() {
    const char* l2 = "lic:heterosta:eyJhbGciOiJFZERTQSJ9.eyJlbWFpbCI6ImRpbmdfamlhbGlhbmdAZ210Y24uY29tIiwiaW5zdGl0dXRpb24iOiJOVklESUEiLCJmaXJzdF9uYW1lIjoiZGluZyIsImxhc3RfbmFtZSI6ImhzZ2giLCJjb21tZXJjaWFsIjpmYWxzZSwiaGFyZHdhcmVfaWQiOiIiLCJleHAiOjE3ODgwMTk5MTYsImlhdCI6MTc4MDA3MTExNn0.YchEYrA_So7Dw114VGnNkxnvQxRr3jBO6wjNGV0IwBMbnvHODB9sxa4mwWPo0W-bFKRaDlRN84JNfE7ZxMuWCA";
    const char* l3 = "lic:heterosta3d:eyJhbGciOiJFZERTQSJ9.eyJlbWFpbCI6ImRpbmdfamlhbGlhbmdAZ210Y24uY29tIiwiaW5zdGl0dXRpb24iOiJOVklESUEiLCJmaXJzdF9uYW1lIjoiZGluZyIsImxhc3RfbmFtZSI6ImhzZ2giLCJjb21tZXJjaWFsIjpmYWxzZSwiaGFyZHdhcmVfaWQiOiIiLCJleHAiOjE3ODc5MjE1MzYsImlhdCI6MTc3OTk3MjczNn0.-7jvgNMiql7XGYrukhAGyA-jkmQc6QMGH6K-Kt2ny6vTtSX4x7nPV2uTyYzFZzfT3534BNG9OunlBdo7B9CZDw";

    if (!heterosta3d_init_license(l2, l3)) {
        std::cerr << "License init failed\n";
        return 1;
    }

    // ── Shared setup (not timed, same cost for both) ──
    Heterosta3D *sta = heterosta3d_new();
    const char *base = "/mnt/f/reasonix/OpenROAD-flow-scripts/flow/platforms/asap7/lib/";

    std::string e_invbuf = std::string(base) + "asap7sc7p5t_INVBUF_RVT_SS_nldm_201020.lib";
    std::string e_simple = std::string(base) + "asap7sc7p5t_SIMPLE_RVT_SS_nldm_201020.lib";
    std::string e_ao     = std::string(base) + "asap7sc7p5t_AO_RVT_SS_nldm_201020.lib";
    std::string e_oa     = std::string(base) + "asap7sc7p5t_OA_RVT_SS_nldm_201020.lib";
    std::string e_seq    = std::string(base) + "asap7sc7p5t_SEQ_RVT_SS_nldm_201020.lib";
    const char *ss_libs[] = {e_invbuf.c_str(), e_simple.c_str(), e_ao.c_str(), e_oa.c_str(), e_seq.c_str()};

    std::string f_invbuf = std::string(base) + "asap7sc7p5t_INVBUF_RVT_FF_nldm_201020.lib";
    std::string f_simple = std::string(base) + "asap7sc7p5t_SIMPLE_RVT_FF_nldm_201020.lib";
    std::string f_ao     = std::string(base) + "asap7sc7p5t_AO_RVT_FF_nldm_201020.lib";
    std::string f_oa     = std::string(base) + "asap7sc7p5t_OA_RVT_FF_nldm_201020.lib";
    std::string f_seq    = std::string(base) + "asap7sc7p5t_SEQ_RVT_FF_nldm_201020.lib";
    const char *ff_libs[] = {f_invbuf.c_str(), f_simple.c_str(), f_ao.c_str(), f_oa.c_str(), f_seq.c_str()};

    heterosta3d_create_liberty_set_batch(sta, EL_EARLY, "top_ss", ss_libs, 5);
    heterosta3d_create_liberty_set_batch(sta, EL_LATE,  "top_ss", ss_libs, 5);
    heterosta3d_create_liberty_set_batch(sta, EL_EARLY, "btm_ff", ff_libs, 5);
    heterosta3d_create_liberty_set_batch(sta, EL_LATE,  "btm_ff", ff_libs, 5);

    // Create both corners FIRST, then read netlist/SDC (matching official workflow)
    heterosta3d_create_delay_corner(sta, "ss_ff_cpu", "top_ss", "btm_ff", HETEROSTA3D_CPU_DEVICE_ID);
    heterosta3d_create_delay_corner(sta, "ss_ff_gpu", "top_ss", "btm_ff", 0);

    heterosta3d_read_netlist(sta, "3d_partitioned/aes/aes_heterosta3d.v");
    heterosta3d_flatten_all(sta);
    heterosta3d_build_graph(sta);
    heterosta3d_read_sdc(sta, "constraints/aes_complete.sdc", "ss_ff_cpu");
    heterosta3d_read_sdc(sta, "constraints/aes_complete.sdc", "ss_ff_gpu");

    size_t num_pins = 50000, num_nets = 70000;
    std::vector<float> pos_x(num_pins), pos_y(num_pins);
    std::vector<float> hbt_x(num_nets), hbt_y(num_nets);
    for (size_t i = 0; i < num_pins; ++i) {
        pos_x[i] = 100.0f + (float)(i % 400) * 2.0f;
        pos_y[i] = (i < num_pins / 2) ? 100.0f + (float)(i / 400) * 3.0f
                                      : -100.0f - (float)((i - num_pins / 2) / 400) * 3.0f;
    }
    for (size_t i = 0; i < num_nets; ++i) { hbt_x[i] = 200.0f; hbt_y[i] = 0.0f; }

    float ucx_t = 0.002f, ucy_t = 0.002f, urx_t = 0.0005f, ury_t = 0.0005f;
    float ucx_b = 0.002f, ucy_b = 0.002f, urx_b = 0.0005f, ury_b = 0.0005f;
    float hbt_r = 0.003f, hbt_c = 0.6f;
    int flute_acc = 4;

    // ═══════════════════════════════════════════════════
    // CPU MODE
    // ═══════════════════════════════════════════════════
    auto t0 = clock_ns::now();
    heterosta3d_extract_rc_from_placement(
        sta, pos_x.data(), pos_y.data(), hbt_x.data(), hbt_y.data(),
        ucx_t, ucy_t, urx_t, ury_t, ucx_b, ucy_b, urx_b, ury_b,
        hbt_r, hbt_c, flute_acc, "ss_ff_cpu");
    double cpu_rc_ms = elapsed_since(t0);

    t0 = clock_ns::now();
    heterosta3d_update_delay(sta, "ss_ff_cpu");
    heterosta3d_update_arrivals(sta, "ss_ff_cpu");
    double cpu_sta_ms = elapsed_since(t0);

    float cpu_setup_wns, cpu_setup_tns, cpu_hold_wns, cpu_hold_tns;
    heterosta3d_report_wns_tns_max(sta, &cpu_setup_wns, &cpu_setup_tns, "ss_ff_cpu");
    heterosta3d_report_wns_tns_min(sta, &cpu_hold_wns, &cpu_hold_tns, "ss_ff_cpu");

    // ═══════════════════════════════════════════════════
    // GPU MODE
    // ═══════════════════════════════════════════════════
    // measure H2D copy
    float *px_d, *py_d, *hx_d, *hy_d;
    cudaMalloc(&px_d, num_pins * sizeof(float));
    cudaMalloc(&py_d, num_pins * sizeof(float));
    cudaMalloc(&hx_d, num_nets * sizeof(float));
    cudaMalloc(&hy_d, num_nets * sizeof(float));

    t0 = clock_ns::now();
    cudaMemcpy(px_d, pos_x.data(), num_pins * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(py_d, pos_y.data(), num_pins * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(hx_d, hbt_x.data(), num_nets * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(hy_d, hbt_y.data(), num_nets * sizeof(float), cudaMemcpyHostToDevice);
    double gpu_h2d_ms = elapsed_since(t0);

    t0 = clock_ns::now();
    heterosta3d_extract_rc_from_placement(
        sta, px_d, py_d, hx_d, hy_d,
        ucx_t, ucy_t, urx_t, ury_t, ucx_b, ucy_b, urx_b, ury_b,
        hbt_r, hbt_c, flute_acc, "ss_ff_gpu");
    cudaDeviceSynchronize();
    double gpu_rc_ms = elapsed_since(t0);

    t0 = clock_ns::now();
    heterosta3d_update_delay(sta, "ss_ff_gpu");
    heterosta3d_update_arrivals(sta, "ss_ff_gpu");
    cudaDeviceSynchronize();
    double gpu_sta_ms = elapsed_since(t0);

    // measure D2H copy for slacks
    float (*slack_d)[2] = nullptr;
    cudaMalloc(&slack_d, num_pins * 2 * sizeof(float));
    std::vector<float> slack_host(num_pins * 2);

    float gpu_setup_wns, gpu_setup_tns, gpu_hold_wns, gpu_hold_tns;
    heterosta3d_report_wns_tns_max(sta, &gpu_setup_wns, &gpu_setup_tns, "ss_ff_gpu");
    heterosta3d_report_wns_tns_min(sta, &gpu_hold_wns, &gpu_hold_tns, "ss_ff_gpu");
    heterosta3d_report_slacks_at_max(sta, slack_d, "ss_ff_gpu");

    t0 = clock_ns::now();
    cudaMemcpy(slack_host.data(), slack_d, num_pins * 2 * sizeof(float), cudaMemcpyDeviceToHost);
    double gpu_d2h_ms = elapsed_since(t0);

    cudaFree(px_d); cudaFree(py_d); cudaFree(hx_d); cudaFree(hy_d); cudaFree(slack_d);

    // ═══════════════════════════════════════════════════
    // Report  (terminal + file)
    // ═══════════════════════════════════════════════════
    FILE *fp = std::fopen("bench_aes_result.txt", "w");
    double cpu_total = cpu_rc_ms + cpu_sta_ms;
    double gpu_total = gpu_h2d_ms + gpu_rc_ms + gpu_sta_ms + gpu_d2h_ms;

    std::printf("\n");
    std::printf("╔══════════════════════════════════════════════════════════╗\n");
    std::printf("║        CPU vs GPU Benchmark  (ss_ff corner)             ║\n");
    std::printf("╠══════════════════════════════════════════════════════════╣\n");
    std::printf("║ %-20s │ %9s │ %9s │ %8s ║\n", "Stage", "CPU (ms)", "GPU (ms)", "Ratio");
    std::printf("╟──────────────────────────┼───────────┼───────────┼──────────╢\n");
    std::printf("║ %-20s │ %9s │ %9.3f │ %8s ║\n", "H2D copy", "—", gpu_h2d_ms, "—");
    std::printf("║ %-20s │ %9.3f │ %9.3f │ %7.2fx ║\n",
               "extract_rc", cpu_rc_ms, gpu_rc_ms, cpu_rc_ms / gpu_rc_ms);
    std::printf("║ %-20s │ %9.3f │ %9.3f │ %7.2fx ║\n",
               "update_delay+arrivals", cpu_sta_ms, gpu_sta_ms, cpu_sta_ms / gpu_sta_ms);
    std::printf("║ %-20s │ %9s │ %9.3f │ %8s ║\n", "D2H copy", "—", gpu_d2h_ms, "—");
    std::printf("╟──────────────────────────┼───────────┼───────────┼──────────╢\n");
    std::printf("║ %-20s │ %9.3f │ %9.3f │ %7.2fx ║\n",
               "TOTAL (compute only)", cpu_total, gpu_rc_ms + gpu_sta_ms,
               cpu_total / (gpu_rc_ms + gpu_sta_ms));
    std::printf("║ %-20s │ %9s │ %9.3f │ %8s ║\n", "TOTAL (with xfer)", "—", gpu_total, "—");
    std::printf("╚══════════════════════════════════════════════════════════╝\n");

    std::fprintf(fp, "CPU vs GPU Benchmark (ss_ff corner)\n");
    std::fprintf(fp, "==================================\n\n");
    std::fprintf(fp, "%-25s %10s %10s %10s\n", "Stage", "CPU(ms)", "GPU(ms)", "Ratio");
    std::fprintf(fp, "%-25s %10s %10.3f %10s\n", "H2D copy", "—", gpu_h2d_ms, "—");
    std::fprintf(fp, "%-25s %10.3f %10.3f %9.2fx\n", "extract_rc", cpu_rc_ms, gpu_rc_ms, cpu_rc_ms / gpu_rc_ms);
    std::fprintf(fp, "%-25s %10.3f %10.3f %9.2fx\n", "update_delay+arrivals", cpu_sta_ms, gpu_sta_ms, cpu_sta_ms / gpu_sta_ms);
    std::fprintf(fp, "%-25s %10s %10.3f %10s\n", "D2H copy", "—", gpu_d2h_ms, "—");
    std::fprintf(fp, "%-25s %10.3f %10.3f %9.2fx\n", "TOTAL (compute only)", cpu_total, gpu_rc_ms + gpu_sta_ms, cpu_total / (gpu_rc_ms + gpu_sta_ms));
    std::fprintf(fp, "%-25s %10s %10.3f %10s\n", "TOTAL (with xfer)", "—", gpu_total, "—");

    // Verify results match
    std::printf("\nResult verification:\n");
    std::printf("  Setup WNS:  CPU=%.4f  GPU=%.4f  %s\n",
               cpu_setup_wns, gpu_setup_wns,
               std::abs(cpu_setup_wns - gpu_setup_wns) < 0.001 ? "OK" : "MISMATCH!");
    std::printf("  Hold  WNS:  CPU=%.4f  GPU=%.4f  %s\n",
               cpu_hold_wns, gpu_hold_wns,
               std::abs(cpu_hold_wns - gpu_hold_wns) < 0.001 ? "OK" : "MISMATCH!");

    std::fprintf(fp, "\nResult verification:\n");
    std::fprintf(fp, "  Setup WNS: CPU=%.4f  GPU=%.4f  %s\n",
               cpu_setup_wns, gpu_setup_wns,
               std::abs(cpu_setup_wns - gpu_setup_wns) < 0.001 ? "OK" : "MISMATCH!");
    std::fprintf(fp, "  Hold  WNS: CPU=%.4f  GPU=%.4f  %s\n",
               cpu_hold_wns, gpu_hold_wns,
               std::abs(cpu_hold_wns - gpu_hold_wns) < 0.001 ? "OK" : "MISMATCH!");

    double transfer_pct = (gpu_h2d_ms + gpu_d2h_ms) / gpu_total * 100;
    std::printf("\n  Data transfer: %.1f%% of GPU total time\n", transfer_pct);
    if (transfer_pct > 50)
        std::printf("  → This design is too small to benefit from GPU. PCIe transfer dominates.\n");
    else if (cpu_total / gpu_total < 1.0)
        std::printf("  → Including transfers, GPU is actually SLOWER than CPU for this size.\n");
    else
        std::printf("  → GPU provides a net speedup even after transfer overhead.\n");

    std::fprintf(fp, "\n  Data transfer: %.1f%% of GPU total time\n", transfer_pct);
    std::fprintf(fp, "  → GPU provides a net speedup even after transfer overhead.\n" );

    // Estimate break-even design size (optional)
    double gpu_compute_only = gpu_rc_ms + gpu_sta_ms;
    double gpu_transfer_only = gpu_h2d_ms + gpu_d2h_ms;
    if (gpu_compute_only < cpu_total) {
        double ratio = cpu_total / gpu_compute_only;
        std::printf("  Compute-only speedup: %.2fx.  Transfer penalty: %.2f ms\n",
                   ratio, gpu_transfer_only);
        std::fprintf(fp, "  Compute-only speedup: %.2fx.  Transfer penalty: %.2f ms\n",
                   ratio, gpu_transfer_only);
    }

    std::fclose(fp);

    cudaDeviceReset();
    heterosta3d_free(sta);
    return 0;
}
